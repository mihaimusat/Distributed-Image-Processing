build: tema3
tema3: tema3.c
	mpicc -o tema3 tema3.c -lm
clean:
	rm -f tema3
