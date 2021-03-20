a.out: source.o command.o
	g++ command.o source.o -o a.out

command.o: command.cpp
	g++ command.cpp -c

source.o: source.cpp
	g++ source.cpp -c
