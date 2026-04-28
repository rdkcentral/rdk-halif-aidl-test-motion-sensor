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
#include <ut_kvp_profile.h>
#include <ut_log.h>
#include <string>

using namespace std;

/* Global device info structure */
MotionSensorDeviceInfo_t gMotionSensorDeviceInfo = {};

void vts_motionsensor_parse_hfp(void)
{
    UT_LOG_INFO("vts_motionsensor_parse_hfp: Parsing HFP profile");

    /* Number of sensors */
    gMotionSensorDeviceInfo.numSensors =
        UT_KVP_PROFILE_GET_UINT32("IMotionSensorManager/numSensors");

    /* Sensitivity range */
    gMotionSensorDeviceInfo.minSensitivity =
        UT_KVP_PROFILE_GET_UINT32("IMotionSensor/capabilities/minSensitivity");
    gMotionSensorDeviceInfo.maxSensitivity =
        UT_KVP_PROFILE_GET_UINT32("IMotionSensor/capabilities/maxSensitivity");

    /* Operational mode support */
    gMotionSensorDeviceInfo.supportsMotionMode =
        UT_KVP_PROFILE_GET_BOOL("IMotionSensor/capabilities/supportsMotionMode");
    gMotionSensorDeviceInfo.supportsNoMotionMode =
        UT_KVP_PROFILE_GET_BOOL("IMotionSensor/capabilities/supportsNoMotionMode");

    /* Deep sleep autonomous support */
    gMotionSensorDeviceInfo.supportsAutonomousDeepSleep =
        UT_KVP_PROFILE_GET_BOOL("IMotionSensor/capabilities/supportsAutonomousDeepSleep");

    /* Active windows support */
    gMotionSensorDeviceInfo.supportsActiveWindows =
        UT_KVP_PROFILE_GET_BOOL("IMotionSensor/capabilities/supportsActiveWindows");
    gMotionSensorDeviceInfo.maxActiveWindows =
        UT_KVP_PROFILE_GET_UINT32("IMotionSensor/capabilities/maxActiveWindows");

    /* noMotionSeconds valid range */
    gMotionSensorDeviceInfo.minNoMotionSeconds = 1;
    gMotionSensorDeviceInfo.maxNoMotionSeconds = 86400;

    UT_LOG_INFO("vts_motionsensor_parse_hfp: numSensors[%d] minSensitivity[%d] maxSensitivity[%d]",
        gMotionSensorDeviceInfo.numSensors,
        gMotionSensorDeviceInfo.minSensitivity,
        gMotionSensorDeviceInfo.maxSensitivity);

    UT_LOG_INFO("vts_motionsensor_parse_hfp: supportsMotionMode[%s] supportsNoMotionMode[%s] "
        "supportsAutonomousDeepSleep[%s] supportsActiveWindows[%s] maxActiveWindows[%d]",
        gMotionSensorDeviceInfo.supportsMotionMode ? "true" : "false",
        gMotionSensorDeviceInfo.supportsNoMotionMode ? "true" : "false",
        gMotionSensorDeviceInfo.supportsAutonomousDeepSleep ? "true" : "false",
        gMotionSensorDeviceInfo.supportsActiveWindows ? "true" : "false",
        gMotionSensorDeviceInfo.maxActiveWindows);
}
