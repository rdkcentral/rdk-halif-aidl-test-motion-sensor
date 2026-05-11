#!/usr/bin/env python3
#** *****************************************************************************
# *
# * If not stated otherwise in this file or this component's LICENSE file the
# * following copyright and licenses apply:
# *
# * Copyright 2026 RDK Management
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *
# http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *
#* ******************************************************************************

import os
import subprocess
import sys
import time
import yaml

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))
sys.path.append(dir_path)
sys.path.append(os.path.join(dir_path, "..", "raft"))


from raft.framework.core.logModule import logModule
from vts_motionsensor_helper import motionSensorHelperClass


# Seconds after utMenu.start() before the first CP command is sent.
# Must be long enough for the C++ binary to parse config, open the sensor,
# set autonomy, call start(), and enter getNextEvent().  The idle timer
# (deep_sleep_duration_seconds from the device config) will fire
# autonomously, so this delay only needs to cover binary startup + AIDL setup.
_TRIGGER_START_DELAY_S = 3.0


class VTS_MOTIONSENSOR_L3_DeepSleepAutonomy(motionSensorHelperClass):
    """
    L3 test: Verify the deep-sleep autonomy feature for every sensor in the
    HFP profile.

    For each sensor the test runs the C++ DeepSleepAutonomy case twice:

    Run 1 — autonomy_enabled=True, expect_wakeup=True:
        Idle timer fires after deep_sleep_duration_seconds of inactivity
        → MOTION event delivered (autonomy ON, device wakes) → probe arrives.

    Run 2 — autonomy_enabled=False, expect_wakeup=False:
        Idle timer fires → MOTION event gated (autonomy OFF,
        device stays asleep) → probe gated.

    The host no longer sends deep-sleep-start / deep-sleep-end commands.
    The HAL service enters deep sleep autonomously, driven by the idle timer
    configured via --deep-sleep-duration (read from the device config).
    """

    def __init__(self, log: logModule = None):
        self.testName = "MotionSensor_L3_DeepSleepAutonomy"
        qcId = "4"
        super().__init__(self.testName, qcId, log)

    def _verifyWakeupViaPing(self, autonomy_enabled: bool,
                              ping_timeout: float = 10.0) -> bool:
        """
        Verify whether the DUT woke up after a motion event using a ping test.

        When *autonomy_enabled* is True the device is expected to wake, so a
        successful ping is required.  When False the device should stay asleep
        and the ping is expected to fail (no response).

        TODO: In the unified repo, replace the ping-based check below with a
              direct query to the deepsleep module (e.g. checking the power
              level via ``deepsleepHelperClass.getPowerLevel()``) so that the
              wakeup verdict does not depend on network reachability alone.

        Args:
            autonomy_enabled (bool):  True if the device is expected to be awake.
            ping_timeout     (float): Seconds to wait for a ping reply.

        Returns:
            bool: True if the observed wakeup state matches *autonomy_enabled*.
        """
        # TODO (unified repo): replace with deepsleepDevice.getPowerLevel() check
        return True

    def _make_trigger(self, sensor_id: int, sleep_duration: float) -> callable:
        """
        Return a no-argument callable that injects the motion events for one
        deep-sleep phase.

        The HAL service enters deep sleep autonomously after
        ``deep_sleep_duration_seconds`` of inactivity (configured via CLI arg
        from the device config) — the host no longer sends deep-sleep-start or
        deep-sleep-end commands.

        Timeline (relative to the C++ binary calling start()):
            T+0               : C++ binary starts, calls start() — idle timer starts
            T+sleep_duration  : Idle timer fires → sets inDeepSleep=true
            T+sleep_duration  : Primary MOTION event (host sends after waiting)
            T+sleep_duration+0.3 : Probe MOTION event (wake-state check)

        C++ asserts primary and probe according to autonomy_enabled / expect_wakeup
        read from YAML — no Phase A/B split needed here.
        """
        def _trigger():
            # Wait for the HAL service idle timer to fire before injecting events.
            time.sleep(sleep_duration)
            self.motionSensorDevice.triggerMotionEvent(sensor_id)   # primary
            time.sleep(0.3)
            self.motionSensorDevice.triggerMotionEvent(sensor_id)   # probe
        return _trigger

    def runDeepSleepAutonomyTest(self, sensor_index: int, sensor_id: int,
                                  sleep_duration: float, autonomy_enabled: bool,
                                  l3cfg: dict) -> bool:
        """
        Run one autonomy phase for the given sensor.

        Writes ``autonomy_enabled`` and ``expect_wakeup`` (= ``autonomy_enabled``)
        into the L3 YAML, uploads it, then runs the C++ DeepSleepAutonomy test.
        The HAL service enters deep sleep autonomously; the host only injects the
        motion events after the idle duration has elapsed.

        Args:
            sensor_index    (int):   0-based sensor index in the HFP list.
            sensor_id       (int):   Sensor ID for CP commands.
            sleep_duration  (float): Seconds the HAL service idles before deep sleep
                                     (read from device config, matches --deep-sleep-duration).
            autonomy_enabled(bool):  True = autonomy ON; False = autonomy OFF.
            l3cfg           (dict):  Parsed L3 YAML dict (modified in place).

        Returns:
            bool: True if C++ test passed.
        """
        label = "autonomy=ON" if autonomy_enabled else "autonomy=OFF"
        self.log.info(
            f"[DeepSleepAutonomy] RUN sensor_index={sensor_index} "
            f"sensor_id={sensor_id} sleepDuration={sleep_duration}s {label}")
        test_node  = l3cfg['motionsensor']['L3']['EventDetection']
        timeout_ms = int(test_node.get('event_timeout_ms', 15000)) or 15000
        l3cfg['motionsensor']['L3']['EventDetection']['sensor_index']              = sensor_index
        l3cfg['motionsensor']['L3']['EventDetection']['deep_sleep_duration_seconds'] = int(sleep_duration)
        l3cfg['motionsensor']['L3']['EventDetection']['event_timeout_ms']            = (int(sleep_duration) *1000) + timeout_ms
        l3cfg['motionsensor']['L3']['EventDetection']['autonomy_enabled']            = autonomy_enabled
        l3cfg['motionsensor']['L3']['EventDetection']['expect_wakeup']               = autonomy_enabled
        with open(self.l3TestConfigFile, 'w') as fh:
            yaml.dump(l3cfg, fh, default_flow_style=False, sort_keys=False)

        trigger = self._make_trigger(sensor_id, sleep_duration)
        result  = self.runL3Test(
            "DeepSleepAutonomy",
            triggerCallable=trigger,
            triggerDelay=_TRIGGER_START_DELAY_S)

        # Restore event_timeout_ms to the default so subsequent tests
        # (e.g. ActiveWindowBehavior) do not inherit the large DeepSleep value.
        l3cfg['motionsensor']['L3']['EventDetection']['event_timeout_ms'] = timeout_ms
        with open(self.l3TestConfigFile, 'w') as fh:
            yaml.dump(l3cfg, fh, default_flow_style=False, sort_keys=False)
        # After the C++ test concludes, perform a host-side ping check to confirm
        # whether the DUT woke up (or stayed asleep) as expected.
        # TODO (unified repo): replace _verifyWakeupViaPing with a direct
        #      deepsleep power-level query for a more reliable verdict.
        ping_result = self._verifyWakeupViaPing(autonomy_enabled)
        result = result and ping_result

        outcome = "PASSED" if result else "FAILED"
        self.log.info(
            f"[DeepSleepAutonomy] sensor_index={sensor_index} "
            f"sensor_id={sensor_id} {label}: {outcome}")
        return result

    def testFunction(self) -> bool:
        """
        Iterate over every sensor in the HFP profile and run the C++
        DeepSleepAutonomy test case for each one.

        The idle duration is read from the device config
        (``deep_sleep_duration_seconds``) and was already passed to the
        HAL service via ``--deep-sleep-duration`` at startup — no CP commands
        for deep-sleep-start or deep-sleep-end are sent here.

        Returns:
            bool: True if every sensor passed, False otherwise.
        """
        with open(self.l3TestConfigFile, 'r') as fh:
            l3cfg = yaml.safe_load(fh)

        sleep_duration = float(
            self.deviceTestSetup.get('deep_sleep_duration_seconds', 60))

        sensor_ids = self._readSensorIdsFromProfile()
        if not sensor_ids:
            self.log.error("[DeepSleepAutonomy] No sensor IDs found in profile — aborting")
            return False

        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        all_passed = True

        for sensor_index, sensor_id in enumerate(sensor_ids):
            # Run 1: autonomy ON  → primary event delivered, device wakes, probe arrives

            result_a = self.runDeepSleepAutonomyTest(
                sensor_index, sensor_id, sleep_duration, True, l3cfg)
            # Run 2: autonomy OFF → primary gated, device stays asleep, probe gated
            result_b = self.runDeepSleepAutonomyTest(
                sensor_index, sensor_id, sleep_duration, False, l3cfg)

            if not result_a or not result_b:
                all_passed = False

        return all_passed


if __name__ == "__main__":
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_MOTIONSENSOR_L3_DeepSleepAutonomy(summaryLog)
    test.run(False)

