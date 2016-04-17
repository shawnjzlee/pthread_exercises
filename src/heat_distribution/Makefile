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
	cp -f ./Makefile $(BUILD_DIR)/Makefile
	$(COMPILE) $(FLAGS) ./$(FILE).cpp ./$(FILE).h -o ./bin/$(FILE)

debug: clean
	mkdir ./bin
	cp -f ./input.txt $(BUILD_DIR)/input.txt
	cp -f ./output.txt $(BUILD_DIR)/output.txt
	cp -f ./Makefile $(BUILD_DIR)/Makefile
	$(COMPILE) $(FLAGS) -g ./$(FILE).cpp ./$(FILE).h -o ./bin/$(FILE)

clean:
	rm -rf ./bin
	
check:
	./heat_distribution input.txt output.txt ${ARGS}
	
check_d:
	gdb --args ./heat_distribution input.txt output.txt ${ARGS}