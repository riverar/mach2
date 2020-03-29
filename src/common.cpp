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

void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::system_error{ hr, std::system_category() };
    }
}

void ThrowIfNtFailed(NTSTATUS ntstatus)
{
    if (NT_ERROR(ntstatus))
    {
        throw std::system_error{ ntstatus, std::system_category() };
    }
}

void ThrowIfInvalidHandle(HANDLE h)
{
    if (h == INVALID_HANDLE_VALUE)
    {
        throw std::system_error{ HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE), std::system_category() };
    }
}

std::wstring StringToWideString(const std::string& string)
{
    std::wstring wideString;
    auto requiredBufferSize = MultiByteToWideChar(CP_ACP, 0, string.c_str(), -1, nullptr, 0);
    if (requiredBufferSize > 0)
    {
        std::unique_ptr<wchar_t[]> rawWideString(new wchar_t[requiredBufferSize]);
        if (MultiByteToWideChar(CP_ACP, 0, string.c_str(), -1, rawWideString.get(), requiredBufferSize) > 0)
        {
            wideString = std::wstring(rawWideString.get());
        }
    }

    return wideString;
}