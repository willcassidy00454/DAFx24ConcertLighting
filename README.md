# Intro
This is a C-based Max/MSP DMX controller, developed for the visual component of the concert at the Digital Audio Effects Conference 2024.

The controller is used to modulate lighting (60 LED fixtures and 4 laser fixtures) using 10 channels of audio input in addition to manual user control. The idea is to map the lighting intensity to the direction of the sound emitted from 10 loudspeakers in a horizontal-plane ring.

A lot of this is hard-coded for the fixtures we're using, but the principle should work with any DMX fixture.

# Quick Start

1. Connect the Enttec DMX USB Pro (or other DMX USB interface) to your computer system.
2. Open ```DMXController.maxpat``` and ```DMXParser.maxpat```.
3. Select the USB serial port the interface is connected to in ```DMXParser.maxpat```. This patcher must remain open during operation.
4. In ```DMXController.maxpat```, toggle on `DMX send on/off' (found at the top-left of the Globals panel).
5. To turn the lasers on, toggle on `Init lasers' on the Laser Control panel.
6. To control the LEDs with audio:
- First, connect the computer system to the Dante network (or other audio interface)
- Select the audio interface in Max and enable audio IO (bottom right IO button)
- Toggle on `Listen to audio input for DMX intensity', found at the top-left of the Audio panel.
