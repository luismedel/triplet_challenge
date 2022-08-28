
bin:
	@mkdir bin

bin/main: bin
	@gcc main.c -o bin/main

run: bin/main
	@bin/main ../pg2009.txt

time: bin/main
	@time bin/main ../pg2009.txt

clean:
	@rm -rf bin
