CXX      := g++
CXXFLAGS := -Wall -std=c++14

ifdef DEBUG
CXXFLAGS += -g
else
CXXFLAGS += -O2
endif

TARGET := debug
MODULES := $(patsubst %.cpp,%,$(wildcard *.cpp))

.PHONY: all clean

all: ${TARGET}

clean:
	@rm -fv ${TARGET} ${MODULES:%=%.o}

${TARGET}: ${MODULES:%=%.o}
	${CXX} ${CXXFLAGS} -o $@ $^

%.o: %.cpp
	${CXX} -c ${CXXFLAGS} -o $@ $^
