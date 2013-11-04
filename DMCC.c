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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <linux/i2c-dev.h>

#include "DMCC.h"

// The maximum number of seconds to wait before killing a program
#define TIME_LIMIT 30

char *Compatible_Versions[] = {"05", "06"};

// ------------------------
// Threshold Values Depending on Encoder
// ------------------------
int QEI_Threshold_1 = 30;
int QEI_Vel_Threshold_1 = 5;
int QEI_Threshold_2 = 30;
int QEI_Vel_Threshold_2 = 5;

// -----------------------
// DMCC Session Functions
// -----------------------

// putByte - Writes the data byte at the given address
//           Prints an error when data is read incorrectly or invalid address
// Parameters: fd - file descriptor
//             addr - address of the desired write
void putByte(int fd, unsigned char addr, unsigned char data)
{
    unsigned char buf[2];
    buf[0] = addr;
    buf[1] = data;

    if (write(fd, buf, 2) != 2) {
        printf("Error in write address 0x02\n");
        close(fd);
        exit(1);
    }
}

// getByte - Reads the data byte at the given address
//           Prints an error when data is read incorrectly or invalid address
// Parameters: fd - file descriptor
//             addr - address of the desired read
unsigned char getByte(int fd, unsigned char addr)
{
    unsigned char buf[2];

    buf[0] = addr;
    if (write(fd, buf, 1) != 1) {
        printf("Error in write address 0x02\n");
        close(fd);
        exit(1);
    }

    if (read(fd, buf, 1) != 1) {
        printf("Error in read\n");
        close(fd);
        exit(1);
    }
    return buf[0];
}

// getWord - Reads the data word (2 bytes) at the given address
//           Prints an error when data is read incorrectly or invalid address
// Parameters: fd - file descriptor
//             addr - address of the desired read
unsigned int getWord(int fd, unsigned char addr)
{
    unsigned int result;

    result = ((unsigned int) getByte(fd, addr)) +
                ((unsigned int) (getByte(fd, addr+1) << 8));

    return result;
}

// getDWord - Reads the data word (4 bytes) at the given address
//           Prints an error when data is read incorrectly or invalid address
// Parameters: fd - file descriptor
//             addr - address of the desired read
// Returns: int from the 4 bytes read in
unsigned int getDWord(int fd, unsigned char addr)
{
    unsigned int result;
    result = ((unsigned int) getByte(fd, addr)) +
                ((unsigned int) getByte(fd, addr+1) << 8) +
                ((unsigned int) getByte(fd, addr+2) << 16) +
                ((unsigned int) getByte(fd, addr+3) << 24);
    return result;
}

// getNumberOfBytes - Reads a given number of bytes at the given address
//           Prints an error when data is read incorrectly or invalid address
// Parameters: fd - file descriptor
//             num - number of bytes to be read
//             addr - address of the desired read
// Returns: pointer to an array of strings for bytes read in
//          (Note: pointer must be freed when function called to
//                 prevent memory leaks)
char *getNumberOfBytes(int fd, int num, unsigned char addr) 
{
    if (num <= 0) {
        printf("Error: Too few bytes specified\n");
        return NULL;
    } 

    char *result = (char *)malloc(sizeof(char)*num);
    // Check that the memory has been allocated
    if (result == NULL) {
        printf("Error: memory allocation failure\n");
    }

    // Get bytes at the given address
    int i = 0;
    while(i < num) {
        result[i] = (char)(getByte(fd, addr+i));
        i++;
    }
    //printf("Bytes are %s\n", result);
    return result;
}

// validCapeAddress - checks if the given address has a DMCC cape connected
//                    and returns the directory for the cape
// Parameters: addr - address of the DMCC cape given
// Returns: Directory for cape if the address is valid
//          NULL - if the address is invalid
char *validCapeAddress(unsigned char addr)
{
    // Check if the DMCC cape is attached to the beaglebone at given addr
    if (addr == 0x2c) {
        return "1-0054";
    } else if (addr == 0x2d) {
        return "1-0055";
    } else if (addr == 0x2e) {
        return "1-0056";
    } else if (addr == 0x2f) {
        return "1-0057";
    } else {
        return NULL;
    }
}

int getVersionNumber(unsigned char capeAddr)
{
    capeAddr += 0x2c;
    // Check if the given address is valid and get the directory
    char *addr = validCapeAddress(capeAddr);
    if (addr == NULL) {
        printf("Error: invalid cape address specified\n");
        return -1;
    }

    // Find the desired file and store the path in file
    char file[40];
    strcpy(file, "/sys/bus/i2c/devices/");
    strcat(file, addr);
    strcat(file, "/eeprom");

    FILE *p = NULL;
    int pos = 1;
    int c;
   
    p = fopen(file, "rb");
    if (p == NULL) {
        printf("Error in opening file\n");
        return -1;
    }

    // Version number is located at the 40 byte 
    for (c = fgetc(p); c != EOF; c = fgetc(p)) {
        pos++;
        if (pos == 42) {
            int temp = fgetc(p);
            if (temp == EOF) {
                break;
            }
            temp = temp & 0x0f;
            c = c & 0x0f;
            if ((c > 0x09) || (temp > 0x09)) {
                printf("Error: Invalid version number\n");
                break;
            }
            fclose(p);
            return ((c * 10) + temp);
        }
    }
    fclose(p);
    printf("Error: No version number found\n");
    return -1;
}

// checkID - checks the ID of the cape to ensure connection to DMCC
// Parameters: fd - session from DMCC start
//             version - version number
// Returns: 0 - if the ID has a compatible version number
//          -1 - if an error occurs
//          Software version number - otherwise
int checkID(int fd, int version) 
{
    // Get ID from cape
    char *ID = getNumberOfBytes(fd, 16, 0xe0);
    if (ID == NULL) {
        printf("Error: No ID found for DMCC cape\n");
        free(ID);
        return -1;
    } 

    // Check ID is DMCC cape
    if (strncmp(ID, "DMCC Mk.", 7) != 0) {
        free(ID);
        printf("Error: Cape specified is not a DMCC cape\n");
        printf("ID: %s\n", ID);
        return -1;
    }

    // Get the software version number as an integer to 
    //      check if board version is the same
    int v = ((int)(ID[8] - 0x30) * 10) + (int)(ID[9] - 0x30);
    if (v != version) {
        printf("Error: Board and board base software are not the same\n");
        free(ID);
        return -1;
    }

    // Check that the version number is compatible
    char pV[2];
    pV[0] = ID[8];
    pV[1] = ID[9];
    // Software is compatible with versions listed in Compatible_Versions array
    int i = 0;
    while(Compatible_Versions[i] != NULL) {
        if (strncmp(pV, Compatible_Versions[i], 2) == 0) {
            free(ID);
            return 0;
        }
        i++;
    }
    
    // Get the software version number as an integer
    free(ID);
    return v;
}

int DMCCstart(unsigned char capeAddr)
{
    capeAddr+=0x2c;
    char filename[20];
    strcpy(filename,"/dev/i2c-1");

    int fd;

    //Opens a file descriptor to the board
    fd = open(filename, O_RDWR);
    if (fd <0) {
        printf("Error: cannot open %s\n",filename);
        exit(1);
    }
    if (ioctl(fd, I2C_SLAVE, capeAddr) < 0) {
        printf("Error: cannot ioctl to addr 0x%x\n", capeAddr);
        close(fd);
        exit(1);
    }
   
    int boardVer = getVersionNumber(capeAddr - 0x2c);
    int softVer = checkID(fd, boardVer);
    if (softVer != 0) { 
        printf("Error: software version and board version are incompatible\n");
        printf("       Board version = %d, Software version = %d\n",
                boardVer, softVer);
        printf("NOTE: if any version number is -1, the version is not found\n");
        close(fd);
        exit(1);
    }

    return fd;
}


void setDefaultPIDConstants(int fd)
{
    //Default PID constants
    setPIDConstants(fd, 1, 1, -19200, -8000, -150);
    setPIDConstants(fd, 2, 1, -19200, -8000, -150);
    setPIDConstants(fd, 1, 0, -5248, -75, -500);
    setPIDConstants(fd, 2, 0, -10000, -75, -500);
}



void DMCCend(int session)
{
    close(session);
}

unsigned int getQEI(int fd, unsigned int motor)
{
    // Send status update command (required call before reading)
    putByte(fd, 0xff, 0x00);

    if (motor == 1) {
        return getDWord(fd, 0x10);
    } else if (motor == 2) {
        return getDWord(fd, 0x14);
    } else {
        printf("Error: invalid motor number\n");
        return 0;
    }
}

int getQEIVel(int fd, unsigned int motor)
{
    // Send status update command (required call before reading)
    putByte(fd, 0xff, 0x00);
    
    int result;

    if (motor == 1) {
        result = (int)getWord(fd, 0x18);
        if (result > 32767) {
            return (-1 * (0xffff-result));
        }    
        return result;
    } else if (motor == 2) {
        result = getWord(fd, 0x1A);
        if (result > 32767) {
            return (-1 * (0xffff-result));
        }
            return result;
    } else {
        printf("Error: invalid motor number\n");
        return 0;
    }

}

int getQEIDir(int fd, unsigned int motor)
{
    // Send status update command
    putByte(fd, 0xff, 0x00);
    
    if (motor == 1) {
        return ((getByte(fd, 0x01) >> 2) & 1);
    } else if (motor == 2) {
        return ((getByte(fd, 0x01) >> 3) & 1);
    } else {
        printf("Error: invalid motor number\n");
        return -1;
    }
}

void configQEIDir(int fd, unsigned int motor, int dir)
{
    unsigned char byte1 = getByte(fd, 0x01);
    if (motor == 1) {
        byte1 = ((byte1 & 0x0b) | (unsigned char)((dir & 0x1) << 2));
        putByte(fd, 0x01, byte1);
    } else if (motor == 2) {
        byte1 = ((byte1 & 0x07) | (unsigned char)((dir & 0x1) << 3));
        putByte(fd, 0x01, byte1);
    } else {
        printf("Error: invalid motor number\n");
    }
}

void resetQEI(int fd, unsigned int motor)
{
    if (motor == 1) {
        putByte(fd, 0xff, 0x30);
    } else if (motor == 2) {
        putByte(fd, 0xff, 0x31);
    } else {
        printf("Error: invalid motor number\n");
    }
}

void resetAllQEI(int fd)
{
    putByte(fd, 0xff, 0x32);
}

void setMotorPower(int fd, unsigned int motor, int pwm)
{
    short int pwm16 = (short int) pwm;

    // Check for a valid motor selection
    if ((motor != 1) && (motor != 2)) {
        printf("Error: motor number must be 1 or 2\n");
        exit(1);
    }

    // Check for a valid power input (boundaries for motor control)
    if (pwm < -10000) {
        printf("min is -10000, %d is invalid\n",pwm);
        exit(1);
    }
    if (pwm > 10000) {
        printf("max is 10000, %d is invalid\n",pwm);
        exit(1);
    }

    // Set power to given motor
    putByte(fd, (motor * 2), ((unsigned char)(pwm16 & 0xff)));
    putByte(fd, ((motor * 2) + 1), (unsigned char) ((pwm16 & 0xff00) >> 8));
//    printf("Setting pwm to %d\n",pwm);

    // Send the set motor power command
    putByte(fd, 0xff, ((unsigned char) motor));
}

void setAllMotorPower(int fd, int pwm)
{
    short int pwm16 = (short int) pwm;

    // Check for a valid power input (boundaries for motor control)
    if (pwm < -10000) {
        printf("min is -10000, %d is invalid\n",pwm);
        exit(1);
    }
    if (pwm > 10000) {
        printf("max is 10000, %d is invalid\n",pwm);
        exit(1);
    }

    // Set power to motor 1
    putByte(fd, 0x02, ((unsigned char)(pwm16 & 0xff)));
    putByte(fd, 0x03, ((unsigned char)((pwm16 >> 8) & 0xff)));

    // Set power to motor 2
    putByte(fd, 0x04, ((unsigned char)(pwm16 & 0xff)));
    putByte(fd, 0x05, ((unsigned char)((pwm16 >> 8) & 0xff)));
   
    printf("Setting pwm to %d\n", pwm); 
    // Send the set motor power 1 and 2 command
    putByte(fd, 0xff, 0x03);
}

unsigned int getMotorCurrent(int fd, unsigned int motor)
{
    // Grab the data at the current point
    putByte(fd, 0xff, 0x00);

    // Get the current for the given motor number
    if (motor == 1) {
        return getWord(fd, 0x1C);
    } else if (motor == 2) {
        return getWord(fd, 0x1E);
    } else {
        printf("Error: invalid motor number\n");
    }
    return 0;
}

unsigned int getMotorVoltage(int fd)
{
    //Grab data at current point
    putByte(fd, 0xff, 0x00);
    
    //Get motor supply voltage
    return getWord(fd, 0x06);
}

unsigned int getTargetPos(int fd, unsigned int motor)
{
    // Grab data
    putByte(fd, 0xff, 0x00);

    if (motor == 1) {
        return getDWord(fd, 0x20);
    } else if (motor == 2) {
        return getDWord(fd, 0x24);
    } else {
        printf("Error: invalid motor number\n");
        return 0;
    } 
}

// setTargetPos - Sets the target position for the desired motor
//                Prints an error if there is a problem
//                  communicating to motor
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             motor - motor number desired
//             position - motor position
void setTargetPos(int fd, unsigned int motor, unsigned int pos)
{
    unsigned char start;

    // Perform check on motor number
    if (motor == 1) {
        start = 0x20;
        motor = 0x11;
    } else if (motor == 2){
        start = 0x24;
        motor = 0x12;
    } else {
        // neither motor 1 or 2 is called so exit the function
        printf("In set target pos\n");
        printf("Error: invalid motor number\n");
        return;
    }

    // Write the new position into the array
    putByte(fd, start, ((unsigned char)(pos & 0xff)));
    putByte(fd, (start + 1), ((unsigned char)((pos >> 8) & 0xff)));
    putByte(fd, (start + 2), ((unsigned char)((pos >> 16) & 0xff)));
    putByte(fd, (start + 3), ((unsigned char)((pos >> 24) & 0xff)));

    // Send the command to start the PID mode
    putByte(fd, 0xff, ((unsigned char) motor)); 
}

// setAllTargetPos - Sets the target position for all motors
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             position - motor position
void setAllTargetPos(int fd, unsigned int pos)
{
    // Write the new target position to the first motor
    putByte(fd, 0x20, ((unsigned char)(pos & 0xff)));
    putByte(fd, 0x21, ((unsigned char)((pos >> 8) & 0xff)));
    putByte(fd, 0x22, ((unsigned char)((pos >> 16) & 0xff)));
    putByte(fd, 0x23, ((unsigned char)((pos >> 24) & 0xff))); 

    // Write the new target position to the second motor
    putByte(fd, 0x24, ((unsigned char)(pos & 0xff)));
    putByte(fd, 0x25, ((unsigned char)((pos >> 8) & 0xff)));
    putByte(fd, 0x26, ((unsigned char)((pos >> 16) & 0xff)));
    putByte(fd, 0x27, ((unsigned char)((pos >> 24) & 0xff)));

    // Send the command to start the PID mode
    putByte(fd, 0xff, 0x13);
}

int getTargetVel(int fd, unsigned int motor)
{
    // Grab data
    putByte(fd, 0xff, 0x00);

    if (motor == 1) {
        return ((int) getDWord(fd, 0x20));
    } else if (motor == 2) {
        return ((int) getDWord(fd, 0x24));
    } else {
        printf("Error: invalid motor number\n");
        return 0;
    }
}

// setTargetVel - Sets the target velocity for the desired motor
//                Prints an error if there is a problem
//                  communicating to motor
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             motor - motor number desired
//             velocity - motor velocity
void setTargetVel(int fd, unsigned int motor, int vel)
{

    short int vel16 = (short int) vel;
    unsigned char start;

    // Perform check on motors
    if (motor == 1) {
        start = 0x28;
        motor = 0x21;
    } else if (motor == 2) {
        start = 0x2A;
        motor = 0x22;
    } else {
        // Neither motor 1 or 2 is called so exit the function
        printf("Error: invalid motor number\n");
        return;
    }

    // Write the new target velocity to the array
    putByte(fd, start, ((unsigned char)(vel16 & 0xff)));
    putByte(fd, (start+1), ((unsigned char)((vel16 >> 8) & 0xff)));

    // Send the command to start the PID mode
    putByte(fd, 0xff, ((unsigned char) motor));
        
}

// setAllTargetVel - Sets the target velocity for all motors
// Parameters: fd - connection to the board (value returned from DMCCstart)
//             velocity - motor velocity
void setAllTargetVel(int fd, int vel)
{
    short int vel16 = (short int) vel;

    // Write the new target velocity for the first motor
    putByte(fd, 0x28, ((unsigned char)(vel16 & 0xff)));
    putByte(fd, 0x29, ((unsigned char)((vel16 >> 8) & 0xff)));

    // Write the new target velocity for the second motor
    putByte(fd, 0x2A, ((unsigned char)(vel16 & 0xff)));
    putByte(fd, 0x2B, ((unsigned char)((vel16 >> 8) & 0xff)));

	// Send the command to start the PID mode
    putByte(fd, 0xff, 0x23);

}

int getMotorDir(int fd, unsigned int motor)
{
    // Send read command
    putByte(fd, 0xff, 0x00);

    if (motor == 1) {
        return (getByte(fd, 0x01) & 1);
    } else if (motor == 2) {
        return ((getByte(fd, 0x01) >> 1) & 1);
    } else {
        printf("Error: invalid motor number\n");
        return -1;
    }
}

void configMotorDir(int fd, unsigned int motor, int dir)
{
    unsigned char byte1 = getByte(fd, 0x01);
    if (motor == 1) {
        putByte(fd, 0x01, 
                ((byte1 & 0xe) | (unsigned char)(dir & 0x1)));
    } else if (motor == 2) {
        putByte(fd, 0x01, 
                ((byte1 & 0xd) | (unsigned char)((dir & 0x1) << 1)));
    } else {
        printf("Error: invalid motor number\n");
    }
}

void DMCCwait(unsigned int microseconds)
{ 
    usleep(microseconds);
}

void DMCCwaitSec(unsigned int seconds)
{
    if (seconds > 2147) {
        printf("Error: too long a wait time");
        printf(" (must be less than 2147 seconds)\n");
    }
    usleep(seconds*1000000);
}

void moveUntilTime(int fd, unsigned int time, 
                        unsigned int motor, unsigned int power)
{
    if ((motor != 1) && (motor != 2)) {
        printf("Error: invalid motor number\n");
        return;
    }
    setMotorPower(fd, motor, power);
    usleep(time);
    setMotorPower(fd, motor, 0);
}

void moveUntilPos(int fd, unsigned int motor, unsigned int pos)
{
    // Set the target position desired
    setTargetPos(fd, pos, motor);

    // Threshold value for QEI
    int threshold;
    
    if (motor == 1) {
        threshold = QEI_Threshold_1;
    } else if (motor == 2) {
        threshold = QEI_Threshold_2;
    } else {
        printf("Error: invalid motor number\n");
        return;
    }

    // Motor timeout (if the motor does not reach the desired position)
    time_t startTime, currentTime;
    time(&startTime);
    time(&currentTime);

    // Error value
    int error = (int)(getTargetPos(fd, motor)) - (int)(getQEI(fd, motor));
    if (error < 0) {
        error *= -1;
    }

    // Wait until the motor has reached the desired position or timeout
    while (error > threshold) {
        // Check if have waited longer than maximum time limit
        if ((currentTime - startTime) > TIME_LIMIT) {
            break;
        }
        time(&currentTime);

        // Get error between QEI target position and current position
        error = (int)(getTargetPos(fd, motor)) - (int)(getQEI(fd, motor));
        if (error < 0) {
            error *= -1;
        }
        // printf("Error = %d\n", error);
    }
}

void moveUntilVel(int fd, unsigned int motor, int vel)
{
    // Set the target speed desired
    setTargetVel(fd, vel, motor);

    // Set threshold values
    int threshold;

    if (motor == 1) {
        threshold = QEI_Vel_Threshold_1;
    } else if (motor == 2) {
        threshold = QEI_Vel_Threshold_2;
    } else {
        printf("Error: invalid motor number\n");
        return;
    }

    // Motor timeout (if the motor does not reach the desired velocity)
    time_t startTime, currentTime;
    time(&startTime);
    time(&currentTime);

    // Error value
    int error = (int)(getTargetVel(fd, motor)) - (int)(getQEIVel(fd, motor));
    if (error < 0) {
        error *= -1;
    }

    DMCCwaitSec(2);

    // Wait until the motor has reached the desired position or timeout
    while (error > threshold) {
        if ((currentTime - startTime) > TIME_LIMIT) {
            break;
        }
        time(&currentTime);

        error = (int)(getTargetVel(fd, motor)) - (int)(getQEIVel(fd, motor));
        if (error < 0) {
            error *= -1;
        }
    }
}

void moveAllUntilPos(int fd, unsigned int pos)
{
    // Set the new target position for both motors
    setAllTargetPos(fd, pos);

    // Motor timeout (if the motor does not reach the desired velocity)
    time_t startTime, currentTime;
    time(&startTime);
    time(&currentTime);

    // Error value
    int error1 = (int)(getTargetPos(fd,1)) - (int)(getQEI(fd, 1));
    if (error1 < 0) {
        error1 *= -1;
    }
    int error2 = (int)(getTargetPos(fd,2)) - (int)(getQEI(fd, 2));
    if (error2 < 0) {
        error2 *= -1;
    }

    // Wait until both motors are within the desired threshold or timeout
    while ((error1 > QEI_Threshold_1) || (error2 > QEI_Threshold_2)) {
        // Check if the time limit for the motors has been passed
        if ((currentTime - startTime) > TIME_LIMIT) {
            break;
        }
        time(&currentTime);

        error1 = (int)(getTargetPos(fd,1)) - (int)(getQEI(fd, 1));
        if (error1 < 0) {
            error1 *= -1;
        }
        error2 = (int)(getTargetPos(fd,2)) - (int)(getQEI(fd, 2));
        if (error2 < 0) {
            error2 *= -1;
        }

    }
}

void moveAllUntilVel(int fd, int vel)
{
    // Set the new target velocity for both motors
    setAllTargetVel(fd, vel);

    // Motor timeout (if the motor does not reach the desired velocity)
    time_t startTime, currentTime;
    time(&startTime);
    time(&currentTime);

    // Error value
    int error1 = (int)(getTargetVel(fd,1)) - (int)(getQEIVel(fd, 1));
    if (error1 < 0) {
        error1 *= -1;
    }
    int error2 = (int)(getTargetVel(fd,2)) - (int)(getQEIVel(fd, 2));
    if (error2 < 0) {
        error2 *= -1;
    }

    // Wait until both motors are within the desired threshold or timeout
    while ((error1 > QEI_Vel_Threshold_1) || (error2 > QEI_Vel_Threshold_2)) {
        // Check if the time limit for the motors has been passed
        if ((currentTime - startTime) > TIME_LIMIT) {
            break;
        }
        time(&currentTime);

        error1 = (int)(getTargetVel(fd,1)) - (int)(getQEIVel(fd, 1));
        if (error1 < 0) {
            error1 *= -1;
        }
        error2 = (int)(getTargetVel(fd,2)) - (int)(getQEIVel(fd, 2));
        if (error2 < 0) {
            error2 *= -1;
        }

    }
}

void moveAllUntilTime(int fd, unsigned int power, unsigned int time)
{
    setAllMotorPower(fd, power);
    usleep(time);
    setAllMotorPower(fd, 0);
}

// returnPIDConstants - gets the PID constants starting at the addr (helper)
// Parameters: fd - connection to board
//             addr - starting address
//             P - constant P
//             I - constant I
//             D - constant D
void returnPIDConstants(int fd, unsigned char addr, int *P, int *I, int *D)
{
    // Temporary variable to change from unsigned 16bit int to signed
    int result;

    // Get constant P
    result = (int)getWord(fd, addr);
    if (result > 32767) {
            result = (-1 * (0xffff-result));
    }
    *P = result;

    // Put constant I
    result = (int)getWord(fd, (addr + 0x02));
    if (result > 32767) {
            result = (-1 * (0xffff-result));
    }
    *I = result;
    
    // Put constant D
    result = (int)getWord(fd, (addr + 0x04));
    if (result > 32767) {
            result = (-1 * (0xffff-result));
    }
    *D = result;
}

void getPIDConstants(int fd, unsigned int motor, unsigned int posOrVel, 
                        int *P, int *I, int *D ) 
{
    if (motor == 1) {
        if (posOrVel == 0) {
            returnPIDConstants(fd, 0x30, P, I, D);
        } else if (posOrVel == 1) {
            returnPIDConstants(fd, 0x36, P, I, D);
        } else {
            printf("Error: posOrVel is not given as 0 or 1\n");
        }
    } else if (motor == 2) {
        if (posOrVel == 0) {
            returnPIDConstants(fd, 0x40, P, I, D);
        } else if (posOrVel == 1) {
            returnPIDConstants(fd, 0x46, P, I, D);
        } else {
            printf("Error: posOrVel is not given as 0 or 1\n");
        }
    } else {
        printf("Error: invalid motor number specified\n");
    }
}

// putPIDConstants - puts the PID constants starting at the given addr (helper)
// Parameters: fd - connection to board
//             addr - starting address
//             P - constant P
//             I - constant I
//             D - constant D
void putPIDConstants(int fd, unsigned char addr, int P, int I, int D)
{
    // Put constant P 
    putByte(fd, addr, (unsigned char)(P & 0xff));
    putByte(fd, (addr +0x01), (unsigned char)((P & 0xff00)>>8));

    // Put constant I 
    putByte(fd, (addr+0x02), (unsigned char)(I & 0xff));
    putByte(fd, (addr+0x03), (unsigned char)((I & 0xff00)>>8));

    // Put constant D 
    putByte(fd, (addr+0x04), (unsigned char)(D & 0xff));
    putByte(fd, (addr+0x05), (unsigned char)((D & 0xff00)>>8));    
}

void setPIDConstants(int fd, unsigned int motor, unsigned int posOrVel, 
                        int P, int I, int D) 
{
    if (motor == 1) {
        if (posOrVel == 0) {
            putPIDConstants(fd, 0x30, P, I, D);
        } else if (posOrVel == 1) {
            putPIDConstants(fd, 0x36, P, I, D);
        } else {
            printf("Error: posOrVel is not given as 0 or 1\n");
        }
    } else if (motor == 2) {
        if (posOrVel == 0) {
            putPIDConstants(fd, 0x40, P, I, D);
        } else if (posOrVel == 1) {
            putPIDConstants(fd, 0x46, P, I, D);
        } else {
            printf("Error: posOrVel is not given as 0 or 1\n");
        }
    } else {
        printf("Error: invalid motor number specified\n");
    }
}
