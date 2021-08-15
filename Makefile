mendel_corssing: main.c genotypes.c
	gcc `pkg-config gtk+-3.0 --cflags` main.c genotypes.c -Wall -o mendel_corssing -lm -ggdb3 `pkg-config gtk+-3.0 --libs` -rdynamic