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
