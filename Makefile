CC = gcc
LIBS =-lglfw -lGL -lm -lglad
CFLAGS = --std=c11 -O2

all: main

main: src/main.c
	@mkdir -p build/bin
	@echo Compiling "$<"
	@$(CC) $(CFLAGS) src/main.c -o build/bin/main -Llib $(LIBS)

triangle: src/triangle/main.c
	@mkdir -p build/bin
	@echo Compiling "$<"
	@$(CC) $(CFLAGS) src/triangle/main.c -o build/bin/triangle -Llib $(LIBS)

rectangle: src/rectangle/main.c
	@mkdir -p build/bin
	@echo Compiling "$<"
	@$(CC) $(CFLAGS) src/rectangle/main.c -o build/bin/rectangle -Llib $(LIBS)

firste: src/two-triangles-exercise1/main.c 
	@mkdir -p build/bin
	@echo Compiling "$<"
	@$(CC) $(CFLAGS) src/two-triangles-exercise1/main.c -o build/bin/firste -Llib $(LIBS)

clear:
	@rm -rf build
