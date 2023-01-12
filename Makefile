CC = cc
LIBS=-lglfw -lGL -lm -lglad

all: build/bin/main

build/bin/main: src/main.c
	@mkdir -p "$(@D)"
	@echo Compiling "$<"
	@$(CC) -O2 src/main.c -o "$@" -Llib $(LIBS)


clear:
	@rm -rf build
