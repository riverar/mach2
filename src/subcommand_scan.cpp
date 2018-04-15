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

void SymbolProcessingCallback(const std::wstring& symbol_path)
{
    std::wstring display_text = L"Scanning: ";
    wchar_t compact_path[MAX_PATH];

    CONSOLE_SCREEN_BUFFER_INFO console_info;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_info);

    std::uint32_t max_chars_for_path = console_info.dwSize.X - display_text.length() - sizeof('\r');
    if (PathCompactPathExW(compact_path, symbol_path.c_str(), max_chars_for_path, /* dwFlags */ 0))
    {
        std::wcout << L"Scanning: " << std::left << std::setw(max_chars_for_path) << compact_path << '\r' << std::flush;
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

void ScanSubcommand(const std::string& symbols_path, const std::string& output_path, bool omit_symbol_hits)
{
    try
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> to_wstring;
        std::wstring wide_symbols_path = to_wstring.from_bytes(symbols_path);
        std::wstring wide_output_path;
        bool writing_to_file = false;
        if (!output_path.empty())
        {
            writing_to_file = true;
            wide_output_path = to_wstring.from_bytes(output_path);
        }

        mach2::Scanner scanner;
        scanner.SetCallback(SymbolProcessingCallback);
        auto feature_map = scanner.GetFeaturesFromSymbolsAtPath(wide_symbols_path);
        ClearConsoleProgress();

        if (writing_to_file)
        {
            std::wofstream wide_output_stream(wide_output_path);
            for (auto& value : feature_map)
            {
                auto feature = value.second;
                wide_output_stream << feature.Name;
                wide_output_stream << L": " << feature.Id;
                if (!omit_symbol_hits)
                {
                    wide_output_stream << std::endl;
                    for (auto& symbol_path : feature.SymbolPaths)
                        wide_output_stream << L"# " << symbol_path << std::endl;
                }
                wide_output_stream << std::endl;
            }
            wide_output_stream.close();
        }
        else
        {
            for each (auto& value in feature_map)
            {
                auto feature = value.second;
                if (omit_symbol_hits)
                {
                    std::wcout << feature.Name << L": " << feature.Id;
                }
                else
                {
                    std::wcout << feature.Name << L": " << feature.Id << std::endl;
                    for (auto& symbol_path : feature.SymbolPaths)
                        std::wcout << L"# " << symbol_path << std::endl;
                }
                std::wcout << std::endl;
            }
        }
    }
    catch (...)
    {
        ClearConsoleProgress();
        throw;
    }
}