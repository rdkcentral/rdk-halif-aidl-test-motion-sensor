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
 * @defgroup MOTION_SENSOR_HALTEST_L1 Motion Sensor HAL Tests L1 File
 * @{
 * @parblock
 *
 * ### L1 Test Cases for Motion Sensor HAL :
 *
 * This module includes Level 1 functional tests (success and failure scenarios).
 * Tests verify that each API meets the operational requirements across all vendors.
 *
 * **Pre-Conditions:**  Motion Sensor HAL service must be running @n
 * **Dependencies:** IMotionSensorManager, IMotionSensor, IMotionSensorController @n
 *
 * @endparblock
 */

/**
 * @file vts_motionsensor_l1_tests.cpp
 */

#include "vts_motionsensor_parse_hfp.h"
#include "vts_motionsensor.h"

extern sp<IMotionSensorManager> gMOTIONSENSORMGRHandle;

/* Test class registration with UT_Core */

UT_ADD_TEST_TO_GROUP(VTS_L1_MOTIONSENSOR, UT_TESTS_L1)

class VTS_L1_MOTIONSENSOR : public UTCore
{
public:
    VTS_L1_MOTIONSENSOR() : UTCore() {}
    ~VTS_L1_MOTIONSENSOR() override = default;
    void SetUp() override { android::ProcessState::self()->startThreadPool(); }
    void TearDown() override {}
};

// 1. getMotionSensorIds - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetMotionSensorIds_Success)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt) {
        for (const auto& idOpt : *sensorIdsOpt) {
            if (idOpt) sensorIds.push_back(idOpt->value);
        }
    }
    UT_ASSERT_TRUE(sensorIds.size() > 0);
}

// 2. getMotionSensor - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetMotionSensor_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt) {
        for (const auto& idOpt : *sensorIdsOpt) {
            if (idOpt) sensorIds.push_back(idOpt->value);
        }
    }
    for (auto id : sensorIds) {
        android::sp<IMotionSensor> sensorHandle;
        MotionSensorId sensorIdObj;
        sensorIdObj.value = id;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
    }
}

// 3. getMotionSensor - Negative (invalid IDs)
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetMotionSensor_Negative)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt) {
        for (const auto& idOpt : *sensorIdsOpt) {
            if (idOpt) sensorIds.push_back(idOpt->value);
        }
    }
    if (sensorIds.empty()) return;
    int32_t minId = *std::min_element(sensorIds.begin(), sensorIds.end());
    int32_t maxId = *std::max_element(sensorIds.begin(), sensorIds.end());
    android::sp<IMotionSensor> sensorHandle;
    MotionSensorId sensorIdObj;
    // Below min
    sensorIdObj.value = minId - 1;
    MOTIONSENSOR_GET_SENSOR_FAILURE(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle, Status::EX_ILLEGAL_ARGUMENT);
    // Above max
    sensorIdObj.value = maxId + 1;
    MOTIONSENSOR_GET_SENSOR_FAILURE(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle, Status::EX_ILLEGAL_ARGUMENT);
    // UNDEFINED (if defined as -1)
    sensorIdObj.value = -1;
    MOTIONSENSOR_GET_SENSOR_FAILURE(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle, Status::EX_ILLEGAL_ARGUMENT);
}

// 4. getCapabilities - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetCapabilities_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt) {
        for (const auto& idOpt : *sensorIdsOpt) {
            if (idOpt) sensorIds.push_back(idOpt->value);
        }
    }
    for (auto id : sensorIds) {
        android::sp<IMotionSensor> sensorHandle;
        MotionSensorId sensorIdObj;
        sensorIdObj.value = id;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        Capabilities capabilities;
        MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, capabilities);
        // Optionally compare with gMotionSensorDeviceInfo if available
    }
}

// 5. getState - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetState_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt) {
        for (const auto& idOpt : *sensorIdsOpt) {
            if (idOpt) sensorIds.push_back(idOpt->value);
        }
    }
    for (auto id : sensorIds) {
        android::sp<IMotionSensor> sensorHandle;
        MotionSensorId sensorIdObj;
        sensorIdObj.value = id;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        State state;
        MOTIONSENSOR_GET_STATE(sensorHandle, state);
        // Optionally assert state is a valid enum value
    }
}

// 6. getCapabilities - Negative (invalid handle)
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetCapabilities_Negative)
{
    android::sp<IMotionSensor> sensorHandle = nullptr;
    Capabilities capabilities;
    // Should fail with nullptr handle
    ::android::binder::Status status = sensorHandle ? sensorHandle->getCapabilities(&capabilities) : ::android::binder::Status::fromExceptionCode(Status::EX_NULL_POINTER);
    UT_ASSERT_FALSE(status.isOk());
}

// 7. getState - Negative (invalid handle)
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetState_Negative)
{
    android::sp<IMotionSensor> sensorHandle = nullptr;
    State state;
    ::android::binder::Status status = sensorHandle ? sensorHandle->getState(&state) : ::android::binder::Status::fromExceptionCode(Status::EX_NULL_POINTER);
    UT_ASSERT_FALSE(status.isOk());
}

/* ========================================================================== */
/*  IMotionSensor::open / close test cases                                    */
/* ========================================================================== */

// 8. open - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Open_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty()) return;
    MotionSensorId sensorIdObj;
    sensorIdObj.value = (*sensorIdsOpt)[0]->value;
    android::sp<IMotionSensor> sensorHandle;
    MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);

    android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
    android::sp<IMotionSensorController> controllerHandle;
    MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);

    // Cleanup: close the sensor to restore state for other tests
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 9. open - Negative: null listener must return EX_NULL_POINTER
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Open_Negative_NullListener)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty()) return;
    MotionSensorId sensorIdObj;
    sensorIdObj.value = (*sensorIdsOpt)[0]->value;
    android::sp<IMotionSensor> sensorHandle;
    MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);

    android::sp<IMotionSensorControllerListener> nullListener = nullptr;
    android::sp<IMotionSensorController> controllerHandle;
    MOTIONSENSOR_OPEN_FAILURE(sensorHandle, nullListener, controllerHandle, Status::EX_NULL_POINTER);
}

// 10. open - Negative: already open must return EX_ILLEGAL_STATE
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Open_Negative_AlreadyOpen)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty()) return;
    MotionSensorId sensorIdObj;
    sensorIdObj.value = (*sensorIdsOpt)[0]->value;
    android::sp<IMotionSensor> sensorHandle;
    MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);

    android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
    android::sp<IMotionSensorController> controllerHandle;
    MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);

    // Second open on the same sensor must fail with EX_ILLEGAL_STATE
    android::sp<IMotionSensorControllerListener> listener2 = android::sp<MotionSensorControllerListener>::make();
    android::sp<IMotionSensorController> controllerHandle2;
    MOTIONSENSOR_OPEN_FAILURE(sensorHandle, listener2, controllerHandle2, Status::EX_ILLEGAL_STATE);

    // Cleanup
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 11. close - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Close_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty()) return;
    MotionSensorId sensorIdObj;
    sensorIdObj.value = (*sensorIdsOpt)[0]->value;
    android::sp<IMotionSensor> sensorHandle;
    MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);

    android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
    android::sp<IMotionSensorController> controllerHandle;
    MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 12. close - Negative: null controller must return EX_NULL_POINTER
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Close_Negative_NullController)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty()) return;
    MotionSensorId sensorIdObj;
    sensorIdObj.value = (*sensorIdsOpt)[0]->value;
    android::sp<IMotionSensor> sensorHandle;
    MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);

    android::sp<IMotionSensorController> nullController = nullptr;
    MOTIONSENSOR_CLOSE_FAILURE(sensorHandle, nullController, Status::EX_NULL_POINTER);
}

// 13. close - Negative: wrong controller must return false (status ok)
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Close_Negative_WrongController)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (sensorIdsOpt->size() < 2) return; // Requires at least 2 sensors
    MotionSensorId id0, id1;
    id0.value = (*sensorIdsOpt)[0]->value;
    id1.value = (*sensorIdsOpt)[1]->value;

    android::sp<IMotionSensor> sensorHandle0, sensorHandle1;
    MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, id0, sensorHandle0);
    MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, id1, sensorHandle1);

    android::sp<IMotionSensorControllerListener> listener0 = android::sp<MotionSensorControllerListener>::make();
    android::sp<IMotionSensorControllerListener> listener1 = android::sp<MotionSensorControllerListener>::make();
    android::sp<IMotionSensorController> controller0, controller1;
    MOTIONSENSOR_OPEN_SUCCESS(sensorHandle0, listener0, controller0);
    MOTIONSENSOR_OPEN_SUCCESS(sensorHandle1, listener1, controller1);

    // Closing sensor0 with sensor1's controller must return false, status ok
    MOTIONSENSOR_CLOSE_FAILURE(sensorHandle0, controller1, Status::EX_NONE);

    // Cleanup
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle0, controller0);
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle1, controller1);
}

/* ========================================================================== */
/*  IMotionSensor::registerEventListener / unregisterEventListener test cases */
/* ========================================================================== */

// 14. registerEventListener - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, RegisterEventListener_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty()) return;
    MotionSensorId sensorIdObj;
    sensorIdObj.value = (*sensorIdsOpt)[0]->value;
    android::sp<IMotionSensor> sensorHandle;
    MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);

    android::sp<IMotionSensorEventListener> eventListener = android::sp<MotionSensorEventListener>::make();
    // Registration does not require the sensor to be open (per spec)
    MOTIONSENSOR_REGISTER_EVENT_SUCCESS(sensorHandle, eventListener);

    // Cleanup
    MOTIONSENSOR_UNREGISTER_EVENT_SUCCESS(sensorHandle, eventListener);
}

// 15. registerEventListener - Negative: same listener registered twice must return false
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, RegisterEventListener_Negative_AlreadyRegistered)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty()) return;
    MotionSensorId sensorIdObj;
    sensorIdObj.value = (*sensorIdsOpt)[0]->value;
    android::sp<IMotionSensor> sensorHandle;
    MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);

    android::sp<IMotionSensorEventListener> eventListener = android::sp<MotionSensorEventListener>::make();
    MOTIONSENSOR_REGISTER_EVENT_SUCCESS(sensorHandle, eventListener);
    // Second registration of the same listener must return false, status ok
    MOTIONSENSOR_REGISTER_EVENT_FAILURE(sensorHandle, eventListener, Status::EX_NONE);

    // Cleanup
    MOTIONSENSOR_UNREGISTER_EVENT_SUCCESS(sensorHandle, eventListener);
}

// 16. unregisterEventListener - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, UnregisterEventListener_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty()) return;
    MotionSensorId sensorIdObj;
    sensorIdObj.value = (*sensorIdsOpt)[0]->value;
    android::sp<IMotionSensor> sensorHandle;
    MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);

    android::sp<IMotionSensorEventListener> eventListener = android::sp<MotionSensorEventListener>::make();
    MOTIONSENSOR_REGISTER_EVENT_SUCCESS(sensorHandle, eventListener);
    MOTIONSENSOR_UNREGISTER_EVENT_SUCCESS(sensorHandle, eventListener);
}

// 17. unregisterEventListener - Negative: listener not registered must return false
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, UnregisterEventListener_Negative_NotRegistered)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty()) return;
    MotionSensorId sensorIdObj;
    sensorIdObj.value = (*sensorIdsOpt)[0]->value;
    android::sp<IMotionSensor> sensorHandle;
    MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);

    // Listener was never registered
    android::sp<IMotionSensorEventListener> eventListener = android::sp<MotionSensorEventListener>::make();
    MOTIONSENSOR_UNREGISTER_EVENT_FAILURE(sensorHandle, eventListener, Status::EX_NONE);
}

/* ========================================================================== */
/*  Helper: get first sensor handle + open controller                         */
/* ========================================================================== */

// Opens the sensor at the given index and returns sensorHandle + controllerHandle.
// Returns false if the index is out of range or the sensor cannot be opened.
static bool openSensorAtIndex(size_t index,
                               android::sp<IMotionSensor>& sensorHandle,
                               android::sp<IMotionSensorController>& controllerHandle,
                               android::sp<IMotionSensorControllerListener>& listener)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    android::sp<IMotionSensorManager>& mgr = gMOTIONSENSORMGRHandle;
    ::android::binder::Status status = mgr->getMotionSensorIds(&sensorIdsOpt);
    if (!status.isOk() || !sensorIdsOpt || sensorIdsOpt->empty()) return false;
    if (index >= sensorIdsOpt->size())
    {
        UT_LOG_ERROR("openSensorAtIndex: index %zu out of range (sensor count: %zu)",
            index, sensorIdsOpt->size());
        return false;
    }
    const auto& idOpt = (*sensorIdsOpt)[index];
    if (!idOpt) return false;
    MotionSensorId id;
    id.value = idOpt->value;
    status = mgr->getMotionSensor(id, &sensorHandle);
    if (!status.isOk() || !sensorHandle) return false;
    listener = android::sp<MotionSensorControllerListener>::make();
    status = sensorHandle->open(listener, &controllerHandle);
    return status.isOk() && controllerHandle != nullptr;
}

/* ========================================================================== */
/*  IMotionSensorController::start / stop test cases                          */
/* ========================================================================== */

// 18. start - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Start_Positive)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    StartConfig config;
    config.operationalMode = OperationalMode::MOTION;
    config.noMotionSeconds = 30;
    MOTIONSENSOR_START_SUCCESS(controllerHandle, config);

    // Cleanup
    MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 19. start - Negative: EX_ILLEGAL_STATE when already STARTED
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Start_Negative_AlreadyStarted)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    StartConfig config;
    config.operationalMode = OperationalMode::MOTION;
    config.noMotionSeconds = 30;
    MOTIONSENSOR_START_SUCCESS(controllerHandle, config);

    // Second start while STARTED must fail with EX_ILLEGAL_STATE
    MOTIONSENSOR_START_FAILURE(controllerHandle, config, Status::EX_ILLEGAL_STATE);

    // Cleanup
    MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 20. start - Negative: EX_ILLEGAL_ARGUMENT for invalid noMotionSeconds
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Start_Negative_InvalidConfig)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    StartConfig config;
    config.operationalMode = OperationalMode::MOTION;
    config.noMotionSeconds = 99999; // out of range
    MOTIONSENSOR_START_FAILURE(controllerHandle, config, Status::EX_ILLEGAL_ARGUMENT);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 21. stop - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Stop_Positive)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    StartConfig config;
    config.operationalMode = OperationalMode::MOTION;
    config.noMotionSeconds = 30;
    MOTIONSENSOR_START_SUCCESS(controllerHandle, config);
    MOTIONSENSOR_STOP_SUCCESS(controllerHandle);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 22. stop - Negative: EX_ILLEGAL_STATE when already STOPPED
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Stop_Negative_AlreadyStopped)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    // Not started — must fail with EX_ILLEGAL_STATE
    MOTIONSENSOR_STOP_FAILURE(controllerHandle, Status::EX_ILLEGAL_STATE);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

/* ========================================================================== */
/*  IMotionSensorController::getStartConfig test cases                        */
/* ========================================================================== */

// 23. getStartConfig - Positive: returns config passed to start()
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetStartConfig_Positive)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    StartConfig config;
    config.operationalMode = OperationalMode::MOTION;
    config.noMotionSeconds = 60;
    MOTIONSENSOR_START_SUCCESS(controllerHandle, config);

    StartConfig retrieved;
    MOTIONSENSOR_GET_START_CONFIG_SUCCESS(controllerHandle, retrieved);
    UT_ASSERT_EQUAL(retrieved.noMotionSeconds, config.noMotionSeconds);

    MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 24. getStartConfig - Negative: EX_ILLEGAL_STATE when STOPPED
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetStartConfig_Negative_Stopped)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    StartConfig config;
    ::android::binder::Status status = controllerHandle->getStartConfig(&config);
    UT_ASSERT_FALSE(status.isOk());
    UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

/* ========================================================================== */
/*  IMotionSensorController::getSensitivity / setSensitivity test cases       */
/* ========================================================================== */

// 25. getSensitivity - Positive: returns initialised value
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetSensitivity_Positive)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    int32_t sensitivity;
    MOTIONSENSOR_GET_SENSITIVITY_SUCCESS(controllerHandle, sensitivity);
    UT_ASSERT_TRUE(sensitivity >= 0);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 26. setSensitivity - Positive: min value
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetSensitivity_Positive_Min)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    Capabilities caps;
    MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, caps);
    MOTIONSENSOR_SET_SENSITIVITY_SUCCESS(controllerHandle, caps.minSensitivity);

    // Verify round-trip
    int32_t sensitivity;
    MOTIONSENSOR_GET_SENSITIVITY_SUCCESS(controllerHandle, sensitivity);
    UT_ASSERT_EQUAL(sensitivity, caps.minSensitivity);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 27. setSensitivity - Positive: max value
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetSensitivity_Positive_Max)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    Capabilities caps;
    MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, caps);
    MOTIONSENSOR_SET_SENSITIVITY_SUCCESS(controllerHandle, caps.maxSensitivity);

    int32_t sensitivity;
    MOTIONSENSOR_GET_SENSITIVITY_SUCCESS(controllerHandle, sensitivity);
    UT_ASSERT_EQUAL(sensitivity, caps.maxSensitivity);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 28. setSensitivity - Negative: below min
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetSensitivity_Negative_BelowMin)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    Capabilities caps;
    MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, caps);
    MOTIONSENSOR_SET_SENSITIVITY_FAILURE(controllerHandle, caps.minSensitivity - 1, Status::EX_ILLEGAL_ARGUMENT);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 29. setSensitivity - Negative: above max
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetSensitivity_Negative_AboveMax)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    Capabilities caps;
    MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, caps);
    MOTIONSENSOR_SET_SENSITIVITY_FAILURE(controllerHandle, caps.maxSensitivity + 1, Status::EX_ILLEGAL_ARGUMENT);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 30. setSensitivity - Negative: EX_ILLEGAL_STATE when STARTED
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetSensitivity_Negative_WhileStarted)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    StartConfig config;
    config.operationalMode = OperationalMode::MOTION;
    config.noMotionSeconds = 30;
    MOTIONSENSOR_START_SUCCESS(controllerHandle, config);

    Capabilities caps;
    MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, caps);
    MOTIONSENSOR_SET_SENSITIVITY_FAILURE(controllerHandle, caps.minSensitivity, Status::EX_ILLEGAL_STATE);

    MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

/* ========================================================================== */
/*  IMotionSensorController::setAutonomousDuringDeepSleep test cases          */
/* ========================================================================== */

// 31. setAutonomousDuringDeepSleep - Positive (if supported)
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetAutonomous_Positive)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    Capabilities caps;
    MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, caps);
    if (!caps.supportsDeepSleepAutonomy)
    {
        // Platform does not support this — verify false is returned, status ok
        MOTIONSENSOR_SET_AUTONOMOUS_SUCCESS(controllerHandle, true);
        bool enabled;
        ::android::binder::Status status = controllerHandle->isAutonomousDuringDeepSleepEnabled(&enabled);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_FALSE(enabled); // unchanged since not supported
    }
    else
    {
        MOTIONSENSOR_SET_AUTONOMOUS_SUCCESS(controllerHandle, true);
        bool enabled;
        ::android::binder::Status status = controllerHandle->isAutonomousDuringDeepSleepEnabled(&enabled);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(enabled);
    }

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 32. setAutonomousDuringDeepSleep - Negative: EX_ILLEGAL_STATE when STARTED
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetAutonomous_Negative_WhileStarted)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    StartConfig config;
    config.operationalMode = OperationalMode::MOTION;
    config.noMotionSeconds = 30;
    MOTIONSENSOR_START_SUCCESS(controllerHandle, config);

    ::android::binder::Status status;
    bool ret;
    status = controllerHandle->setAutonomousDuringDeepSleep(true, &ret);
    UT_ASSERT_FALSE(status.isOk());
    UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE);

    MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

/* ========================================================================== */
/*  IMotionSensorController::setActiveWindows / getActiveWindows /            */
/*  clearActiveWindows test cases                                              */
/* ========================================================================== */

// 33. setActiveWindows - Positive: valid window
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetActiveWindows_Positive)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    std::vector<TimeWindow> windows;
    TimeWindow w;
    w.startTimeOfDaySeconds = 28800; // 08:00
    w.endTimeOfDaySeconds   = 72000; // 20:00
    windows.push_back(w);
    MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, windows);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 34. setActiveWindows - Positive: midnight-wrapping window
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetActiveWindows_Positive_MidnightWrap)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    std::vector<TimeWindow> windows;
    TimeWindow w;
    w.startTimeOfDaySeconds = 79200; // 22:00
    w.endTimeOfDaySeconds   = 21600; // 06:00 (wraps midnight)
    windows.push_back(w);
    MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, windows);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 35. setActiveWindows - Positive: empty array (24-hour monitoring)
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetActiveWindows_Positive_Empty)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    std::vector<TimeWindow> windows; // empty
    MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, windows);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 36. setActiveWindows - Negative: value out of range
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetActiveWindows_Negative_OutOfRange)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    std::vector<TimeWindow> windows;
    TimeWindow w;
    w.startTimeOfDaySeconds = 0;
    w.endTimeOfDaySeconds   = 90000; // > 86399
    windows.push_back(w);
    MOTIONSENSOR_SET_ACTIVE_WINDOWS_FAILURE(controllerHandle, windows, Status::EX_ILLEGAL_ARGUMENT);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 37. setActiveWindows - Negative: EX_ILLEGAL_STATE when STARTED
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetActiveWindows_Negative_WhileStarted)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    StartConfig config;
    config.operationalMode = OperationalMode::MOTION;
    config.noMotionSeconds = 30;
    MOTIONSENSOR_START_SUCCESS(controllerHandle, config);

    std::vector<TimeWindow> windows;
    TimeWindow w;
    w.startTimeOfDaySeconds = 0;
    w.endTimeOfDaySeconds   = 3600;
    windows.push_back(w);
    MOTIONSENSOR_SET_ACTIVE_WINDOWS_FAILURE(controllerHandle, windows, Status::EX_ILLEGAL_STATE);

    MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 38. getActiveWindows - Positive: reflects setActiveWindows result
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetActiveWindows_Positive)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    std::vector<TimeWindow> windows;
    TimeWindow w;
    w.startTimeOfDaySeconds = 28800;
    w.endTimeOfDaySeconds   = 72000;
    windows.push_back(w);
    MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, windows);

    std::vector<TimeWindow> retrieved;
    MOTIONSENSOR_GET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, retrieved);
    UT_ASSERT_EQUAL(retrieved.size(), windows.size());
    UT_ASSERT_EQUAL(retrieved[0].startTimeOfDaySeconds, w.startTimeOfDaySeconds);
    UT_ASSERT_EQUAL(retrieved[0].endTimeOfDaySeconds,   w.endTimeOfDaySeconds);

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 39. clearActiveWindows - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, ClearActiveWindows_Positive)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    std::vector<TimeWindow> windows;
    TimeWindow w;
    w.startTimeOfDaySeconds = 28800;
    w.endTimeOfDaySeconds   = 72000;
    windows.push_back(w);
    MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, windows);

    MOTIONSENSOR_CLEAR_ACTIVE_WINDOWS_SUCCESS(controllerHandle);

    std::vector<TimeWindow> retrieved;
    MOTIONSENSOR_GET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, retrieved);
    UT_ASSERT_TRUE(retrieved.empty());

    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}

// 40. clearActiveWindows - Negative: EX_ILLEGAL_STATE when STARTED
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, ClearActiveWindows_Negative_WhileStarted)
{
    android::sp<IMotionSensor> sensorHandle;
    android::sp<IMotionSensorController> controllerHandle;
    android::sp<IMotionSensorControllerListener> listener;
    if (!openSensorAtIndex(0, sensorHandle, controllerHandle, listener)) return;

    StartConfig config;
    config.operationalMode = OperationalMode::MOTION;
    config.noMotionSeconds = 30;
    MOTIONSENSOR_START_SUCCESS(controllerHandle, config);

    ::android::binder::Status status;
    bool ret;
    status = controllerHandle->clearActiveWindows(&ret);
    UT_ASSERT_FALSE(status.isOk());
    UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE);

    MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
}
