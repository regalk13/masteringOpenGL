CC = cc
LIBS=-lglfw -lGL -lm -lglad

all: main

main: src/main.c
	@mkdir -p build/bin
	@echo Compiling "$<"
	@$(CC) -O2 src/main.c -o build/bin/main -Llib $(LIBS)

triangle: src/triangle/main.c
	@mkdir -p build/bin
	@echo Compiling "$<"
	@$(CC) -O2 src/triangle/main.c -o build/bin/triangle -Llib $(LIBS)

rectangle: src/rectangle/main.c
	@mkdir -p build/bin
	@echo Compiling "$<"
	@$(CC) -O2 src/rectangle/main.c -o build/bin/rectangle -Llib $(LIBS)

clear:
	@rm -rf build
