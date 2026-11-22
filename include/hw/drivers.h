/* Prototypes of stock driver helpers (addresses in symbols/manual.csv). */
#ifndef HW_DRIVERS_H
#define HW_DRIVERS_H
#include "hw/v3s.h"




extern volatile unsigned char g_fpga_bus_is_output;                /* 0x42E68D18 */


#define PIO_PG 0x01C208D8u
#define GT_SDA 6   /* PG6 */
#define GT_SCL 7   /* PG7 */

/* MS5351M (Si5351-compatible) clock generator on PE20 (SCL) / PE21 (SDA), device 0xC0 */
#define MS_SCL 20
#define MS_SDA 21

#define SYS_INFO_ADDR 0x40544E68u    /* stock SYS_INFO structure (Hack docs/SYS_INFO.md) */
#endif
