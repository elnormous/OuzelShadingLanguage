CFLAGS=-c -Wpedantic -O2 -Iosl
CXXFLAGS=-c -std=c++11 -Wpedantic -O2 -Iosl
SOURCES=osl/Parser.cpp
BASE_NAMES=$(basename $(SOURCES))
OBJECTS=$(BASE_NAMES:=.o)
OUTDIR=bin
EXECUTABLE=$(OUTDIR)/osl
TEST_EXECUTABLE=$(OUTDIR)/test
LIBRARY=$(OUTDIR)/libosl.a
DEPENDENCIES=$(OBJECTS:.o=.d)

.PHONY: all
ifeq ($(debug),1)
all: CXXFLAGS+=-DDEBUG -g
all: CFLAGS+=-DDEBUG -g
endif
all: libosl
all: osl
all: test

.PHONY: libosl
libosl: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	mkdir -p $(OUTDIR)
	$(AR) rs $@ $^

.PHONY: osl
osl: $(EXECUTABLE)

$(EXECUTABLE): src/main.o $(LIBRARY)
	mkdir -p $(OUTDIR)
	$(CXX) $^ $(LDFLAGS) -o $@

.PHONY: test
test: $(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): test/main.o $(LIBRARY)
	mkdir -p $(OUTDIR)
	$(CXX) $^ $(LDFLAGS) -o $@

-include $(DEPENDENCIES)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP $< -o $@

.PHONY: clean
clean:
	$(RM) -r $(OUTDIR)
	find "osl" -name "*.o" -type f -delete
	find "osl" -name "*.d" -type f -delete