DEBUG=1
CXXFLAGS=-std=c++17 -Wpedantic -O2 -I../osl
SOURCES=main.cpp
BASE_NAMES=$(basename $(SOURCES))
OBJECTS=$(BASE_NAMES:=.o)
DEPENDENCIES=$(OBJECTS:.o=.d)
OUTDIR=bin
EXECUTABLE=$(OUTDIR)/osl

.PHONY: all
ifeq ($(DEBUG),1)
all: CXXFLAGS+=-DDEBUG -g
else
all: CXXFLAGS+=-O3
all: LDFLAGS+=-O3
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