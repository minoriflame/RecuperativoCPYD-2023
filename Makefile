# Compiler
CC = g++

# Compiler flags
CFLAGS = -std=c++11 -Wall -Wextra -fopenmp

# Source files
SRCS = main.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
EXEC = main

all: $(EXEC)

release: $(OBJS)
	$(CC) $(CFLAGS) -O2 $(OBJS) -o $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)

