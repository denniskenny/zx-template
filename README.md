# zx-template

Reusable pieces for building a ZX Spectrum game in C with z88dk: the
skills that hold the platform knowledge, a ZX0 decompression path, and the
Tritone beeper-music pipeline.

Extracted from a working 48K/128K/+3 game. Everything here has shipped in
something and the awkward parts are documented where they bit rather than
where they belong.

## What is here

```
.devin/skills/          nine skills: the platform knowledge
src/dzx0.c              ZX0 decompression, C wrapper round z88dk's asm
include/dzx0.h
src/music.c             tunes, unpacked on the way in
include/music.h
assets/music/           the Tritone engine and its Beepola template
tools/                  the build-time converters
```

### The skills

Read these first; they are the reason this repo exists.

| skill | what it knows |
|---|---|
| `zx-memory` | the 48K/128K/+3 map, contended vs uncontended, banking, the ROM-select trap, compressing music and text |
| `zx-loader` | building a multi-block `.tap` by hand, loading into a bank, the loading screen, silencing the ROM's messages |
| `zx-tiles` | tile and sprite sheets in ZX-Paintbrush, masks, attribute modes |
| `tiled-maps` | importing Tiled `.tmx` maps, GID mapping, per-map headers |
| `tritone-music` | arranging 3-channel + drums beeper music and linking it |
| `compile-scr` | full-screen pictures: `.scr`, cropping, dithered reveals |
| `floating-bus-vsync` | tear-free frames without interrupts, on all three machines |
| `zesarux-test` | driving the emulator headlessly over ZRCP |
| `test-design` | writing emulator tests that are capable of failing |
| `zx0-layout` | **start here on a new project**: which region code, graphics, text and audio each belong in, and what is worth compressing |

`zx0-layout` is the one to read first on a greenfield project -- it is the
layout decision, made once, that everything else depends on.

`test-design` is the one to read before writing a harness. Every rule in it
is there because a test passed while the bug was still on screen.

### The ZX0 path

`dzx0_decompress(src, dst)` over z88dk's `dzx0_standard`. Compress with
`z88dk-zx0 -f in out`; verify a round trip with `z88dk-dzx0` in the build
rather than trusting the compressor.

Worth knowing before relocating anything compressed: **data with internal
pointers must be assembled at the address it will be unpacked to.** See
`zx-memory` § Compressing music — a song's order table is absolute
addresses, and unpacking it elsewhere silently corrupts every one.

### The Tritone pipeline

```
assets/music/NAME.txt          the arrangement, by hand
  tools/txt2tritone.py         -> NAME.asm  (Beepola-style, engine + data)
  tools/gen_tritone_module.py  -> NAME_linkable.asm  (data only)
zcc links ONE shared engine + one blob per tune
```

`src/music.c` unpacks a tune into a single shared buffer and calls the
engine. One buffer, because tunes block: only one is ever live.

### The tools

| tool | job |
|---|---|
| `checkmem.py` | fail the build if the linker crosses `0xC000`; report free memory per region |
| `mktap.py` | build a multi-block `.tap`: code blocks, bank blocks, a loading screen |
| `mkassets.py` | sweep `*_zx0` arrays out of generated headers into one low block |
| `zxp_tiles_zx0.py` | ZX-Paintbrush strips -> compressed tile/sprite headers |
| `mklogo.py` | a `.zxp` -> raw display-file blocks for a loading screen |
| `txt2tritone.py`, `gen_tritone_module.py` | the music pipeline |

## What is NOT here

No game: no state machine, no rules, no map format beyond the Tiled
importer, no `Makefile`. Those were specific to the project this came from,
and a bootstrap that dictates them is a fork rather than a template.

The tools expect a few conventions the absent Makefile used to supply --
`MEM_*` constants in `include/memmap.h`, generated headers in `include/`,
`*_zx0` naming for compressed arrays. Each tool's docstring says what it
needs.

## Provenance

The skills quote real numbers, real failures and real dead ends, because
the specifics are what make them usable. Names of that game's rules have
been generalised; the measurements have not been touched.
