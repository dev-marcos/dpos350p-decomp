/* The 1022-pixel-wide RGB565 frame buffers: fb_front (shown), fb_back (drawn into),
 * fb_off (off-screen copy) and their geometry (build 2025-06-19, see docs/DRIVERS.md). */
#ifndef SHARED_FRAMEBUFFER_H
#define SHARED_FRAMEBUFFER_H

#define FB_PIXELS 0x95B50u                                   /* 1022 * 600 */
#define FB_W 1022u
#define fb_back ((unsigned short *)0x41BFBF68u)
#define fb_front ((unsigned short *)0x41FB8FE8u)
#define fb_off ((unsigned short *)0x4233CBBCu)

#endif /* SHARED_FRAMEBUFFER_H */
