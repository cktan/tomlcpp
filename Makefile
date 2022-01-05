HFILES = toml.h tomlcpp.hpp
CFILES = toml.c
CPPFILES = tomlcpp.cpp
OBJ = $(CFILES:.c=.o)  $(CPPFILES:.cpp=.o)
EXEC = toml_json toml_sample

CFLAGS = -Wall -Wextra -fpic
LIB = libtomlcpp.a
LIB_SHARED = libtomlcpp.so

# to compile for debug: make DEBUG=1
# to compile for no debug: make
ifdef DEBUG
    CFLAGS += -O0 -g
else
    CFLAGS += -O2 -DNDEBUG
endif
CXXFLAGS := $(CFLAGS) -std=c++17
CFLAGS += -std=c99


all: $(LIB) $(LIB_SHARED) $(EXEC)

*.o: $(HFILES)

libtomlcpp.a: $(OBJ)
	ar -rcs $@ $^

libtomlcpp.so: $(OBJ)
	$(CXX) $(CXXFLAGS) -shared -o $@ $^

$(EXEC): $(LIB)

prefix ?= /usr/local

install: all
	install -d ${prefix}/include ${prefix}/lib
	install toml.h ${prefix}/include
	install tomlcpp.hpp ${prefix}/include
	install $(LIB) ${prefix}/lib
	install $(LIB_SHARED) ${prefix}/lib

clean:
	rm -f *.o $(EXEC) $(LIB) $(LIB_SHARED)

format:
	clang-format -i $(shell find . -name '*.[ch]') $(shell find . -name '*.[ch]pp')

.PHONY: all clean install format
