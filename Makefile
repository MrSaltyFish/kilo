all: kilo.c ./build/
	$(CC) kilo.c -o ./build/kilo -Wall -Wextra -pedantic -std=gnu99
	./build/kilo

kilo: kilo.c
	$(CC) kilo.c -o kilo -Wall -Wextra -pedantic -std=gnu99

./build/:
	mkdir -p ./build/


run: ./kilo.c ./build/
	./build/kilo
clean:
	rm -rf ./build