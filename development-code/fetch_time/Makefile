# Compiler and flags
CC := g++
CFLAGS := -Wall -g

# Libraries
LIBS := -lcurl -ljsoncpp

# JSONCPP Include Path (Add the correct path if jsoncpp is in a custom location)
JSONCPP_INCLUDE := -I/usr/include/jsoncpp   # Adjust this if necessary, e.g. for non-default locations

# Source file and output executable
SRC_FILE := fetch_time.cpp
OBJ_FILE := fetch_time.o
EXEC := fetch_time

# Targets
.PHONY: all clean run

# Default target (compile the program)
all: $(EXEC)

# Link object file to create the executable
$(EXEC): $(OBJ_FILE)
	$(CC) $(OBJ_FILE) -o $(EXEC) $(LIBS)

# Compile C++ source file to an object file
$(OBJ_FILE): $(SRC_FILE)
	$(CC) $(CFLAGS) $(JSONCPP_INCLUDE) -c $(SRC_FILE) -o $(OBJ_FILE)

# Run the compiled program
run: $(EXEC)
	./$(EXEC)

# Clean up object file and executable
clean:
	rm -f $(OBJ_FILE) $(EXEC)

