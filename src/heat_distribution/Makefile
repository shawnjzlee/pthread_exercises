#variables
COMPILE = g++
FLAGS = -pthread -std=c++11
DEBUG = $(FLAGS) -Wall -Werror -g 
FILE = heat_distribution
BUILD_DIR = ./bin

all: clean
	mkdir ./bin
	cp -f ./input.txt $(BUILD_DIR)/input.txt
	cp -f ./output.txt $(BUILD_DIR)/output.txt
	$(COMPILE) $(FLAGS) ./$(FILE).cpp ./$(FILE).h -o ./bin/$(FILE)

debug: clean
	mkdir ./bin
	cp -f ./input.txt $(BUILD_DIR)/input.txt
	cp -f ./output.txt $(BUILD_DIR)/output.txt
	$(COMPILE) $(FLAGS) -g ./$(FILE).cpp ./$(FILE).h -o ./bin/$(FILE)

clean:
	rm -rf ./bin

