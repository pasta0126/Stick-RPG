# Herramientas de PC (simulador de balance y pruebas). El firmware se compila
# con PlatformIO: `pio run` / `pio run -t upload`.

CXX      ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
CORE     := $(wildcard src/core/*.cpp)
HDRS     := $(wildcard src/core/*.h)

.PHONY: all sim test clean sprites art release

ASEPRITE ?= /Applications/Aseprite.app/Contents/MacOS/aseprite
SPRITES  := $(wildcard assets/sprites/*.aseprite)
SPRITE_H := $(patsubst assets/sprites/%.aseprite,src/gfx/%_gfx.h,$(SPRITES))

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

# Exporta cada assets/sprites/<n>.aseprite a src/gfx/<n>_gfx.h
sprites: $(SPRITE_H)

src/gfx/%_gfx.h: assets/sprites/%.aseprite tools/sprites/export_c.lua
	$(ASEPRITE) -b $< --script-param out=$@ --script-param name=$* \
		--script tools/sprites/export_c.lua

# Regenera TODO el arte desde tools/art (SOBRESCRIBE los retoques a mano en
# assets/sprites/*.aseprite) y lo exporta a C.
art:
	ASEPRITE=$(ASEPRITE) python3 tools/art/build.py
	$(MAKE) sprites

# Un solo .bin para grabar en 0x0 (M5Burner / esptool / web flasher).
PIO_BUILD := .pio/build/m5stickc_plus2
ESPTOOL   := $(HOME)/.platformio/packages/tool-esptool/esptool
BOOT_APP0 := $(HOME)/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin
release:
	pio run
	mkdir -p release
	$(ESPTOOL) --chip esp32 merge-bin -o release/StickRPG-M5StickCPlus2.bin \
	  --flash-mode dio --flash-freq 80m --flash-size 8MB \
	  0x1000 $(PIO_BUILD)/bootloader.bin 0x8000 $(PIO_BUILD)/partitions.bin \
	  0xe000 $(BOOT_APP0) 0x10000 $(PIO_BUILD)/firmware.bin
