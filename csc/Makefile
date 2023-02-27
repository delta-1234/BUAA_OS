all: code
	gcc code/fibo.c code/main.c -I include -o code/fibo.o
	gcc code/main.c code/fibo.c -I include -o code/main.o
	gcc code/fibo.c code/main.c -I include -o fibo

clean:
	$(MAKE) clean -C code
