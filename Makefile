source=main.c
shell: ${source} config.h
	gcc ${source} -o main -O0 -g