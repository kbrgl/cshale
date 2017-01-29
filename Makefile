.PHONY: build

clean:
	rm cshale

build:
	gcc -o cshale cshale.c -O3

install: build
	cp cshale /usr/local/bin/cshale

