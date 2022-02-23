
#include "config.h"
#include "macro.h"
#include "linkage.h"
ENTRY(_main)


/*
 * Clear BSS section
 */
	ldr	x0, =__bss_start
	ldr	x1, =__bss_end
    cmp	x0, x1
    b.eq	1f
	mov	x2, #0
clear_loop:
	str	x2, [x0]
	add	x0, x0, #8
	cmp	x0, x1
	b.lo	clear_loop

/*
 * Set up initial C runtime environment and call board_init_f(0).
 */
1:
        ldr     x0, =(CONFIG_SYS_INIT_SP_ADDR)
        bic     sp, x0, #0xf    /* 16-byte alignment for ABI compliance */
        mov     x0, #0
        bl      board_init_f

	/* NOTREACHED - board_init_r() does not return */

ENDPROC(_main)
