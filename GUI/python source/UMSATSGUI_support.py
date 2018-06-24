#! /usr/bin/env python
#  -*- coding: utf-8 -*-
#
# Support module generated by PAGE version 4.13
# In conjunction with Tcl version 8.6
#    May 31, 2018 11:43:13 PM


import sys

import serialFunctions as serialF
import datetime
import threading
import queue

LOGGING = False
commandList = {'POWER':1,'PAYLOAD':2,'GROUND':3,'ON':99,'OFF':100}
baudrateList = [9600,115200]
canReceive = ""
try:
    from Tkinter import *
except ImportError:
    from tkinter import *

try:
    import ttk
    py3 = False
except ImportError:
    import tkinter.ttk as ttk
    py3 = True
def set_Tk_var():
    global Com1Input,Com2Input,Baud1Input,Baud2Input
    Com1Input = StringVar()
    Com2Input = StringVar()
    Baud1Input= StringVar()
    Baud2Input= StringVar()


    global powerInput,payloadWellInput,payloadValueInput,groundWellInput,groundValueInput

    powerInput = StringVar()
    payloadWellInput = StringVar()
    payloadValueInput = StringVar()
    groundWellInput = StringVar()
    groundValueInput = StringVar()


    global che47
    che47 = StringVar()
    global che48
    che48 = StringVar()

    global cmd1Text,cmd2Text

    cmd1Text = StringVar()
    cmd2Text = StringVar()

def btnGroundControl_lclick(p1):
    #print(date)
    sys.stdout.flush()

def init(top, gui, *args, **kwargs):
    global w, top_level, root, running
    global serial1, serial2, baud1, baud2, com1, com2, logFile1, logFile2, date
    global closeEvent,threadQueue1,threadQueue2,thread1,thread2

    running = True


    #canReceive = ""
    w = gui
    top_level = top
    root = top

    baud1 = 115200
    baud2 = 9600

    com1 = ""
    com2 = ""

    logFile1 = None
    logFile2 = None

    top_level.bind_all('<<newSerialData>>', updateDisplay)

    closeEvent = threading.Event()  # This event signals to stop the read function when the program is closed

    threadQueue1 = queue.Queue()  # This queue is used to transfer data from the read thread to the gui to be displayed.
    thread1 = None

    threadQueue2 = queue.Queue()  # This queue is used to transfer data from the read thread to the gui to be displayed.
    thread2 = None

    date = str(datetime.datetime.today().month) + "-" + str(datetime.datetime.today().day) + "_" + str(datetime.datetime.today().hour) + "h" + str(datetime.datetime.today().minute) + "m"

    serial1 = None
    serial2 = None

def clearDisplay(e,display1,display2):
    print("clearing display")
    display1.delete("0",END)
    display2.delete("0",END)

def updateDisplay(event):
    """ This function prints any new serial data to the display.
    """
    display1 = event.widget.children['!labelframe'].children['!listbox']
    display2 = event.widget.children['!labelframe'].children['!listbox2']
    #print("updating display")
    global canReceive
    if not threadQueue1.empty():
        data1 = threadQueue1.get_nowait()  # Get any data from the queue.
        if LOGGING:logFile1.write(data1) ; # save it to log file
        data = data1.split(" ")
        id = data[1]
        if(id == "0x600"):
            length = int(data[3])
            #print("ID: "+ id+ " len: "+ str(length))
            for index in range(5,length+5):


                 characterHex = data[index].replace("0x","")
                 ascii = chr(int(characterHex,16))

                 if(ascii != "\0"):
                    canReceive += ascii


                 if(ascii == "\n"):
                     print("received new line")
                     display2.insert(END, canReceive[:-1])
                     display2.see(display2.size())
                     canReceive = ""

                 display2.see(display2.size())
        else:
            display1.insert(END, data1)
            display1.see(display1.size())  # This scrolls the window so the new text is always visible.

    if not threadQueue2.empty():
        data2 = threadQueue2.get_nowait()  # Get any data from the queue.
        if LOGGING : logFile2.write(data2)  # save it to log file;
        display2.insert(END, data2)
        display2.see(display2.size())  # This scrolls the window so the new text is always visible.


def destroy_window():
    # Function which closes the window.
    global top_level,closeEvent
    print("closing ")
    closeEvent.set()

    if logFile1 is not None:
        logFile1.close()
    if logFile2 is not None:
        logFile2.close()

    

    if serial1 is not None:
        serial1.close()

    if serial2 is not None:
        serial2.close()

    top_level.destroy()
    top_level = None

def command1_enter(e,display):
    #print('command1_enter\n')
    writeCommand(e.widget,display,1,cmd1Text.get())


def command2_enter(e,display):
    #print('command2_enter\n')
    if serial2 is not None:
        writeCommand(e.widget,display,2,cmd2Text.get())
    else:
        setupSerial()

def writeCommand(inputBox,display,can,data):
    """
       The function takes the text in the input box and sends it over the can bus.
       The text is also written to the display box.
       can should be either 1 or 2.
    """

    # Store the text in the cmdLine

    data = data.strip('\n')
    #print(data)
    command = None

    checkCommand = data.split(' ')

    if checkCommand[0] in commandList.keys():
        command = checkCommand[0]
        #print("commmand in list")
    value = checkCommand[1:]

    # Get a reference to the display associated with the terminal and write to correct serial port.
    if (can == 1) and (serial1 is not None):
        if command is None:
         #print("send serial")
         serial1.write(data)
         if LOGGING : logFile1.write("-> "+data);
        else:
            #print("sending command")
            #print("command: " +str(commandList[command]))
            full = ""
            extra = ""
            # if command== "POWER":
            #     extra = " "
            full += str(commandList[command])+extra+" "
            for v in value:
                #print("value: "+str(v))
                full += (str(v))+" "
            serial1.write(full)
            if LOGGING : logFile1.write("-> "+ full +"\n");


    elif (can == 2) and (serial2 is not None):


        if command is None:
        # print("send serial")
         serial2.write(data)
         if LOGGING: logFile2.write("-> "+data);
        else:
            #print("sending command")
            #print("command: " +str(commandList[command]))
            full = ""
            extra = ""
            # if command== "POWER":
            #     extra = " "
            full += str(commandList[command])+extra+" "
            for v in value:
                #print("value: "+str(v))
                full += (str(v))+" "
            serial2.write(full)
            if LOGGING : logFile2.write("-> "+ full +"\n");
    #print("ID of display: " + str(display))

    # Remove the text from the cmdLine
    inputBox.delete(0,END)


    # Scroll to insertion point if not visible.


    # Show command in display
    display.insert(END, "-> "+data + '\n')
    display.see(display.size())

def read(closedEvent, queue,serialPort):
    """ This function reads data from the serial port.
        When ever there is actual data, a <<newSerialEvent>> is generated.
        When a <<closedEvent>> is received the function is stopped.

    """

    if running:
        while (True):

            if (closedEvent.is_set()):
                #print("break")
                break
            data = serialPort.read()
            print(data)
            # If there is data to read put in queue and generate event to update display.
            if not (data is None) and (data != "") and (data != '\0'):

                queue.put(data)
                print("putting data in queue")
                try:

                    root.event_generate('<<newSerialData>>', when='tail')

                    print("newSerialData event generated")

                except:

                    print("newSerialData event generated FAILED")

                    break


def updateComList(e):
    #print("updating com port list")

    compPortList = serialF.serialFunctions.listCOMports()
    try:
        compPortList.remove(Com1Input.get())
    except:
        pass
    try:
        compPortList.remove(Com2Input.get())
    except:
        pass
    e.configure(values=compPortList)


def setupSerial(e,port,baudrate,bus):
    print("Starting serial communication on CAN bus {}.\nCOM port: {}\nBaudrate: {}".format(bus,port,baudrate))

    date = str(datetime.datetime.today().month) + "-" + str(datetime.datetime.today().day) + "_" + str(
        datetime.datetime.today().hour) + "h" + str(datetime.datetime.today().minute) + "m"

    global serial1, serial2,logFile1,logFile2,thread1, thread2
    if bus == 1:

    # If the Can bus already has a serial port , close it.
        if serial1 is not None:
            print("closed serial port")
            serial1.close()


        if LOGGING: logFile1 = open("CAN1Log_" + date + ".txt", 'w');
        serial1 = serialF.serialFunctions(baudrate, port)
        thread1 = threading.Thread(target=read, args=(closeEvent, threadQueue1, serial1))
        thread1.start()

    else:

        if serial2 is not None:
            serial2.close()


        if LOGGING : logFile2 = open("CAN2Log_" + date + ".txt", 'w');
        serial2 = serialF.serialFunctions(int(baudrate), port)
        thread2 = threading.Thread(target=read, args=(closeEvent, threadQueue2, serial2))
        thread2.start()



def sendPower(e,frame):


    #print("sending Power")
    writeCommand( frame.children['!entry'],frame.children['!listbox'], 1, "POWER " + powerInput.get())
    powerInput.set("")

def sendPayload(e,frame):

    #print("Sending payload data.")
    writeCommand(frame.children['!entry'],frame.children['!listbox'], 1, "PAYLOAD " + payloadWellInput.get()+" " + payloadValueInput.get())
    payloadWellInput.set("")
    payloadValueInput.set("")


def sendGroundOn(e,frame):

    #print("Sending ground command.")
    writeCommand(frame.children['!entry'],frame.children['!listbox'], 1, "GROUND " +"1 "+ groundWellInput.get()+" " + groundValueInput.get())
    groundWellInput.set("")
    groundValueInput.set("")


def sendGroundOff(e,frame):

    #print("Sending ground command.")
    writeCommand(frame.children['!entry'],frame.children['!listbox'], 1, "GROUND " + "2 " +groundWellInput.get()+" " + groundValueInput.get())
    groundWellInput.set("")
    groundValueInput.set("")

def sendDumpPayload(e,frame):
    print("sending dump payload command")

    writeCommand(frame.children['!entry'],frame.children['!listbox'], 1, "GROUND " + "3 " +"0"+" " + groundValueInput.get())
    groundWellInput.set("")
    groundValueInput.set("")

def showWells(combo):

    wells = [x for x in range(1,6)]
    combo.configure(values=wells)
    #print("showing wells")

if __name__ == '__main__':
    import UMSATSGUI
    UMSATSGUI.vp_start_gui()

