# Herramientas de PC (simulador de balance y pruebas). El firmware se compila
# con PlatformIO: `pio run` / `pio run -t upload`.

CXX      ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
CORE     := $(wildcard src/core/*.cpp)
HDRS     := $(wildcard src/core/*.h)

.PHONY: all sim test clean

all: test sim

build/sim: tools/sim/sim.cpp $(CORE) $(HDRS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ tools/sim/sim.cpp $(CORE)

build/test_core: tools/test/test_core.cpp $(CORE) $(HDRS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ tools/test/test_core.cpp $(CORE)

sim: build/sim
	./build/sim $(ARGS)

test: build/test_core
	./build/test_core

clean:
	rm -rf build
