
CC := g++
CXX := g++
CXXFLAGS := -g -Wall -std=c++17
LDLIBS := -lgsl

# See
# 'http://make.mad-scientist.net/papers/advanced-auto-dependency-generation'.
SRCS := $(shell ls *.cpp)

.PHONY : clean

all : modgraph

modgraph : $(SRCS:.cpp=.o)

clean :
	@rm -fv [0-9]*.asy
	@rm -fv *.eps
	@rm -fv modgraph
	@rm -fv *.neato
	@rm -fv *.o
	@rm -fv *.pdf
	@rm -fv *.png
	@rm -fv texput.*

# See
# 'http://make.mad-scientist.net/papers/advanced-auto-dependency-generation'.
DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
COMPILE.cxx = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
%.o : %.cpp
%.o : %.cpp $(DEPDIR)/%.d | $(DEPDIR); $(COMPILE.cxx) $(OUTPUT_OPTION) $<
$(DEPDIR): ; @mkdir -p $@
DEPFILES := $(SRCS:%.cpp=$(DEPDIR)/%.d)
$(DEPFILES):
include $(wildcard $(DEPFILES))
