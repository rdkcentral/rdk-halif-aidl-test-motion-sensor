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
 * @defgroup MOTION_SENSOR_HALTEST_L3 Motion Sensor HAL Tests L3 File
 * @{
 * @parblock
 *
 * ### L3 Test Cases for MOTION SENSOR HAL :
 *
 * This module includes Level 3 functional tests that require external stimulus
 * from the host via the UT control plane.  The host sends YAML commands to the
 * vcomponent which injects synthetic motion/no-motion events; these tests then
 * validate event delivery, timing, active-window gating, and deep-sleep
 * autonomy.
 *
 * **Pre-Conditions:**  vcomponent must be running with control-plane enabled @n
 * **Dependencies:** UT-control plane @n
 *
 * @endparblock
 */

/**
 * @file vts_motionsensor_l3_tests.cpp
 */

#include <optional>
#include <tuple>

#include "vts_motionsensor.h"
#include "vts_motionsensor_parse_hfp.h"
#include "vts_motionsensor_parse_testconfig.h"

extern android::sp<IMotionSensorManager> gMOTIONSENSORMGRHandle;

static const char *gModuleName  = "motionsensor";
static const char *gTestGroup   = "L3";

/* ---- UT_Core suite registration ----------------------------------------- */
UT_ADD_TEST_TO_GROUP(VTS_L3_MOTIONSENSOR, UT_TESTS_L3)

/* ========================================================================== */
/*  Test fixture                                                               */
/* ========================================================================== */

class VTS_L3_MOTIONSENSOR : public UTCore
{
public:
    VTS_L3_MOTIONSENSOR() : UTCore() {}
    ~VTS_L3_MOTIONSENSOR() override = default;

    /* Per-sensor tuple: (id, sensorHandle, controllerListener, eventListener, controllerHandle) */
    using SensorTuple = tuple<
        MotionSensorId,
        sp<IMotionSensor>,
        sp<MotionSensorControllerListener>,
        sp<MotionSensorEventListener>,
        sp<IMotionSensorController>
    >;
    vector<SensorTuple> mSensorList;

    void SetUp() override
    {
        android::ProcessState::self()->startThreadPool();

        std::optional<std::vector<std::optional<MotionSensorId>>> sensorIds;
        MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIds);

        for (const auto& idOpt : *sensorIds)
        {
            if (!idOpt) continue;
            const MotionSensorId& id = *idOpt;

            sp<IMotionSensor> sensor;
            MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, id, sensor);

            sp<MotionSensorControllerListener> ctrlListener = new MotionSensorControllerListener();
            sp<MotionSensorEventListener>      evtListener  = new MotionSensorEventListener();
            sp<IMotionSensorController>        controller;

            MOTIONSENSOR_REGISTER_EVENT_SUCCESS(sensor, evtListener);
            MOTIONSENSOR_OPEN_SUCCESS(sensor, ctrlListener, controller);

            mSensorList.push_back({id, sensor, ctrlListener, evtListener, controller});
        }
    }

    void TearDown() override
    {
        for (auto& tup : mSensorList)
        {
            auto& sensor     = std::get<1>(tup);
            auto& evtListener= std::get<3>(tup);
            auto& controller = std::get<4>(tup);

            /* Ensure sensor is stopped before close */
            State state;
            MOTIONSENSOR_GET_STATE_SUCCESS(sensor, state);
            if (state == State::STARTED)
            {
                MOTIONSENSOR_STOP_SUCCESS(controller);
                State lastState;
                MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STOPPED, lastState);
            }

            MOTIONSENSOR_CLOSE_SUCCESS(sensor, controller);
            MOTIONSENSOR_UNREGISTER_EVENT_SUCCESS(sensor, evtListener);
        }
        mSensorList.clear();
    }

    /* Helper: get the SensorTuple for a given 0-based index */
    bool getSensorAtIndex(int32_t index, SensorTuple*& out)
    {
        if (index < 0 || (size_t)index >= mSensorList.size())
            return false;
        out = &mSensorList[index];
        return true;
    }
};

/* ========================================================================== */
/*  Test 1 — Motion Event, Timing, Accuracy & Listener Validation             */
/* ========================================================================== */

/**
 * @brief Validate MOTION mode event delivery for a sensor in MOTION mode.
 *
 * The sensor index (sensor_index) and number of motion events to receive
 * (num_events) are driven by the Python host test, which writes them
 * into the YAML config before uploading it to the target.  Python is also
 * responsible for triggering exactly num_events MOTION events via the
 * UT control plane — one per iteration of the receive loop below.
 *
 * Steps:
 * 1. Parse test config: sensor_index, num_events (N), timeouts.
 * 2. Verify no spurious/stale events exist before the test begins.
 * 3. Confirm no events arrive before listener registration (listener-gate).
 * 4. Re-register listener; start sensor in MOTION mode; wait for STARTED.
 * 5. Loop N times — for each iteration:
 *    a. Block until the next MOTION event arrives (within eventTimeoutMs).
 *    b. Verify event type == OperationalMode::MOTION.
 * 6. Stop sensor; verify no spurious events after stop.
 * 7. Unregister listener; verify no events after unregistration.
 *
 * **Test Group ID:** Basic: 03@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 *
 * **Test Procedure:**@n
 * Refer to [motion_sensor_High_Level_TestSpecification.md](../docs/pages/motion_sensor_High_Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L3_MOTIONSENSOR, MotionEventDetection)
{
    const char *testName = "MotionEventDetection";
    motionSensorL3EventTestConfig_t cfg;

    UT_ASSERT_EQUAL_FATAL(
        vts_motionsensor_parse_eventtest_config(
            VTS_MOTIONSENSOR_TEST_CONFIG_PATH, gModuleName, gTestGroup, testName, cfg),
        UT_STATUS_OK);

    SensorTuple *tup = nullptr;
    UT_ASSERT_TRUE_FATAL(getSensorAtIndex(cfg.sensorIndex, tup));

    auto& sensor     = std::get<1>(*tup);
    auto& ctrlLsn    = std::get<2>(*tup);
    auto& evtLsn     = std::get<3>(*tup);
    auto& controller = std::get<4>(*tup);

    UT_LOG_INFO("[L3 MotionEventDetection] sensorIndex=%d numEvents=%d timeoutMs=%d",
        cfg.sensorIndex, cfg.numEvents, cfg.eventTimeoutMs);

    ctrlLsn->resetStateQueue();
    evtLsn->resetEventQueue();

    /* Step 2: No spurious/stale events before test begins */
    {
        OperationalMode staleMode;
        bool gotStale = evtLsn->getNextEvent(staleMode, chrono::milliseconds(200));
        UT_ASSERT_FALSE(gotStale);
    }

    /* Step 3: Listener-gate — unregister, confirm silence, then re-register */
    MOTIONSENSOR_UNREGISTER_EVENT_SUCCESS(sensor, evtLsn);
    {
        OperationalMode preRegMode;
        bool gotPreReg = evtLsn->getNextEvent(preRegMode, chrono::milliseconds(500));
        UT_ASSERT_FALSE(gotPreReg);
    }
    MOTIONSENSOR_REGISTER_EVENT_SUCCESS(sensor, evtLsn);
    evtLsn->resetEventQueue();

    /* Step 4: Start sensor in MOTION mode */
    StartConfig startCfg;
    startCfg.operationalMode    = OperationalMode::MOTION;
    startCfg.noMotionSeconds    = cfg.noMotionSeconds;
    startCfg.activeStartSeconds = cfg.activeStartSeconds;
    startCfg.activeStopSeconds  = cfg.activeStopSeconds;

    MOTIONSENSOR_START_SUCCESS(controller, startCfg);

    State lastState;
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STARTED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STARTED);

    /* Step 5: Receive N MOTION events — Python injects one per iteration */
    for (int32_t evtNum = 0; evtNum < cfg.numEvents; ++evtNum)
    {
        OperationalMode receivedMode;
        bool gotEvent = evtLsn->getNextEvent(receivedMode, chrono::milliseconds(cfg.eventTimeoutMs));
        UT_ASSERT_TRUE(gotEvent);
        UT_ASSERT_EQUAL(receivedMode, OperationalMode::MOTION);
        UT_LOG_INFO("[L3 MotionEventDetection] Event %d/%d received: mode=MOTION",
            evtNum + 1, cfg.numEvents);
    }

    /* Step 6: Stop sensor; verify no spurious events after stop */
    MOTIONSENSOR_STOP_SUCCESS(controller);
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STOPPED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STOPPED);

    evtLsn->resetEventQueue();
    {
        OperationalMode postStopMode;
        bool gotPostStop = evtLsn->getNextEvent(postStopMode, chrono::milliseconds(500));
        UT_ASSERT_FALSE(gotPostStop);
    }

    /* Step 7: Unregister listener; host injects event — must NOT be delivered */
    MOTIONSENSOR_UNREGISTER_EVENT_SUCCESS(sensor, evtLsn);
    evtLsn->resetEventQueue();
    {
        OperationalMode unregMode;
        bool gotUnreg = evtLsn->getNextEvent(unregMode, chrono::milliseconds(2000));
        UT_ASSERT_FALSE(gotUnreg);
    }

    /* Re-register for TearDown */
    MOTIONSENSOR_REGISTER_EVENT_SUCCESS(sensor, evtLsn);
}

/* ========================================================================== */
/*  Test 2 — No-Motion Event, Timing, Accuracy & Listener Validation          */
/* ========================================================================== */

/**
 * @brief Validate no-motion event delivery in NO_MOTION mode.
 *
 * Steps:
 * 1. Parse test config (sensor index, noMotionSeconds, timeout).
 * 2. Start sensor in NO_MOTION mode.
 * 3. Wait for STARTED state.
 * 4. Host simulates noMotionSeconds of no activity, then injects NO_MOTION event.
 * 5. Verify event arrives with type NO_MOTION.
 * 6. Unregister listener; verify no subsequent events arrive.
 * 7. Stop sensor.
 *
 * **Test Group ID:** Basic: 03@n
 * **Test Case ID:** 002@n
 * **Priority:** High@n
 */
UT_ADD_TEST(VTS_L3_MOTIONSENSOR, NoMotionEventDetection)
{
    const char *testName = "NoMotionEventDetection";
    motionSensorL3EventTestConfig_t cfg;

    UT_ASSERT_EQUAL_FATAL(
        vts_motionsensor_parse_eventtest_config(
            VTS_MOTIONSENSOR_TEST_CONFIG_PATH, gModuleName, gTestGroup, testName, cfg),
        UT_STATUS_OK);

    SensorTuple *tup = nullptr;
    UT_ASSERT_TRUE_FATAL(getSensorAtIndex(cfg.sensorIndex, tup));

    auto& sensor     = std::get<1>(*tup);
    auto& ctrlLsn    = std::get<2>(*tup);
    auto& evtLsn     = std::get<3>(*tup);
    auto& controller = std::get<4>(*tup);

    ctrlLsn->resetStateQueue();
    evtLsn->resetEventQueue();

    StartConfig startCfg;
    startCfg.operationalMode    = OperationalMode::NO_MOTION;
    startCfg.noMotionSeconds    = cfg.noMotionSeconds;
    startCfg.activeStartSeconds = cfg.activeStartSeconds;
    startCfg.activeStopSeconds  = cfg.activeStopSeconds;

    MOTIONSENSOR_START_SUCCESS(controller, startCfg);

    State lastState;
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STARTED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STARTED);

    /* Host waits noMotionSeconds then injects NO_MOTION event */
    OperationalMode receivedMode;
    bool gotEvent = evtLsn->getNextEvent(receivedMode, chrono::milliseconds(cfg.eventTimeoutMs));
    UT_ASSERT_TRUE(gotEvent);
    UT_ASSERT_EQUAL(receivedMode, OperationalMode::NO_MOTION);

    /* Unregister — subsequent events must not be delivered */
    MOTIONSENSOR_UNREGISTER_EVENT_SUCCESS(sensor, evtLsn);
    evtLsn->resetEventQueue();

    OperationalMode staleMode;
    bool gotStale = evtLsn->getNextEvent(staleMode, chrono::milliseconds(2000));
    UT_ASSERT_FALSE(gotStale);

    /* Re-register for TearDown */
    MOTIONSENSOR_REGISTER_EVENT_SUCCESS(sensor, evtLsn);

    MOTIONSENSOR_STOP_SUCCESS(controller);
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STOPPED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STOPPED);
}

/* ========================================================================== */
/*  Test 3 — Active Window Behavior                                           */
/* ========================================================================== */

/**
 * @brief Validate that active windows gate event emission correctly.
 *
 * Steps:
 * 1. Parse test config (sensor index, window start/end, timeout).
 * 2. Configure an active window covering the test period.
 * 3. Start sensor in MOTION mode.
 * 4. Host injects a MOTION_SENSOR_EVENT while within the window.
 * 5. Verify event is received (window is open).
 * 6. Clear active windows (unconstrained).
 * 7. Host injects another event — verify it is still received.
 * 8. Stop sensor.
 *
 * **Test Group ID:** Basic: 03@n
 * **Test Case ID:** 003@n
 * **Priority:** High@n
 */
UT_ADD_TEST(VTS_L3_MOTIONSENSOR, ActiveWindowBehavior)
{
    const char *testName = "ActiveWindowBehavior";
    motionSensorL3ActiveWindowTestConfig_t cfg;

    UT_ASSERT_EQUAL_FATAL(
        vts_motionsensor_parse_activewindow_config(
            VTS_MOTIONSENSOR_TEST_CONFIG_PATH, gModuleName, gTestGroup, testName, cfg),
        UT_STATUS_OK);

    SensorTuple *tup = nullptr;
    UT_ASSERT_TRUE_FATAL(getSensorAtIndex(cfg.sensorIndex, tup));

    auto& sensor     = std::get<1>(*tup);
    auto& ctrlLsn    = std::get<2>(*tup);
    auto& evtLsn     = std::get<3>(*tup);
    auto& controller = std::get<4>(*tup);

    ctrlLsn->resetStateQueue();
    evtLsn->resetEventQueue();

    /* Configure active window (sensor must be STOPPED) */
    std::vector<TimeWindow> windows;
    TimeWindow win;
    win.startTimeOfDaySeconds = cfg.windowStartSeconds;
    win.endTimeOfDaySeconds   = cfg.windowEndSeconds;
    windows.push_back(win);
    MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controller, windows);

    /* Start in MOTION mode */
    StartConfig startCfg;
    startCfg.operationalMode    = OperationalMode::MOTION;
    startCfg.noMotionSeconds    = 0;
    startCfg.activeStartSeconds = 0;
    startCfg.activeStopSeconds  = 0;

    MOTIONSENSOR_START_SUCCESS(controller, startCfg);

    State lastState;
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STARTED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STARTED);

    /* Phase 1: host injects event; check against expected outcome.
     * expectEventReceived=1 → window open → event must arrive (PASS if true).
     * expectEventReceived=0 → window closed → event suppressed → getNextEvent
     *   times out (PASS if false). */
    OperationalMode receivedMode;
    bool gotEvent = evtLsn->getNextEvent(receivedMode, chrono::milliseconds(cfg.eventTimeoutMs));
    if (cfg.expectEventReceived)
    {
        UT_LOG_INFO("[L3 ActiveWindowBehavior] Phase-1: expecting event — gotEvent=%s",
            gotEvent ? "true" : "false");
        UT_ASSERT_TRUE(gotEvent);
        UT_ASSERT_EQUAL(receivedMode, OperationalMode::MOTION);
    }
    else
    {
        UT_LOG_INFO("[L3 ActiveWindowBehavior] Phase-1: expecting suppression — gotEvent=%s",
            gotEvent ? "true (UNEXPECTED)" : "false (suppressed OK)");
        UT_ASSERT_FALSE(gotEvent);
    }

    /* Stop to reconfigure */
    MOTIONSENSOR_STOP_SUCCESS(controller);
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STOPPED, lastState);

    /* Clear all windows — events should arrive any time */
    MOTIONSENSOR_CLEAR_ACTIVE_WINDOWS_SUCCESS(controller);

    std::vector<TimeWindow> emptyWindows;
    MOTIONSENSOR_GET_ACTIVE_WINDOWS_SUCCESS(controller, emptyWindows);
    UT_ASSERT_EQUAL(emptyWindows.size(), (size_t)0);

    evtLsn->resetEventQueue();
    MOTIONSENSOR_START_SUCCESS(controller, startCfg);
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STARTED, lastState);

    /* Host injects event again — now with no window restriction */
    bool gotEvent2 = evtLsn->getNextEvent(receivedMode, chrono::milliseconds(cfg.eventTimeoutMs));
    UT_ASSERT_TRUE(gotEvent2);
    UT_ASSERT_EQUAL(receivedMode, OperationalMode::MOTION);

    MOTIONSENSOR_STOP_SUCCESS(controller);
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STOPPED, lastState);
}

/* ========================================================================== */
/*  Test 4 — Deep Sleep Autonomy                                              */
/* ========================================================================== */

/**
 * @brief Validate motion detection during deep sleep when autonomy is enabled.
 *
 * Steps:
 * 1. Parse test config (sensor index, sleep duration, timeout).
 * 2. Check sensor capabilities for deep sleep autonomy support; skip if not.
 * 3. Enable setAutonomousDuringDeepSleep(true).
 * 4. Start sensor in MOTION mode.
 * 5. Host sends DEEP_SLEEP_START command, waits deepSleepDurationSeconds,
 *    injects a MOTION_SENSOR_EVENT, then sends DEEP_SLEEP_END.
 * 6. Verify the event was delivered (autonomy enabled → events during sleep).
 * 7. Stop, set autonomous=false, start again.
 * 8. Host sends DEEP_SLEEP_START, injects event, sends DEEP_SLEEP_END.
 * 9. Verify NO event was delivered (autonomy disabled).
 * 10. Stop sensor.
 *
 * **Test Group ID:** Basic: 03@n
 * **Test Case ID:** 004@n
 * **Priority:** High@n
 */
UT_ADD_TEST(VTS_L3_MOTIONSENSOR, DeepSleepAutonomy)
{
    const char *testName = "DeepSleepAutonomy";
    motionSensorL3DeepSleepTestConfig_t cfg;

    UT_ASSERT_EQUAL_FATAL(
        vts_motionsensor_parse_deepsleep_config(
            VTS_MOTIONSENSOR_TEST_CONFIG_PATH, gModuleName, gTestGroup, testName, cfg),
        UT_STATUS_OK);

    SensorTuple *tup = nullptr;
    UT_ASSERT_TRUE_FATAL(getSensorAtIndex(cfg.sensorIndex, tup));

    auto& sensor     = std::get<1>(*tup);
    auto& ctrlLsn    = std::get<2>(*tup);
    auto& evtLsn     = std::get<3>(*tup);
    auto& controller = std::get<4>(*tup);

    /* Check capability */
    Capabilities caps;
    MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensor, caps);

    if (!caps.supportsDeepSleepAutonomy)
    {
        UT_LOG_INFO("[L3 DeepSleepAutonomy] Sensor does not support deep sleep autonomy — skipping test");
        return;
    }

    ctrlLsn->resetStateQueue();
    evtLsn->resetEventQueue();

    /* --- Phase A: autonomy ENABLED --- */
    MOTIONSENSOR_SET_AUTONOMOUS_SUCCESS(controller, true);

    bool autonomyEnabled = false;
    {
        ::android::binder::Status s = controller->isAutonomousDuringDeepSleepEnabled(&autonomyEnabled);
        UT_ASSERT_TRUE(s.isOk());
    }
    UT_ASSERT_TRUE(autonomyEnabled);

    StartConfig startCfg;
    startCfg.operationalMode    = OperationalMode::MOTION;
    startCfg.noMotionSeconds    = 0;
    startCfg.activeStartSeconds = 0;
    startCfg.activeStopSeconds  = 0;

    MOTIONSENSOR_START_SUCCESS(controller, startCfg);
    State lastState;
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STARTED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STARTED);

    /* Host sends DEEP_SLEEP_START, injects MOTION event, then DEEP_SLEEP_END */
    OperationalMode receivedMode;
    bool gotEventDuringSleep = evtLsn->getNextEvent(receivedMode, chrono::milliseconds(cfg.eventTimeoutMs));
    UT_ASSERT_TRUE(gotEventDuringSleep);
    UT_ASSERT_EQUAL(receivedMode, OperationalMode::MOTION);

    MOTIONSENSOR_STOP_SUCCESS(controller);
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STOPPED, lastState);

    /* --- Phase B: autonomy DISABLED --- */
    MOTIONSENSOR_SET_AUTONOMOUS_SUCCESS(controller, false);

    bool autonomyDisabled = true;
    {
        ::android::binder::Status s = controller->isAutonomousDuringDeepSleepEnabled(&autonomyDisabled);
        UT_ASSERT_TRUE(s.isOk());
    }
    UT_ASSERT_FALSE(autonomyDisabled);

    evtLsn->resetEventQueue();
    MOTIONSENSOR_START_SUCCESS(controller, startCfg);
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STARTED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STARTED);

    /* Host sends DEEP_SLEEP_START, injects event, sends DEEP_SLEEP_END —
     * event must NOT arrive because autonomy is disabled */
    OperationalMode noEventMode;
    bool gotEventNoAutonomy = evtLsn->getNextEvent(noEventMode, chrono::milliseconds(cfg.eventTimeoutMs));
    UT_ASSERT_FALSE(gotEventNoAutonomy);

    MOTIONSENSOR_STOP_SUCCESS(controller);
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STOPPED, lastState);
}

/**
 * @} // End of MOTION_SENSOR_HALTEST_L3
 * @} // End of MOTION_SENSOR_HALTEST
 * @} // End of MOTION_SENSOR
 * @} // End of HPK
 */
/* End of vts_motionsensor_l3_tests.cpp */
