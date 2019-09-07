CXXFLAGS=-c -std=c++11 -Wpedantic -O2 -Iosl
SOURCES=osl/Output.cpp \
	osl/OutputGLSL.cpp \
	osl/OutputHLSL.cpp \
	osl/OutputMSL.cpp \
	osl/Parser.cpp \
	osl/Preprocessor.cpp \
	osl/Tokenizer.cpp
BASE_NAMES=$(basename $(SOURCES))
OBJECTS=$(BASE_NAMES:=.o)
OUTDIR=bin
EXECUTABLE=$(OUTDIR)/osl
LIBRARY=$(OUTDIR)/libosl.a
DEPENDENCIES=$(OBJECTS:.o=.d)

.PHONY: all
ifeq ($(debug),1)
all: CXXFLAGS+=-DDEBUG -g
all: CFLAGS+=-DDEBUG -g
endif
all: libosl
all: osl

.PHONY: libosl
libosl: $(LIBRARY)

.PHONY: osl
osl: $(EXECUTABLE)

$(LIBRARY): $(OBJECTS)
	mkdir -p $(OUTDIR)
	$(AR) rs $@ $^

$(EXECUTABLE): src/main.o $(LIBRARY)
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