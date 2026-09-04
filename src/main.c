/*
 * main.c -- hello world, with the two things every ZX game needs first:
 * knowing which machine it is on, and a frame it can draw inside.
 *
 * Neither is obvious and both are easy to get subtly wrong, so this is
 * the smallest program that does them properly rather than the smallest
 * program that prints something.
 *
 * WHAT IT DOES
 *
 *   1. hw_detect()    -- 48K or 128K-class, and whether a Kempston is
 *                        present.  Writing a byte into a paged bank and
 *                        reading it back is the only reliable test; the
 *                        ROM's own signature bytes lie on clones.
 *   2. vsync_detect() -- picks a frame-sync method for THIS machine.  A
 *                        48K reads the floating bus at 0x40FF; a 128K
 *                        reads it at 0x0FFD; a +2A/+3 has no usable
 *                        floating bus at all and falls back to HALT.
 *   3. prints what it found, then loops in step with the beam, flipping
 *                        the border so the sync is visible as a stable
 *                        band rather than a flickering one.
 *
 * WHY THE BORDER FLIP
 *
 * A frame loop that is working looks like nothing at all, which makes it
 * impossible to tell from a frame loop that is hung.  Changing the border
 * every frame turns "is it synced?" into something the eye answers
 * immediately: a steady horizontal edge means the loop wakes at the same
 * point every frame, a wandering one means it does not.
 */

#include <stdint.h>

#include "../config/app_config.h"
#include "../include/gfx.h"
#include "../include/hw.h"
#include "../include/strings.h"
#include "../include/vsync.h"

/* Names for what vsync_detect() decided.  Worth printing: which one you
   get is the difference between a tear-free frame and a HALT that drifts,
   and on a +2A/+3 you do NOT get to choose. */
static const char *vsync_name(void)
{
    switch (vsync_mode) {
        case VSYNC_MODE_48K:  return TXT_VSYNC_BUS_48K;
        case VSYNC_MODE_128K: return TXT_VSYNC_BUS_128K;
        default:              return TXT_VSYNC_HALT;
    }
}

/* Frames since start.  Exported and volatile so a debugger can watch it:
   a frame loop that is working and one that is hung look identical from
   the outside, and this is the cheapest way to tell them apart.  See
   .devin/skills/zesarux-test for reading it over ZRCP. */
volatile uint16_t frames;

int main(void)
{

    /* Paging is locked on machines that do not need it unlocked, so
       nothing can page a bank in over the buffers by accident.  Do this
       before anything is placed above 0xC000. */
    hw_detect();
    vsync_detect();

    gfx_target(SCREEN);
    screen_clear(0x07);         /* white on black, the whole screen */

    print_at(1, 1, TXT_TITLE);
    set_attr_rect(0, 1, 32, 1, 0x45);

    print_at(1, 4, TXT_LBL_MACHINE);
    print_at(11, 4, is_128k ? TXT_MACHINE_128K : TXT_MACHINE_48K);

    print_at(1, 5, TXT_LBL_KEMPSTON);
    print_at(11, 5, has_kempston ? TXT_YES : TXT_NO);

    print_at(1, 6, TXT_LBL_VSYNC);
    print_at(11, 6, vsync_name());

    print_at(1, 9, TXT_NOTE_1);
    print_at(1, 10, TXT_NOTE_2);
    print_at(1, 11, TXT_NOTE_3);

    /* THE FRAME LOOP.
     *
     * vsync_wait() returns with the beam just past the bottom of the
     * display, so the border and vblank -- about 28 000 T-states on a
     * 48K -- are free for anything that writes to the screen.  Draw
     * AFTER it returns, not before.
     *
     * Nothing here is timing-critical, so the border is all it does. */
    for (;;) {
        vsync_wait();
        frames++;
        border((uint8_t)((frames >> 4) & 7));
    }
}
