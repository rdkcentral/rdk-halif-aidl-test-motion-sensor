/**
 *  If not stated otherwise in this file or this component's LICENSE
 *  file the following copyright and licenses apply:
 *
 *  Copyright 2026 RDK Management
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @file vts_motionsensor_parse_hfp.cpp
 */

#include "vts_motionsensor_parse_hfp.h"
#include "vts_motionsensor_helper_functions.h"
#include <ut_kvp_profile.h>
#include <ut_log.h>
#include <string>

/* Global HFP structure */
motionsensorHFPFile_t gMotionSensorHFP;

void vts_motionsensor_parse_hfp(void)
{
    char subValue[UT_KVP_MAX_ELEMENT_SIZE];

    const std::string sensorsKey = "sensor.motion";

    int numSensors = UT_KVP_PROFILE_GET_LIST_COUNT(sensorsKey.c_str());
    gMotionSensorHFP.sensors.clear();

    UT_LOG_INFO("vts_motionsensor_parse_hfp: Parsing %d sensor(s) from HFP profile", numSensors);

    for (int i = 0; i < numSensors; ++i)
    {
        motionsensorHFP_t sensor;
        const std::string baseKey = sensorsKey + "." + std::to_string(i);

        // id
        sensor.id = UT_KVP_PROFILE_GET_UINT32((baseKey + ".id").c_str());

        // Capabilities
        UT_KVP_PROFILE_GET_STRING((baseKey + ".sensorName").c_str(), subValue);
        sensor.capabilities.sensorName = subValue;
        sensor.capabilities.minSensitivity           = UT_KVP_PROFILE_GET_UINT32((baseKey + ".sensitivity_range.minSensitivity").c_str());
        sensor.capabilities.maxSensitivity           = UT_KVP_PROFILE_GET_UINT32((baseKey + ".sensitivity_range.maxSensitivity").c_str());
        sensor.capabilities.supportsDeepSleepAutonomy = UT_KVP_PROFILE_GET_BOOL((baseKey + ".supportsDeepSleepAutonomy").c_str());

        // Operational modes
        const std::string modesKey = baseKey + ".operational_modes";
        int numModes = UT_KVP_PROFILE_GET_LIST_COUNT(modesKey.c_str());
        sensor.operationalModes.clear();
        for (int j = 0; j < numModes; ++j)
        {
            UT_KVP_PROFILE_GET_STRING((modesKey + "." + std::to_string(j)).c_str(), subValue);
            OperationalMode mode = (OperationalMode)UT_Control_GetMapValue(operationalMode_mapTable, subValue, (int32_t)OperationalMode::MOTION);
            sensor.operationalModes.push_back(mode);
        }

        // Default StartConfig
        const std::string startConfigKey = baseKey + ".defaultStartConfig";
        UT_KVP_PROFILE_GET_STRING((startConfigKey + ".operationalMode").c_str(), subValue);
        sensor.defaultStartConfig.operationalMode    = (OperationalMode)UT_Control_GetMapValue(operationalMode_mapTable, subValue, (int32_t)OperationalMode::MOTION);
        sensor.defaultStartConfig.noMotionSeconds    = UT_KVP_PROFILE_GET_UINT32((startConfigKey + ".noMotionSeconds").c_str());
        sensor.defaultStartConfig.activeStartSeconds = UT_KVP_PROFILE_GET_UINT32((startConfigKey + ".activeStartSeconds").c_str());
        sensor.defaultStartConfig.activeStopSeconds  = UT_KVP_PROFILE_GET_UINT32((startConfigKey + ".activeStopSeconds").c_str());

        // Active windows
        const std::string windowsKey = baseKey + ".active_windows";
        int numWindows = UT_KVP_PROFILE_GET_LIST_COUNT(windowsKey.c_str());
        sensor.activeWindows.clear();
        for (int j = 0; j < numWindows; ++j)
        {
            const std::string windowBaseKey = windowsKey + "." + std::to_string(j);
            motionsensorHFP_t::ActiveWindow window;
            window.startTimeOfDaySeconds = UT_KVP_PROFILE_GET_UINT32((windowBaseKey + ".startTimeOfDaySeconds").c_str());
            window.endTimeOfDaySeconds   = UT_KVP_PROFILE_GET_UINT32((windowBaseKey + ".endTimeOfDaySeconds").c_str());
            sensor.activeWindows.push_back(window);
        }

        // Requirements
        sensor.minReactionTimeMs = UT_KVP_PROFILE_GET_UINT32((baseKey + ".requirements.min_reaction_time_ms").c_str());

        // Notes
        const std::string notesKey = baseKey + ".notes";
        int numNotes = UT_KVP_PROFILE_GET_LIST_COUNT(notesKey.c_str());
        sensor.notes.clear();
        for (int j = 0; j < numNotes; ++j)
        {
            UT_KVP_PROFILE_GET_STRING((notesKey + "." + std::to_string(j)).c_str(), subValue);
            sensor.notes.push_back(subValue);
        }

        UT_LOG_INFO("vts_motionsensor_parse_hfp: sensor[%d] id=%d name=%s minSensitivity=%d maxSensitivity=%d",
            i, sensor.id, sensor.capabilities.sensorName.c_str(),
            sensor.capabilities.minSensitivity, sensor.capabilities.maxSensitivity);

        gMotionSensorHFP.sensors.push_back(sensor);
    }
}
