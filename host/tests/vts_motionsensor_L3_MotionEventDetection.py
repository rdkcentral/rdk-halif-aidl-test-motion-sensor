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


class VTS_MOTIONSENSOR_L3_MotionEventDetection(motionSensorHelperClass):
    """
    L3 test: Verify MOTION events are delivered end-to-end.

    Iterates over *all* sensors in the HFP profile (N sensors).  For each
    sensor it updates ``sensor_index`` in the L3 YAML, uploads it to the
    target, runs the C++ MotionEventDetection test case, and injects exactly
    ``num_motion_events`` MOTION stimuli via the control-plane one per C++
    loop iteration, spaced ``interEventDelay`` seconds apart.
    """


    def __init__(self, log: logModule = None, moduleName: str = None):
        self.testName = "MotionSensor_L3_MotionEventDetection"
        qcId = "1"
        super().__init__(self.testName, qcId, log, moduleName)
        self.testParams = self._readMotionParameterFromProfile("motion")

    def _make_trigger(self, sid: int, n: int):
        """Return a no-arg callable that fires *n* MOTION stimuli for sensor *sid*."""
        def _trigger():
            interEventDelay = float(self.testParams["minimum_event_interval_seconds"])
            for i in range(n):
                if i > 0:
                    time.sleep(interEventDelay)
                self.motionSensorDevice.triggerMotionEvent(sensorId=sid)
        return _trigger

    def testFunction(self) -> bool:
        """
        Main test function.

        Flow:
          1. Read ``num_motion_events`` and ``event_timeout_ms`` from L3 YAML.
          2. Read sensor IDs from the HFP profile.
          3. For each sensor (sensor_index 0..N-1):
             a. Update ``sensor_index`` in L3 YAML and save.
             b. Run ``runL3Test("MotionEventDetection", triggerCallable=...)``
                which uploads the config, starts the binary, then fires
                ``num_motion_events`` MOTION events at the correct sensor ID.
          4. Return True only if every sensor passed.

        Returns:
            bool: True if all sensors passed, False otherwise.
        """
        # Read test parameters from the local L3 config YAML.
        l3cfg = self._readL3TestConfig()
        if l3cfg is None:
            return False

        max_events = int(self.testParams["max_expected_events"])
        timeout_ms = int(self.testParams["event_timeout_ms"])
        trigger_delay = float(self.testParams["event_simulator_ready_wait_seconds"])


        # Read all sensor IDs from the HFP profile.
        sensor_ids = self._readSensorIdsFromProfile()
        if not sensor_ids:
            self.log.error("[MotionEventDetection] No sensor IDs found in profile aborting")
            return False

        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        all_passed = True
        for sensor_index, sensor_id in enumerate(sensor_ids):
            self.log.info(
                f"[MotionEventDetection] sensor_index={sensor_index} "
                f"sensor_id={sensor_id}  max_events={max_events}")

            # Write a clean, self-consistent YAML for this sensor run.
            l3cfg['motionsensor']['L3']['EventDetection']['sensor_index']       = sensor_index
            l3cfg['motionsensor']['L3']['EventDetection']['operational_mode']   = 'MOTION'
            l3cfg['motionsensor']['L3']['EventDetection']['max_expected_events'] = max_events
            l3cfg['motionsensor']['L3']['EventDetection']['event_timeout_ms']    = timeout_ms
            with open(self.paths["l3TestConfigFile"], 'w') as fh:
                yaml.dump(l3cfg, fh, default_flow_style=False, sort_keys=False)

            result = self.runL3Test(
                "EventDetection",
                triggerCallable=self._make_trigger(sensor_id, max_events * 2),  # Fire extra events to verify C++ test only counts expected number
                triggerDelay=trigger_delay)

            if not result:
                all_passed = False

        return all_passed


if __name__ == "__main__":
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_MOTIONSENSOR_L3_MotionEventDetection(summaryLog)
    test.run(False)

