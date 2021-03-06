# Grab the targets and sources as two batches
SOURCES = $(wildcard src/*.cxx)
HEADERS = $(wildcard include/*)
OBJECTS = $(patsubst src%.cxx,build%.o,$(SOURCES))

# Versioning info
MAJOR=0
MINOR=7.0
SONAME=libfid.so
LIBNAME=$(SONAME).$(MAJOR).$(MINOR)
PREFIX=/usr/local

# Figure out the architecture
UNAME_S = $(shell uname -s)

# Clang compiler
ifeq ($(UNAME_S), Darwin)
	CXX = clang++
	CC  = clang
	FLAGS = -std=c++11
	LDCONFIG = cp $(PREFIX)/lib/$(LIBNAME) $(PREFIX)/lib/$(SONAME).$(MAJOR)
endif

# Gnu compiler
ifeq ($(UNAME_S), Linux)
	CXX = g++
	CC  = gcc
	FLAGS = -std=c++0x
	LDCONFIG = ldconfig -n -v $(PREFIX)/lib
endif

# Some optional compiler flags
ifdef DEBUG
	CC += -g -pg
	CXX += -g -pg
else
	CC += -O3
	CXX += -O3
endif

FLAGS += -Wall -fPIC $(DEBUG) $(OPTIMIZE) -Iinclude

ifdef BOOST_INC
	FLAGS += -I$(BOOST_INC)
	LIBS += -L$(BOOST_LIB)
endif

LIBS += -lboost_filesystem -lboost_system -lfftw3 -lm 

FLAGS += $(shell root-config --cflags)
LIBS  += $(shell root-config --libs)

all: lib/$(LIBNAME)

build/%.o: src/%.cxx
	$(CXX) $(FLAGS) -o $@ -c $<

lib/$(LIBNAME): $(OBJECTS)
	$(CXX) -shared -fPIC $+ -o $@ $(LIBS)

install:
	mkdir -p $(PREFIX)/lib
	cp lib/$(LIBNAME) $(PREFIX)/lib
	mkdir -p $(PREFIX)/include
	cp -r $(HEADERS) $(PREFIX)/include
	ln -sf $(PREFIX)/lib/$(LIBNAME) $(PREFIX)/lib/$(SONAME)
	$(LDCONFIG)

uninstall:
	rm -f $(PREFIX)/lib/$(SONAME)*
	rm -rf $(patsubst include/%,$(PREFIX)/include/%,$(HEADERS))

clean:
	rm -f $(TARGETS) build/* lib/*
