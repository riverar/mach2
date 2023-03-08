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
#include "feature_config.h"

mach2::FeatureConfig::FeatureConfig(std::vector<unsigned char> buffer, mach2::WNF_CHANGE_STAMP stamp)
{
    // Inbound buffer looks like:
    // [configHeader][configFeature1][configFeature2]...[configFeature{configHeader.featureCount}][unsupported trigger data]

    auto headerStructSize = sizeof(_backingConfigHeader);
    memcpy_s(&_backingConfigHeader, headerStructSize, buffer.data(), headerStructSize);
    if (_backingConfigHeader.featureCount > 0)
    {
        _backingConfigFeatures.resize(_backingConfigHeader.featureCount);
        auto featureStructSize = sizeof(wil_details_StagingConfigFeature) * _backingConfigHeader.featureCount;
        memcpy_s(&_backingConfigFeatures[0], featureStructSize, &buffer[headerStructSize], featureStructSize);
    }
    _stamp = stamp;
}

std::vector<unsigned char> mach2::FeatureConfig::serialize()
{
    auto headerStructSize = sizeof(_backingConfigHeader);
    auto featureStructSize = sizeof(wil_details_StagingConfigFeature) * _backingConfigHeader.featureCount;
    std::vector<unsigned char> bytes(headerStructSize + featureStructSize);

    _backingConfigHeader.version = 2;
    _backingConfigHeader.versionMinor = 2;
    _backingConfigHeader.headerSizeBytes = 16;

    // TODO: We don't currently support trigger serialization
    // or manipulation so ensure its zeroed out or WIL
    // will go looking for it after the feature blob and fail
    _backingConfigHeader.featureUsageTriggerCount = 0;

    memcpy_s(
        &bytes[0],
        bytes.size(),
        &_backingConfigHeader,
        headerStructSize);

    if (featureStructSize > 0)
    {
        memcpy_s(
            &bytes[headerStructSize],
            bytes.size() - headerStructSize,
            &_backingConfigFeatures[0],
            featureStructSize);
    }

    return bytes;
}

std::vector<mach2::Feature> mach2::FeatureConfig::features() const
{
    return std::vector<mach2::Feature>(_backingConfigFeatures.begin(), _backingConfigFeatures.end());
}

void mach2::FeatureConfig::add(mach2::Feature feature)
{
    wil_details_StagingConfigFeature rawFeature = {};
    rawFeature.featureId = feature.id();
    rawFeature.serviceState = feature.serviceState();
    rawFeature.userState = feature.userState();
    rawFeature.testState = feature.testState();
    rawFeature.variant = feature.variant();
    rawFeature.changedInSession = 1;

    _backingConfigHeader.featureCount++;
    _backingConfigFeatures.push_back(rawFeature);
}

void mach2::FeatureConfig::remove(unsigned int featureId)
{
    auto find = std::find_if(_backingConfigFeatures.begin(), _backingConfigFeatures.end(),
        [featureId](wil_details_StagingConfigFeature f) { return f.featureId == featureId; });

    if (find != std::end(_backingConfigFeatures))
    {
        _backingConfigFeatures.erase(find);
        _backingConfigHeader.featureCount--;
    }
}