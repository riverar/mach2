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
#include "feature.h"
#include "feature_config.h"
#include "feature_manager.h"

void DisplaySubcommand()
{
    mach2::FeatureManager feature_manager;
    auto feature_config = feature_manager.GetFeatureConfig();
    auto print_feature = [](mach2::FeatureManager &featureManager, mach2::Feature &feature)
    {
        auto feature_name = featureManager.GetFeatureNameFromId(feature.id());
        if (!feature_name.empty())
        {
            std::wcout << L"  " << feature_name.c_str() << L" ";
        }
        else
        {
            std::wcout << "  ";
        }
        std::wcout << feature.id() << L" ";
        if (feature.variant() != 0)
        {
            std::wcout << L" (variant: " << feature.variant() << L")";
        }
        std::wcout << std::endl;
    };

    std::wcout << L"Enabled:" << std::endl;
    for (auto feature : feature_config.features())
    {
        if (feature.serviceState() == mach2::wil_FeatureEnabledState::Enabled)
            print_feature(feature_manager, feature);
    }
    std::wcout << std::endl;

    std::wcout << L"Disabled:" << std::endl;
    for (auto feature : feature_config.features())
    {
        if (feature.serviceState() == mach2::wil_FeatureEnabledState::Disabled)
            print_feature(feature_manager, feature);
    }
    std::wcout << std::endl;

    std::wcout << L"Defaulted:" << std::endl;
    for (auto feature : feature_config.features())
    {
        if (feature.serviceState() == mach2::wil_FeatureEnabledState::Default)
            print_feature(feature_manager, feature);
    }
    std::wcout << std::endl;
}