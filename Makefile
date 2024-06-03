build:
	mkdir -p bin 
	gcc -Wall server.c -o ./bin/server -lm
	gcc -Wall client.c -o ./bin/client -lm

clean:
	rm -f bin/client bin/server 
