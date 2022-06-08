# the compiler: gcc for C program, define as g++ for C++
CC = g++

# compiler flags:
#  -g     - this flag adds debugging information to the executable file
#  -Wall  - this flag is used to turn on most compiler warnings

CPPUNIT = /Users/edrlab/Documents/lib/cppunit
LDFLAGS = -L../lib -lcppunit
CFLAGS = -g -Wall -std=c++20 -stdlib=libc++ $(LDFLAGS) -I$(CPPUNIT)/include

# The build target 
TARGET = test

RM = rm 

SRC_DIR= ./
OBJ_DIR = obj/

SRC_FILE= main.cpp \
					Cpu.cpp

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
