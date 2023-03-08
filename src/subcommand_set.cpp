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
#include "mach2.h"
#include "wnf.h"
#include "feature.h"
#include "feature_config.h"
#include "feature_manager.h"

void SetSubcommand(unsigned int feature_id, unsigned int service_state, unsigned int variant, bool add_to_machine_store)
{
    mach2::FeatureManager feature_manager;
    auto featureConfig = feature_manager.GetFeatureConfig();
    featureConfig.remove(feature_id);
    if (add_to_machine_store)
    {
        featureConfig.add(mach2::Feature(feature_id, service_state, variant));
    }
    feature_manager.SetFeatureConfig(featureConfig);
    std::wcout << L"OK." << std::endl;
}