CC=gcc

default:
	$(CC) -gstabs -O2 -lm lrecursion-list.c
