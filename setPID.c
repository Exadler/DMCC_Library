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
#include <signal.h>

#include "DMCC.h"

// This program allows users to adjust the PID parameters (PID tuning).
// It should be used when there is motor overshoot, undershoot, 
// and/or peaks (oscillations).  Repeat the adjustments until the response
// is satisfactory to you.

// Some Helpful Hints: (referenced from www.controlsoftinc.com)
//      - If there is no correction/control output (CO), increase P by 50%
//      - If there is unstable/sustained oscillation 
//          with overshoot greater than 25%, reduce P and I by 50%
//      - If there is oscillation with tolerable overshoot, 
//          reduce P by 20% and I by 50%
//      - If 3 or more consecutive peaks (oscillations) occur,
//          reduce I by 30% and increase D by 50%
//      - If the target is not reached and there is no substantial changes,
//          increase I by 100%

int session;

void sig_handler(int sig)
{
    setMotorPower(session, 1, 0);
    setMotorPower(session, 2, 0);
    DMCCend(session);
    exit(1);
}

int main(int argc, char *argv[])
{
    // Prints usage statement
    if (argc != 8) {
        printf("usage: ./setPID <board number> <motor> <pos_vel> ");
        printf("<P> <I> <D> <motor>\n");
        printf("       <board number> is [0-3] for placement of cape\n");
        printf("       <target> is the target QEI position or velocity\n");
        printf("       <pos_vel> 0 for position, 1 for velocity\n");
        printf("       <P> is the Kp\n");
        printf("       <I> is the Ki\n");
        printf("       <D> is the Kd\n");
        printf("       <motor> is the motor number\n");
        printf("examples: ./setPID 0 5000 0 -5248 -75 -500 1\n");
        exit(1);
    }

    // Get the arguments from the command line
    int boardNum = atol(argv[1]);
    unsigned int target = atoi(argv[2]);
    int indicator = atol(argv[3]);
    int P = atol(argv[4]);
    int I = atol(argv[5]);
    int D = atol(argv[6]);
    unsigned int nMotor = atoi(argv[7]);
    
    // Catch Ctrl-C to kill all motors
    signal(SIGINT, sig_handler);

    // Begin the session (open a connection to the board)
    session = DMCCstart(boardNum); 

    int error;
    unsigned int current;

    resetQEI(session, nMotor);

    // Set the PID Constants
    setPIDConstants(session, nMotor, indicator, P, I, D);
   
    // Check if the user has specified target position or velocity
    // NOTE: no error values will print during this time
    if (indicator == 0) { 
        moveUntilPos(session, nMotor, target);
    } else if (indicator == 1) {
        moveUntilVel(session, nMotor, target);
    } else {
        setMotorPower(session, nMotor, 0);
        printf("Error: position or velocity not correctly specified\n");
        DMCCend(session);
        return 1;
    }

    // Print out the error, to check the oscillation/overshoot
    while(1) {
        // Get error between QEI target
        if (indicator == 0) {
            error = (int)(target) - (int)(getQEI(session, nMotor));
        } else if (indicator == 1) {
            error = (int)(target) - (int)(getQEIVel(session, nMotor));
        } else {
            setMotorPower(session, nMotor, 0);
            printf("Error: position or velocity not correctly specified\n");
            DMCCend(session);
            return 1;
        }
        current = getMotorCurrent(session, nMotor);
        printf("Error = %d, Current = %u\n", error, current);
        usleep(200000); 
    }

    DMCCend(session);

    return 0;
}

