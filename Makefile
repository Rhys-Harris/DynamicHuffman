all: build run

build:
		gcc *.c DynHuff/*.c -o "dev.exe"

run:
		dev
