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

void mach2::Scanner::InternalGetFeaturesFromSymbolsAtPath(std::wstring const &symbol_path_root, mach2::Scanner::FeatureMap &feature_map)
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

                    InternalGetFeaturesFromSymbolsAtPath(symbols_path, feature_map);
                }
                else
                {
                    if (current_file.extension() == L".pdb")
                    {
                        std::experimental::filesystem::path pdb_path(symbol_path_root);
                        pdb_path /= find_data.cFileName;

                        ExecuteCallback(pdb_path);
                        GetFeaturesFromSymbolAtPath(pdb_path, feature_map);
                    }
                }
            }
        } while (FindNextFile(find_handle, &find_data));

        FindClose(find_handle);
    }
}

mach2::Scanner::FeatureMap mach2::Scanner::GetFeaturesFromSymbolsAtPath(std::wstring const &symbols_path)
{
    FeatureMap feature_map;
    InternalGetFeaturesFromSymbolsAtPath(symbols_path, feature_map);
    return feature_map;
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

void mach2::Scanner::GetFeaturesFromSymbolAtPath(std::wstring const &path, mach2::Scanner::FeatureMap &feature_map)
{
    auto feature_search_str = L"*WilFeatureTraits_Feature_*";

    std::vector<std::wstring> feature_id_search_templates
    {
        L"*WilFeatureTraits_Feature_$s::id",
        L"*wil::Feature<__WilFeatureTraits_Feature_$s>::id",
    };

    CComPtr<IDiaDataSource> data_source;
    ThrowIfFailed(NoRegCoCreate(L"msdia140.dll", CLSID_DiaSource, IID_PPV_ARGS(&data_source)));
    ThrowIfFailed(data_source->loadDataFromPdb(path.c_str()));

    CComPtr<IDiaSession> session;
    ThrowIfFailed(data_source->openSession(&session));

    CComPtr<IDiaSymbol> root_symbol;
    ThrowIfFailed(session->get_globalScope(&root_symbol));

    CComPtr<IDiaEnumSymbols> symbols;
    ThrowIfFailed(session->findChildren(root_symbol, SymTagEnum::SymTagNull, feature_search_str, NameSearchOptions::nsfRegularExpression, &symbols));

    CComPtr<IDiaSymbol> symbol;
    unsigned long results_retrieved = 0;
    while (SUCCEEDED(symbols->Next(1, &symbol, &results_retrieved)) && results_retrieved == 1)
    {
        CComPtr<IDiaSymbol> dia_symbol;
        dia_symbol.Attach(symbol.Detach());

        CComBSTR raw_symbol_name;
        ThrowIfFailed(dia_symbol->get_name(&raw_symbol_name));
        auto feature_name = GetFeatureNameFromSymbolName(raw_symbol_name.m_str);
        auto& feature = feature_map[feature_name];

        feature.SymbolPaths.push_back(path);

        if (feature.Id > 0)
            continue;

        feature.Name = feature_name;

        for (auto feature_id_search_template : feature_id_search_templates)
        {
            auto feature_id_search_str = feature_id_search_template;
            feature_id_search_str.replace(feature_id_search_template.find(L"$s"), std::wstring(L"$s").size(), feature.Name);

            CComPtr<IDiaEnumSymbols> results;
            ThrowIfFailed(session->findChildren(root_symbol, SymTagEnum::SymTagData, feature_id_search_str.c_str(),
                NameSearchOptions::nsfRegularExpression | NameSearchOptions::nsfUndecoratedName, &results));

            long ids_found = 0;
            ThrowIfFailed(results->get_Count(&ids_found));

            if (ids_found > 0)
            {
                CComPtr<IDiaSymbol> id_symbol;
                ThrowIfFailed(results->Next(1, &id_symbol, &results_retrieved));

                CComVariant id_value;
                ThrowIfFailed(id_symbol->get_value(&id_value));
                feature.Id = id_value.lVal;

                break;
            }
        }
    }
}