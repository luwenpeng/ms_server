.PHONY: all install clean

CXX = gcc

#CFLAGS = std=c++11 -Wall -g -fsanitize=leak -fsanitize=address -D DEBUG_SWITCH
#LIB = -lasan

CFLAGS = -O2
LIB = 

INCLUDES = 
BIN = myserver
INSTALLDIR = /usr/sbin/

SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SOURCES))

all: $(BIN)
$(BIN): $(OBJS)
	$(CXX) $^ -o $@ $(LIB)

%.o: %.c
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

install:
	test -d $(INSTALLDIR) || mkdir $(INSTALLDIR)
	cp $(BIN) $(INSTALLDIR)

clean:
	rm -rf $(BIN) $(OBJS)
