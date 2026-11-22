/*
 * usb_device_set_ep0_state -- stock 0x4006AC5C (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * usb_ep0_state = state.
 */
#include "hw/usb.h"
#include "usb/core.h"

void usb_device_set_ep0_state(int state) { usb_ep0_state = state; }
