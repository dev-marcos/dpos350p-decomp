/*
 * Phase 1: the whole application image as one blob.
 * build/extract/app.bin is extracted from the user's ISO by `make extract`
 * (FNIRSI binaries are never committed). Later phases replace this file with
 * per-function assembly / C.
 */
    .arm
    .section .text.app, "ax"
    .global app_start
app_start:
    .incbin "build/extract/app.bin"
    .global app_end
app_end:
