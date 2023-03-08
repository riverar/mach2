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
#include "wnf.h"
#include "feature.h"

mach2::Feature::Feature(mach2::wil_details_StagingConfigFeature feature)
{
    _backingData = feature;
}

mach2::Feature::Feature(unsigned int id, unsigned int service_state, unsigned int variant)
{
    _backingData = {};
    _backingData.featureId = id;

    // WARNING: Not currently setting user/test states
    // Generally, serviceState trumps all.

    _backingData.serviceState = service_state;
    _backingData.userState = 0;
    _backingData.testState = 0;
    _backingData.variant = variant;
}