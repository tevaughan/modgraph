
CC := g++
CXX := g++
CXXFLAGS := -Wall -std=c++17

# See
# 'http://make.mad-scientist.net/papers/advanced-auto-dependency-generation'.
SRCS := $(shell ls *.cpp)

.PHONY : clean

all : modgraph

modgraph : $(SRCS:.cpp=.o)

clean :
	@rm -fv *.neato
	@rm -fv *.png
	@rm -fv *.o
	@rm -fv modgraph

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
