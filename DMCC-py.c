//
// DMCC-py.c - DMCC python interface
//

#include "Python.h"
#include <stdlib.h>
#include "DMCC.h"

static PyObject *
dmcc_setMotor(PyObject *self, PyObject *args)
{
    int nBoard;
    int nMotor;
    int nPower;
    // Make sure szErrorMsg is not on the stack
    // -- downside is that we could have concurrency issues with different
    // threads, but you know what, there should only be one error message
    // at a time.  If you have it from multiple threads, your code is fubar'ed 
    // anyways!
    static char szErrorMsg[80];

    // DMCC.setMotor takes 3 arguments: board number, motor number, power
    if (!PyArg_ParseTuple(args, "iii:setMotor", &nBoard, &nMotor, &nPower)) {
        return NULL;
    }
    // validate the board number
    if ((nBoard < 0) || (nBoard > 3)) {
        sprintf(szErrorMsg, "Board number %d is invalid.  Board number must be between 0 and 3.",
                nBoard);
        PyErr_SetString(PyExc_IndexError,szErrorMsg);
        return NULL;
    }
    // validate the motor number
    if ((nMotor < 1) || (nMotor > 2)) {
        sprintf(szErrorMsg, "Motor number %d is invalid.  Motor number must be between 1 and 2.",
                nMotor);
        PyErr_SetString(PyExc_IndexError,szErrorMsg);
        return NULL;
    }
    // validate the power
    if ((nPower < -10000) || (nPower > 10000)) {
        sprintf(szErrorMsg, "Power %d is invalid.  Power must be between -10000 and 10000.",
                nPower);
        PyErr_SetString(PyExc_IndexError,szErrorMsg);
        return NULL;
    }

    int session;
    session = DMCCstart(nBoard);
    setMotorPower(session, nMotor, nPower);
    DMCCend(session);

    return Py_BuildValue("i", 0);
}

static PyMethodDef
module_functions[] = {
    { "setMotor", dmcc_setMotor, METH_VARARGS, "Set motor (board, motorNum, power)" },
    { NULL }
};

void
initDMCC(void)
{
    Py_InitModule3("DMCC", module_functions, "DMCC module by Exadler");
}
