CXXFLAGS=-c -std=c++11 -Wall -O2
SOURCES=src/main.cpp \
	src/Output.cpp \
	src/OutputGLSL.cpp \
	src/OutputHLSL.cpp \
	src/OutputMSL.cpp \
	src/Parser.cpp \
	src/Tokenizer.cpp
BASE_NAMES=$(basename $(SOURCES))
OBJECTS=$(BASE_NAMES:=.o)
EXECUTABLE=osl
DEPENDENCIES=$(OBJECTS:.o=.d)

.PHONY: all
ifeq ($(debug),1)
all: CXXFLAGS+=-DDEBUG -g
all: CFLAGS+=-DDEBUG -g
endif
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

-include $(DEPENDENCIES)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP $< -o $@

.PHONY: clean
clean:
	$(RM) $(EXECUTABLE)
	find "src" -name "*.o" -type f -delete
	find "src" -name "*.d" -type f -delete