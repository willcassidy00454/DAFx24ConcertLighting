# Quick Start

1. Connect the Enttec DMX USB Pro (or other DMX USB interface) to your computer system.
2. Open ```DMXController.maxpat``` and ```DMXParser.maxpat```.
3. Select the USB serial port the interface is connected to in ```DMXParser.maxpat```. This patcher must remain open during operation.
4. In ```DMXController.maxpat```, toggle on `DMX send on/off' (found at the top-left of the Globals panel).
5. To turn the lasers on, toggle on `Init lasers' on the Laser Control panel.
6. To control the LEDs with audio:
- First, connect the computer system to the Dante network (or other audio interface)
- Select the audio interface in Max and unmute audio in Max (bottom right IO button)
- Toggle on `Listen to audio input for DMX intensity', found at the top-left of the Audio panel.
