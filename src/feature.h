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
    class Feature
    {
        mach2::wil_details_StagingConfigFeature _backingData;

    public:
        Feature(mach2::wil_details_StagingConfigFeature feature);
        Feature(unsigned int id, unsigned int serviceState, unsigned int variant);

        unsigned int id() const { return _backingData.featureId; };
        unsigned int userState() const { return _backingData.userState; };
        unsigned int serviceState() const { return _backingData.serviceState; };
        unsigned int testState() const { return _backingData.testState; };
        unsigned int variant() const { return _backingData.variant; };
    };
}