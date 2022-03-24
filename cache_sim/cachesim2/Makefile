CC := clang
CFLAGS := -O1 -g -Wall
LDFLAGS := 
objects := $(patsubst %.c,%.o,$(wildcard *.c))

.PHONY: test all clean test_inner
.SILENT: test_inner

cachesim2_test: $(objects)
	$(CC) -o $@ $(objects)

clean:
	rm -f results/* $(objects) cachesim2_test

all: $(objects) cachesim2_test

test_inner: cachesim2_test
	$(foreach var,$(sort $(wildcard tests/*)), ./cachesim2_test $(var) $(join results/,$(notdir $(var)));)

test: test_inner	
	sh showresults.sh

%.o : %.c
	$(CC) -c $(CFLAGS) $^

