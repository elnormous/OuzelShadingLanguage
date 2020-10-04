CXXFLAGS=-c -std=c++17 -Wpedantic -O2 -I../osl
OUTDIR=bin
EXECUTABLE=$(OUTDIR)/test

.PHONY: all
ifeq ($(debug),1)
all: CXXFLAGS+=-DDEBUG -g
endif
all: $(EXECUTABLE)

$(EXECUTABLE): main.o tests.o
	mkdir -p $(OUTDIR)
	$(CXX) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP $< -o $@

.PHONY: clean
clean:
	$(RM) -r $(OUTDIR) *.o *.d