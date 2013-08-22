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
    // Prints usage statement
    if (argc != 2) {
        printf("usage: ./getCurrent <board number>\n");
        printf("       <board number> is [0-3] for placement of cape\n");
        printf("example: ./getCurrent 0\n");
        exit(1);
    }

    int boardNum = atol(argv[1]);

    int session = DMCCstart(boardNum);
    unsigned int curr1, curr2, voltage;

    while (1) {
        // Read the current for motor 1 and 2
        curr1 = getMotorCurrent(session, 1);
        curr2 = getMotorCurrent(session, 2);

        // Read the motor supply voltage
        voltage = getMotorVoltage(session);

        printf("Current Motor 1 = %u (0x%x), Motor 2 = %u (0x%x), ",
                curr1, curr1, curr2, curr2);
        printf("Voltage = %u (0x%x)\n", voltage, voltage);

        // Wait 0.2 seconds before next reading
        usleep(200000);
    }

    DMCCend(session);

    return 0;
}
