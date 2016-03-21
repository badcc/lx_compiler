CC=g++
CFLAGS=-c -Wall -wwrite-strings
LDFLAGS=-I../ab -g
SOURCES=lang2.c
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=lang2

all: $(SOURCES) $(EXECUTABLE)
	./lang2
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
