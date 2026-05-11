/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**
 * @addtogroup HPK Hardware Porting Kit
 * @{
 *
 */

/**
 * @addtogroup MOTION_SENSOR Motion Sensor Module
 * @{
 *
 */

/**
 * @defgroup MOTION_SENSOR_HALTEST Motion Sensor HAL Tests
 * @{
 *
 */

/**
 * @defgroup MOTION_SENSOR_HALTEST_L2 Motion Sensor HAL Tests L2 File
 * @{
 * @parblock
 *
 * ### L2 Test Cases for Motion Sensor HAL :
 *
 * This module includes Level 2 functional tests (success and failure scenarios).
 * This is to ensure that the API meets the operational requirements of the module across all vendors.
 *
 * **Pre-Conditions:** @n
 * **Dependencies:** None@n
 *
 * @endparblock
 *
 */

/**
 * @file vts_motionsensor_l2_tests.cpp
 *
 */
#include <optional>
#include <vector>

#include "vts_motionsensor.h"
#include "vts_motionsensor_parse_hfp.h"

extern sp<IMotionSensorManager> gMOTIONSENSORMGRHandle;

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L2_MOTIONSENSOR, UT_TESTS_L2)

class VTS_L2_MOTIONSENSOR : public UTCore
{
public:
    VTS_L2_MOTIONSENSOR() : UTCore() {}

    ~VTS_L2_MOTIONSENSOR() override = default;

    void SetUp() override
    {
        android::ProcessState::self()->startThreadPool();
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Validate Motion Sensor lifecycle state transitions.
 *
 * This test verifies the state machine behavior of the Motion Sensor interface by exercising
 * all major state transitions: STOPPED → STARTING → STARTED → STOPPING → STOPPED.
 * It ensures that each transition occurs as expected and that the controller listener receives
 * the correct onStateChanged notifications for each step. It also verifies that getStartConfig()
 * returns the configured operational mode while in STARTED state.
 *
 * **Test Group ID:** Basic: 02@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**@n
 * - Motion Sensor HAL service must be running@n
 * - Sensors must be available in State::STOPPED initially@n
 *
 * **Dependencies:**@n
 * - A valid gMOTIONSENSORMGRHandle must be available@n
 *
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * Refer to UT specification documentation [motion_sensor_High_Level_TestSpecification.md](../docs/pages/motion_sensor_High_Level_TestSpecification.md)
 *
 */
UT_ADD_TEST(VTS_L2_MOTIONSENSOR, StateTransitions)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    pair<State, State> stateChange;

    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    UT_ASSERT_TRUE(sensorIdsOpt && !sensorIdsOpt->empty());

    for (const auto& idOpt : *sensorIdsOpt)
    {
        UT_ASSERT_TRUE(idOpt.has_value());

        android::sp<IMotionSensor> sensorHandle;
        android::sp<MotionSensorControllerListener> controllerListener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        State state;

        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);

        // Check initial state is STOPPED
        MOTIONSENSOR_GET_STATE_SUCCESS(sensorHandle, state, State::STOPPED);

        // Open the sensor
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, controllerListener, controllerHandle);

        // Start the sensor in MOTION mode
        StartConfig config;
        config.operationalMode = OperationalMode::MOTION;
        config.noMotionSeconds = 30;
        MOTIONSENSOR_START_SUCCESS(controllerHandle, config);

        // Wait for STARTED state before checking callbacks
        MOTIONSENSOR_WAIT_FOR_STATE(sensorHandle, State::STARTED, state);
        UT_ASSERT_EQUAL(state, State::STARTED);

        // Verify onStateChanged for STOPPED → STARTING
        UT_ASSERT_TRUE(controllerListener->getNextStateChange(stateChange, chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS)));
        UT_ASSERT_EQUAL(stateChange.first, State::STOPPED);
        UT_ASSERT_EQUAL(stateChange.second, State::STARTING);

        // Verify onStateChanged for STARTING → STARTED
        UT_ASSERT_TRUE(controllerListener->getNextStateChange(stateChange, chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS)));
        UT_ASSERT_EQUAL(stateChange.first, State::STARTING);
        UT_ASSERT_EQUAL(stateChange.second, State::STARTED);

        // Verify getStartConfig() returns the configured operational mode while STARTED
        StartConfig retrievedConfig;
        MOTIONSENSOR_GET_START_CONFIG_SUCCESS(controllerHandle, retrievedConfig, config);

        // Stop the sensor
        MOTIONSENSOR_STOP_SUCCESS(controllerHandle);

        // Wait for STOPPED state before checking callbacks
        MOTIONSENSOR_WAIT_FOR_STATE(sensorHandle, State::STOPPED, state);
        UT_ASSERT_EQUAL(state, State::STOPPED);

        // Verify onStateChanged for STARTED → STOPPING
        UT_ASSERT_TRUE(controllerListener->getNextStateChange(stateChange, chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS)));
        UT_ASSERT_EQUAL(stateChange.first, State::STARTED);
        UT_ASSERT_EQUAL(stateChange.second, State::STOPPING);

        // Verify onStateChanged for STOPPING → STOPPED
        UT_ASSERT_TRUE(controllerListener->getNextStateChange(stateChange, chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS)));
        UT_ASSERT_EQUAL(stateChange.first, State::STOPPING);
        UT_ASSERT_EQUAL(stateChange.second, State::STOPPED);

        // Close the sensor
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Verify configuration methods are accepted only in STOPPED state.
 *
 * This test validates that configuration APIs (setSensitivity, setActiveWindows,
 * setAutonomousDuringDeepSleep) are rejected when the sensor is in STARTED state
 * and are accepted when the sensor is in STOPPED state. This ensures the HAL enforces
 * the correct state guard on all configuration entry points.
 *
 * **Test Group ID:** Basic: 02@n
 * **Test Case ID:** 002@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**@n
 * - Motion Sensor HAL service must be running@n
 * - Sensors must be available in State::STOPPED initially@n
 *
 * **Dependencies:**@n
 * - A valid gMOTIONSENSORMGRHandle must be available@n
 *
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * Refer to UT specification documentation [motion_sensor_High_Level_TestSpecification.md](../docs/pages/motion_sensor_High_Level_TestSpecification.md)
 *
 */
UT_ADD_TEST(VTS_L2_MOTIONSENSOR, ConfigurationMethodsRequireStopped)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;

    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    UT_ASSERT_TRUE(sensorIdsOpt && !sensorIdsOpt->empty());

    for (const auto& idOpt : *sensorIdsOpt)
    {
        UT_ASSERT_TRUE(idOpt.has_value());

        android::sp<IMotionSensor> sensorHandle;
        android::sp<MotionSensorControllerListener> controllerListener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;

        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, controllerListener, controllerHandle);

        // Retrieve capabilities to get valid sensitivity range
        Capabilities caps;
        MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, caps);

        // ----------------------------------------------------------------
        // Phase 1: verify all config APIs are rejected while STARTED
        // ----------------------------------------------------------------
        StartConfig config;
        config.operationalMode = OperationalMode::MOTION;
        config.noMotionSeconds = 30;
        MOTIONSENSOR_START_SUCCESS(controllerHandle, config);

        // Wait for STARTED state before exercising config APIs
        State phaseState;
        MOTIONSENSOR_WAIT_FOR_STATE(sensorHandle, State::STARTED, phaseState);
        UT_ASSERT_EQUAL(phaseState, State::STARTED);

        // setSensitivity must fail with EX_ILLEGAL_STATE
        MOTIONSENSOR_SET_SENSITIVITY_FAILURE(controllerHandle, caps.minSensitivity,
                                             Status::EX_ILLEGAL_STATE);

        // setActiveWindows must fail with EX_ILLEGAL_STATE
        std::vector<TimeWindow> windows;
        TimeWindow w;
        w.startTimeOfDaySeconds = 28800; // 08:00
        w.endTimeOfDaySeconds   = 72000; // 20:00
        windows.push_back(w);
        MOTIONSENSOR_SET_ACTIVE_WINDOWS_FAILURE(controllerHandle, windows,
                                                Status::EX_ILLEGAL_STATE);

        // setAutonomousDuringDeepSleep must fail with EX_ILLEGAL_STATE
        ::android::binder::Status status;
        bool retVal;
        status = controllerHandle->setAutonomousDuringDeepSleep(true, &retVal);
        UT_ASSERT_FALSE(status.isOk());
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_ILLEGAL_STATE);

        // Stop to return to STOPPED
        MOTIONSENSOR_STOP_SUCCESS(controllerHandle);

        // Wait for STOPPED state before exercising config APIs
        MOTIONSENSOR_WAIT_FOR_STATE(sensorHandle, State::STOPPED, phaseState);
        UT_ASSERT_EQUAL(phaseState, State::STOPPED);

        // ----------------------------------------------------------------
        // Phase 2: verify all config APIs succeed in STOPPED state
        // ----------------------------------------------------------------

        // setSensitivity must succeed
        MOTIONSENSOR_SET_SENSITIVITY_SUCCESS(controllerHandle, caps.minSensitivity);

        // Verify getSensitivity reflects the new value
        int32_t sensitivity;
        MOTIONSENSOR_GET_SENSITIVITY_SUCCESS(controllerHandle, sensitivity);
        UT_ASSERT_EQUAL(sensitivity, caps.minSensitivity);

        // setActiveWindows must succeed
        MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, windows);

        // Verify getActiveWindows reflects what was set
        std::vector<TimeWindow> retrieved;
        MOTIONSENSOR_GET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, retrieved);
        UT_ASSERT_EQUAL(retrieved.size(), windows.size());
        UT_ASSERT_EQUAL(retrieved[0].startTimeOfDaySeconds, w.startTimeOfDaySeconds);
        UT_ASSERT_EQUAL(retrieved[0].endTimeOfDaySeconds,   w.endTimeOfDaySeconds);

        // clearActiveWindows must succeed and result in an empty window list
        MOTIONSENSOR_CLEAR_ACTIVE_WINDOWS_SUCCESS(controllerHandle);

        std::vector<TimeWindow> clearedWindows;
        MOTIONSENSOR_GET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, clearedWindows);
        UT_ASSERT_TRUE(clearedWindows.empty());

        // setAutonomousDuringDeepSleep must succeed
        MOTIONSENSOR_SET_AUTONOMOUS_SUCCESS(controllerHandle, true);

        // isAutonomousDuringDeepSleepEnabled must reflect the value set above
        bool autonomousEnabled = false;
        MOTIONSENSOR_GET_AUTONOMOUS_SUCCESS(controllerHandle, autonomousEnabled, caps.supportsDeepSleepAutonomy);

        // Disable and re-verify
        MOTIONSENSOR_SET_AUTONOMOUS_SUCCESS(controllerHandle, false);
        MOTIONSENSOR_GET_AUTONOMOUS_SUCCESS(controllerHandle, autonomousEnabled, false);

        // Cleanup
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/** @} */ // End of MOTION_SENSOR_HALTEST_L2
/** @} */ // End of MOTION_SENSOR_HALTEST
/** @} */ // End of MOTION_SENSOR Module
/** @} */ // End of HPK
