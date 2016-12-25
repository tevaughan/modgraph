
CXXFLAGS = -Wall -std=c++11

.PHONY : clean

all : modgraph

modgraph : modgraph.cpp node.hpp graph.hpp
	g++ -o $@ $(CXXFLAGS) $<

clean :
	@rm -fv *.neato
	@rm -fv *.png
	@rm -fv *.o
	@rm -fv modgraph

