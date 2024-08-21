# Compiler
CC = gcc

# Flags
CFLAGS = -Wall

# Application
PROG1 = application
SRC1 = application.c
OBJ1 = application.o

# Child
PROG2 = child
SRC2 = child.c
OBJ2 = child.o

all: $(PROG1) $(PROG2)

# First .exe
$(PROG1): $(OBJ1)
	$(CC) $(CFLAGS) -o $(PROG1) $(OBJ1)

# Second .exe
$(PROG2): $(OBJ2)
	$(CC) $(CFLAGS) -o $(PROG2) $(OBJ2)

# Generate first .o
$(OBJ1): $(SRC1)
	$(CC) $(CFLAGS) -c $(SRC1) -o $(OBJ1)

# Generate second .o
$(OBJ2): $(SRC2)
	$(CC) $(CFLAGS) -c $(SRC2) -o $(OBJ2)

clean:
	rm -f $(OBJ1) $(PROG1) $(OBJ2) $(PROG2)

# Indicates clean is not a file.
.PHONY: clean
