all: project

lex.o: lex.cpp
	g++ -Wall -g lex.cpp -c

rpn.o: lex.o rpn.cpp
	g++ -Wall -g rpn.cpp -c

syntax.o: syntax.cpp
	g++ -Wall -g syntax.cpp -c

executor.o: executor.cpp
	g++ -Wall -g executor.cpp -c

project.o: lex.o rpn.o syntax.o project
	g++ -Wall -g project.cpp -c

project:
	g++ -Wall -g lex.cpp rpn.cpp syntax.cpp executor.cpp project.cpp -o project


