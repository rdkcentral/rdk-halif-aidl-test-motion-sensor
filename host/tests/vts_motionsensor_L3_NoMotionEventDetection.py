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
        Send multiple MOTION events before no_motion_seconds expires.
        Each MOTION should reset the NO_MOTION timer.
        After the final MOTION, exactly one NO_MOTION event is expected.

    Phase 2:
        Do not send MOTION.
        Verify exactly one NO_MOTION event is generated after
        no_motion_seconds.

    Phase 3:
        Set no_motion_seconds = 0.
        Verify that no NO_MOTION event is generated.
    """

    # ------------------------------------------------------------------
    # Test timing configuration
    # ------------------------------------------------------------------
    MOTION_RESET_ITERATIONS = 5
    NO_MOTION_TIMEOUT_MARGIN_S = 2.0
    # Keep the MOTION interval below the NO_MOTION timeout.
    MOTION_RESET_MARGIN_S = 1.0
    # Additional wait time after the expected NO_MOTION timeout.
    FINAL_TIMEOUT_EXTRA_S = 2.0
    # Phase 3 timeout.
    PHASE3_TIMEOUT_MS = 3000

    def __init__(
        self,
        log: logModule = None,
        moduleName: str = None
    ):
        self.testName = "MotionSensor_L3_NoMotionEventDetection"
        qcId = "2"

        super().__init__(
            self.testName,
            qcId,
            log,
            moduleName
        )

    def _calculate_no_motion_timeout_ms(
        self,
        no_motion_seconds: int,
        configured_timeout_ms: int
    ) -> int:
        """
        Calculate the C++ event timeout in milliseconds.

        Args:
            no_motion_seconds: NO_MOTION timer duration in seconds.
            configured_timeout_ms: Timeout configured in YAML.

        Returns:
            Timeout in milliseconds, including the required margin.
        """

        required_timeout_ms = int(
            (
                float(no_motion_seconds) +
                self.NO_MOTION_TIMEOUT_MARGIN_S
            ) * 1000
        )

        timeout_ms = max(
            configured_timeout_ms,
            required_timeout_ms
        )

        self.log.info(
            f"[Timing] no_motion_seconds={no_motion_seconds}, "
            f"configured_timeout_ms={configured_timeout_ms}, "
            f"required_timeout_ms={required_timeout_ms}, "
            f"selected_timeout_ms={timeout_ms}"
        )
        return timeout_ms

    def _make_timer_reset_trigger(
        self,
        sid: int,
        n: int,
        noMotionSec: int
    ):
        """
        Create a trigger that sends MOTION events at intervals
        shorter than the NO_MOTION timer.

        Args:
            sid: Motion sensor ID.
            n: Number of MOTION events to send.
            noMotionSec: NO_MOTION timer duration in seconds.

        Returns:
            A trigger function that sends the configured MOTION events.
        """
        def _trigger():

            if n <= 0:
                self.log.error(
                    "[Phase-1] Invalid MOTION iteration count: "
                    f"{n}"
                )
                return

            if noMotionSec <= 0:
                self.log.error(
                    "[Phase-1] no_motion_seconds must be > 0"
                )
                return

            # Keep MOTION interval safely below the NO_MOTION timeout.
            reset_interval = max(
                0.1,
                float(noMotionSec) -
                self.MOTION_RESET_MARGIN_S
            )

            self.log.info(
                f"[Phase-1] MOTION reset interval="
                f"{reset_interval:.1f}s, "
                f"no_motion_seconds={noMotionSec}"
            )

            for i in range(n):
                self.log.info(
                    f"[Phase-1] Sending MOTION "
                    f"{i + 1}/{n} to sensor_id={sid}"
                )

                self.motionSensorDevice.triggerMotionEvent(
                    sensorId=sid
                )

                # Do not wait after the final MOTION.
                # The final MOTION starts/restarts the NO_MOTION timer.
                # C++ will wait for the resulting NO_MOTION event.
                if i < n - 1:

                    self.log.info(
                        f"[Phase-1] Waiting "
                        f"{reset_interval:.1f}s before next MOTION"
                    )

                    time.sleep(reset_interval)

            self.log.info(
                "[Phase-1] Final MOTION sent. "
                "C++ will wait for NO_MOTION timer expiry."
            )

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

        with open(self.paths["l3TestConfigFile"], 'r') as fh:
            l3cfg = yaml.safe_load(fh)

        if not l3cfg:
            self.log.error("[NoMotionEventDetection] L3 configuration is empty")
            return False

        try:
            node = l3cfg['motionsensor']['L3']['EventDetection']
        except KeyError as exc:
            self.log.error(f"[NoMotionEventDetection] Missing YAML configuration key: {exc}")
            return False

        configured_max_events = int(node.get('max_expected_events', 1))
        configured_timeout_ms = int(node.get('event_timeout_ms', 30000))
        no_motion_seconds = int(node.get('no_motion_seconds', 5))
        if configured_max_events <= 0:
            self.log.error("[NoMotionEventDetection] max_expected_events must be > 0 for Phase 1/Phase 2")
            return False

        if no_motion_seconds <= 0:
            self.log.error("[NoMotionEventDetection] no_motion_seconds must be > 0 for Phase 1/Phase 2")
            return False

        if configured_timeout_ms <= 0:
            self.log.error("[NoMotionEventDetection] event_timeout_ms must be > 0")
            return False

        event_max_wait_timeout_ms = self._calculate_no_motion_timeout_ms(no_motion_seconds, configured_timeout_ms)

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

            # ==========================================================
            # PHASE 1
            # ==========================================================
            self.log.info("====================================================")
            self.log.info("[Phase-1] NO_MOTION timer reset test")
            self.log.info(f"[Phase-1] sensor_id={sensor_id}")
            self.log.info(f"[Phase-1] no_motion_seconds={no_motion_seconds}")
            self.log.info(f"[Phase-1] MOTION iterations={self.MOTION_RESET_ITERATIONS}")
            self.log.info("[Phase-1] Expected NO_MOTION events=1")
            # max_expected_events describes the number of expected
            # NO_MOTION events, NOT the number of injected MOTION events.
            phase1_expected_events = 1
            reset_interval = max(0.1, float(no_motion_seconds) - self.MOTION_RESET_MARGIN_S)

            phase1_timeout_ms = int(
                (
                    (self.MOTION_RESET_ITERATIONS - 1) * reset_interval
                    + no_motion_seconds
                    + self.FINAL_TIMEOUT_EXTRA_S
                    + 2.0
                ) * 1000 )

            self.log.info(
                f"[Phase-1] Calculated C++ timeout={phase1_timeout_ms}ms "
                f"(motions={configured_max_events}, "
                f"reset_interval={reset_interval:.1f}s, "
                f"no_motion_seconds={no_motion_seconds})"
            )
            self._write_event_config(
                l3cfg, sensor_index, 'NO_MOTION',
                phase1_expected_events, phase1_timeout_ms, no_motion_seconds)

            result = self.runL3Test(
                "NoMotionTimerReset",
                triggerCallable=self._make_timer_reset_trigger(
                    sensor_id,
                    self.MOTION_RESET_ITERATIONS,
                    no_motion_seconds
                ),
                triggerDelay=1
            )

            if not result:
                self.log.error(f"[Phase-1] FAILED for sensor_id={sensor_id}")
                all_passed = False
            else:
                self.log.info(f"[Phase-1] PASSED for sensor_id={sensor_id}")

            # ==========================================================
            # PHASE 2
            # ==========================================================

            self.log.info("====================================================")
            self.log.info("[Phase-2] NO_MOTION timer normal expiry test")
            self.log.info(f"[Phase-2] sensor_id={sensor_id}")
            self.log.info(f"[Phase-2] no_motion_seconds={no_motion_seconds}")
            self.log.info("[Phase-2] No MOTION injection")
            self.log.info("[Phase-2] Expected NO_MOTION events=1")
            phase2_expected_events = 1

            self._write_event_config(
                l3cfg,
                sensor_index,
                'NO_MOTION',
                phase2_expected_events,
                event_max_wait_timeout_ms,
                no_motion_seconds
            )
            result = self.runL3Test("NoMotionTimerReset", triggerCallable=None,triggerDelay=0)

            if not result:
                self.log.error(f"[Phase-2] FAILED for sensor_id={sensor_id}")
                all_passed = False
            else:
                self.log.info(f"[Phase-2] PASSED for sensor_id={sensor_id}")

            # ==========================================================
            # PHASE 3
            # ==========================================================

            self.log.info("====================================================")
            self.log.info("[Phase-3] NO_MOTION disabled test")
            self.log.info(f"[Phase-3] sensor_id={sensor_id}")
            self.log.info("[Phase-3] no_motion_seconds=0")
            self.log.info("[Phase-3] Expected NO_MOTION events=0")
            phase3_expected_events = 0
            phase3_timeout_ms = self.PHASE3_TIMEOUT_MS

            self._write_event_config(
                l3cfg,
                sensor_index,
                'NO_MOTION',
                phase3_expected_events,
                phase3_timeout_ms,
                0
            )

            self.log.info(f"[Phase-3] C++ event timeout={phase3_timeout_ms}ms")
            result = self.runL3Test("NoMotionTimerReset", triggerCallable=None, triggerDelay=0)
            if not result:
                self.log.error(f"[Phase-3] FAILED for sensor_id={sensor_id}")
                all_passed = False
            else:
                self.log.info(f"[Phase-3] PASSED for sensor_id={sensor_id}")

        self._write_event_config(
                l3cfg,
                sensor_index,
                'MOTION',
                configured_max_events,
                configured_timeout_ms,
                configured_max_events
            )
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
