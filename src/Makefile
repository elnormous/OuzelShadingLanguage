CFLAGS=-c -Wpedantic -O2 -I../osl
CXXFLAGS=-c -std=c++11 -Wpedantic -O2 -I../osl
OUTDIR=bin
EXECUTABLE=$(OUTDIR)/osl

.PHONY: all
ifeq ($(debug),1)
all: CXXFLAGS+=-DDEBUG -g
all: CFLAGS+=-DDEBUG -g
endif
all: $(EXECUTABLE)

$(EXECUTABLE): main.o
	mkdir -p $(OUTDIR)
	$(CXX) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP $< -o $@

.PHONY: clean
clean:
	$(RM) -r $(OUTDIR) *.o *.d