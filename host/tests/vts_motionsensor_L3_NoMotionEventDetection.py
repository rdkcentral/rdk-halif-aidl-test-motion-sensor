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


class VTS_MOTIONSENSOR_L3_NoMotionEventDetection(
        motionSensorHelperClass):
    """
    L3 test for NO_MOTION timer behaviour.
    Phase 1:
        Send multiple MOTION events at the configured interval and verify
        that the NO_MOTION timer is reset by each MOTION event.
    Phase 2:
        Disable the NO_MOTION timer by setting no_motion_seconds to 0 and
        verify that no NO_MOTION event is generated.
    """

    def __init__(
        self,
        log: logModule = None,
        moduleName: str = None
    ):
        self.testName = "MotionSensor_L3_NoMotionEventDetection"
        qcId = "3"

        super().__init__(
            self.testName,
            qcId,
            log,
            moduleName
        )
        self.testParams = self._readMotionParameterFromProfile("nomotion")

    def _make_timer_reset_trigger(
        self,
        sid: int,
        max_events: int,
        noMotionSec: int
    ):
        """
        Create a trigger that sends MOTION events at the configured
        minimum interval.

        The interval must be shorter than no_motion_seconds so that
        every MOTION resets the NO_MOTION timer before it expires.
        """

        def _trigger():

            if max_events <= 0:
                self.log.error(
                    " Invalid MOTION iteration count: "
                    f"{max_events}"
                )
                return

            if noMotionSec <= 0:
                self.log.error(
                    " no_motion_seconds must be > 0"
                )
                return

            reset_interval = float(
                self.testParams["minimum_event_interval_seconds"]
            )
            if reset_interval <= 0:
                self.log.error(
                    " minimum_event_interval_seconds "
                    "must be > 0"
                )
                return

            if reset_interval >= noMotionSec:
                self.log.error(
                    " minimum_event_interval_seconds "
                    f"({reset_interval}s) must be less than "
                    f"no_motion_seconds ({noMotionSec}s)"
                )
                return

            self.log.info(
                f" MOTION interval={reset_interval:.1f}s, "
                f"no_motion_seconds={noMotionSec}"
            )
            for cycle in range(max_events):
                self.log.info(
                        "[NoMotionTimerReset] "
                        f"Sending MOTION {cycle + 1}/{max_events} "
                        f"to sensor_id={sid}"
                    )

                self.motionSensorDevice.triggerMotionEvent(sensorId=sid)

                # --------------------------------------------------
                # Allow the NO_MOTION timer to expire.
                # --------------------------------------------------
                self.log.info(
                    f"[NoMotionTimerReset] Cycle {cycle + 1}: "
                    f"Waiting {noMotionSec} + {reset_interval}s for NO_MOTION"
                )
                time.sleep(noMotionSec+reset_interval)
        return _trigger

    def _write_event_config(
        self,
        l3cfg,
        sensor_index,
        operational_mode,
        max_expected_events,
        event_timeout_ms,
        no_motion_seconds
    ):
        """
        Update the L3 EventDetection configuration and write it to YAML.
        """

        node = l3cfg['motionsensor']['L3']['EventDetection']
        node['sensor_index'] = sensor_index
        node['operational_mode'] = operational_mode
        node['max_expected_events'] = max_expected_events
        node['event_timeout_ms'] = event_timeout_ms
        node['no_motion_seconds'] = no_motion_seconds

        self.log.info(
            f"[Config] sensor_index={sensor_index} "
            f"mode={operational_mode} "
            f"max_expected_events={max_expected_events} "
            f"event_timeout_ms={event_timeout_ms} "
            f"no_motion_seconds={no_motion_seconds}"
        )
        with open(self.paths["l3TestConfigFile"], 'w') as fh:
            yaml.dump(l3cfg, fh, default_flow_style=False, sort_keys=False)

    def testFunction(self) -> bool:

        l3cfg = self._readL3TestConfig()
        if l3cfg is None:
            return False

        # --------------------------------------------------------------
        # Read NO_MOTION test parameters from
        # hfp-vts-motionsensor-test-params.yaml.
        # --------------------------------------------------------------
        try:
            configured_timeout_ms = int(self.testParams["event_timeout_ms"])
            no_motion_seconds = int(self.testParams["no_motion_seconds"])
            reset_interval = float(self.testParams["minimum_event_interval_seconds"])
            max_expected_events = int(self.testParams["max_expected_events"])
            event_simulator_ready_wait_seconds = float(self.testParams["event_simulator_ready_wait_seconds"])
        except (KeyError, TypeError, ValueError) as exc:
            self.log.error(
                "[NoMotionEventDetection] Invalid NO_MOTION "
                f"test parameters: {exc}"
            )
            return False

        # --------------------------------------------------------------
        # Validate timing configuration.
        # --------------------------------------------------------------
        if no_motion_seconds <= 0:
            self.log.error("[NoMotionEventDetection] no_motion_seconds must be > 0 for Phase 1/Phase 2")
            return False

        if configured_timeout_ms <= 0:
            self.log.error("[NoMotionEventDetection] event_timeout_ms must be > 0")
            return False

        if configured_timeout_ms < (no_motion_seconds * 1000):
            self.log.error(
                f"[NoMotionEventDetection] event_timeout_ms ({configured_timeout_ms}ms) must be >= "
                f"no_motion_seconds ({no_motion_seconds}s)"
            )
            return False

        if reset_interval <= 0:
            self.log.error(
                "[NoMotionEventDetection] reset_interval must be > 0"
            )
            return False

        if reset_interval >= no_motion_seconds:
            self.log.error(
                "[NoMotionEventDetection] reset_interval "
                f"({reset_interval}s) must be less than "
                f"no_motion_seconds ({no_motion_seconds}s)"
            )
            return False

        if max_expected_events <= 0:
            self.log.error(
                "[NoMotionEventDetection] max_expected_events must be > 0"
            )
            return False

        if configured_timeout_ms <= 0:
            self.log.error(
                "[NoMotionEventDetection] configured_timeout_ms must be > 0"
            )
            return False

        if event_simulator_ready_wait_seconds < 0:
            self.log.error(
                "[NoMotionEventDetection] event_simulator_ready_wait_seconds must be >= 0"
            )
            return False

        sensor_ids = self._readSensorIdsFromProfile()

        if not sensor_ids:
            self.log.error("[NoMotionEventDetection] No sensor IDs found in profile")
            return False

        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        all_passed = True

        # Run all 3 phases for every sensor
        for sensor_index, sensor_id in enumerate(sensor_ids):

            self.log.info("====================================================")
            self.log.info(f"Sensor index={sensor_index} sensor_id={sensor_id}")
            self.log.info("====================================================")

            self.log.info("====================================================")
            self.log.info(" Phase1: NO_MOTION timer reset test")
            self.log.info(f" sensor_id={sensor_id}")
            self.log.info(f" no_motion_seconds={no_motion_seconds}")
            self.log.info(" Expected NO_MOTION events=1")
            # max_expected_events describes the number of expected
            # NO_MOTION events, NOT the number of injected MOTION events.
            self.log.info(
                f"C++ event timeout={configured_timeout_ms}ms "
                f"(MOTION iterations={max_expected_events}, "
                f"reset_interval={reset_interval:.1f}s, "
                f"no_motion_seconds={no_motion_seconds}s)"
            )
            self._write_event_config(
                l3cfg, sensor_index, 'NO_MOTION',
                max_expected_events, configured_timeout_ms, no_motion_seconds)

            phase1_result = self.runL3Test(
                "NoMotionTimerReset",
                triggerCallable=self._make_timer_reset_trigger(
                    sensor_id,
                    max_expected_events,
                    no_motion_seconds
                ),
                triggerDelay=event_simulator_ready_wait_seconds
            )

            self.log.info("====================================================")
            self.log.info("[Phase-2] NO_MOTION disabled test")
            self.log.info(f"sensor_id={sensor_id}")
            self.log.info("no_motion_seconds=0")
            self.log.info("Expected NO_MOTION events=0")

            self._write_event_config(
                l3cfg,
                sensor_index,
                'NO_MOTION',
                0,
                configured_timeout_ms,
                0
            )

            self.log.info(f"C++ event timeout={configured_timeout_ms}ms")
            phase2_result = self.runL3Test("NoMotionTimerReset", triggerCallable=None, triggerDelay=0)
            sensor_passed = phase1_result and phase2_result

            if sensor_passed:
                self.log.info(f"[Phase-1 & 2] PASSED for sensor_id={sensor_id}")
            else:
                self.log.error(f"[Phase-1 & 2] FAILED for sensor_id={sensor_id}")
                all_passed = False
        # ==============================================================
        # Final result
        # ==============================================================
        if all_passed:
            self.log.info("====================================================")
            self.log.info("[NoMotionEventDetection] ALL PHASES PASSED")
            self.log.info("====================================================")
        else:
            self.log.error("====================================================")
            self.log.error("[NoMotionEventDetection] ONE OR MORE PHASES FAILED")
            self.log.error("====================================================")

        return all_passed

if __name__ == "__main__":
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_MOTIONSENSOR_L3_NoMotionEventDetection(summaryLog)
    test.run(False)
