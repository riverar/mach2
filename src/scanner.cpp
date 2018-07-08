/*
    mach2 - feature control multi-tool
    Copyright (c) Rafael Rivera

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
#include <dia2.h>
#include <diacreate.h>

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

void mach2::Scanner::ExecuteCallback(std::wstring const &pdb_path)
{
    if (_callback)
    {
        _callback(pdb_path);
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
            std::experimental::filesystem::path current_file(find_data.cFileName);
            if (current_file != L"." && current_file != L"..")
            {
                if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    std::experimental::filesystem::path symbols_path(symbol_path_root);
                    symbols_path /= find_data.cFileName;

                    InternalGetFeaturesFromSymbolsAtPath(symbols_path, features);
                }
                else
                {
                    if (current_file.extension() == L".pdb")
                    {
                        std::experimental::filesystem::path pdb_path(symbol_path_root);
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
    else
    {
        return mach2::Scanner::SymbolHitType::Feature;
    }
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

    std::wregex pattern(L"WilFeatureTraits_Feature_(\\w*)");
    std::wsmatch match;
    std::regex_search(symbol_name, match, pattern, std::regex_constants::match_any);
    return match.str(1);
}

void mach2::Scanner::GetFeaturesFromSymbolAtPath(std::wstring const &path, mach2::Scanner::Features &features)
{
    CComPtr<IDiaDataSource> data_source;
    ThrowIfFailed(NoRegCoCreate(L"msdia140.dll", CLSID_DiaSource, IID_PPV_ARGS(&data_source)));
    ThrowIfFailed(data_source->loadDataFromPdb(path.c_str()));

    CComPtr<IDiaSession> session;
    ThrowIfFailed(data_source->openSession(&session));

    CComPtr<IDiaSymbol> root_symbol;
    ThrowIfFailed(session->get_globalScope(&root_symbol));

    CComPtr<IDiaEnumSymbols> feature_symbols;
    ThrowIfFailed(session->findChildren(root_symbol, SymTagEnum::SymTagNull, L"*WilFeatureTraits_Feature_*",
        NameSearchOptions::nsfRegularExpression, &feature_symbols));

    unsigned long results_retrieved = 0;
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
            feature.Id = id_value.lVal;
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
                    feature.Stage = static_cast<FeatureStage>(feature_stage_value.iVal + 1);

                    auto & unknown_features = features.FeaturesByStage[FeatureStage::Unknown];
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

        feature.SymbolPaths.insert(path);
    }

    assert(features.FeaturesByName.size() == (
        features.FeaturesByStage[FeatureStage::Unknown].size() +
        features.FeaturesByStage[FeatureStage::AlwaysDisabled].size() +
        features.FeaturesByStage[FeatureStage::AlwaysEnabled].size() +
        features.FeaturesByStage[FeatureStage::DisabledByDefault].size() +
        features.FeaturesByStage[FeatureStage::EnabledByDefault].size()));
}