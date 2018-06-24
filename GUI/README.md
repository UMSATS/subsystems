## Subsystems GUI

Holds the python and arduino code for the subsystems board demo/diagnostics.

## How To Build

### Python GUI

Have the following installed:
* Python 3.6.1
* PySerial 3.4
* Pyinstaller 3.3.1

Create the executable by running:
  * pyinstaller --onefile UMSATSGUI.py

### Arduino Code

Have the following libraries installed:
* https://github.com/collin80/due_can
* https://github.com/collin80/can_common
* https://github.com/greiman/FreeRTOS-Arduino

Upload the UMSATS_GUI.ino to the Arduino Due.
