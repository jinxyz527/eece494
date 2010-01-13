
CC = gcc

LIB = -lpthread -lrt -lX11

LIB_PATH = 

FLAGS = -O2 -I/ubc/ece/home/courses/eece494/libs/X11/

EXE = sim
OBJ = main.o control.o rover.o graphics.o
SRC = main.c control.c rover.c graphics.c
H = defs.h ext.h control.h rover.h graphics.h


$(EXE): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(EXE) $(LIB_PATH) $(LIB)

graphics.o: graphics.c $(H)
	$(CC) -c $(FLAGS) graphics.c

rover.o: rover.c $(H)
	$(CC) -c $(FLAGS) rover.c

control.o: control.c $(H)
	$(CC) -c $(FLAGS) control.c

main.o: main.c $(H)
	$(CC) -c $(FLAGS) main.c

clean:
	rm -rf *o ${EXE}

