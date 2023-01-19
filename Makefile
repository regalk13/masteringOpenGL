CC = cc
LIBS=-lglfw -lGL -lm -lglad

all: main

build/bin/main: src/main.c
	@mkdir -p "$(@D)"
	@echo Compiling "$<"
	@$(CC) -O2 src/main.c -o "$@" -Llib $(LIBS)

build/bin/template: src/template.c
	@mkdir -p "$(@D)"
	@echo Compiling "$<"
	@$(CC) -O2 src/template.c -o "$@" -Llib $(LIBS)

clear:
	@rm -rf build
