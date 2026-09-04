# zx-template — hello world with hardware detection and a synced frame.
#
# This is deliberately the smallest Makefile that gets the ORDER right,
# because the order is what is hard to rediscover:
#
#   1. compile and link with -create-app  ->  one CODE block
#   2. checkmem the map                   ->  fail if the linker crossed
#                                             0xC000, where a 128K pages
#   3. build the .tap with tools/mktap.py ->  a real BASIC loader
#
# Step 2 is not optional.  Anything the linker puts at 0xC000 or above
# disappears the moment something pages a bank, and the failure looks
# like random corruption rather than a crash.  See .devin/skills/zx-memory.

# Explicit, because Make takes the FIRST target in the file as the goal
# and a rule added above `all:` silently becomes the default -- which is
# what happened when the strings generator was added.
.DEFAULT_GOAL := all

Z88DK  ?= $(HOME)/z88dk
ZCC    ?= $(Z88DK)/bin/zcc
ZCCCFG ?= $(Z88DK)/lib/config
PYTHON ?= python3
USER_CFLAGS ?=

APP        = zxgame
ORG_DEF    = -zorg=32768
USR_ADDR   = 32768
CLEAR_ADDR = 32767      # 0x7FFF: BASIC keeps below, the program starts above
MEM_LIMIT  = 0xC000

# -SO3 with --opt-code-size: this project measured 565 bytes between
# --opt-code-speed and --opt-code-size with no perceptible difference in a
# scrolling display.  Start small; switch if a frame budget says to.
#
# CRT_ENABLE_STDIO=0 because the game prints through print_at(), not
# printf.  It does NOT remove the console driver -- that is pulled in
# unconditionally by the zx target's own crt0 -- but it stops anything
# else being dragged along behind it.
CFLAGS = +zx -vn -SO3 $(ORG_DEF) -startup=31 --opt-code-size \
         -compiler=sdcc -mz80 -pragma-define:CRT_ENABLE_STDIO=0 \
         --reserve-regs-iy --allow-unsafe-read \
         -Cc--max-allocs-per-node=50000
LDFLAGS = -lm -create-app

SRCS = src/main.c src/strings.c src/hw_detect.c src/vsync.c src/gfx.c \
       src/no_font64.asm
HEADERS = config/app_config.h include/gfx.h include/hw.h include/vsync.h \
          include/memmap.h include/strings.h

# text/strings.txt -> the generated pair.  Every string the player reads
# lives in that one file, identical text is stored once, and the build
# fails on a string wider than the screen.
#
# Add --zx0 $(ZX0) when the text is worth compressing; tools/mktext.py
# prints the before and after so the decision is a measurement.  Below a
# few hundred bytes it is not worth the buffer.
ZX0 ?= $(Z88DK)/bin/z88dk-zx0

include/strings.h src/strings.c: text/strings.txt tools/mktext.py
	$(PYTHON) tools/mktext.py $< --header include/strings.h \
	    --source src/strings.c --width 32

.PHONY: all clean map run
all: $(APP).tap

# ONE zcc invocation, then mktap builds the tape.  -create-app emits a
# 30-byte BASIC loader of its own; mktap replaces it because anything
# beyond a single CODE block -- a low asset block, a bank, a loading
# screen -- needs a loader that knows about them.
$(APP).tap: $(SRCS) $(HEADERS) tools/mktap.py
	PATH=$(Z88DK)/bin:$$PATH Z88DK=$(Z88DK) ZCCCFG=$(ZCCCFG) \
	    $(ZCC) $(CFLAGS) $(USER_CFLAGS) -o $(APP) $(SRCS) $(LDFLAGS)
	$(PYTHON) tools/mktap.py $(APP).tap --name ' ' \
	    --clear $(CLEAR_ADDR) --usr $(USR_ADDR) \
	    --code $(USR_ADDR) $(APP)

# `make map` is the one to run before committing: it rebuilds with a link
# map, enforces the ceiling, and prints what is left in each region.
map:
	$(MAKE) clean
	$(MAKE) USER_CFLAGS="-m"
	$(PYTHON) tools/checkmem.py $(APP).map --limit $(MEM_LIMIT)
	@$(PYTHON) tools/checkmem.py $(APP).map --free --limit $(MEM_LIMIT)

run: $(APP).tap
	fuse --machine 128 $(APP).tap

clean:
	rm -f $(APP) $(APP).tap $(APP).map $(APP)_* *.o src/*.o
