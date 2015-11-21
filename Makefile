
SRC=src/request.c src/measurements.c src/main.c src/ezxml.c


measure-host: $(SRC)
	gcc $(SRC) -o ./measure-host

.PHONY: test-request test-measurements

test: test-request test-measurements

test-request:
	gcc -DTEST src/request.c src/ezxml.c -o test && \
	./test && \
	rm ./test

test-measurements:
	gcc -DTEST src/measurements.c src/ezxml.c -o test && \
	./test  && \
	rm ./test


