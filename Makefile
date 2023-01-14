# Specifiy the target
all: cache-sim

# Specify the object files that the target depends on
# Also specify the object files needed to create the executable
cache-sim: Cache-sim.o
	g++ Cache-sim.o -o cache-sim

# Specify how the object files should be created from source files
Cache-sim.o: Cache-sim.cpp
	g++ -c Cache-sim.cpp


# Specify the object files and executables that are generated
# and need to be removed to re-compile the whole thing
clean:
	rm -f *.o cache-sim
