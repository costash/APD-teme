CC = g++
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))
CC_FLAGS = -Wall -O2
LD_FLAGS = 
BIN = serial

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LD_FLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CC_FLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(OBJ) $(BIN)
