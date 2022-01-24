
.PHONY : all build clean modgraph

all : build modgraph

modgraph:
	(cd modgraph; make)

build:
	@rm -fr build
	@mkdir build
	@(cd build; CXX=clang++ cmake ..)

clean :
	@rm -frv build
	@(cd modgraph; make clean)
