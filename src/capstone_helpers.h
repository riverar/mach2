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
namespace mach2
{
    class CapstoneHelpers
    {
    public:
        static cs_mode GetModeForSystemArchitecture(std::uint16_t architecture)
        {
            switch (architecture) {
            case PROCESSOR_ARCHITECTURE_ARM64:
                return CS_MODE_ARM;
            case PROCESSOR_ARCHITECTURE_AMD64:
                return CS_MODE_64;
            default:
                throw std::system_error(E_INVALIDARG, std::generic_category(), "Unsupported architecture");
            }
        }

        static cs_arch GetArchitectureForSystemArchitecture(std::uint16_t architecture)
        {
            switch (architecture) {
            case PROCESSOR_ARCHITECTURE_ARM64:
                return cs_arch::CS_ARCH_ARM64;
            case PROCESSOR_ARCHITECTURE_AMD64:
                return cs_arch::CS_ARCH_X86;
            default:
                throw std::system_error(E_INVALIDARG, std::generic_category(), "Unsupported architecture");
            }
        }
    };
}