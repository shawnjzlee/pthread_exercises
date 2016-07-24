COMPILE = g++
FLAGS= -pthread -std=c++11
DEBUG = -g
FILE = riemann
BUILD_DIR = ./bin

all: clean
	mkdir $(BUILD_DIR)
	cp -f ./Makefile $(BUILD_DIR)/Makefile
	cp -f ./input.txt $(BUILD_DIR)/input.txt
	$(COMPILE) $(FLAGS) ./$(FILE).cpp ./riemann.h ./rbarrier.h -o $(BUILD_DIR)/$(FILE)

debug: clean
	mkdir $(BUILD_DIR)
	cp -f ./Makefile $(BUILD_DIR)/Makefile
	cp -f ./input.txt $(BUILD_DIR)/input.txt
	$(COMPILE) $(FLAGS) -g ./$(FILE).cpp ./riemann.h ./rbarrier.h -o $(BUILD_DIR)/$(FILE)

clean:
	rm -rf $(BUILD_DIR)
	
check:
	./$(FILE) input.txt ${ARGS}
	
check_d:
	gdb --args ./$(FILE) input.txt ${ARGS}