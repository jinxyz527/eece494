 
CC = gcc
 
LIB = -lpthread -lrt -lm
 
FLAGS = -O2 -g
 
EXE = wgen
OBJ = wgen.o
SRC = wgen.c
H =
 
 
$(EXE): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(EXE) $(LIB_PATH) $(LIB)
 
wgen.o: wgen.c $(H)
	$(CC) -c $(FLAGS) wgen.c
 
clean:
	rm -r *.o $(EXE)
