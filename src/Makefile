
bin:
	@mkdir bin

bin/main: bin
	@gcc main.c -o bin/main

build: bin/main

run: build
	@bin/main ../pg2009.txt

time: build
	@time bin/main ../pg2009.txt

clean:
	@rm -rf bin
