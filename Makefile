
.PHONY : all clean modgraph

all : modgraph

modgraph:
	(cd modgraph; make)

clean :
	@(cd modgraph; make clean)
