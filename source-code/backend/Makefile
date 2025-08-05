CC = gcc
SRC = src/main.c 
OUT = bin/main

#Remember that make command without arguments executes the first target

exec: compile #it means that exec depends on all 
#	it changes the directory and executes compiled file
	cd bin && ./$(notdir $(OUT))

compile:
# 	creates bin directory if it doesn't exist
# 	it compiles only 
	mkdir -p bin 
	$(CC) $(SRC) -o $(OUT)

clean: 
# 	it removes bin directory 
	rm -rf bin

#	it useful to tell make that the target names aren't files
.PHONY: all clean exec

