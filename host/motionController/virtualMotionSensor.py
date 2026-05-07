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

import os
import sys
import time
import yaml

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(dir_path)
sys.path.append(os.path.join(dir_path, "../"))

from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils
from motionController.MotionSensorControllerInterface import MotionSensorControllerInterface


class virtualMotionSensor(MotionSensorControllerInterface):
    """
    Virtual MotionSensor controller that sends YAML commands to the vcomponent
    control-plane endpoint via utPlaneController to simulate motion sensor events.
    """

    def __init__(self, session, prompt: str = "~#", port: int = 8080):
        """
        Initialises the virtual MotionSensor controller.

        Args:
            session: Console/SSH session object with a write() method.
            prompt (str): Shell prompt string.
            port (int): Control-plane HTTP port of the vcomponent (default 8080).
        """
        super().__init__(session, prompt, port)
        self.utilities = utBaseUtils()

    def _sendCommand(self, payload: dict):
        """
        Writes *payload* as YAML to a temp file on the device via a bash

        Args:
            payload (dict): Command payload to serialise and POST.
        """
        yaml_str = yaml.dump(payload)
        tmp = "/tmp/ms_cp.yaml"

        # Write YAML to a temp file using a bash heredoc.
        # sshConsole.write(list) sends each element as a separate line.
        heredoc_lines = [f"cat > {tmp} << 'YAML_EOF'"]
        heredoc_lines.extend(yaml_str.rstrip("\n").splitlines())
        heredoc_lines.append("YAML_EOF")
        self.session.write(heredoc_lines)

        # POST the file no quoting issues, newlines preserved exactly.
        self.session.write(
            f'curl -s -X POST -H "Content-Type: application/x-yaml" '
            f'--data-binary @{tmp} "http://localhost:{self.port}/api/postKVP"'
        )

    def triggerMotionEvent(self, sensorId: int):
        """
        Simulate a MOTION event for the given sensor.

        Args:
            sensorId (int): ID of the sensor to trigger.
        """
        self._sendCommand({
            "IMotionSensor": {
                "command": "MOTION_SENSOR_EVENT",
                "sensor_id": sensorId,
                "event": "MOTION",
            }
        })

    def triggerNoMotionEvent(self, sensorId: int):
        """
        Simulate a NO_MOTION event for the given sensor.

        Args:
            sensorId (int): ID of the sensor to trigger.
        """
        self._sendCommand({
            "IMotionSensor": {
                "command": "MOTION_SENSOR_EVENT",
                "sensor_id": sensorId,
                "event": "NO_MOTION",
            }
        })

    def triggerDeepSleepStart(self, sensorId: int):
        """
        Simulate the system entering deep sleep for the given sensor.

        Args:
            sensorId (int): ID of the sensor affected.
        """
        self._sendCommand({
            "IMotionSensor": {
                "command": "MOTION_SENSOR_DEEP_SLEEP_START",
                "sensor_id": sensorId,
            }
        })

    def triggerDeepSleepEnd(self, sensorId: int):
        """
        Simulate the system exiting deep sleep for the given sensor.

        Args:
            sensorId (int): ID of the sensor affected.
        """
        self._sendCommand({
            "IMotionSensor": {
                "command": "MOTION_SENSOR_DEEP_SLEEP_END",
                "sensor_id": sensorId,
            }
        })

    def simulateIdleDeepSleep(self, sensorId: int, idle_seconds: float):
        """
        Simulate the platform idle-timer that triggers deep sleep.

        Because there is no dedicated deep-sleep component, the Python host
        acts as the system sleep manager.  This method idles for
        *idle_seconds* (no events = inactivity) and then sends
        MOTION_SENSOR_DEEP_SLEEP_START to the vcomponent.

        Typical test sequence::

            # Device has been idle — system decides to enter deep sleep
            controller.simulateIdleDeepSleep(sensor_id, idle_seconds=5)

            # Physical motion detected during sleep
            controller.triggerMotionEvent(sensor_id)

            # Wake-up complete
            controller.triggerDeepSleepEnd(sensor_id)

        Args:
            sensorId (int):       ID of the sensor affected.
            idle_seconds (float): Seconds to wait before entering deep sleep
                                  (simulates the platform inactivity timeout;
                                  use a small value in tests, e.g. 5 s instead
                                  of the real 10 minutes).
        """
        time.sleep(idle_seconds)
        self.triggerDeepSleepStart(sensorId)
