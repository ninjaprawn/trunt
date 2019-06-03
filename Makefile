CC = gcc
CC_FLAGS = 

all: 
	@mkdir -p bin
	$(CC) -o ./bin/trunt $(CC_FLAGS) linked_list.c trunt.c

clean:
	rm -r ./bin/trunt

test:
	@./bin/trunt test.c
