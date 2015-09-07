.PHONY: proj all debug clean

all: proj

proj: $(patsubst %.cpp,%.o,$(wildcard *.cpp))

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDES) -c $^ -o $@
	
clean:
	rm -vf *.o
