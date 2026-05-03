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

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(dir_path)
sys.path.append(os.path.join(dir_path, "../"))

from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils
from motionController.MotionSensorControllerInterface import MotionSensorControllerInterface


class virtualMotionSensor(MotionSensorControllerInterface):
    """
    Virtual MotionSensor controller that sends curl commands to the vcomponent
    control-plane endpoint to simulate motion sensor events.
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

    def _sendCommand(self, yaml_content: str):
        """
        Sends a YAML command string to the vcomponent control-plane endpoint.

        Args:
            yaml_content (str): YAML payload to POST.
        """
        escaped = yaml_content.replace('"', '\\"')
        cmd = (
            f'curl -X POST '
            f'-H "Content-Type: application/x-yaml" '
            f'--data-binary "{escaped}" '
            f'"http://localhost:{self.port}/api/postKVP"'
        )
        self.session.write(cmd)

    def triggerMotionEvent(self, sensorId: int):
        """
        Simulate a MOTION event for the given sensor.

        Args:
            sensorId (int): ID of the sensor to trigger.
        """
        yaml_content = (
            "IMotionSensor:\n"
            "  command: MOTION_SENSOR_EVENT\n"
            f"  sensor_id: {sensorId}\n"
            "  event: MOTION\n"
        )
        self._sendCommand(yaml_content)

    def triggerNoMotionEvent(self, sensorId: int):
        """
        Simulate a NO_MOTION event for the given sensor.

        Args:
            sensorId (int): ID of the sensor to trigger.
        """
        yaml_content = (
            "IMotionSensor:\n"
            "  command: MOTION_SENSOR_EVENT\n"
            f"  sensor_id: {sensorId}\n"
            "  event: NO_MOTION\n"
        )
        self._sendCommand(yaml_content)

    def triggerDeepSleepStart(self, sensorId: int):
        """
        Simulate the system entering deep sleep for the given sensor.

        Args:
            sensorId (int): ID of the sensor affected.
        """
        yaml_content = (
            "IMotionSensor:\n"
            "  command: MOTION_SENSOR_DEEP_SLEEP_START\n"
            f"  sensor_id: {sensorId}\n"
        )
        self._sendCommand(yaml_content)

    def triggerDeepSleepEnd(self, sensorId: int):
        """
        Simulate the system exiting deep sleep for the given sensor.

        Args:
            sensorId (int): ID of the sensor affected.
        """
        yaml_content = (
            "IMotionSensor:\n"
            "  command: MOTION_SENSOR_DEEP_SLEEP_END\n"
            f"  sensor_id: {sensorId}\n"
        )
        self._sendCommand(yaml_content)
