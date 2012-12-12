CC = mpic++
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))
CC_FLAGS = -Wall
LD_FLAGS = -Wall
BIN = Tema3

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LD_FLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CC_FLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(OBJ) $(BIN)
	rm -f $(BIN)
