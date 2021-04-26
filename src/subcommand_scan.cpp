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
#include "mach2.h"
#include "wnf.h"
#include "scanner.h"

void SymbolProcessingCallback(const std::wstring& path)
{
    std::wstring display_text = L"Scanning: ";
    wchar_t compact_path[MAX_PATH];

    CONSOLE_SCREEN_BUFFER_INFO console_info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_info);

    std::uint32_t max_chars_for_path = console_info.dwSize.X - static_cast<uint32_t>(display_text.length()) - sizeof('\r');
    if (PathCompactPathExW(compact_path, path.c_str(), max_chars_for_path, /* dwFlags */ 0))
    {
        std::wcout << display_text << std::left << std::setw(max_chars_for_path) << compact_path << '\r' << std::flush;
    }
}

void ClearConsoleProgress()
{
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_info))
    {
        std::uint32_t console_width = console_info.dwSize.X;
        std::wcout << std::setw(console_width - sizeof("\r\n")) << ' ' << '\r' << std::flush;
    }
}

void WriteFeatureMapToScreen(const mach2::Scanner::Features& features, bool omit_symbol_hits)
{
    std::vector<std::wstring> stage_strings{ L"Unknown", L"Always Disabled", L"Disabled By Default", L"Enabled By Default", L"Always Enabled" };
    for(const auto & stage : mach2::Scanner::FeatureStages)
    {
        std::wcout << L"## " << stage_strings[static_cast<int>(stage)] << L":" << std::endl << std::endl;

        if (features.FeaturesByStage.count(stage) > 0)
        {
            for each (auto& kvp in features.FeaturesByStage.at(stage))
            {
                auto feature = kvp.second;
                if (omit_symbol_hits)
                {
                    std::wcout << L"  " << feature->Name << L": " << feature->Id;
                }
                else
                {
                    std::wcout << L"  " << feature->Name << L": " << feature->Id << std::endl;
                    for (auto& symbol : feature->Symbols)
                        std::wcout << L"  # " << symbol.Path << std::endl;
                }
                std::wcout << std::endl;
            }
        }

        std::wcout << std::endl;
    }
}

void WriteFeatureMapToFile(const mach2::Scanner::Features& features, const std::wstring& output_path, bool omit_symbol_hits)
{
    std::vector<std::wstring> stage_strings{ L"Unknown", L"Always Disabled", L"Disabled By Default", L"Enabled By Default", L"Always Enabled" };

    std::wofstream wide_output_stream(output_path);
    for (const auto & stage : mach2::Scanner::FeatureStages)
    {
        wide_output_stream << L"## " << stage_strings[static_cast<int>(stage) + 1] << L":" << std::endl << std::endl;

        if (features.FeaturesByStage.count(stage) > 0)
        {
            for (auto& kvp : features.FeaturesByStage.at(stage))
            {
                auto feature = kvp.second;
                wide_output_stream << L"  " << feature->Name << L": " << feature->Id;
                if (!omit_symbol_hits)
                {
                    wide_output_stream << std::endl;
                    for (auto& symbol : feature->Symbols)
                        wide_output_stream << L"  # " << symbol.Path << std::endl;
                }
                wide_output_stream << std::endl;
            }
        }

        wide_output_stream << std::endl;
    }
    wide_output_stream.close();
}

void ScanSubcommand(const std::string& symbols_path, const std::string& output_path, const std::string& image_path, bool omit_symbol_hits, bool use_capstone)
{
    try
    {
        bool writing_to_file = false;
        std::wstring wide_symbols_path = GetLongAbsolutePath(StringToWideString(symbols_path));
        std::wstring wide_image_path = GetLongAbsolutePath(StringToWideString(image_path));
        std::wstring wide_output_path;
        if (!output_path.empty())
        {
            writing_to_file = true;
            wide_output_path = GetLongAbsolutePath(StringToWideString(output_path));
        }

        mach2::Scanner scanner;
        scanner.SetCallback(SymbolProcessingCallback);
        auto feature_map = scanner.GetFeaturesFromSymbolsAtPath(wide_symbols_path);
        if (use_capstone)
        {
            scanner.GetMissingFeatureIdsFromImagesAtPath(feature_map, wide_symbols_path, wide_image_path);
        }
        ClearConsoleProgress();

        if (writing_to_file)
        {
            WriteFeatureMapToFile(feature_map, wide_output_path, omit_symbol_hits);
        }
        else
        {
            WriteFeatureMapToScreen(feature_map, omit_symbol_hits);
        }
    }
    catch (...)
    {
        ClearConsoleProgress();
        throw;
    }
}