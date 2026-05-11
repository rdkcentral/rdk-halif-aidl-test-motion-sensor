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
 * @addtogroup HPK Hardware Porting Kit
 * @{
 */

/**
 * @addtogroup MOTION_SENSOR Motion Sensor Module
 * @{
 */

/**
 * @addtogroup MOTION_SENSOR_HALTEST Motion Sensor HAL Tests
 * @{
 */

/**
 * @defgroup MOTION_SENSOR_HALTEST_HEADER Motion Sensor HAL Tests Header File
 * @{
 * @parblock
 *
 * ### Common functions/macros required for the motion sensor :
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 */

/**
 * @file vts_motionsensor.h
 */

#ifndef __VTS_MOTIONSENSOR_H__
#define __VTS_MOTIONSENSOR_H__

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>

#include <ut.h>
#include <ut_log.h>
#include <ut_kvp_profile.h>
#include <ut_control_plane.h>

#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>

#include "com/rdk/hal/sensor/motion/BnMotionSensorControllerListener.h"
#include "com/rdk/hal/sensor/motion/BnMotionSensorEventListener.h"
#include "com/rdk/hal/sensor/motion/IMotionSensorManager.h"
#include "com/rdk/hal/sensor/motion/IMotionSensor.h"
#include "com/rdk/hal/sensor/motion/IMotionSensorController.h"
#include "com/rdk/hal/sensor/motion/IMotionSensorControllerListener.h"
#include "com/rdk/hal/sensor/motion/IMotionSensorEventListener.h"
#include "com/rdk/hal/sensor/motion/State.h"
#include "com/rdk/hal/sensor/motion/Capabilities.h"
#include "com/rdk/hal/sensor/motion/StartConfig.h"
#include "com/rdk/hal/sensor/motion/TimeWindow.h"
#include "com/rdk/hal/sensor/motion/LastEventInfo.h"
#include "com/rdk/hal/sensor/motion/OperationalMode.h"

#include "vts_motionsensor_helper_functions.h"

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;
using namespace std;
using namespace std::chrono;

using namespace com::rdk::hal::sensor::motion;
#define MAX_TRANSITORY_STATE_WAIT_MS   5
#define MAX_FINAL_STATE_WAIT_MS        500
#define MAX_EVENT_WAIT_MS              5000
#define MAX_NO_MOTION_WAIT_BUFFER_MS   2000

typedef com::rdk::hal::sensor::motion::IMotionSensor::Id MotionSensorId;

/* ========================================================================== */
/*  IMotionSensorManager Macros                                                */
/* ========================================================================== */

#define MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(managerHandle, sensorIdsOpt) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getMotionSensorIds(OUT:sensorIds[])"); \
        status = (managerHandle)->getMotionSensorIds(&sensorIdsOpt); \
        size_t count = 0; \
        if (sensorIdsOpt && sensorIdsOpt->size()) count = sensorIdsOpt->size(); \
        UT_LOG_DEBUG("Result getMotionSensorIds(OUT:sensorIds[count:%zu]), status:[isOK:%s, exception:%s]", \
            count, \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define MOTIONSENSOR_GET_SENSOR_SUCCESS(managerHandle, sensorId, sensorHandle) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getMotionSensor(IN:sensorId[%d], OUT:sensorHandle[])", sensorId); \
        status = (managerHandle)->getMotionSensor(sensorId, &sensorHandle); \
        UT_LOG_DEBUG("Result getMotionSensor(IN:sensorId[%d], OUT:sensorHandle[%p]), status:[isOK:%s, exception:%s]", \
            sensorId, sensorHandle.get(), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_NOT_EQUAL(sensorHandle.get(), nullptr); \
    } while (0)

#define MOTIONSENSOR_GET_SENSOR_FAILURE(managerHandle, sensorId, sensorHandle, expectedException) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getMotionSensor(IN:sensorId[%d], OUT:sensorHandle[])", sensorId); \
        status = (managerHandle)->getMotionSensor(sensorId, &sensorHandle); \
        UT_LOG_DEBUG("Result getMotionSensor(IN:sensorId[%d], OUT:sensorHandle[%p]), status:[isOK:%s, exception:%s]", \
            sensorId, sensorHandle.get(), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
    } while (0)

/* ========================================================================== */
/*  IMotionSensor Macros                                                       */
/* ========================================================================== */

#define MOTIONSENSOR_GET_STATE_SUCCESS(sensorHandle, state, expectedState) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getState(OUT:state[])"); \
        status = (sensorHandle)->getState(&state); \
        UT_LOG_DEBUG("Result getState(OUT:state[%s]), status:[isOK:%s, exception:%s]", \
            ::com::rdk::hal::sensor::motion::toString(state).c_str(), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_EQUAL(state, expectedState); \
    } while (0)

#define MOTIONSENSOR_GET_STATE_FAILURE(sensorHandle, statePtr, expectedException) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getState(OUT:state[%p])", statePtr); \
        status = (sensorHandle)->getState(statePtr); \
        UT_LOG_DEBUG("Result getState(OUT:state[%p]), status:[isOK:%s, exception:%s]", \
            statePtr, \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
    } while (0)

#define MOTIONSENSOR_WAIT_FOR_STATE(sensorHandle, desiredState, lastState) \
    do { \
        int32_t sleep_time = 0; \
        do \
        { \
            this_thread::sleep_for(chrono::milliseconds(MAX_TRANSITORY_STATE_WAIT_MS)); \
            sleep_time += MAX_TRANSITORY_STATE_WAIT_MS; \
            MOTIONSENSOR_GET_STATE_SUCCESS((sensorHandle), lastState, desiredState); \
        } while (sleep_time < MAX_FINAL_STATE_WAIT_MS && lastState != desiredState); \
    } while (0)

#define MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, capabilities) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getCapabilities(OUT:capabilities[])"); \
        status = (sensorHandle)->getCapabilities(&capabilities); \
        UT_LOG_DEBUG("Result getCapabilities(OUT:capabilities[minSensitivity:%d, maxSensitivity:%d]), status:[isOK:%s, exception:%s]", \
            capabilities.minSensitivity, capabilities.maxSensitivity, \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define MOTIONSENSOR_GET_CAPABILITIES_FAILURE(sensorHandle, capabilitiesPtr, expectedException) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getCapabilities(OUT:capabilities[%p])", capabilitiesPtr); \
        status = (sensorHandle)->getCapabilities(capabilitiesPtr); \
        UT_LOG_DEBUG("Result getCapabilities(OUT:capabilities[%p]), status:[isOK:%s, exception:%s]", \
            capabilitiesPtr, \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.MOTIONSENSOR_REGISTER_EVENT_SUCCESS()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
    } while (0)

#define MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, controllerListener, controllerHandle) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling open(IN:controllerListener[%p], OUT:controllerHandle[])", controllerListener.get()); \
        status = (sensorHandle)->open(controllerListener, &controllerHandle); \
        UT_LOG_DEBUG("Result open(IN:controllerListener[%p], OUT:controllerHandle[%p]), status:[isOK:%s, exception:%s]", \
            controllerListener.get(), controllerHandle.get(), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_NOT_EQUAL_FATAL(controllerHandle.get(), nullptr); \
        UT_ASSERT_EQUAL_FATAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define MOTIONSENSOR_OPEN_FAILURE(sensorHandle, controllerListener, controllerHandle, expectedException) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling open(IN:controllerListener[%p], OUT:controllerHandle[])", (controllerListener ? controllerListener.get() : nullptr)); \
        status = (sensorHandle)->open(controllerListener, &controllerHandle); \
        UT_LOG_DEBUG("Result open(IN:controllerListener[%p], OUT:controllerHandle[%p]), status:[isOK:%s, exception:%s]", \
            (controllerListener ? controllerListener.get() : nullptr), controllerHandle.get(), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
        UT_ASSERT_EQUAL(controllerHandle.get(), nullptr); \
    } while (0)

#define MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling close(IN:controllerHandle[%p], OUT:_aidl_return[])", controllerHandle.get()); \
        status = (sensorHandle)->close(controllerHandle, &_aidl_return); \
        UT_LOG_DEBUG("Result close(IN:controllerHandle[%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            controllerHandle.get(), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_TRUE_FATAL(_aidl_return); \
        UT_ASSERT_EQUAL_FATAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define MOTIONSENSOR_CLOSE_FAILURE(sensorHandle, controllerHandle, expectedException) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling close(IN:controllerHandle[%p], OUT:_aidl_return[])", controllerHandle.get()); \
        status = (sensorHandle)->close(controllerHandle, &_aidl_return); \
        UT_LOG_DEBUG("Result close(IN:controllerHandle[%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            controllerHandle.get(), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(_aidl_return); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
    } while (0)

#define MOTIONSENSOR_REGISTER_EVENT_SUCCESS(sensorHandle, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling registerEventListener(IN:eventListener[%p], OUT:_aidl_return[])", eventListener.get()); \
        status = (sensorHandle)->registerEventListener(eventListener, &_aidl_return); \
        UT_LOG_DEBUG("Result registerEventListener(IN:eventListener[%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            eventListener.get(), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_TRUE(_aidl_return); \
    } while (0)

#define MOTIONSENSOR_REGISTER_EVENT_FAILURE(sensorHandle, eventListener, expectedException) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling registerEventListener(IN:eventListener[%p], OUT:_aidl_return[])", (eventListener ? eventListener.get() : nullptr)); \
        status = (sensorHandle)->registerEventListener(eventListener, &_aidl_return); \
        UT_LOG_DEBUG("Result registerEventListener(IN:eventListener[%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            (eventListener ? eventListener.get() : nullptr), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define MOTIONSENSOR_UNREGISTER_EVENT_SUCCESS(sensorHandle, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling unregisterEventListener(IN:eventListener[%p], OUT:_aidl_return[])", eventListener.get()); \
        status = (sensorHandle)->unregisterEventListener(eventListener, &_aidl_return); \
        UT_LOG_DEBUG("Result unregisterEventListener(IN:eventListener[%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            eventListener.get(), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define MOTIONSENSOR_UNREGISTER_EVENT_FAILURE(sensorHandle, eventListener, expectedException) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling unregisterEventListener(IN:eventListener[%p], OUT:_aidl_return[])", (eventListener ? eventListener.get() : nullptr)); \
        status = (sensorHandle)->unregisterEventListener(eventListener, &_aidl_return); \
        UT_LOG_DEBUG("Result unregisterEventListener(IN:eventListener[%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            (eventListener ? eventListener.get() : nullptr), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

/* ========================================================================== */
/*  IMotionSensorController Macros                                             */
/* ========================================================================== */

#define MOTIONSENSOR_START_SUCCESS(controllerHandle, config) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling start(IN:config[mode:%s, noMotionSeconds:%d])", \
            ::com::rdk::hal::sensor::motion::toString(config.operationalMode).c_str(), config.noMotionSeconds); \
        status = (controllerHandle)->start(config); \
        UT_LOG_DEBUG("Result start(), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define MOTIONSENSOR_START_FAILURE(controllerHandle, config, expectedException) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling start(IN:config[mode:%s, noMotionSeconds:%d])", \
            ::com::rdk::hal::sensor::motion::toString(config.operationalMode).c_str(), config.noMotionSeconds); \
        status = (controllerHandle)->start(config); \
        UT_LOG_DEBUG("Result start(), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
    } while (0)

#define MOTIONSENSOR_STOP_SUCCESS(controllerHandle) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling stop()"); \
        status = (controllerHandle)->stop(); \
        UT_LOG_DEBUG("Result stop(), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define MOTIONSENSOR_STOP_FAILURE(controllerHandle, expectedException) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling stop()"); \
        status = (controllerHandle)->stop(); \
        UT_LOG_DEBUG("Result stop(), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
    } while (0)

#define MOTIONSENSOR_GET_START_CONFIG_SUCCESS(controllerHandle, config, expectedConfig) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getStartConfig(OUT:config[])"); \
        status = (controllerHandle)->getStartConfig(&config); \
        UT_LOG_DEBUG("Result getStartConfig(OUT:config[mode:%s, noMotionSeconds:%d]), status:[isOK:%s, exception:%s]", \
            ::com::rdk::hal::sensor::motion::toString(config.operationalMode).c_str(), config.noMotionSeconds, \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_EQUAL(config.operationalMode, (expectedConfig).operationalMode); \
        UT_ASSERT_EQUAL(config.noMotionSeconds, (expectedConfig).noMotionSeconds); \
    } while (0)

#define MOTIONSENSOR_GET_START_CONFIG_FAILURE(controllerHandle, config, expectedException) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getStartConfig(OUT:config[]) expecting failure"); \
        status = (controllerHandle)->getStartConfig(&config); \
        UT_LOG_DEBUG("Result getStartConfig(), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
    } while (0)

#define MOTIONSENSOR_GET_SENSITIVITY_SUCCESS(controllerHandle, sensitivity) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getSensitivity(OUT:sensitivity[])"); \
        status = (controllerHandle)->getSensitivity(&sensitivity); \
        UT_LOG_DEBUG("Result getSensitivity(OUT:sensitivity[%d]), status:[isOK:%s, exception:%s]", \
            sensitivity, \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define MOTIONSENSOR_SET_SENSITIVITY_SUCCESS(controllerHandle, sensitivity) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling setSensitivity(IN:sensitivity[%d], OUT:_aidl_return[])", sensitivity); \
        status = (controllerHandle)->setSensitivity(sensitivity, &_aidl_return); \
        UT_LOG_DEBUG("Result setSensitivity(IN:sensitivity[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            sensitivity, \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define MOTIONSENSOR_SET_SENSITIVITY_FAILURE(controllerHandle, sensitivity, expectedException) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling setSensitivity(IN:sensitivity[%d], OUT:_aidl_return[])", sensitivity); \
        status = (controllerHandle)->setSensitivity(sensitivity, &_aidl_return); \
        UT_LOG_DEBUG("Result setSensitivity(IN:sensitivity[%d], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            sensitivity, \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define MOTIONSENSOR_SET_AUTONOMOUS_SUCCESS(controllerHandle, enabled) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling setAutonomousDuringDeepSleep(IN:enabled[%s], OUT:_aidl_return[])", \
            UT_Control_GetMapString(bool_mapTable_motionsensor, enabled)); \
        status = (controllerHandle)->setAutonomousDuringDeepSleep(enabled, &_aidl_return); \
        UT_LOG_DEBUG("Result setAutonomousDuringDeepSleep(IN:enabled[%s], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable_motionsensor, enabled), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define MOTIONSENSOR_SET_AUTONOMOUS_FAILURE(controllerHandle, enabled, expectedException) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling setAutonomousDuringDeepSleep(IN:enabled[%s], OUT:_aidl_return[])", \
            UT_Control_GetMapString(bool_mapTable_motionsensor, enabled)); \
        status = (controllerHandle)->setAutonomousDuringDeepSleep(enabled, &_aidl_return); \
        UT_LOG_DEBUG("Result setAutonomousDuringDeepSleep(IN:enabled[%s], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable_motionsensor, enabled), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
    } while (0)

#define MOTIONSENSOR_GET_AUTONOMOUS_SUCCESS(controllerHandle, enabled, supportsAutonomy) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling isAutonomousDuringDeepSleepEnabled(OUT:enabled[])"); \
        status = (controllerHandle)->isAutonomousDuringDeepSleepEnabled(&(enabled)); \
        UT_LOG_DEBUG("Result isAutonomousDuringDeepSleepEnabled(OUT:enabled[%s]), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable_motionsensor, enabled), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        if (supportsAutonomy) \
            UT_ASSERT_TRUE(enabled); \
        else \
            UT_ASSERT_FALSE(enabled); \
    } while (0)

#define MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, windows) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling setActiveWindows(IN:windows[count:%zu], OUT:_aidl_return[])", windows.size()); \
        status = (controllerHandle)->setActiveWindows(windows, &_aidl_return); \
        UT_LOG_DEBUG("Result setActiveWindows(IN:windows[count:%zu], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            windows.size(), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define MOTIONSENSOR_SET_ACTIVE_WINDOWS_FAILURE(controllerHandle, windows, expectedException) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling setActiveWindows(IN:windows[count:%zu], OUT:_aidl_return[])", windows.size()); \
        status = (controllerHandle)->setActiveWindows(windows, &_aidl_return); \
        UT_LOG_DEBUG("Result setActiveWindows(IN:windows[count:%zu], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            windows.size(), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define MOTIONSENSOR_GET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, windows) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getActiveWindows(OUT:windows[])"); \
        status = (controllerHandle)->getActiveWindows(&windows); \
        UT_LOG_DEBUG("Result getActiveWindows(OUT:windows[count:%zu]), status:[isOK:%s, exception:%s]", \
            windows.size(), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define MOTIONSENSOR_CLEAR_ACTIVE_WINDOWS_SUCCESS(controllerHandle) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling clearActiveWindows(OUT:_aidl_return[])"); \
        status = (controllerHandle)->clearActiveWindows(&_aidl_return); \
        UT_LOG_DEBUG("Result clearActiveWindows(OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define MOTIONSENSOR_CLEAR_ACTIVE_WINDOWS_FAILURE(controllerHandle, expectedException) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling clearActiveWindows(OUT:_aidl_return[])"); \
        status = (controllerHandle)->clearActiveWindows(&_aidl_return); \
        UT_LOG_DEBUG("Result clearActiveWindows(OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable_motionsensor, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_motionsensor, status.isOk()), \
            UT_Control_GetMapString(statusMotionSensorException_mapTable, status.exceptionCode())); \
        if (expectedException == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expectedException); \
    } while (0)

/* ========================================================================== */
/*  Listener Classes                                                           */
/* ========================================================================== */

class MotionSensorControllerListener : public BnMotionSensorControllerListener
{
public:
    MotionSensorControllerListener() {}
    virtual ~MotionSensorControllerListener() = default;

    android::binder::Status onStateChanged(
        ::com::rdk::hal::sensor::motion::State oldState,
        ::com::rdk::hal::sensor::motion::State newState) override;

    android::binder::Status onActiveWindowEntered() override;
    android::binder::Status onActiveWindowExited() override;

    bool getNextStateChange(pair<State, State>& stateChange);
    bool getNextStateChange(pair<State, State>& stateChange, chrono::milliseconds timeout);
    bool hasActiveWindowEntered();
    bool hasActiveWindowExited();
    void resetStateQueue();
    void resetActiveWindowFlags();

private:
    mutex mStateMutex;
    condition_variable mStateCv;
    mutex mWindowMutex;
    queue<pair<State, State>> mStateChanges;
    bool mActiveWindowEntered{false};
    bool mActiveWindowExited{false};
};

class MotionSensorEventListener : public BnMotionSensorEventListener
{
public:
    MotionSensorEventListener() {}
    virtual ~MotionSensorEventListener() = default;

    android::binder::Status onEvent(
        ::com::rdk::hal::sensor::motion::OperationalMode mode) override;

    bool getNextEvent(OperationalMode& outMode);
    bool getNextEvent(OperationalMode& outMode, chrono::milliseconds timeout);
    bool hasEvent();
    void resetEventQueue();

private:
    mutex mEventMutex;
    condition_variable mEventCv;
    queue<OperationalMode> mEventQueue;
};

#endif // __VTS_MOTIONSENSOR_H__

/** @} */ // End of MOTION_SENSOR_HALTEST_HEADER
/** @} */ // End of MOTION_SENSOR_HALTEST
/** @} */ // End of MOTION_SENSOR Module
/** @} */ // End of HPK
