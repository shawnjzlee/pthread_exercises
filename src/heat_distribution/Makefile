#variables
COMPILE = g++
FLAGS = -pthread -std=c++11
DEBUG = $(FLAGS) -Wall -Werror -g 
FILE = heat_distribution

all: clean
	mkdir ./bin
	$(COMPILE) $(FLAGS) ./$(FILE).cpp -o ./bin/$(FILE)

debug: clean
	mkdir ./bin
	$(COMPILE) $(DEBUG) -g ./$(FILE).cpp -o ./bin/$(FILE)

clean:
	rm -rf ./bin

