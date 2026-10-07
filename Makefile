CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude -Ithird_party/include/harfbuzz -Ithird_party/include/fribidi
LDFLAGS ?= -Lthird_party/lib
LDLIBS = -lharfbuzz -lfribidi
RPATH = '-Wl,-rpath,$$ORIGIN/../third_party/lib'

LIB_SOURCES = $(wildcard src/*.cpp)
LIB_OBJECTS = $(patsubst src/%.cpp,build/%.o,$(LIB_SOURCES))
SHARED_OBJECTS = $(patsubst src/%.cpp,build/%.pic.o,$(LIB_SOURCES))

.PHONY: all test examples clean
all: build/libbidi_line245.a build/libbidi_line245.so bin/mixed_example bin/selection_example

build bin:
	mkdir -p $@

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.pic.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

build/libbidi_line245.a: $(LIB_OBJECTS)
	ar rcs $@ $^

build/libbidi_line245.so: $(SHARED_OBJECTS)
	$(CXX) -shared $^ $(LDFLAGS) $(LDLIBS) -Wl,-rpath,'$$ORIGIN/../../third_party/lib' -o $@

bin/%: examples/%.cpp build/libbidi_line245.a | bin
	$(CXX) $(CXXFLAGS) $< build/libbidi_line245.a $(LDFLAGS) $(LDLIBS) $(RPATH) -o $@

bin/selftest: tests/selftest.cpp build/libbidi_line245.a | bin
	$(CXX) $(CXXFLAGS) $< build/libbidi_line245.a $(LDFLAGS) $(LDLIBS) $(RPATH) -o $@

test: bin/selftest
	./bin/selftest

examples: bin/mixed_example bin/selection_example

clean:
	rm -rf build bin
