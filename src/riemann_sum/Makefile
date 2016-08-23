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
	./$(FILE) input.txt ${ARGS} 0 1
	
check-multiplier2:
	./$(FILE) input.txt ${ARGS} 5 1

check-nosharing:
	./$(FILE) input.txt ${ARGS} 0 0
	
check_d:
	gdb --args ./$(FILE) input.txt ${ARGS} 0 1

n ?= 1
check_r:
	n=$(n); \
	while [ $${n} -le ${ARGS} ] ; do \
		echo $$n ; \
		./$(FILE) input.txt $${n} 0 1; \
		echo ; \
		n=`expr $$n + 1`; \
	done; \
	true

n ?= 1
check_r-nosharing:
	n=$(n); \
	while [ $${n} -le ${ARGS} ] ; do \
		echo $$n ; \
		./$(FILE) input.txt $${n} 0 0; \
		echo ; \
		n=`expr $$n + 1`; \
	done; \
	true