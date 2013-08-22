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

#include "DMCC.h"

int main(int argc, char *argv[])
{
    // Usage help printout for motor control
    if (argc != 4) {
        printf("usage: ./setMotor <board number> <motor number> <value>\n");
        printf("       <board number> is [0-3] for placement of cape\n");
        printf("       <motor number> is 1 or 2\n");
        printf("       <value> is between -10000 and 10000 (decimal)\n");
        printf("example: ./setMotor 0 1 5000\n");
        exit(1);
    }

    // Get arguments from the command line
    int boardNum = atol(argv[1]);
    unsigned int nMotor = atoi(argv[2]);
    int pwm = atol(argv[3]);
   
    // Function setup (start session then end session)
    int session = DMCCstart(boardNum);    
 
    setMotorPower(session, nMotor, pwm);

    DMCCend(session);
    return 0;
}

