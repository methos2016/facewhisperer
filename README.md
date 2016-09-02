FaceWhisperer
-------------

FaceWhisperer is a hardware add-on for the [ChipWhisperer](https://newae.com/tools/chipwhisperer/) side-channel analysis tool, for working with devices that primarily communicate over USB. The goal is to create a USB host controller scripted with an experiment, all running totally synchronous with the target. This should give predictable timing each time the experiment is run from a target reset.

The (untested) goal is to use standard USB requests as a data exfiltration method while glitching the device code that fulfills these requests.

The experiments are scripted from an ATxmega128 processor, same as the one included on ChipWhisperer-Lite. The USB host is a MAX3241E, inspired by Travis Goodspeed's [Facedancer21](http://goodfet.sourceforge.net/hardware/facedancer21/) tool.

For keeping the target device in sync, this board provides a 12 MHz clock output and an open-collector reset output.

This is a quick hack that builds on the work and inspiration of several great projects:

- [GoodFET](http://goodfet.sourceforge.net)
- [ChipWhisperer](https://newae.com/tools/chipwhisperer)
- [Xmegaduino](https://github.com/Xmegaduino/Xmegaduino)
- [Arduino USB Host Shield library](https://github.com/felis/USB_Host_Shield_2.0)
