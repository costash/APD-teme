CC = g++
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))
CC_FLAGS = -Wall -O2 -fopenmp
LD_FLAGS = -Wall -O2 -fopenmp
BIN_SERIAL = serial
BIN_PARALEL = paralel
BIN_SERIALO = serialo
BIN_PARALELO = paralelo

all: $(BIN_SERIAL) $(BIN_PARALEL) $(BIN_SERIALO) $(BIN_PARALELO)

$(BIN_SERIAL): serial.o
	$(CC) $(LD_FLAGS) -o $@ $^

$(BIN_PARALEL): paralel.o
	$(CC) $(LD_FLAGS) -o $@ $^

$(BIN_SERIALO): serialoptimized.o
	$(CC) $(LD_FLAGS) -o $@ $^

$(BIN_PARALELO): paraleloptimized.o
	$(CC) $(LD_FLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CC_FLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(OBJ) $(BIN)
	rm -f $(BIN_SERIAL)
	rm -f $(BIN_PARALEL)

