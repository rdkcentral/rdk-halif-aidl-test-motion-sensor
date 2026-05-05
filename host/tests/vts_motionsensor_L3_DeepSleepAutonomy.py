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
# enable autonomy, call start(), and enter Phase-A getNextEvent().
_TRIGGER_START_DELAY_S = 3.0

# Seconds to wait after Phase-A completes (event received + DEEP_SLEEP_END)
# before sending the Phase-B stimulus sequence.  Covers C++ stop() +
# setAutonomousDuringDeepSleep(false) + resetEventQueue() + start() +
# getNextEvent() call setup.
_PHASE_B_SETUP_WAIT_S = 2.0


class VTS_MOTIONSENSOR_L3_DeepSleepAutonomy(motionSensorHelperClass):
    """
    L3 test: Verify the deep-sleep autonomy feature for every sensor in the
    HFP profile.

    For each sensor the test runs the C++ DeepSleepAutonomy case which has
    two internal phases:

    Phase A — autonomy ENABLED:
        Host sends DEEP_SLEEP_START -> waits sleep_duration -> sends MOTION
        event -> sends DEEP_SLEEP_END.
        C++ asserts the event IS received (autonomy on -> events during sleep).

    Phase B — autonomy DISABLED:
        Host sends DEEP_SLEEP_START -> waits sleep_duration -> sends MOTION
        event -> sends DEEP_SLEEP_END.
        C++ asserts the event is NOT received (autonomy off -> events gated).

    Both phases are driven by a single trigger callable.  runL3Test() launches
    it in a background thread _TRIGGER_START_DELAY_S seconds after the C++
    binary starts.
    """

    def __init__(self, log: logModule = None):
        self.testName = "MotionSensor_L3_DeepSleepAutonomy"
        qcId = "4"
        super().__init__(self.testName, qcId, log)

    def _make_trigger(self, sensor_id: int, sleep_duration: float) -> callable:
        """
        Return a no-argument callable that drives the full deep-sleep stimulus
        sequence for *sensor_id* across both Phase A and Phase B.

        Timeline (relative to callable invocation):
          Phase A:
            T+0          : DEEP_SLEEP_START
            T+sleep_dur  : MOTION event  (delivered — autonomy ON)
            T+sleep_dur+0.5 : DEEP_SLEEP_END
          Gap (_PHASE_B_SETUP_WAIT_S): wait for C++ Phase-B setup
          Phase B:
            T+...        : DEEP_SLEEP_START
            T+...+sleep_dur : MOTION event (gated — autonomy OFF)
            T+...+sleep_dur+0.5 : DEEP_SLEEP_END

        Args:
            sensor_id     (int):   Target sensor ID.
            sleep_duration(float): Seconds to hold simulated deep-sleep per phase.

        Returns:
            callable: No-argument callable for use as triggerCallable.
        """
        def _trigger():
            # Phase A: autonomy ON
            self.motionSensorDevice.triggerDeepSleepStart(sensor_id)
            time.sleep(sleep_duration)
            self.motionSensorDevice.triggerMotionEvent(sensor_id)
            time.sleep(0.5)
            self.motionSensorDevice.triggerDeepSleepEnd(sensor_id)

            # Wait for C++ to finish Phase A teardown and enter Phase B
            # getNextEvent (stop -> setAutonomous(false) -> resetQueue -> start).
            time.sleep(_PHASE_B_SETUP_WAIT_S)

            # Phase B: autonomy OFF
            # vcomponent gates the MOTION event (inDeepSleep + autonomy OFF).
            # C++ asserts FALSE for getNextEvent, so the phase times out.
            self.motionSensorDevice.triggerDeepSleepStart(sensor_id)
            time.sleep(sleep_duration)
            self.motionSensorDevice.triggerMotionEvent(sensor_id)
            time.sleep(0.5)
            self.motionSensorDevice.triggerDeepSleepEnd(sensor_id)
        return _trigger

    def testFunction(self) -> bool:
        """
        Iterate over every sensor in the HFP profile and run the C++
        DeepSleepAutonomy test case for each one.

        Returns:
            bool: True if every sensor passed, False otherwise.
        """
        with open(self.l3TestConfigFile, 'r') as fh:
            l3cfg = yaml.safe_load(fh)

        ds_node        = l3cfg['motionsensor']['L3']['DeepSleepAutonomy']
        sleep_duration = float(ds_node.get('deep_sleep_duration_seconds', 2))

        sensor_ids = self._readSensorIdsFromProfile()
        if not sensor_ids:
            self.log.error("[DeepSleepAutonomy] No sensor IDs found in profile — aborting")
            return False

        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        all_passed = True

        for sensor_index, sensor_id in enumerate(sensor_ids):
            self.log.info(
                f"[DeepSleepAutonomy] RUN sensor_index={sensor_index} "
                f"sensor_id={sensor_id} sleepDuration={sleep_duration}s")

            # Write sensor_index to YAML so the C++ test targets the right sensor.
            l3cfg['motionsensor']['L3']['DeepSleepAutonomy']['sensor_index'] = sensor_index
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
                f"sensor_id={sensor_id}: {outcome}")

            if not result:
                all_passed = False

        return all_passed


if __name__ == "__main__":
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_MOTIONSENSOR_L3_DeepSleepAutonomy(summaryLog)
    test.run(False)
