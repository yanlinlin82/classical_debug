.PHONY: all clean

all: debug

clean:
	@rm -fv debug

debug: x86-debug.cpp
	g++ -Wall -std=c++17 x86-debug.cpp -o debug
