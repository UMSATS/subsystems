# Subsystems Software
Contains software/documents related to the subsystems board for UMSATS 2018.

## Rules
1. Do not commit code to the main branch embedded software without first verifying that it is working on the subsystems board.
2. Update file headers for files that are changed, and add file headers for new files.
3. You are free to branch or fork this repo as you please. In fact, making a branch on this repo is encouraged, as you can regularly commit to the branch, and then make a pull request when you are ready to merge to the main branch.
4. Document any new information to the Sandbox Wiki.
5. When working on software meant to run on the subsystems board, keep hardware specific code as separate as possible to make it easy to move code between platforms. Similarly, keep only Arduino specific code in Arduino \*.ino files.
6. Do NOT deallocate memory on the embedded software. If memory allocation is unavoidable, it can be "allocated" via the FreeRTOS vPortMalloc function.

## Atmel Studio
Atmel Studio 7 is required to build and run the Atmel Studio project for this repo. It can be downloaded from here: http://www.microchip.com/mplab/avr-support/atmel-studio-7

## Arduino Model
For non-hardware specific portions of the software, they can be developed with the Arduino Due model in:
* source/SubSystemsBoardSoftware/src/cdh

The Arduino model requires the following libraries:
* https://github.com/collin80/due_can
* https://github.com/collin80/can_common
* https://github.com/greiman/FreeRTOS-Arduino 

## Additional Documentation
* https://github.com/UMSATS/Sandbox/wiki - General Purpose Wiki with information about using Atmel Studio and the Arduino Due.
* https://github.com/UMSATS/Documents

Note that if/when it comes time to obsolete this repo (e.g. due to using a new chip), it can be renamed to "subsystems-2018", and the README should be updated to reflect that this repo is obsolete and point to the next repo.
