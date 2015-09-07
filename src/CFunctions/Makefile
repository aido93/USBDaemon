.PHONY: proj all debug clean

all: proj

proj: $(patsubst %.c,%.o,$(wildcard *.c))

%.o : %.c
	$(CC) $(CFLAGS) -I$(INCLUDES) -c $^ -o $@
	
clean:
	rm -vf *.o
