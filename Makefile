.PHONY: all clean

all: debug

clean:
	@rm -fv debug

debug:
	g++ -Wall -std=c++17 x86-debug.cpp -o debug
