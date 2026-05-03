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
sys.path.append(os.path.join(dir_path, "../"))
sys.path.append(dir_path)
sys.path.append(os.path.join(dir_path, "raft"))

from raft.framework.core.logModule import logModule
from vts_motionsensor_helper import motionSensorHelperClass


class VTS_MOTIONSENSOR_L3_NoMotionEventDetection(motionSensorHelperClass):
    """
    L3 test: Verify that the motion sensor correctly reports a NO_MOTION event
    when started in NO_MOTION operational mode and a no-motion stimulus arrives.
    Also verifies that unregistering the event listener stops event delivery.
    """

    def __init__(self, log: logModule = None):
        self.testName = "MotionSensor_L3_NoMotionEventDetection"
        qcId = "2"
        super().__init__(self.testName, qcId, log)

    def testFunction(self) -> bool:
        """
        Main test function.

        Flow:
          1. Upload L3 YAML config to target.
          2. Schedule a NO_MOTION event trigger 3 s after test start.
          3. Run the NoMotionEventDetection test case on the DUT.
          4. Return pass/fail.

        Returns:
            bool: True if the test case passed, False otherwise.
        """
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        triggerThread = self.triggerEventAfterDelay(
            self.motionSensorDevice.triggerNoMotionEvent,
            delay=3.0,
            sensorId=0)

        return self.runL3Test("NoMotionEventDetection", triggerThread)


if __name__ == "__main__":
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summary"
    summaryLog = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_MOTIONSENSOR_L3_NoMotionEventDetection(summaryLog)
    test.run(False)
