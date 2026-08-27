#!/usr/bin/env python3
#** *****************************************************************************
# *
# * Copyright 2026 RDK Management
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# * http://www.apache.org/licenses/LICENSE-2.0
# *
#* ******************************************************************************

"""
L3 ActiveWindowBehavior test for the MotionSensor HAL.
"""

import os
import re
import sys
import time
import yaml
import datetime

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))
sys.path.append(dir_path)
sys.path.append(os.path.join(dir_path, "..", "raft"))

from raft.framework.core.logModule import logModule
from vts_motionsensor_helper import motionSensorHelperClass


# 10 window scenarios for the ActiveWindowBehavior test.
# dynamic: false  -> use window_start_seconds / window_end_seconds as-is.
# dynamic: true   -> compute actual times as (now_secs + offset) % 86400.
# expect_event    -> True  = event must arrive   (C++ asserts event received).
#                   False = event must be suppressed (C++ asserts no event received).
_WINDOW_SCENARIOS = [
    # Positive scenarios (window contains current time)
    # 1. Full-day window [00:00:00, 23:59:59] — always contains any time.
    {'name': 'full_day_window',          'dynamic': False, 'window_start_seconds': 0,     'window_end_seconds': 86399, 'expect_event': True},
    # 2. 2-hour window centred on current time [now-1h, now+1h].
    {'name': 'two_hour_centred_window',  'dynamic': True,  'offset_start_seconds': -3600,  'offset_end_seconds': 3600,  'expect_event': True},
    # 3. 10-minute window centred on current time [now-5m, now+5m].
    {'name': 'ten_minute_centred_window','dynamic': True,  'offset_start_seconds': -300,   'offset_end_seconds': 300,   'expect_event': True},
    # 4. 8-hour window centred on current time [now-4h, now+4h].
    {'name': 'eight_hour_centred_window','dynamic': True,  'offset_start_seconds': -14400, 'offset_end_seconds': 14400, 'expect_event': True},
    # 5. Fixed midnight-spanning window [22:00, 02:00].
    #    Skipped at runtime when the current time is outside [22:00, 02:00].
    {'name': 'midnight_wrap_window',     'dynamic': False, 'window_start_seconds': 79200,  'window_end_seconds': 7200,  'expect_event': True},
    # Suppression scenarios (window excludes current time)
    # 6. Window that has not yet opened (opens 30 s from now — after the 15 s Phase-1 timeout).
    {'name': 'window_not_yet_open',      'dynamic': True,  'offset_start_seconds': 30,     'offset_end_seconds': 3630,  'expect_event': False},
    # 7. Window that closed 1-2 hours ago.
    {'name': 'window_already_closed',    'dynamic': True,  'offset_start_seconds': -7200,  'offset_end_seconds': -3600, 'expect_event': False},
    # 8. Narrow window that closed 1-2 minutes ago.
    {'name': 'narrow_past_window',       'dynamic': True,  'offset_start_seconds': -120,   'offset_end_seconds': -60,   'expect_event': False},
    # 9. Narrow window that opens in 1-2 minutes.
    {'name': 'narrow_future_window',     'dynamic': True,  'offset_start_seconds': 60,     'offset_end_seconds': 120,   'expect_event': False},
    # 10. Far-future window that opens 2-3 hours from now.
    {'name': 'far_future_window',        'dynamic': True,  'offset_start_seconds': 7200,   'offset_end_seconds': 10800, 'expect_event': False},
]


class VTS_MOTIONSENSOR_L3_ActiveWindowBehavior(motionSensorHelperClass):

    def __init__(self, log: logModule = None, moduleName: str = None):
        self.testName = "MotionSensor_L3_ActiveWindowBehavior"
        super().__init__(self.testName, "3", log, moduleName)
        self.testParams = self._readMotionParameterFromProfile("activewindow")

    # ------------------------------------------------------------------
    # Helpers
    # ------------------------------------------------------------------

    @staticmethod
    def _resolve_window(entry: dict, now_secs: int) -> tuple:
        """
        Compute concrete (start_secs, end_secs) for a YAML window entry.

        For ``dynamic: true`` entries the actual times are:
            actual = (now_secs + offset) % 86400
        where offsets may be negative (past) or positive (future).

        For ``dynamic: false`` entries the YAML ``window_start_seconds`` and
        ``window_end_seconds`` values are used directly.

        Args:
            entry    (dict): One element from the ``active_windows`` list.
            now_secs (int):  Current time of day in seconds since midnight.

        Returns:
            (int, int): (window_start_seconds, window_end_seconds) both in [0, 86399].
        """
        if entry.get('dynamic', False):
            start = int((now_secs + int(entry['offset_start_seconds'])) % 86400)
            end   = int((now_secs + int(entry['offset_end_seconds']))   % 86400)
        else:
            start = int(entry['window_start_seconds'])
            end   = int(entry['window_end_seconds'])
        return start, end

    @staticmethod
    def _is_time_in_window(now_secs: int, start: int, end: int) -> bool:
        """Return True if *now_secs* falls inside [start, end] (with midnight wrap)."""
        if start <= end:
            return start <= now_secs <= end
        # Midnight-spanning window: [start, 86399] ∪ [0, end]
        return now_secs >= start or now_secs <= end

    def _log_window_enter_exit(self, name: str, start: int, end: int,
                               now_secs: int, expect_event: bool) -> None:
        """
        Emit window-enter / window-exit diagnostic lines into the VTS report.

        For every scenario this logs:
          • Whether the current time is currently inside or outside the window.
          • If inside:  seconds until the window closes  (EXIT countdown).
          • If outside: seconds until the window opens   (ENTER countdown) if
            calculable, or a note that it requires midnight arithmetic.
          • The test expectation (PASS=event / PASS=suppression).

        Args:
            name        (str):  Scenario name (for the log prefix).
            start       (int):  Window start seconds of day [0, 86399].
            end         (int):  Window end seconds of day   [0, 86399].
            now_secs    (int):  Current time of day in seconds.
            expect_event(bool): True = event expected; False = suppression expected.
        """
        start_h, start_m, start_s = start // 3600, (start % 3600) // 60, start % 60
        end_h,   end_m,   end_s   = end   // 3600, (end   % 3600) // 60, end   % 60
        now_h,   now_m,   now_s   = now_secs // 3600, (now_secs % 3600) // 60, now_secs % 60

        in_window = self._is_time_in_window(now_secs, start, end)
        wrap_str  = "(midnight-wrap)" if start > end else ""

        self.log.info(
            f"[WindowStatus] scenario={name} "
            f"window=[{start_h:02d}:{start_m:02d}:{start_s:02d}"
            f"->{end_h:02d}:{end_m:02d}:{end_s:02d}]{wrap_str} "
            f"currentTime={now_h:02d}:{now_m:02d}:{now_s:02d} "
            f"status={'INSIDE (window ENTERED)' if in_window else 'OUTSIDE (window NOT entered)'} "
            f"expect={'event_received' if expect_event else 'event_suppressed'}")

        if in_window:
            # Compute seconds until EXIT
            if start <= end:
                secs_to_exit = end - now_secs
            else:
                # now_secs >= start: exit at end (next day)
                if now_secs >= start:
                    secs_to_exit = (86400 - now_secs) + end
                else:
                    secs_to_exit = end - now_secs
            self.log.info(
                f"[WindowStatus] scenario={name} "
                f"[EXIT] window closes in {secs_to_exit} s")
        else:
            # Compute seconds until ENTER
            if start <= end:
                if now_secs < start:
                    secs_to_enter = start - now_secs
                else:
                    # Window already passed today
                    secs_to_enter = (86400 - now_secs) + start
            else:
                # Midnight-spanning; now_secs is in the gap [end+1, start-1]
                secs_to_enter = start - now_secs
            self.log.info(
                f"[WindowStatus] scenario={name} "
                f"[ENTER] window opens in {secs_to_enter} s")

    def _make_positive_trigger(self, sensor_id: int,
                               trigger_interval_s: float,
                               timeout_s: float) -> callable:
        """
        Build a trigger callable for positive scenarios (event expected in Phase 1).

        Phase-1 triggers are fired every ``trigger_interval_s`` until Phase-1
        succeeds (event delivered quickly) OR until ``timeout_s`` elapses.  The
        Phase-2 trigger is then sent ``phase_delay`` seconds after the
        Phase-1 period ends.

        Timing guarantee:
          Phase-2 trigger is always sent AFTER C++ ``resetEventQueue()`` + ``start()``
          + ``getNextEvent()`` sequence begins.  The callable runs Phase-1 for the
          full ``timeout_s`` window, which covers both the fast-success path
          (Phase-1 done in <1 s) and the unexpected timeout path.  After
          ``timeout_s + phase_delay`` the Phase-2 stimulus arrives safely
          inside the Phase-2 ``getNextEvent`` window.

        Args:
            sensor_id         (int):   Target sensor ID for the CP command.
            trigger_interval_s(float): Seconds between consecutive Phase-1 triggers.
            timeout_s         (float): Phase-1 event_timeout in seconds (from YAML).

        Returns:
            callable: No-argument callable for use as *triggerCallable*.
        """
        def _trigger():
            # Phase 1 — periodic triggers while window is open.
            # Run for the full timeout_s so the callable always finishes at the
            # same wall-clock point regardless of how quickly Phase-1 resolves.
            phase_delay = float(self.testParams["phase_delay_seconds"])
            deadline = time.monotonic() + timeout_s
            trigger_count = 0
            while True:
                self.motionSensorDevice.triggerMotionEvent(sensorId=sensor_id)
                trigger_count += 1
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    break
                time.sleep(min(trigger_interval_s, remaining))
            self.log.info(
                f"[trigger] Phase-1 period complete, sent {trigger_count} trigger(s); "
                f"waiting {phase_delay:.1f}s for Phase-2 setup (stop/clear/start)")
            # Wait for C++ to complete stop / clearActiveWindows / resetEventQueue
            # / start / enter getNextEvent before sending the Phase-2 stimulus.
            time.sleep(phase_delay)
            self.motionSensorDevice.triggerMotionEvent(sensorId=sensor_id)
            self.log.info("[trigger] Phase-2 trigger sent")
        return _trigger

    def _make_suppression_trigger(self, sensor_id: int,
                                  trigger_interval_s: float,
                                  timeout_s: float) -> callable:
        """
        Build a trigger callable for suppression scenarios (event gated in Phase 1).

        The callable fires MOTION events every ``trigger_interval_s`` for the full
        Phase-1 duration (``timeout_s`` seconds).  All events are suppressed by the
        HAL service active-window filter; the C++ ``getNextEvent`` eventually times
        out.  After the Phase-1 timeout the callable waits ``phase_delay``
        for Phase-2 setup, then fires a single Phase-2 trigger (no window -> passes).

        Args:
            sensor_id         (int):   Target sensor ID.
            trigger_interval_s(float): Seconds between consecutive triggers.
            timeout_s         (float): Phase-1 timeout in seconds (from YAML
                                       ``event_timeout_ms / 1000``).

        Returns:
            callable: No-argument callable for use as *triggerCallable*.
        """
        def _trigger():
            # Phase 1 — periodic triggers (all suppressed by window gating)
            phase_delay = float(self.testParams["phase_delay_seconds"])
            deadline = time.monotonic() + timeout_s
            trigger_count = 0
            while True:
                self.motionSensorDevice.triggerMotionEvent(sensorId=sensor_id)
                trigger_count += 1
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    break
                time.sleep(min(trigger_interval_s, remaining))
            self.log.info(
                f"[trigger] Phase-1 complete (all suppressed), sent {trigger_count} "
                f"trigger(s); waiting {phase_delay:.1f}s for Phase-2 setup")
            # Wait for C++ Phase-2 (stop + clearWindows + start)
            time.sleep(phase_delay)
            # Phase 2 — single trigger with no window restriction
            self.motionSensorDevice.triggerMotionEvent(sensorId=sensor_id)
            self.log.info("[trigger] Phase-2 trigger sent")
        return _trigger

    # ------------------------------------------------------------------
    # Main test
    # ------------------------------------------------------------------

    def testFunction(self) -> bool:
        """
        Main test function.

        Steps:
        1. Load ``vts_l3_MotionSensor.yaml``; read ``event_timeout_ms``,
           ``trigger_interval_seconds``, and the ``active_windows`` list.
        2. Retrieve sensor IDs from the HFP profile.
        3. Compute ``now_secs`` (current time of day in seconds).
        4. For each sensor vs each of the 10 window scenarios:
           a. Resolve actual window start/end (dynamic offset or static value).
           b. Check if the scenario is applicable:
              - Positive  static windows: skip if current time is not inside.
              - Suppression scenarios:    skip if current time is accidentally
                inside (offset calculation should prevent this; log a warning).
           c. Emit window-enter/exit diagnostic log lines.
           d. Write ``window_start_seconds`` / ``window_end_seconds`` to YAML
              and upload to the DUT.
           e. Build the appropriate trigger callable.
           f. Run the C++ test via ``runL3Test``.
           g. Evaluate result:
              - expect_event=True  : passed = (cpp_result == True)
              - expect_event=False : passed = (cpp_result == False) — inverted

        Returns:
            bool: True if every applicable scenario on every sensor passed.
        """
        with open(self.paths["l3TestConfigFile"], 'r') as fh:
            l3cfg = yaml.safe_load(fh)

        timeout_ms = int(self.testParams["event_timeout_ms"])
        trigger_interval_s = float(self.testParams["minimum_event_interval_seconds"])
        start_delay = float(self.testParams["event_simulator_ready_wait_seconds"])

        timeout_s         = timeout_ms / 1000.0
        window_scenarios  = _WINDOW_SCENARIOS

        if not window_scenarios:
            self.log.error("[ActiveWindowBehavior] No window scenarios defined — aborting")
            return False

        sensor_ids = self._readSensorIdsFromProfile()
        if not sensor_ids:
            self.log.error("[ActiveWindowBehavior] No sensor IDs found in profile — aborting")
            return False

        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        # Query DUT's current local time
        # Window computations MUST use the DUT's clock, not the host's clock.
        # The HAL service calls localtime_r(time(nullptr)) which uses the DUT's
        # system timezone.  If the host and DUT are in different timezones the
        # windows would be miscalculated and every positive scenario would fail.
        prompt   = self.cpe.get("prompt", "~#")
        dut_now_secs = None
        try:
            self.hal_session_1.write("date '+%H:%M:%S'")
            raw = self.hal_session_1.read_until(prompt, timeout=5)
            match = re.search(r'(\d{2}):(\d{2}):(\d{2})', raw)
            if match:
                dut_now_secs = (int(match.group(1)) * 3600
                                + int(match.group(2)) * 60
                                + int(match.group(3)))
        except Exception as exc:
            self.log.warn(f"[ActiveWindowBehavior] Could not query DUT time: {exc}")

        host_now  = datetime.datetime.now()
        host_secs = host_now.hour * 3600 + host_now.minute * 60 + host_now.second

        if dut_now_secs is None:
            self.log.warn(
                "[ActiveWindowBehavior] DUT time unavailable — falling back to host time")
            dut_now_secs = host_secs

        now_secs = dut_now_secs
        dut_h, dut_m, dut_s = now_secs // 3600, (now_secs % 3600) // 60, now_secs % 60

        self.log.info(
            f"[ActiveWindowBehavior] "
            f"hostTime={host_now.strftime('%H:%M:%S')} ({host_secs}s) "
            f"dutTime={dut_h:02d}:{dut_m:02d}:{dut_s:02d} ({dut_now_secs}s) "
            f"sensors={sensor_ids} scenarios={len(window_scenarios)}")

        all_passed = True

        for sensor_index, sensor_id in enumerate(sensor_ids):
            for entry in window_scenarios:
                scenario_name = entry.get('name', 'unnamed')
                expect_event  = bool(entry.get('expect_event', True))

                # Re-query DUT time per scenario
                # Dynamic windows (e.g. window_not_yet_open) use offsets from
                # *now*.  If we used the initial DUT time throughout, earlier
                # scenarios (each ~30 s long) would push the clock far enough
                # that a "opens in 30 s" window is already open by the time we
                # reach it.  Querying fresh each iteration keeps offsets valid.
                try:
                    self.hal_session_1.write("date '+%H:%M:%S'")
                    raw = self.hal_session_1.read_until(prompt, timeout=5)
                    m = re.search(r'(\d{2}):(\d{2}):(\d{2})', raw)
                    if m:
                        now_secs = (int(m.group(1)) * 3600
                                    + int(m.group(2)) * 60
                                    + int(m.group(3)))
                except Exception as exc:
                    self.log.warn(
                        f"[ActiveWindowBehavior] scenario={scenario_name}: "
                        f"Could not re-query DUT time — using previous value: {exc}")

                # Resolve actual window times
                start, end = self._resolve_window(entry, now_secs)

                # Applicability check
                in_window = self._is_time_in_window(now_secs, start, end)

                if expect_event and not in_window:
                    # Positive scenario: current time is outside the window.
                    # This can happen for 'midnight_wrap_window' when the test
                    # runs outside the [22:00, 02:00] slot.  Skip gracefully.
                    self.log.warn(
                        f"[ActiveWindowBehavior] SKIP sensor={sensor_index} "
                        f"scenario={scenario_name} — expect_event=True but current "
                        f"time ({now_secs}s) is outside computed window "
                        f"[{start}, {end}] (not applicable at this time of day)")
                    continue

                if not expect_event and in_window:
                    # Suppression scenario: offset calculation should guarantee
                    # the window is closed, but log a warning and skip to avoid
                    # a misleading C++ PASS that masks a real problem.
                    self.log.warn(
                        f"[ActiveWindowBehavior] SKIP sensor={sensor_index} "
                        f"scenario={scenario_name} — expect_event=False but current "
                        f"time ({now_secs}s) accidentally falls inside computed "
                        f"window [{start}, {end}] — offset may need adjustment")
                    continue

                # Enter/exit diagnostic
                self._log_window_enter_exit(
                    scenario_name, start, end, now_secs, expect_event)

                self.log.info(
                    f"[ActiveWindowBehavior] RUN sensor_index={sensor_index} "
                    f"sensor_id={sensor_id} scenario={scenario_name} "
                    f"window=[{start}, {end}] "
                    f"expect={'event' if expect_event else 'suppression'}")

                # Write scenario-specific window to YAML
                l3cfg['motionsensor']['L3']['EventDetection']['sensor_index']       = sensor_index
                l3cfg['motionsensor']['L3']['EventDetection']['operational_mode']   = 'MOTION'
                l3cfg['motionsensor']['L3']['EventDetection']['event_timeout_ms']    = timeout_ms
                l3cfg['motionsensor']['L3']['EventDetection']['window_start_seconds']  = start
                l3cfg['motionsensor']['L3']['EventDetection']['window_end_seconds']    = end
                l3cfg['motionsensor']['L3']['EventDetection']['max_expected_events']   = 1 if expect_event else 0
                with open(self.paths["l3TestConfigFile"], 'w') as fh:
                    yaml.dump(l3cfg, fh, default_flow_style=False, sort_keys=False)

                # Build trigger callable
                if expect_event:
                    trigger = self._make_positive_trigger(
                        sensor_id, trigger_interval_s, timeout_s)
                else:
                    trigger = self._make_suppression_trigger(
                        sensor_id, trigger_interval_s, timeout_s)

                # Run C++ test
                # C++ reads num_events from YAML: >0 = assert event arrived,
                # accordingly — it passes for both positive and suppression
                # scenarios, so no result inversion is needed here.
                cpp_result = self.runL3Test(
                    "ActiveWindowBehavior",
                    triggerCallable=trigger,
                    triggerDelay=start_delay)

                # Evaluate result
                passed = cpp_result

                if expect_event:
                    outcome = "PASSED" if passed else "FAILED"
                else:
                    outcome = (
                        "PASSED (suppression confirmed — HAL service gated all Phase-1 events)"
                        if passed else
                        "FAILED (event leaked through closed window — window gating broken)"
                    )

                self.log.info(
                    f"[ActiveWindowBehavior] sensor_index={sensor_index} "
                    f"scenario={scenario_name}: {outcome}")

                if not passed:
                    all_passed = False

        return all_passed


if __name__ == "__main__":
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_MOTIONSENSOR_L3_ActiveWindowBehavior(summaryLog)
    test.run(False)

