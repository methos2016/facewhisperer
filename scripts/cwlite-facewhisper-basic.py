#!/usr/bin/env python

from chipwhisperer.common.api.CWCoreAPI import CWCoreAPI
from chipwhisperer.common.scripts.base import UserScriptBase
from chipwhisperer.capture.api.programmers import XMEGAProgrammer
import os


def getFirmwarePath(name):
    return os.path.join(os.path.dirname(__file__), '../firmware/%s/%s.hex' % (name, name))


class UserScript(UserScriptBase):
    _name = "ChipWhisperer-Lite: Facewhisperer basic test"
    _description = "Getting the Facewhisperer going"

    def run(self):
        for cmd in [
            ['Generic Settings', 'Scope Module', 'ChipWhisperer/OpenADC'],
            ['Generic Settings', 'Target Module', 'Simple Serial'],
            ['Generic Settings', 'Trace Format', 'ChipWhisperer/Native'],
            ['Simple Serial', 'Connection', 'NewAE USB (CWLite/CW1200)'],
            ['ChipWhisperer/OpenADC', 'Connection', 'NewAE USB (CWLite/CW1200)'],
            ]:
            self.api.setParameter(cmd)

        self.api.connect()

        # Flash the firmware
        xmega = XMEGAProgrammer()
        xmega.setUSBInterface(self.api.getScope().scopetype.dev.xmega)
        xmega.find()
        xmega.erase()
        xmega.program(getFirmwarePath('usb-descriptor-simple'), memtype='flash', verify=True)
        xmega.close()

        # Capture parameters
        for cmd in [
            ['Simple Serial', 'Load Key Command', u''],
            ['Simple Serial', 'Go Command', u'\n'],
            ['Simple Serial', 'Output Format', u'$GLITCH$'],

            ['CW Extra Settings', 'Trigger Pins', 'Target IO4 (Trigger Line)', True],
            ['CW Extra Settings', 'Target IOn Pins', 'Target IO1', 'Serial RXD'],
            ['CW Extra Settings', 'Target IOn Pins', 'Target IO2', 'Serial TXD'],
            ['CW Extra Settings', 'Target HS IO-Out', 'CLKGEN'],

            ['OpenADC', 'Clock Setup', 'CLKGEN Settings', 'Desired Frequency', 12e6],
            ['OpenADC', 'Clock Setup', 'ADC Clock', 'Source', 'CLKGEN x4 via DCM'],
            ['OpenADC', 'Trigger Setup', 'Total Samples', 24400],
            ['OpenADC', 'Trigger Setup', 'Pre-Trigger Samples', 0],
            ['OpenADC', 'Trigger Setup', 'Offset', 14100],
            ['OpenADC', 'Gain Setting', 'Setting', 38],
            ['OpenADC', 'Trigger Setup', 'Mode', 'rising edge'],
            ['OpenADC', 'Clock Setup', 'ADC Clock', 'Reset ADC DCM', None],

            ['Generic Settings', 'Acquisition Settings', 'Number of Traces', 50],

            ['CW Extra Settings', 'HS-Glitch Out Enable (Low Power)', True],
            ['Glitch Module', 'Clock Source', 'CLKGEN'],
            ['Glitch Module', 'Single-Shot Arm', 'Before Scope Arm'],
            ['Glitch Module', 'Glitch Trigger', 'Ext Trigger:Continous'],
            ['Glitch Module', 'Output Mode', 'Glitch Only'],
            ['Glitch Explorer', 'Normal Response', u's.find("code 5 len 34 ") >= 0'],
            ]:
            self.api.setParameter(cmd)

        self.api.capture1()
        self.api.capture1()


if __name__ == '__main__':
    import chipwhisperer.capture.ui.CWCaptureGUI as cwc
    from chipwhisperer.common.utils.parameter import Parameter
    Parameter.usePyQtGraph = True
    api = CWCoreAPI()
    app = cwc.makeApplication("Capture")
    gui = cwc.CWCaptureGUI(api)
    api.runScriptClass(UserScript)
    app.exec_()
