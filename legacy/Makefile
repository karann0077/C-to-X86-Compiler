CXX = g++
CXXFLAGS = -std=c++17 -O0 -g -Wall -Wextra
SRC = src
OBJ = obj

all: tinycc

tinycc: $(SRC)/main.cpp $(SRC)/lexer.cpp $(SRC)/parser.cpp $(SRC)/codegen.cpp
	$(CXX) $(CXXFLAGS) -I$(SRC) -o tinycc $(SRC)/main.cpp $(SRC)/lexer.cpp $(SRC)/parser.cpp $(SRC)/codegen.cpp

clean:
	rm -f tinycc *.s prog
