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
#include <string>

static void* kvp_create_instance(const char *fileName)
{
    ut_kvp_instance_t* instance = ut_kvp_createInstance();
    if (!instance)
    {
        UT_LOG_ERROR("[MOTIONSENSOR_TESTCONFIG] Failed to create kvp instance");
        return nullptr;
    }
    if (ut_kvp_open(instance, const_cast<char*>(fileName)) != UT_KVP_STATUS_SUCCESS)
    {
        UT_LOG_ERROR("[MOTIONSENSOR_TESTCONFIG] Failed to open config file: %s", fileName);
        ut_kvp_destroyInstance(instance);
        return nullptr;
    }
    return instance;
}

static void kvp_destroy_instance(void *instance)
{
    ut_kvp_destroyInstance((ut_kvp_instance_t*)instance);
}

UT_status_t vts_motionsensor_parse_eventtest_config(
    const char *configFile,
    const char *module,
    const char *testGroup,
    const char *testName,
    motionSensorL3EventTestConfig_t& config)
{
    ut_kvp_instance_t* kvp = (ut_kvp_instance_t*)kvp_create_instance(configFile);
    if (!kvp)
        return UT_STATUS_FAILURE;

    const std::string node = std::string(module) + "." + testGroup + "." + testName;

    config.sensorIndex        = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".sensor_index").c_str());
    config.noMotionSeconds    = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".no_motion_seconds").c_str());
    config.activeStartSeconds = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".active_start_seconds").c_str());
    config.activeStopSeconds  = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".active_stop_seconds").c_str());
    config.eventTimeoutMs     = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".event_timeout_ms").c_str());
    config.numEvents = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".num_events").c_str());
    if (config.numEvents <= 0) config.numEvents = 1;

    char modeStr[64] = {};
    ut_kvp_getStringField(kvp, (node + ".operational_mode").c_str(), modeStr, sizeof(modeStr));
    config.operationalMode = (std::string(modeStr) == "NO_MOTION") ? 1 : 0;

    UT_LOG_INFO("[MOTIONSENSOR_TESTCONFIG] %s: sensorIndex=%d mode=%s noMotionSecs=%d timeoutMs=%d numEvents=%d",
        testName, config.sensorIndex, modeStr, config.noMotionSeconds, config.eventTimeoutMs,
        config.numEvents);

    kvp_destroy_instance(kvp);
    return UT_STATUS_OK;
}

UT_status_t vts_motionsensor_parse_activewindow_config(
    const char *configFile,
    const char *module,
    const char *testGroup,
    const char *testName,
    motionSensorL3ActiveWindowTestConfig_t& config)
{
    ut_kvp_instance_t* kvp = (ut_kvp_instance_t*)kvp_create_instance(configFile);
    if (!kvp)
        return UT_STATUS_FAILURE;

    const std::string node = std::string(module) + "." + testGroup + "." + testName;

    config.sensorIndex          = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".sensor_index").c_str());
    config.windowStartSeconds   = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".window_start_seconds").c_str());
    config.windowEndSeconds     = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".window_end_seconds").c_str());
    config.expectEventReceived  = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".expect_event_received").c_str());
    config.eventTimeoutMs       = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".event_timeout_ms").c_str());

    UT_LOG_INFO("[MOTIONSENSOR_TESTCONFIG] %s: sensorIndex=%d window=[%d,%d] expectEvent=%d timeoutMs=%d",
        testName, config.sensorIndex, config.windowStartSeconds, config.windowEndSeconds, config.expectEventReceived, config.eventTimeoutMs);

    kvp_destroy_instance(kvp);
    return UT_STATUS_OK;
}

UT_status_t vts_motionsensor_parse_deepsleep_config(
    const char *configFile,
    const char *module,
    const char *testGroup,
    const char *testName,
    motionSensorL3DeepSleepTestConfig_t& config)
{
    ut_kvp_instance_t* kvp = (ut_kvp_instance_t*)kvp_create_instance(configFile);
    if (!kvp)
        return UT_STATUS_FAILURE;

    const std::string node = std::string(module) + "." + testGroup + "." + testName;

    config.sensorIndex              = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".sensor_index").c_str());
    config.deepSleepDurationSeconds = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".deep_sleep_duration_seconds").c_str());
    config.eventTimeoutMs           = (int32_t)ut_kvp_getUInt32Field(kvp, (node + ".event_timeout_ms").c_str());

    UT_LOG_INFO("[MOTIONSENSOR_TESTCONFIG] %s: sensorIndex=%d sleepDuration=%d timeoutMs=%d",
        testName, config.sensorIndex, config.deepSleepDurationSeconds, config.eventTimeoutMs);

    kvp_destroy_instance(kvp);
    return UT_STATUS_OK;
}
