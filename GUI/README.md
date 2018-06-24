## Subsystems GUI

Holds the python and arduino code for the subsystems board demo/diagnostics.

## How to Use
* Download the UMSATSGUI.exe and run it to start the GUI. The GUI is self-contained nothing else needs to be installed to run it.
* Download the folowing arduino libraries:
  * https://github.com/collin80/due_can
  * https://github.com/collin80/can_common
  * https://github.com/greiman/FreeRTOS-Arduino
* The first 2 libraries should be downloaded as a zip file then added using the "add .ZIP Library" option in the Arduino IDE.
* The 3rd library FreeRTOS can be found by searching for it in the Library Manager in the Arduino IDE. Install the FreeRtos library by Richard Barry.
* You should now be able to compile and upload the UMSATS_GUI.ino file to the Due.

* Once the arduino sketch is running and the GUI is open, you can select the correct serial port and baudrate(9600) in the settings tab of the GUI.
* In the GUI Commands box you should see "Initialize buffers.Finished Setup", this means the arduino code is working.
* Now you can enter values in the boxes to the right and use the buttons to send them:
  * In the power tab you can send a power reading to the CDH board. This simulates what the CDH would receive from the power module.
  * In the payload tab, you can send data from the payload module to the CDH. 
  * In the ground control tab, you can send time delayed tasks to the CDH. To send a message to turn on/off the payload wells , choose a well and enter the number of seconds to delay, the click the on or  off button. To to CDH to dump the payload data, enter a time then click the dump payload button. Lastly to request a reading from one of the ADCs, pick an ADC and enter a time then hit the send button.
  
* When sending anything from the ground control tab, choose a small value for time, since the countdown will be printed out on the CDH Scheduler message box.
* In the Communications tab, you can start/stop the radio demo. Make sure you stop the radio demo before removing power to any of the boards, after the stop button is clicked the code must still receive one last transmission or the code/GUI will freeze. It's probably a good idea to run  the radio demo after showing all the CDH commands.



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
