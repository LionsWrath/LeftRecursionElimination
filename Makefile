CC=gcc

default:
	$(CC) -gstabs -O2 -lm left-recursion-elimination.c
