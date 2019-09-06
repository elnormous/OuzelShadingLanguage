CXXFLAGS=-c -std=c++11 -Wpedantic -O2
SOURCES=osl/main.cpp \
	osl/Output.cpp \
	osl/OutputGLSL.cpp \
	osl/OutputHLSL.cpp \
	osl/OutputMSL.cpp \
	osl/Parser.cpp \
	osl/Preprocessor.cpp \
	osl/Tokenizer.cpp
BASE_NAMES=$(basename $(SOURCES))
OBJECTS=$(BASE_NAMES:=.o)
OUTDIR=bin
EXECUTABLE=osl
DEPENDENCIES=$(OBJECTS:.o=.d)

.PHONY: all
ifeq ($(debug),1)
all: CXXFLAGS+=-DDEBUG -g
all: CFLAGS+=-DDEBUG -g
endif
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	mkdir -p $(OUTDIR)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(OUTDIR)/$@

-include $(DEPENDENCIES)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP $< -o $@

.PHONY: clean
clean:
	$(RM) $(OUTDIR)/$(EXECUTABLE)
	find "osl" -name "*.o" -type f -delete
	find "osl" -name "*.d" -type f -delete