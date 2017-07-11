# CameraSYNC
A project under the FH Ravensburg-Weingarten

The project involves the synchronization between two Cameras. The first camera is O3M151 from ifm electronics. It is configured with the help of ifm VisionAssistant software. The following are the configurations which may be important for the project.

CAN Protocol : SAE J1939
Camera Address : 239
CAN Speed : 250KBPS
Capture Frequency : 50Hz

The .dbc file elaborates about the J1939 CAN database and can be opened and extracted with the help of MATLAB-Vehicle Network Toolbox.
The camera is configured as a master, sending SYNC data(Time Stamp) to all slaves connected on the CAN Bus. In addition to that, the master sends the Global Data(Time Stamp) of the last captured photo frame. This data is received with the following CAN ID's

CAN ID for SYNC :   83820783
CAN ID for Global : 83821039

These are the default CAN ID's, but can be changed and configured differently.

The microprocessor used for interacting and processing these CAN messages is ATmega328p on a standard Arduino Uno development board. Along with that is procured, a CAN Bus Shield which allows communication between the CAN controller MCP2515 and serial UART of Arduino. The library files map the physical addresses and create different functions for initializing and performing communications on CAN.

IDS UI-3240-ML-C-HQ is the second camera which is configfured to capture frames on the receipt of a hardware trigger. It is connected to Arduino Uno board with the help of a HIROSE HR-25 cable. The +5V supply and GND along with the TRIGGER and its GND are provided from the Arduino. The trigger pin is currently set at Digital I/O Pin 7.

Additionally to test the logic with a simulator, another Arduino simulating the behavior of the camera on the CAN Bus has been set up. The default capture frequency has been set at 1 Hz.
