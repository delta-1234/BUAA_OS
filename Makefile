test:
	gcc test.c -o test
run: test
	./test
clean:
	rm -f test
