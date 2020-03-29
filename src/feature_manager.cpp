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
#include "wnf.h"
#include "feature.h"
#include "feature_config.h"
#include "feature_manager.h"

mach2::FeatureConfig mach2::FeatureManager::GetFeatureConfig()
{
    WNF_STATE_NAME wnf_machine_store{ 0xA3BC7C75, 0x418A073A };

    std::vector<unsigned char> wnf_state_buffer(8192);
    unsigned long state_buffer_size = static_cast<unsigned long>(wnf_state_buffer.size());
    WNF_CHANGE_STAMP wnf_change_stamp;

    ThrowIfNtFailed(NtQueryWnfStateData(&wnf_machine_store, nullptr, nullptr, &wnf_change_stamp,
        wnf_state_buffer.data(), &state_buffer_size));

    return mach2::FeatureConfig(wnf_state_buffer, wnf_change_stamp);
}

void mach2::FeatureManager::SetFeatureConfig(mach2::FeatureConfig feature_config)
{
    mach2::WNF_STATE_NAME wnf_machine_store{ 0xA3BC7C75, 0x418A073A };
    auto serialized_config = feature_config.serialize();

    ThrowIfNtFailed(NtUpdateWnfStateData(&wnf_machine_store, &serialized_config[0], static_cast<unsigned long>(serialized_config.size()),
        nullptr, nullptr, feature_config.stamp(), 1));
}

std::string mach2::FeatureManager::GetFeatureNameFromId(long featureId)
{
    std::fstream data(L"features.txt");
    if (!data.bad())
    {
        if (_feature_map.empty())
        {
            std::string line;
            while (std::getline(data, line, '\n'))
            {
                if (line.compare(0, 1, "#") == 0 || line.empty())
                    continue;

                auto feature_name = line.substr(0, line.find(":"));
                auto feature_id = std::stoul(line.substr(line.find(":") + 1));
                _feature_map.insert(std::pair<unsigned long, std::string>(feature_id, feature_name));
            }
        }
    }
    return _feature_map[featureId];
}
