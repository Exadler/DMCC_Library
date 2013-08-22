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
        printf("usage: ./getQEI <board number>\n");
        printf("       <board number> is [0-3] for placement of cape\n");
		printf("example: ./getQEI 0\n");
        exit(1);
    }

    int boardNum = atol(argv[1]);
    int session = DMCCstart(boardNum);

    unsigned int motor1QEI, motor2QEI;
    int vel1, vel2;

    while (1) {
        // Read the Quadrature Encoder Interface (QEI) for motor 1 and 2
        motor1QEI = getQEI(session, 1);
        motor2QEI = getQEI(session, 2);
        vel1 = getQEIVel(session, 1);
        vel2 = getQEIVel(session, 2);
        printf("QEI Motor 1 = %u (0x%x) [v = %d], Motor 2 = %u (0x%x) [v = %d]\n",
                motor1QEI, motor1QEI, vel1, motor2QEI, motor2QEI, vel2);

        // Wait 0.2 seconds before next reading
        usleep(200000);
    }

    DMCCend(session);

    return 0;
}



