CFLAGS = -I. -Itests -g -O2 -Wall -W

all: driver

main.c:
	bash tests/gentest.sh tests/test-*.c > $@

driver: main.c tests/test-cirbuf.c tests/unit-test.c main.c
	$(CC) $(CFLAGS) -o $@ $^
	./$@

clean:
	rm -f main.c driver
