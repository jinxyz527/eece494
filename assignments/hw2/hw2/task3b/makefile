
CC = gcc

LIB = -lpthread -lrt

#LIB_PATH = -L/usr/openwin/lib
#LIB_PATH = -L/usr/X11R6/lib

#FLAGS = -Wall -Wpointer-arith -Wcast-qual -Wstrict-prototypes -O -D__USE_FIXED_PROTOTYPES__ -ansi -pedantic -Wmissing-prototypes -Wshadow -Wcast-align -D_POSIX_SOURCE
#FLAGS = -g
FLAGS = -O2

EXE = task2
OBJ = main.o cam.o misc.o
SRC = main.c cam.c misc.c
H = defs.h ext.h cam.h misc.h


$(EXE): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(EXE) $(LIB_PATH) $(LIB)

main.o: main.c $(H)
	$(CC) -c $(FLAGS) main.c 

misc.o: misc.c $(H)
	$(CC) -c $(FLAGS) misc.c 

cam.o: cam.c $(H)
	$(CC) -c $(FLAGS) cam.c 

clean:
	rm -r *.o task2
