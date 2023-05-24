/*
    mach2 - feature control multi-tool
    Copyright (c) Rafael Rivera and Contributors

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "common.h"
#include "scanner.h"
#include "pdb.h"
#include "pe.h"
#include <dia2.h>
#include "dia2_internal.h"
#include <diacreate.h>
#include <capstone/capstone.h>
#include "capstone_helpers.h"
#include <DbgHelp.h>

const std::vector<mach2::Scanner::FeatureStage> mach2::Scanner::FeatureStages
{
    mach2::Scanner::FeatureStage::Unknown,
    mach2::Scanner::FeatureStage::AlwaysEnabled,
    mach2::Scanner::FeatureStage::EnabledByDefault,
    mach2::Scanner::FeatureStage::DisabledByDefault,
    mach2::Scanner::FeatureStage::AlwaysDisabled
};

void mach2::Scanner::SetCallback(Callback const &callback)
{
    _callback = callback;
}

void mach2::Scanner::ExecuteCallback(std::wstring const& path)
{
    if (_callback)
    {
        _callback(path);
    }
}

void mach2::Scanner::InternalGetFeaturesFromSymbolsAtPath(std::wstring const &symbol_path_root, mach2::Scanner::Features &features)
{
    WIN32_FIND_DATA find_data = {};
    auto find_handle = FindFirstFileEx((symbol_path_root + L"\\*.*").c_str(), FindExInfoBasic, &find_data, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
    if (find_handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            std::filesystem::path current_file(find_data.cFileName);
            if (current_file != L"." && current_file != L"..")
            {
                if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    std::filesystem::path symbols_path(symbol_path_root);
                    symbols_path /= find_data.cFileName;

                    ExecuteCallback(symbols_path);
                    InternalGetFeaturesFromSymbolsAtPath(symbols_path, features);
                }
                else
                {
                    if (_wcsnicmp(current_file.extension().c_str(), L".pdb", 4) == 0)
                    {
                        std::filesystem::path pdb_path(symbol_path_root);
                        pdb_path /= find_data.cFileName;

                        ExecuteCallback(pdb_path);
                        GetFeaturesFromSymbolAtPath(pdb_path, features);
                    }
                }
            }
        } while (FindNextFile(find_handle, &find_data));

        FindClose(find_handle);
    }
}

mach2::Scanner::SymbolHitType mach2::Scanner::GetSymbolHitTypeFromSymbolName(std::wstring const & symbolName)
{
    if (symbolName.find(L"::id") != std::wstring::npos)
    {
        return mach2::Scanner::SymbolHitType::Id;
    }
    else if (symbolName.find(L"::stage") != std::wstring::npos)
    {
        return mach2::Scanner::SymbolHitType::Stage;
    }
    else if (symbolName.find(L"::isAlwaysDisabled") != std::wstring::npos)
    {
        return mach2::Scanner::SymbolHitType::AlwaysDisabledFlag;
    }
    else if (symbolName.find(L"::isAlwaysEnabled") != std::wstring::npos)
    {
        return mach2::Scanner::SymbolHitType::AlwaysEnabledFlag;
    }
    else if (symbolName.find(L"::isEnabledByDefault") != std::wstring::npos)
    {
        return mach2::Scanner::SymbolHitType::EnabledByDefaultFlag;
    }
    else if (symbolName.find(L"::isDisabledByDefault") != std::wstring::npos)
    {
        return mach2::Scanner::SymbolHitType::DisabledByDefaultFlag;
    }
    else if (symbolName.find(L"__private_descriptor") != std::wstring::npos)
    {
        return mach2::Scanner::SymbolHitType::Descriptor;
    }
    else if (
        symbolName.find(L"GetCurrentFeatureEnabledState") != std::wstring::npos ||
        symbolName.find(L"GetCachedFeatureEnabledState") != std::wstring::npos ||
        symbolName.find(L"GetCurrentVariantState") != std::wstring::npos ||
        symbolName.find(L"GetCachedVariantState") != std::wstring::npos ||
        symbolName.find(L"ReportUsage") != std::wstring::npos)
    {
        return mach2::Scanner::SymbolHitType::FeatureGetter;
    }
    else
    {
        return mach2::Scanner::SymbolHitType::Feature;
    }
}

bool mach2::Scanner::InternalIsPortablePdb(std::wstring const& path)
{
    std::ifstream file(path, std::ios::binary);
    std::uint32_t value = 0;
    file.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value == 0x424a5342; // BSJB
}

mach2::Scanner::Features mach2::Scanner::GetFeaturesFromSymbolsAtPath(std::wstring const &symbols_path)
{
    Features features;
    InternalGetFeaturesFromSymbolsAtPath(symbols_path, features);
    return features;
}

std::wstring mach2::Scanner::GetFeatureNameFromSymbolName(std::wstring const &symbol_name)
{
    // Supports undecorated and decorated symbol names

    // e.g. ?GetCurrentFeatureEnabledState@?$Feature@U__WilFeatureTraits_Feature_EZSettingDownloadSpeechTelemetry@@@wil@@CG?AW4wil_details_CachedFeatureEnabledState@@_NPAW4wil_details_CachedHasNotificationState@@@Z
    // e.g. ?ReportUsageToService@?$Feature@U__WilFeatureTraits_Feature_IntentInThePastLogging@@@wil@@CGX_NW4ReportingKind@2@I@Z
    // e.g. wil::Feature<__WilFeatureTraits_Feature_RadialControllerCustomFontGlyphs>
    // e.g. __WilFeatureTraits_Feature_DesktopInclusiveOOBE
    // e.g. Feature_Servicing_2011c_28083526__private_reporting
    // e.g. `GlobalSettings::Feature::Is_Feature_Servicing_31766357_Enabled'::`1'::dtor$0
    // e.g. EmptyCVariant_FeatureStaging_Feature_Servicing_BluetoothBthport9F
    // e.g. ?IsFeature_Servicing_addDeviceAuthForFederatedAccountEnabled@Feature@GlobalSettings@@SA_NXZ
    // e.g. _Feature_Servicing_ConvertDIBIconDBZ_isAlwaysDisabled
    // e.g. _Feature_Servicing_ConvertDIBIconDBZ_id
    // e.g. Microsoft.Internal.FeatureStaging.Servicing.Feature_Servicing_Allow_SwitchToDesktop_25561487
    // e.g. Enable_Feature_Servicing_FatStuckThreadFix

    auto expressions = std::array<std::wstring, 13> {
        L"WilFeatureTraits_Feature_(\\w*)",
        L"Feature_(\\w*)__private",
        L"Feature_(\\w*)_logged_traits",
        L"Is_Feature_(\\w*)_Enabled",
        L"EmptyCVariant_FeatureStaging_Feature_(\\w*)",
        L"Feature_(\\w*)_IsEnabled",
        L"GlobalSettings::Feature::IsFeature_(\\w*)",
        L"IsFeature_(\\w*)@",
        L"Feature_(\\w*)_is",
        L"Feature_(\\w*)_id",
        L"Microsoft.Internal.FeatureStaging.Servicing.Feature_(\\w*)",
        L"Enable_Feature_(\\w*)",
        L"Feature_Servicing_(\\w*)"
    };

    for (auto &expr : expressions)
    {
        std::wregex pattern(expr);
        std::wsmatch match;
        std::regex_search(symbol_name, match, pattern, std::regex_constants::match_any);
        if (match.size() == 2)
            return match.str(1);
    }

    throw std::runtime_error{ "GetFeatureNameFromSymbolName failed" };
}

void mach2::Scanner::GetFeaturesFromSymbolAtPath(std::wstring const &path, mach2::Scanner::Features &features)
{
    CComPtr<IDiaDataSource10> data_source;
    ThrowIfFailed(NoRegCoCreate(L"msdia140.dll", CLSID_DiaSource, IID_PPV_ARGS(&data_source)));
    auto hr = data_source->loadDataFromPdb(path.c_str());
    if (FAILED(hr))
    {
        if (!InternalIsPortablePdb(path.c_str()))
        {
            ThrowIfFailed(hr);
        }
        else
        {
            return;
        }
    }

    CComPtr<IDiaSession> session;
    ThrowIfFailed(data_source->openSession(&session));
    
    PDB1* raw_pdb = nullptr;
    ThrowIfFailed(data_source->getRawPDBPtr(reinterpret_cast<void**>(&raw_pdb)));
    
    GUID signature;
    ThrowIf(raw_pdb->QuerySignature2(&signature) == FALSE);

    CComPtr<IDiaSymbol> root_symbol;
    ThrowIfFailed(session->get_globalScope(&root_symbol));

    std::vector<CComPtr<IDiaEnumSymbols>> feature_symbol_search_results(4);
    ThrowIfFailed(session->findChildren(root_symbol, SymTagEnum::SymTagNull, L"*WilFeatureTraits_Feature_*",
        NameSearchOptions::nsfRegularExpression, &feature_symbol_search_results[0]));

    ThrowIfFailed(session->findChildren(root_symbol, SymTagEnum::SymTagNull, L"*Feature_Servicing_*",
        NameSearchOptions::nsfRegularExpression, &feature_symbol_search_results[1]));

    ThrowIfFailed(session->findChildren(root_symbol, SymTagEnum::SymTagNull, L"*Feature_*__private*",
        NameSearchOptions::nsfRegularExpression, &feature_symbol_search_results[2]));

    ThrowIfFailed(session->findChildren(root_symbol, SymTagEnum::SymTagNull, L"*Feature_*_logged_traits*",
        NameSearchOptions::nsfRegularExpression, &feature_symbol_search_results[3]));

    unsigned long results_retrieved = 0;
    for each (auto feature_symbols in feature_symbol_search_results)
    {
        CComPtr<IDiaSymbol> feature_symbol;
        while (SUCCEEDED(feature_symbols->Next(1, &feature_symbol, &results_retrieved)) && results_retrieved == 1)
        {
            CComPtr<IDiaSymbol> feature_symbol_hit;
            feature_symbol_hit.Attach(feature_symbol.Detach());

            CComBSTR raw_symbol_name;
            ThrowIfFailed(feature_symbol_hit->get_name(&raw_symbol_name));
            auto feature_name = GetFeatureNameFromSymbolName(raw_symbol_name.m_str);
            auto& feature = features.FeaturesByName[feature_name];

            if (feature.Name.empty())
            {
                feature.Name = feature_name;
            }

            auto hit_type = GetSymbolHitTypeFromSymbolName(raw_symbol_name.m_str);
            if (!feature.Id && hit_type == Scanner::SymbolHitType::Id)
            {
                CComVariant id_value;
                ThrowIfFailed(feature_symbol_hit->get_value(&id_value));

                if (feature.Id != id_value.lVal && feature.Id != 0)
                {
                    auto unique_name = GetUniqueNameForDuplicateFeature(feature, features);
                    feature = features.FeaturesByName[unique_name];
                    feature.Name = unique_name;
                    features.FeaturesByStage[FeatureStage::Unknown][feature.Name] = &feature;
                }
                feature.Id = id_value.lVal;
            }

            if (hit_type == Scanner::SymbolHitType::AlwaysDisabledFlag)
            {
                CComVariant flag_value;
                ThrowIfFailed(feature_symbol_hit->get_value(&flag_value));

                if (flag_value.bVal == 1 && feature.Stage != FeatureStage::AlwaysDisabled)
                {
                    features.FeaturesByStage[feature.Stage].erase(feature.Name);

                    feature.Stage = FeatureStage::AlwaysDisabled;
                    features.FeaturesByStage[FeatureStage::AlwaysDisabled][feature.Name] = &feature;
                }
            }
            else if (hit_type == Scanner::SymbolHitType::AlwaysEnabledFlag)
            {
                CComVariant flag_value;
                ThrowIfFailed(feature_symbol_hit->get_value(&flag_value));

                if (flag_value.bVal == 1 && feature.Stage != FeatureStage::AlwaysEnabled)
                {
                    features.FeaturesByStage[feature.Stage].erase(feature.Name);

                    feature.Stage = FeatureStage::AlwaysEnabled;
                    features.FeaturesByStage[FeatureStage::AlwaysEnabled][feature.Name] = &feature;
                }
            }
            else if (hit_type == Scanner::SymbolHitType::EnabledByDefaultFlag)
            {
                CComVariant flag_value;
                ThrowIfFailed(feature_symbol_hit->get_value(&flag_value));

                if (flag_value.bVal == 1 && feature.Stage < FeatureStage::EnabledByDefault)
                {
                    features.FeaturesByStage[feature.Stage].erase(feature.Name);

                    feature.Stage = FeatureStage::EnabledByDefault;
                    features.FeaturesByStage[FeatureStage::EnabledByDefault][feature.Name] = &feature;
                }
            }
            else if (hit_type == Scanner::SymbolHitType::DisabledByDefaultFlag)
            {
                CComVariant flag_value;
                ThrowIfFailed(feature_symbol_hit->get_value(&flag_value));

                if (flag_value.bVal == 1 && feature.Stage < FeatureStage::DisabledByDefault)
                {
                    features.FeaturesByStage[feature.Stage].erase(feature.Name);

                    feature.Stage = FeatureStage::DisabledByDefault;
                    features.FeaturesByStage[FeatureStage::DisabledByDefault][feature.Name] = &feature;
                }
            }

            if (feature.Stage == FeatureStage::Unknown)
            {
                features.FeaturesByStage[FeatureStage::Unknown][feature.Name] = &feature;

                if (hit_type == Scanner::SymbolHitType::Stage)
                {
                    CComVariant feature_stage_value;
                    ThrowIfFailed(feature_symbol_hit->get_value(&feature_stage_value));

                    if (feature_stage_value.iVal >= 0 && feature_stage_value.iVal <= 4)
                    {
                        feature.Stage = static_cast<FeatureStage>(feature_stage_value.iVal);

                        auto& unknown_features = features.FeaturesByStage[FeatureStage::Unknown];
                        if (unknown_features.count(feature.Name) > 0)
                        {
                            auto feature_to_move = unknown_features.at(feature.Name);
                            unknown_features.erase(feature.Name);
                            features.FeaturesByStage[feature.Stage][feature_to_move->Name] = feature_to_move;
                        }
                        else // First time FeaturesByStage has seen this feature
                        {
                            features.FeaturesByStage[feature.Stage][feature.Name] = &feature;
                        }
                    }
                    else // Stage value is out of range or new to the ByStage map
                    {
                        features.FeaturesByStage[FeatureStage::Unknown][feature.Name] = &feature;
                    }
                }
            }

            feature.Symbols.emplace(path, signature);

            if (hit_type == Scanner::SymbolHitType::FeatureGetter)
            {
                DWORD location_type;
                ThrowIfFailed(feature_symbol_hit->get_locationType(&location_type));
                if (location_type == LocationType::LocIsStatic)
                {
                    DWORD rva;
                    hr = feature_symbol_hit->get_relativeVirtualAddress(&rva);
                    ThrowIfFailed(hr);
                    if (hr == S_OK)
                    {
                        feature.GetterRvasBySignature[signature].insert(rva);
                    }
                }
            }
        }
    }

    for (auto& unresolved_feature_entry : features.FeaturesByStage[FeatureStage::Unknown])
    {
        auto unresolved_feature = unresolved_feature_entry.second;
        for (auto const& getter : unresolved_feature->GetterRvasBySignature)
        {
            features.FeaturesByGetterSymbolSignature[getter.first].insert(unresolved_feature);
        }
    }

    assert(features.FeaturesByName.size() == (
        features.FeaturesByStage[FeatureStage::Unknown].size() +
        features.FeaturesByStage[FeatureStage::AlwaysDisabled].size() +
        features.FeaturesByStage[FeatureStage::AlwaysEnabled].size() +
        features.FeaturesByStage[FeatureStage::DisabledByDefault].size() +
        features.FeaturesByStage[FeatureStage::EnabledByDefault].size()));
}

bool mach2::Scanner::HasDuplicateFeatureWithId(std::int64_t featureId, std::wstring& featureName, mach2::Scanner::Features& features)
{
    for (int i = 0; i < 100; i++)
    {
        auto candidate = (featureName + L"|mach2.warning.duplicate") + std::to_wstring(i);
        if (features.FeaturesByName.find(candidate) != features.FeaturesByName.end())
        {
            return features.FeaturesByName[candidate].Id == featureId;
        }
    }
    return false;
}

std::wstring mach2::Scanner::GetUniqueNameForDuplicateFeature(mach2::Scanner::Feature& feature, mach2::Scanner::Features& features)
{
    for (int i = 0; i < 100; i++)
    {
        auto candidate = (feature.Name + L"|mach2.warning.duplicate") + std::to_wstring(i);
        if (features.FeaturesByName.find(candidate) == features.FeaturesByName.end())
        {
            return candidate;
        }
    }
    return nullptr;
}

void mach2::Scanner::GetMissingFeatureIdsFromImagesAtPath(mach2::Scanner::Features& features, std::wstring const& symbol_path_root, std::wstring const& image_path_root)
{
    return InternalGetMissingFeatureIdsFromImagesAtPath(features, image_path_root);
}

void mach2::Scanner::InternalGetMissingFeatureIdsFromImagesAtPath(mach2::Scanner::Features& features, std::wstring const& image_path_root)
{
    WIN32_FIND_DATA find_data = {};
    auto find_handle = FindFirstFileEx((image_path_root + L"\\*.*").c_str(), FindExInfoBasic, &find_data, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
    if (find_handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (find_data.dwReserved0 == IO_REPARSE_TAG_SYMLINK)
                continue;

            std::filesystem::path current_file(find_data.cFileName);
            if (current_file != L"." && current_file != L"..")
            {
                if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    std::filesystem::path image_subpath(image_path_root);
                    image_subpath /= find_data.cFileName;
                    
                    ExecuteCallback(image_subpath);
                    InternalGetMissingFeatureIdsFromImagesAtPath(features, image_subpath);
                }
                else
                {
                    if ((_wcsnicmp(current_file.extension().c_str(), L".dll", 4) == 0) || 
                        (_wcsnicmp(current_file.extension().c_str(), L".exe", 4) == 0) ||
                        (_wcsnicmp(current_file.extension().c_str(), L".cpl", 4) == 0) ||
                        (_wcsnicmp(current_file.extension().c_str(), L".scr", 4) == 0) ||
                        (_wcsnicmp(current_file.extension().c_str(), L".sys", 4) == 0))
                    {
                        std::filesystem::path image_path(image_path_root);
                        image_path /= find_data.cFileName;

                        ExecuteCallback(image_path);
                        GetMissingFeatureIdsFromImageAtPath(image_path, features);
                    }
                }
            }
        } while (FindNextFile(find_handle, &find_data));

        FindClose(find_handle);
    }
}

void mach2::Scanner::GetMissingFeatureIdsFromImageAtPath(std::wstring const& image_path, mach2::Scanner::Features& features)
{
    auto file_handle = CreateFile(image_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    ThrowIfInvalidHandle(file_handle);

    auto file_mapping = CreateFileMapping(file_handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
    ThrowIfInvalidHandle(file_mapping);

    auto file_view = MapViewOfFile(file_mapping, FILE_MAP_READ, 0, 0, 0);
    ThrowIf(file_view == nullptr);

    auto headers = ImageNtHeader(file_view);
    if (headers != nullptr)
    {
        if (headers->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        {
            GUID signature{};
            auto debug_entry = headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
            if (debug_entry.Size > 0)
            {
                auto debug_directory = static_cast<PIMAGE_DEBUG_DIRECTORY>(ImageRvaToVa(headers, file_view, debug_entry.VirtualAddress, nullptr));
                if (debug_directory->Type == IMAGE_DEBUG_TYPE_CODEVIEW)
                {
                    auto pdb_file_info = static_cast<PdbFileInfo*>(ImageRvaToVa(headers, file_view, debug_directory->AddressOfRawData, nullptr));
                    if (pdb_file_info->magic == PDB_FILE_INFO_MAGIC)
                    {
                        signature = pdb_file_info->signature;
                    }
                }
            }

            if (!IsEqualGUID(signature, GUID{}))
            {
                csh handle;
                SYSTEM_INFO system_info{};
                GetNativeSystemInfo(&system_info);

                ThrowIf(cs_open(CapstoneHelpers::GetArchitectureForSystemArchitecture(system_info.wProcessorArchitecture), CapstoneHelpers::GetModeForSystemArchitecture(system_info.wProcessorArchitecture), &handle) != CS_ERR_OK);
                ThrowIf(cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON) != CS_ERR_OK);

                for (auto feature : features.FeaturesByGetterSymbolSignature[signature])
                {
                    for (auto const& getter_relative_address : feature->GetterRvasBySignature[signature])
                    {
                        auto function_ptr = ImageRvaToVa(headers, file_view, getter_relative_address, nullptr);
                        const auto CODE_SAMPLE_SIZE = 512;
                        if (function_ptr == nullptr || IsBadReadPtr(function_ptr, CODE_SAMPLE_SIZE))
                        {
                            continue;
                        }
                        cs_insn* instructions;
                        auto instruction_count = cs_disasm(handle, static_cast<uint8_t*>(function_ptr), CODE_SAMPLE_SIZE, 0, 0, &instructions);
                        std::int64_t probable_id = 0;
                        for (int i = 0; i < instruction_count; i++)
                        {
#if X64
                            auto x86 = instructions[i].detail->x86;
                            if (std::string(instructions[i].mnemonic) == "mov")
                            {
                                if (x86.operands[0].reg == X86_REG_ECX || x86.operands[0].reg == X86_REG_EDX ||
                                    x86.operands[0].reg == X86_REG_RCX || x86.operands[0].reg == X86_REG_RDX)
                                {
                                    if (x86.operands[1].type == X86_OP_IMM)
                                    {
                                        if (instructions[i].detail->x86.operands[1].imm == 1)
                                        {
                                            continue;
                                        }
                                        probable_id = instructions[i].detail->x86.operands[1].imm;
                                    }
                                }
                            }
                            if (std::string(instructions[i].mnemonic) == "call" && probable_id != 0)
                            {
                                break;
                            }
#elif ARM64
                            auto arm64 = instructions[i].detail->arm64;
                            auto reg = arm64.operands[0].reg;
                            if (std::string(instructions[i].mnemonic) == "ldr" && reg == std::clamp(reg, ARM64_REG_W0, ARM64_REG_W30) && arm64.operands[1].type == ARM64_OP_IMM)
                            {
                                auto effective_addr = static_cast<std::uint8_t*>(function_ptr) + instructions[i].detail->arm64.operands[1].imm;
                                probable_id = *(reinterpret_cast<std::uint32_t*>(effective_addr));
                            }
                            if (std::string(instructions[i].mnemonic) == "movz" && reg == std::clamp(reg, ARM64_REG_W0, ARM64_REG_W30) && arm64.operands[1].type == ARM64_OP_IMM)
                            {
                                auto working_value = instructions[i].detail->arm64.operands[1].imm;
                                auto working_register = reg;

                                arm64 = instructions[i+1].detail->arm64;
                                reg = arm64.operands[0].reg;
                                if (std::string(instructions[i+1].mnemonic) == "movk" && reg == working_register && arm64.operands[1].type == ARM64_OP_IMM)
                                {
                                    probable_id = working_value | (arm64.operands[1].imm << arm64.operands[1].shift.value);
                                }
                            }
                            if (std::string(instructions[i].mnemonic) == "bl" && probable_id != 0)
                            {
                                break;
                            }
#endif
                            if (std::string(instructions[i].mnemonic) == "ret")
                            {
                                break;
                            }
                        }
                        cs_free(instructions, instruction_count);

                        const auto VELOCITY_ID_MINIMUM = 10000;
                        if (probable_id >= VELOCITY_ID_MINIMUM)
                        {
                            if (feature->Id != 0 && feature->Id != probable_id)
                            {
                                if (HasDuplicateFeatureWithId(probable_id, feature->Name, features))
                                    continue;

                                auto unique_name = GetUniqueNameForDuplicateFeature(*feature, features);
                                auto& new_feature = features.FeaturesByName[unique_name];
                                features.FeaturesByStage[FeatureStage::Unknown][unique_name] = &new_feature;
                                new_feature.Name = unique_name;
                                new_feature.Id = probable_id;
                                new_feature.Symbols = feature->Symbols;
                            }
                            else
                            {
                                feature->Id = probable_id;
                                break;
                            }
                        }
                    }
                }

                ThrowIf(cs_close(&handle) != CS_ERR_OK);
            }
        }
    }

    UnmapViewOfFile(file_view);    
    CloseHandle(file_mapping);
    CloseHandle(file_handle);
}