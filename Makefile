CC=gcc
CFLAGS=-I.

cpu: cpu.c
	$(CC) -o cpu cpu.c
