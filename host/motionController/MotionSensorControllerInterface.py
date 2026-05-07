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

from abc import ABCMeta, abstractmethod


class MotionSensorControllerInterface(metaclass=ABCMeta):
    """
    Abstract base class for MotionSensor controller implementations.
    Both virtual (vDevice) and actual hardware implementations must inherit this.
    """

    def __init__(self, session, prompt: str = "~#", port: int = 8080):
        self.session = session
        self.prompt = prompt
        self.port = port

    @abstractmethod
    def triggerMotionEvent(self, sensorId: int):
        """
        Simulate a MOTION event for the given sensor.

        Args:
            sensorId (int): ID of the sensor to trigger.
        """
        pass

    @abstractmethod
    def triggerNoMotionEvent(self, sensorId: int):
        """
        Simulate a NO_MOTION event for the given sensor.

        Args:
            sensorId (int): ID of the sensor to trigger.
        """
        pass

    @abstractmethod
    def triggerDeepSleepStart(self, sensorId: int):
        """
        Simulate the system entering deep sleep for the given sensor.

        Args:
            sensorId (int): ID of the sensor affected.
        """
        pass

    @abstractmethod
    def triggerDeepSleepEnd(self, sensorId: int):
        """
        Simulate the system exiting deep sleep for the given sensor.

        Args:
            sensorId (int): ID of the sensor affected.
        """
        pass

    @abstractmethod
    def simulateIdleDeepSleep(self, sensorId: int, idle_seconds: float):
        """
        Simulate the platform idle-timer that triggers deep sleep.

        On a real device the SoC enters deep sleep automatically after a
        configurable inactivity period (typically 10 minutes).  Because
        there is no dedicated deep-sleep component in the current stack,
        the Python test host acts as the system sleep manager:

          1. Wait *idle_seconds* (no events injected = simulated inactivity).
          2. Send MOTION_SENSOR_DEEP_SLEEP_START to the vcomponent so its
             internal ``inDeepSleep`` flag is set.

        The caller is then responsible for injecting a MOTION event and
        calling triggerDeepSleepEnd() when the wake sequence is complete.

        Args:
            sensorId (int): ID of the sensor affected.
            idle_seconds (float): Seconds to wait before entering deep sleep
                                  (simulates the platform idle timeout).
        """
        pass
