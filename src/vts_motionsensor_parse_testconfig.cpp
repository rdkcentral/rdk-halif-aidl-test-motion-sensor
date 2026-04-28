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
 * @file vts_motionsensor_parse_testconfig.cpp
 */

#include "vts_motionsensor_parse_testconfig.h"
#include <ut_kvp_profile.h>
#include <ut_log.h>

/* Global L3 test configuration */
MotionSensorL3TestConfig_t gMotionSensorL3Config = {};

void vts_motionsensor_parse_testconfig(void)
{
    UT_LOG_INFO("vts_motionsensor_parse_testconfig: Parsing L3 test configuration");

    gMotionSensorL3Config.operationalMode =
        UT_KVP_PROFILE_GET_UINT32("MotionSensor/L3/operationalMode");

    gMotionSensorL3Config.noMotionSeconds =
        UT_KVP_PROFILE_GET_UINT32("MotionSensor/L3/noMotionSeconds");

    gMotionSensorL3Config.eventWaitTimeoutMs =
        UT_KVP_PROFILE_GET_UINT32("MotionSensor/L3/eventWaitTimeoutMs");

    gMotionSensorL3Config.activeWindowStartSeconds =
        UT_KVP_PROFILE_GET_UINT32("MotionSensor/L3/activeWindowStartSeconds");

    gMotionSensorL3Config.activeWindowEndSeconds =
        UT_KVP_PROFILE_GET_UINT32("MotionSensor/L3/activeWindowEndSeconds");

    gMotionSensorL3Config.useActiveWindow =
        UT_KVP_PROFILE_GET_BOOL("MotionSensor/L3/useActiveWindow");

    UT_LOG_INFO("vts_motionsensor_parse_testconfig: operationalMode[%d] noMotionSeconds[%d] "
        "eventWaitTimeoutMs[%d] useActiveWindow[%s]",
        gMotionSensorL3Config.operationalMode,
        gMotionSensorL3Config.noMotionSeconds,
        gMotionSensorL3Config.eventWaitTimeoutMs,
        gMotionSensorL3Config.useActiveWindow ? "true" : "false");
}
