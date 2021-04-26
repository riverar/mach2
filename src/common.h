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

#pragma once

#include <stdio.h>
#include <atlbase.h>
#include <atlstr.h>
#include <Windows.h>
#include <PathCch.h>
#include <memory>
#include <vector>
#include <functional>
#include <comutil.h>
#include <locale>
#include <set>
#include <codecvt>
#include <map>
#include <algorithm>
#include <regex>
#include <iterator>
#include <fstream>
#include <codecvt>
#include <iomanip>
#include <locale>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include <cassert>
#include <array>

#define NT_ERROR(Status) ((((ULONG)(Status)) >> 30) == 3)

void ThrowIf(bool b);
void ThrowIfFailed(HRESULT hr);
void ThrowIfNtFailed(NTSTATUS ntstatus);
void ThrowIfInvalidHandle(HANDLE h);
std::wstring StringToWideString(const std::string& string);
std::wstring GetLongAbsolutePath(const std::wstring& path);

class CCoInitialize
{
    HRESULT _hr;
    public:
    CCoInitialize() : _hr(CoInitialize(nullptr))
    {
        ThrowIfFailed(_hr);
    }

    ~CCoInitialize()
    {
        if (SUCCEEDED(_hr))
        {
            CoUninitialize();
        }
    }
};