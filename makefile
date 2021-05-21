all:
	g++ -g -Wall -Wextra -std=c++0x MyShell.cpp Utils.cpp BuiltIns.cpp -o shell