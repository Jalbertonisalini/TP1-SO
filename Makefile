# Compiler
CC = gcc

# Flags
CFLAGS = -Wall -pedantic -std=c99

# Application
PROG1 = application
SRC1 = application.c
OBJ1 = application.o

# Child
PROG2 = child
SRC2 = child.c
OBJ2 = child.o

# View
PROG3 = view
SRC3 = view.c
OBJ3 = view.o

all: $(PROG1) $(PROG2) $(PROG3)

# First .exe
$(PROG1): $(OBJ1)
	$(CC) $(CFLAGS) -o $(PROG1) $(OBJ1)

# Second .exe
$(PROG2): $(OBJ2)
	$(CC) $(CFLAGS) -o $(PROG2) $(OBJ2)

# Third .exe
$(PROG3): $(OBJ3)
	$(CC) $(CFLAGS) -o $(PROG3) $(OBJ3)

# Generate first .o
$(OBJ1): $(SRC1)
	$(CC) $(CFLAGS) -c $(SRC1) -o $(OBJ1)

# Generate second .o
$(OBJ2): $(SRC2)
	$(CC) $(CFLAGS) -c $(SRC2) -o $(OBJ2)

# Generate third .o
$(OBJ3): $(SRC3)
	$(CC) $(CFLAGS) -c $(SRC3) -o $(OBJ3)

clean:
	rm -f $(OBJ1) $(PROG1) $(OBJ2) $(PROG2) $(OBJ3) $(PROG3)

# Indicates clean is not a file.
.PHONY: clean
