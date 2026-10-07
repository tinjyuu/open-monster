GBDK_HOME ?= .tools/gbdk
LCC := $(GBDK_HOME)/bin/lcc
PYTHON ?= python3
ROM := dist/open-monster.gbc
.PHONY: all data test clean
all: $(ROM)
data:
	$(PYTHON) scripts/build_data.py
$(ROM): Makefile src/main.c src/core.c src/core.h src/music.c src/music.h scripts/build_data.py scripts/check_rom_layout.py scripts/localization.py $(wildcard locales/*.json) assets/font.json assets/ui-art.json scripts/scene_codegen.py scripts/sprite_codegen.py $(wildcard data/monsters/*.json) data/moves.json
	$(PYTHON) scripts/build_data.py
	mkdir -p dist
	$(LCC) -msm83:gb -Wm-yC -Wm-yt0x1B -Wm-yo8 -Wm-ya1 -Wm-ynOPENMONSTER -Wl-m -Wl-j -Wl-b_HOME=0x0200 -Wl-b_CODE=0x0400 -o $(ROM) src/main.c src/core.c src/generated.c src/music.c
	$(PYTHON) scripts/check_rom_layout.py
test: data
	$(PYTHON) -m unittest discover -s tests -p 'test_*.py'
	cc -std=c99 -Wall -Wextra -Werror -Isrc tests/core_test.c src/core.c src/generated.c -o .tools/core-test
	.tools/core-test
clean:
	rm -f dist/*.gbc dist/*.map dist/*.noi dist/*.sym src/generated.c src/generated.h src/locale_ids.h
