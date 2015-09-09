PROJ_NAME=usb_daemon

export CC=gcc
export CXX=g++

export INCLUDES = $(shell pwd)/include
FLAGS = -O2 -Wall -pedantic -march=native
export CFLAGS  = ${FLAGS} -std=gnu99
export CXXFLAGS=${FLAGS} -std=c++11
OBJS=$(patsubst %.cpp,%.o,$(wildcard src/*.cpp))
OBJS+=$(patsubst %.c,%.o,$(wildcard src/CFunctions/*.c))

.PHONY: all clean

all: $(PROJ_NAME)
	
proj: 
	cd src/CFunctions && $(MAKE)
	cd src && $(MAKE) 
	
$(PROJ_NAME): proj
	$(CXX) $(CXXFLAGS) $(OBJS) -lusb-1.0 -lpthread -lconfig++ -o $@
	
clean:
	rm -v src/CFunctions/*.o
	rm -v src/*.o
