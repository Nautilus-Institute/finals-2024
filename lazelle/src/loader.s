_start:
    b   reset
    b   halt
    b   halt
    b   halt
    b   halt
    b   halt
    b   halt
    b   halt

    b   halt
    b   halt
    b   halt
    b   halt
    b   halt
    b   halt
    b   halt
    b   halt

reset:
    bl setup_lua

halt:
    mov r0,#0xF0000000
    str r0,[r0]

hang: b hang

setup_lua:
    ; the harness will load our initial lua code at a fixed address
    ; therefore to run the code, we'll set up our registers and jump to it

    ; global scratch sram
    mov r7, 0x18000000

    ; global activation frames stack
    mov sp, 0x18000000
    add sp, sp, #0xf00000

    ; lua stack starts here
    mov r9, 0x18000000
    add r9, r9, #0x800000

    add r9, r9, #4

    ; first lua funcinfo is at 0x10000000
    mov r0, #0x10000000

    ; r1 -> pc
    ldr r1, [r0, #0]

    ; protos
    ldr r6, [r0, #0x10]

    ; upvals for the first function are defined in vmain, and already contain _ENV
    ; vmain will tell us where this is via the first value in scratch
    ldr r10, [r7]

    ; constants
    ldr r11, [r0, #0xc]

    b pivot_to_lua
pivot_to_lua:

    ; inject bxl just before start of lua
    sub r1, r1, #4
    mov r3, #0xea
    mov r3, r3, lsl #0x8
    add r3, r3, #0x00
    mov r3, r3, lsl #0x8
    add r3, r3, #0x33
    mov r3, r3, lsl #0x8
    add r3, r3, #0x33
    str r3, [r1]

    ; pivot
    mov pc, r1

putc:
    mov r1, r0
    mov r0, #0xD0000000
    b PUT32

PUT32:
    str r1,[r0]
    mov pc,lr

GET32:
    ldr r0,[r0]
    mov pc,lr

ASMDELAY:
    subs r0,r0,#1
    bne ASMDELAY
    mov pc,lr
