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
import threading

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))
sys.path.append(dir_path)
sys.path.append(os.path.join(dir_path, "..", "raft"))


from raft.framework.core.logModule import logModule
from vts_motionsensor_helper import motionSensorHelperClass


class VTS_MOTIONSENSOR_L3_DeepSleepAutonomy(motionSensorHelperClass):
    """
    L3 test: Verify the deep-sleep autonomy feature.

    Phase A — autonomy ENABLED:
        Host triggers deep-sleep-start → fires a MOTION event → triggers
        deep-sleep-end.  The C++ test expects the event to arrive.

    Phase B — autonomy DISABLED:
        Host triggers deep-sleep-start → fires a MOTION event.  The C++ test
        expects the event NOT to arrive within the timeout.
    """

    def __init__(self, log: logModule = None):
        self.testName = "MotionSensor_L3_DeepSleepAutonomy"
        qcId = "4"
        super().__init__(self.testName, qcId, log)

    def _deepSleepAutonomyThread(self, sensorId: int, sleepDuration: float = 2.0):
        """
        Background thread that:
          1. Sends DEEP_SLEEP_START.
          2. Waits sleepDuration seconds.
          3. Sends a MOTION event (while still in simulated deep sleep).
          4. Sends DEEP_SLEEP_END.

        This covers Phase A (autonomy enabled) where the event must be delivered,
        and the C++ test itself decides whether Phase B is applicable.

        Args:
            sensorId (int): ID of the sensor to stimulate.
            sleepDuration (float): Seconds to stay in simulated deep sleep.
        """
        time.sleep(2.0)  # Let the C++ test reach its blocking wait
        self.motionSensorDevice.triggerDeepSleepStart(sensorId)
        time.sleep(sleepDuration)
        self.motionSensorDevice.triggerMotionEvent(sensorId)
        time.sleep(0.5)
        self.motionSensorDevice.triggerDeepSleepEnd(sensorId)

    def testFunction(self) -> bool:
        """
        Main test function.

        Returns:
            bool: True if the test case passed, False otherwise.
        """
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        triggerThread = threading.Thread(
            target=self._deepSleepAutonomyThread,
            kwargs={"sensorId": 0, "sleepDuration": 2.0},
            daemon=True)
        triggerThread.start()

        return self.runL3Test("DeepSleepAutonomy", triggerThread)


if __name__ == "__main__":
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_MOTIONSENSOR_L3_DeepSleepAutonomy(summaryLog)
    test.run(False)
