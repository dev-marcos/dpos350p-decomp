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
ASM_SRCS  := $(wildcard asm/*.s)
ASM_OBJS  := $(patsubst %.s,$(BUILD)/%.o,$(ASM_SRCS))

.PHONY: all extract check clean distclean
.DELETE_ON_ERROR:

all: $(ISO)

extract: $(EXTRACTED)

$(EXTRACTED) &: $(ORIG_ISO) tools/dposiso.py
	@test -f $(ORIG_ISO) || { echo "Missing $(ORIG_ISO): copy the original FNIRSI ISO there"; exit 1; }
	$(PYTHON) tools/dposiso.py unpack $(ORIG_ISO) $(EXTRACT)

$(BUILD)/asm/app.o: $(EXTRACT)/app.bin
$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -I. -o $@ $<

$(BUILD)/app.elf: $(ASM_OBJS) ld/dpos350p.ld
	$(LD) -T ld/dpos350p.ld -Map=$(BUILD)/app.map -o $@ $(ASM_OBJS)

$(BUILD)/app.bin: $(BUILD)/app.elf
	$(OBJCOPY) -O binary $< $@

$(ISO): $(BUILD)/app.bin $(EXTRACT)/logo.rgb565 tools/dposiso.py
	$(PYTHON) tools/dposiso.py pack --logo $(EXTRACT)/logo.rgb565 --app $(BUILD)/app.bin -o $@

check: $(ISO)
	$(PYTHON) tools/dposiso.py verify $(ISO) --sha256 $(SHA256)
	@echo "$(ISO): MATCH"

clean:
	rm -rf $(BUILD)/asm $(BUILD)/app.elf $(BUILD)/app.bin $(BUILD)/app.map $(ISO)

distclean:
	rm -rf $(BUILD)
