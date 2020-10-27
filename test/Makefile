CXXFLAGS=-std=c++17 -Wpedantic -O2 -I../external/Catch2/single_include -I../osl
SOURCES=main.cpp tests.cpp
BASE_NAMES=$(basename $(SOURCES))
OBJECTS=$(BASE_NAMES:=.o)
DEPENDENCIES=$(OBJECTS:.o=.d)
OUTDIR=bin
EXECUTABLE=$(OUTDIR)/test

.PHONY: all
ifeq ($(debug),1)
all: CXXFLAGS+=-DDEBUG -g
endif
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	mkdir -p $(OUTDIR)
	$(CXX) $^ $(LDFLAGS) -o $@

-include $(DEPENDENCIES)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -MMD -MP $< -o $@

.PHONY: clean
clean:
	$(RM) -r $(OUTDIR) *.o *.d