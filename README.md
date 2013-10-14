DMCC_Library
============

Code library for the beaglebone DMCC (Dual Motor Controller Cape)

You can find the open source hardware (schematics, PCB, Eagle files) at
https://github.com/Exadler/DualMotorControlCape

This version includes preliminary code for the python interface.

To use the python interface, run the following commands on your Beaglebone:

opkg update

opkg install python-distutils

python setupDMCC.py install


To test the setMotor command in python, do the following:

import DMCC

DMCC.setMotor(0,1,5000)

(turns on motor 1 on board 0 with 5000 power)

DMCC.setMotor(0,1,-7000)

(reverse direction with 7000 power)

DMCC.setMotor(0,1,0)

(turn off the motor)

If you run into any problems, feel free to email us at support@exadler.com



