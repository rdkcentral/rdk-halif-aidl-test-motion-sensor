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
# set autonomy, call start(), and enter getNextEvent().
_TRIGGER_START_DELAY_S = 3.0


class VTS_MOTIONSENSOR_L3_DeepSleepAutonomy(motionSensorHelperClass):
    """
    L3 test: Verify the deep-sleep autonomy feature for every sensor in the
    HFP profile.

    For each sensor the test runs the C++ DeepSleepAutonomy case twice:

    Run 1 — autonomy_enabled=True, expect_wakeup=True:
        simulateIdleDeepSleep → MOTION (delivered, device wakes) → probe arrives.

    Run 2 — autonomy_enabled=False, expect_wakeup=False:
        simulateIdleDeepSleep → MOTION (gated, device stays asleep) → probe gated.

    Each run writes autonomy_enabled + expect_wakeup to the L3 YAML before
    launching the C++ binary, which reads both fields and drives a single
    self-contained phase.
    """

    def __init__(self, log: logModule = None):
        self.testName = "MotionSensor_L3_DeepSleepAutonomy"
        qcId = "4"
        super().__init__(self.testName, qcId, log)

    def _make_trigger(self, sensor_id: int, sleep_duration: float) -> callable:
        """
        Return a no-argument callable that drives one deep-sleep phase.

        Timeline:
            T+0          : simulateIdleDeepSleep (idle wait → DEEP_SLEEP_START)
            T+sleep_dur  : Primary MOTION event
            T+sleep_dur+0.3 : Probe MOTION event (wake-state check)

        C++ asserts primary and probe according to autonomy_enabled / expect_wakeup
        read from YAML — no Phase A/B split needed here.
        """
        def _trigger():
            self.motionSensorDevice.simulateIdleDeepSleep(sensor_id, sleep_duration)
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

        Args:
            sensor_index    (int):   0-based sensor index in the HFP list.
            sensor_id       (int):   Sensor ID for CP commands.
            sleep_duration  (float): Seconds to simulate idle before deep sleep.
            autonomy_enabled(bool):  True = autonomy ON; False = autonomy OFF.
            l3cfg           (dict):  Parsed L3 YAML dict (modified in place).

        Returns:
            bool: True if C++ test passed.
        """
        label = "autonomy=ON" if autonomy_enabled else "autonomy=OFF"
        self.log.info(
            f"[DeepSleepAutonomy] RUN sensor_index={sensor_index} "
            f"sensor_id={sensor_id} sleepDuration={sleep_duration}s {label}")

        l3cfg['motionsensor']['L3']['EventDetection']['sensor_index']    = sensor_index
        l3cfg['motionsensor']['L3']['EventDetection']['autonomy_enabled'] = autonomy_enabled
        l3cfg['motionsensor']['L3']['EventDetection']['expect_wakeup']    = autonomy_enabled
        with open(self.l3TestConfigFile, 'w') as fh:
            yaml.dump(l3cfg, fh, default_flow_style=False, sort_keys=False)

        trigger = self._make_trigger(sensor_id, sleep_duration)
        result  = self.runL3Test(
            "DeepSleepAutonomy",
            triggerCallable=trigger,
            triggerDelay=_TRIGGER_START_DELAY_S)

        outcome = "PASSED" if result else "FAILED"
        self.log.info(
            f"[DeepSleepAutonomy] sensor_index={sensor_index} "
            f"sensor_id={sensor_id} {label}: {outcome}")
        return result

    def testFunction(self) -> bool:
        """
        Iterate over every sensor in the HFP profile and run the C++
        DeepSleepAutonomy test case for each one.

        Returns:
            bool: True if every sensor passed, False otherwise.
        """
        with open(self.l3TestConfigFile, 'r') as fh:
            l3cfg = yaml.safe_load(fh)

        ds_node        = l3cfg['motionsensor']['L3']['EventDetection']
        sleep_duration = float(
            self.deviceTestSetup.get('deep_sleep_duration_seconds', 5))

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
