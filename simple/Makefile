CC=g++
CFLAGS=-c -Wall
SOURCES=main.cpp
LDFLAGS=-pthread
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=simple.o

run: all
	./simple.o


all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf ./simple.o main.o