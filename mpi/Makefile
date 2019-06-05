CC=mpicxx
CFLAGS=-c -Wall
SOURCES=main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=mpilab.o
NODES := 4
AMOUNT := 16

# --bind-to socket --map-by socket

run: all
	@mpirun -n $(NODES) ./mpilab.o $(AMOUNT)


all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	@$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	@$(CC) $(CFLAGS) $< -o $@

clean:
	@rm -rf ./mpilab.o main.o