CC = gcc -Wall

all: getQEI setMotor getCurrent setPID

getQEI: getQEI.c DMCC.c DMCC.h 
		$(CC) -o getQEI getQEI.c DMCC.c

setMotor: setMotor.c DMCC.c DMCC.h
		  $(CC) -o setMotor setMotor.c DMCC.c

getCurrent: getCurrent.c DMCC.c DMCC.h
			$(CC) -o getCurrent getCurrent.c DMCC.c

setPID: setPID.c DMCC.c DMCC.h
		$(CC) -o setPID setPID.c DMCC.c