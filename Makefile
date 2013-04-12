CC=gcc
CPP=g++
LD=g++

CFLAGS=-c -Wall -O2 -I/u/yli/include/ -fopenmp
LDFLAGS=-O2 -lopencv_imgproc -lopencv_core -lopencv_highgui -lopencv_video -fopenmp -L/u/yli/lib/
EXEC = rgmRect

OBJ_DIR=obj
SRC_DIR=src

C_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c))
CC_OBJECTS = $(patsubst $(SRC_DIR)/%.cc,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.cc))
CPP_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.cpp))

$(EXEC): $(C_OBJECTS) $(CC_OBJECTS) $(CPP_OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CPP) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(CC_OBJECTS) $(C_OBJECTS) $(CPP_OBJECTS) $(EXEC)
