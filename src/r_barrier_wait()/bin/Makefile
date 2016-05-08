COMPILE = g++
FLAGS= -pthread -std=c++11
DEBUG = -Wall -Werror -ansi -pedantic
FILE = rbarrier
BUILD_DIR = ./bin

all: clean
	mkdir $(BUILD_DIR)
	cp -f ./Makefile $(BUILD_DIR)/Makefile
	$(COMPILE) $(FLAGS) ./$(FILE).cpp -o $(BUILD_DIR)/$(FILE)

debug: clean
	mkdir $(BUILD_DIR)
	cp -f ./Makefile $(BUILD_DIR)/Makefile
	$(COMPILE) $(FLAGS) -g ./$(FILE).cpp -o $(BUILD_DIR)/$(FILE)

clean:
	rm -rf $(BUILD_DIR)
	
check:
	./$(FILE)