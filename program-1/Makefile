# Copyright 2024 Ian Lecker

COMPILER = g++
COMPILER_FLAGS = -pthread


all: sloppySim:
	$(COMPILER) $(COMPILERFLAGS) sloppySim.cc -o sloppySim

targs: thread_args.cc 
	$(COMPILER) $(COMPILERFLAGS) thread_args.cc -o targs

clean:
	rm targs
