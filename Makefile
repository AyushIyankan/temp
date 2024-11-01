.PHONY: all clean run

# Build target for main
all: main

# Compile main.cpp into the executable 'main'
main: main.cpp work.h libwork.so
	g++ -std=c++11 -Wall -Wextra -O2 main.cpp -L. -l:libwork.so -o main

# Run the program
run: main
	./main ${SEED}

# Clean up the executable
clean:
	rm -f main
