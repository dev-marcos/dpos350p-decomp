# DPOS350P matching decompilation -- build 2025-06-19
# Run inside the dpos-re WSL distro with tools/env.sh sourced:
#   wsl -d dpos-re -- bash tools/run.sh make check

ORIG_ISO ?= firmware/DPOS350P-FNIRSI-2025619.iso
SHA256   := 6635ddd661fa6aa6a36672c1428e74d0f78913a24fd5a425cebedff0acefd58c

CROSS   ?= arm-none-eabi-
AS      := $(CROSS)as
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy
PYTHON  ?= python3

ASFLAGS := -mcpu=cortex-a7 -mfpu=neon-vfpv4
BUILD   := build
EXTRACT := $(BUILD)/extract
ISO     := $(BUILD)/DPOS350P.iso

EXTRACTED := $(EXTRACT)/app.bin $(EXTRACT)/logo.rgb565
BLOBS_OK  := $(BUILD)/.blobs_ok
ASM_SRCS  := $(wildcard asm/*.s)
ASM_OBJS  := $(patsubst %.s,$(BUILD)/%.o,$(ASM_SRCS))

.PHONY: all extract check map split nonmatching emutest selftest progress clean distclean
.DELETE_ON_ERROR:

all: $(ISO)

extract: $(EXTRACTED) $(BLOBS_OK)

$(EXTRACTED) &: $(ORIG_ISO) tools/dposiso.py
	@test -f $(ORIG_ISO) || { echo "Missing $(ORIG_ISO): copy the original FNIRSI ISO there"; exit 1; }
	$(PYTHON) tools/dposiso.py unpack $(ORIG_ISO) $(EXTRACT)

# data/*.bin are FNIRSI bytes: extracted from the user's ISO, never committed
$(BLOBS_OK): $(EXTRACT)/app.bin data/blobs.csv tools/split.py
	$(PYTHON) tools/split.py extract
	@touch $@

$(filter $(BUILD)/asm/data_%,$(ASM_OBJS)): $(BLOBS_OK)
$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) -I. -o $@ $<

$(BUILD)/objs.rsp: $(ASM_OBJS)
	$(file >$@,$(ASM_OBJS))

$(BUILD)/app.elf: $(BUILD)/objs.rsp $(ASM_OBJS) ld/dpos350p.ld ld/sections.ld
	$(LD) -T ld/dpos350p.ld -Map=$(BUILD)/app.map -o $@ @$(BUILD)/objs.rsp

$(BUILD)/app.bin: $(BUILD)/app.elf
	$(OBJCOPY) -O binary $< $@

$(ISO): $(BUILD)/app.bin $(EXTRACT)/logo.rgb565 tools/dposiso.py
	$(PYTHON) tools/dposiso.py pack --logo $(EXTRACT)/logo.rgb565 --app $(BUILD)/app.bin -o $@

check: $(ISO)
	$(PYTHON) tools/dposiso.py verify $(ISO) --sha256 $(SHA256)
	@echo "$(ISO): MATCH"

# Phase 3 regeneration of asm/ + ld/sections.ld + data/blobs.csv (overwrites asm/!)
split: map
	$(PYTHON) tools/split.py gen

# Phase 2 static map: symbols/symbols_2025-06-19.csv, build/analysis/map.json
map: $(EXTRACT)/app.bin
	$(PYTHON) tools/import_refs.py
	$(PYTHON) tools/analyze.py
	$(PYTHON) tools/mkmap.py

# Phase 5: equivalent C (never part of the shipped build), verified by emulation
nonmatching: extract
	$(PYTHON) tools/nmbuild.py

emutest: nonmatching
	$(PYTHON) -m tools.emu.run_tests

selftest: nonmatching
	$(PYTHON) -m tools.emu.selftest

progress: $(BUILD)/analysis/map.json
	-$(PYTHON) -m tools.emu.run_tests >/dev/null
	$(PYTHON) tools/progress.py

$(BUILD)/analysis/map.json: $(EXTRACT)/app.bin
	$(MAKE) map

clean:
	rm -rf $(BUILD)/asm $(BUILD)/app.elf $(BUILD)/app.bin $(BUILD)/app.map $(BUILD)/objs.rsp $(ISO)

distclean:
	rm -rf $(BUILD)
