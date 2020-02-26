CFLAGS=-c -Wpedantic -O2 -Iosl
CXXFLAGS=-c -std=c++14 -Wpedantic -O2 -Iosl
OUTDIR=bin
EXECUTABLE=$(OUTDIR)/osl
TEST_EXECUTABLE=$(OUTDIR)/test

.PHONY: all
ifeq ($(debug),1)
all: CXXFLAGS+=-DDEBUG -g
all: CFLAGS+=-DDEBUG -g
endif
all: osl
all: test

.PHONY: osl
osl: $(EXECUTABLE)

$(EXECUTABLE): src/main.o
	mkdir -p $(OUTDIR)
	$(CXX) $^ $(LDFLAGS) -o $@

.PHONY: test
test: $(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): test/main.o
	mkdir -p $(OUTDIR)
	$(CXX) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP $< -o $@

.PHONY: clean
clean:
	$(RM) -r $(OUTDIR)
	find "osl" -name "*.o" -type f -delete
	find "osl" -name "*.d" -type f -delete