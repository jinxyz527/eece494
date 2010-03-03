
CC = gcc

LIB = -lpthread -lrt -lm

FLAGS = -O2 -g

EXE = schedulability
OBJ = schedulability.o 
SRC = schedulability.c
H = 


$(EXE): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(EXE) $(LIB_PATH) $(LIB)

schedulability.o: schedulability.c $(H)
	$(CC) -c $(FLAGS) schedulability.c 

clean:
	rm -r *.o $(EXE)
