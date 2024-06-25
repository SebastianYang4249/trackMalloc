main: main.cpp
	g++ -g -Wall -Wextra -Werror=return-type -rdynamic main.cpp -o main

.PHONY: 
	clean

clean:
	-rm -f main
