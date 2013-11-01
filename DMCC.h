//
// Copyright (C) 2013 - Exadler Technologies Inc., Sarah Tan
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

// DMCC.h - library for the DMCC functions

#ifndef DMCC
#define DMCC

// --------------------------
// Session functions - to start and end the user program
// --------------------------

// DMCCstart - Begins the session by connecting to the given board
//             Prints an error if connection fails
// Parameters: capeAddr - address of motor controller board specified [0-3]
// Returns: connection to the board (session number)
int DMCCstart(unsigned char capeAddr);

// DMCCend - Ends the given session/connection to the board
// Parameters: session - connection to board (value returned from DMCC start)
void DMCCend(int session);

// ---------------------------
// Cape Functions - to determine software updates and connected boards
// ---------------------------

// getVersionNumber - gets the version number of the given cape for
//                    compatible software
// Parameter: capeAddr - address of desired cape to check version number [0-3]
// Returns: Version number for the given cape
//          -1 - if an error occurs
int getVersionNumber(unsigned char capeAddr);

// --------------------------
// Quadrature Encoder Interface (QEI) functions
// --------------------------

// getQEI - Gets the QEI for the desired motor
//          Prints an error if QEI for motor is not received
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             motor - motor number desired
// Returns: QEI for given motor
//          always returns 0 if an error occurs
unsigned int getQEI(int fd, unsigned int motor);

// getQEIVel - Gets the QEI velocity for the desired motor
//             Prints an error if QEI velocity for motor is not received
// Parameters: fd - connection to the board (value returned from DMCC start)
//             motor - motor number desired
// Returns: QEI velocity for given motor
//          always returns 0 if an error occurs
int getQEIVel(int fd, unsigned int motor);

// getQEIDir - Gets the QEI direction for the desired motor
//             Prints an error if the QEI dir is not received
// Parameters: fd - connection to the board (value returned from DMCC start)
//             motor - motor number desired
// Returns: 1 if the QEI is in reverse
//          0 if the QEI is in forward
//          always returns -1 if an error occurs
int getQEIDir(int fd, unsigned int motor);

// reverseQEIDir - Sets the QEI direction for the desired motor
//             Prints an error if the QEI dir is not received
// Parameters: fd - connection to the board (value returned from DMCC start)
//             motor - motor number desired
//             dir -  1 if the QEI is in reverse
//                    0 if the QEI is in forward
void configQEIDir(int fd, unsigned int motor, int dir);

// resetQEI - Resets the QEI for the desired motor to 0
//            Prints an error if QEI was not reset
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             motor - motor number desired
void resetQEI(int fd, unsigned int motor);

// resetAllQEI - Resets the QEI for all motors
// Parameters: fd - connection to the board (value returned from DMCCstart)
void resetAllQEI(int fd);

// --------------------------
// Motor functions
// --------------------------

// getMotorCurrent - Gets the current for the desired motor
//                   Prints an error if the current for the 
//                      motor is not received
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             motor - motor number desired
// Returns: Current for given motor
//          always returns 0 if an error occurs
unsigned int getMotorCurrent(int fd, unsigned int motor);

// getMotorVoltage - Gets the motor supply voltage for all motors
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             motor - motor number desired
// Returns: Motor supply voltage
unsigned int getMotorVoltage(int fd);

// getTargetPos - Gets the target position for the desired motor
//                Prints an error if the target position for the
//                      motor is not received
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             motor - motor number desired
// Returns: Target position for given motor
//          always returns 0 if an error occurs
unsigned int getTargetPos(int fd, unsigned int motor);

// setTargetPos - Sets the target position for the desired motor
//                Prints an error if there is a problem
//                  communicating to motor
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             motor - motor number desired
//             position - motor position
void setTargetPos(int fd, unsigned int motor, unsigned int pos);




// getTargetVel - Gets the target velocity for the desired motor
//                Prints an error if the target velocity for the
//                      motor is not received
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             motor - motor number desired
// Returns: Target velocity for given motor
//          always returns 0 if an error occurs
int getTargetVel(int fd, unsigned int motor);

// setTargetVel - Sets the target velocity for the desired motor
//                Prints an error if there is a problem
//                  communicating to motor
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             motor - motor number desired
//             velocity - motor velocity
void setTargetVel(int fd, unsigned int motor, int vel);


// getMotorDir - Gets the direction for the desired motor
//               Prints an error if the motor dir is not received
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             motor - motor number desired
// Returns: 1 if the motor is in reverse
//          0 if the motor is in forward
//          always returns -1 if an error occurs
int getMotorDir(int fd, unsigned int motor);

// setMotorPower - Sets power for the desired motor 
//                 Prints an error if there is a problem 
//                      communicating to the motor
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             motor - motor number desired
//             pwm - power level
void setMotorPower(int fd, unsigned int motor, int pwm);

// setAllMotorPower - Sets the power for all motors
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             pwm - power level
void setAllMotorPower(int fd, int pwm);

// setMotorDir - Sets the direction for the desired motor
//               Prints an error if there is a problem
//                  communicating to the motor
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             motor - motor number desired
//             dir - direction (1 is reverse the dir, 0 is keep the dir)
void configMotorDir(int fd, unsigned int motor, int dir);

// --------------------------
// Wait functions
// --------------------------

// wait - Creates a delay in the program for a given number of microseconds
// Parameters: microseconds - number of microseconds to wait for
void DMCCwait(unsigned int microseconds);

// waitSec - Creates a delay in the program for a given number of seconds
//           Prints an error if the user asks for more than 2147 seconds
// Parameters: seconds - number of seconds to wait for
void DMCCwaitSec(unsigned int seconds);

// --------------------------
// Move functions
// --------------------------

// moveUntilPos - Powers on a motor until it has reached the desired position
//                Prints an error if there is a problem 
//                      communicating with the motor
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             pos - desired position for motor
//             motor - motor number desired
void moveUntilPos(int fd, unsigned int motor, unsigned int pos );

// moveUntilTime - Powers on a motor for a given time period
//                Prints an error if there is a problem
//                      communicating with the motor
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             time - desired wait time in microseconds
//             motor - motor number desired
//             power - power level for motor
void moveUntilTime(int fd, unsigned int motor, 
                     unsigned int power, unsigned int time );

// moveUntilVel - Powers on a motor until it has reached the desired velocity
//                Prints an error if there is a problem
//                      communicating with the motor
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             vel - desired velocity for motor
//             motor - motor number desired
void moveUntilVel(int fd, unsigned int motor, int vel );

// moveAllUntilPos - Power on both motors until they have both reached
//                      the desired position
//                   Prints an error if there is a problem
//                      communicating with the motor
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             pos - desired position for motors
void moveAllUntilPos(int fd, unsigned int pos);

// moveAllUntilTime - Power on both motors for a given time period
//                   Prints an error if there is a problem
//                      communicating with the motor
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             time - desired wait time in microseconds
//             power - power level for motor
void moveAllUntilTime(int fd, unsigned int power, unsigned int time );

// moveAllUntilPos - Power on both motors until they have both reached
//                      the desired velocity
//                   Prints an error if there is a problem
//                      communicating with the motor
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             vel - desired velocity for motors
void moveAllUntilvel(int fd, int vel);

// ---------------------------
// PID Constant Functions 
// WARNING: Do not change the constants unless you wish to change the overshoot
//          or slow down power behaviours
// ---------------------------

// getPIDConstants - Gets the PID constants and returns them through pointers
//                   Prints an error if there is a problem communicating
//                      with the motor
// Paramters: fd - connection to the board (value returned from DMCCstart)
//            posOrVel - 0 for getting the position PID
//                     - 1 for getting the velocity PID
//            P - constant for P in PID algorithm
//            I - constant for I in PID algorithm
//            D - constant for D in PID algorithm
//            motor - motor number desired
void getPIDConstants(int fd, unsigned int motor, unsigned int posOrVel, 
                            int *P, int *I, int *D);

// setPIDConstants - Set the PID constants for the motors
//                   Prints an error if there is a problem 
//                      communicating with the motor
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             posOrVel - 0 for changing the position PID
//                      - 1 for changing the velocity PID
//             P - constant for P in the PID algorithm
//             I - constant for I in the PID algorithm
//             D - constant for D in the PID algorithm
//             motor - motor number desired
void setPIDConstants(int fd, unsigned int motor, unsigned int posOrVel, 
                            int P, int I, int D);

// setDefaultPIDCOnstants - Set the PID constants to a default
//
void setDefaultPIDConstants(int fd);

#endif
