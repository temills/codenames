export CC = gcc
export CXX = g++
export CFLAGS = -w -g -O3 -pthread

INSTALL_PATH=bin/
BIN = pwe
OBJ = pweinference.o pwelearn.o
.PHONY: clean all

all: $(BIN)

pwe:pwe.cpp pweinference.o pwelearn.o pwe.h
pweinference.o: pweinference.cpp utils.h pweinference.h pwe.h
pwelearn.o: pwelearn.cpp utils.h pwelearn.h pwe.h

$(BIN) : 
	$(CXX) $(CFLAGS) -o $@ $(filter %.cpp %.o %.c, $^)

$(OBJ) :
	$(CXX) -c $(CFLAGS) -o $@ $(filter %.cpp %.c, $^)

install:
	cp -f -r $(BIN) $(INSTALL_PATH)

clean:
	$(RM) $(OBJ) $(BIN) *~
