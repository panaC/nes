# the compiler: gcc for C program, define as g++ for C++
CC = g++

# compiler flags:
#  -g     - this flag adds debugging information to the executable file
#  -Wall  - this flag is used to turn on most compiler warnings
CFLAGS  = -g -Wall

# The build target 
TARGET = nes

RM = rm 

SRC_DIR= src/
OBJ_DIR = obj/

SRC_FILE= main.cpp

OBJ = $(SRC:.c=OBJ_DIR:.o)

SRC = $(addprefix $(SRC_DIR), $(SRC_FILE))

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

clean:
	$(RM) -f $(TARGET)
	$(RM) -rf $(OBJ_DIR)

.PHONY: all clean 
