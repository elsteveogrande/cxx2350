.syntax unified
.cpu	  cortex-m33
.fpu	  fpv5-sp-d16

.text

.globl  _ZN6rp23503sys9hardFaultEv
.p2align 2
.thumb_func
_ZN6rp23503sys9hardFaultEv:
    movs  r2, #0x48
    b     __fault

.globl  _ZN6rp23503sys9memManageEv
.p2align 2
.thumb_func
_ZN6rp23503sys9memManageEv:
    movs  r2, #0x4d
    b     __fault

.globl  _ZN6rp23503sys8busFaultEv
.p2align 2
.thumb_func
_ZN6rp23503sys8busFaultEv:
    movs  r2, #0x42
    b     __fault

.globl  _ZN6rp23503sys10usageFaultEv
.p2align 2
.thumb_func
_ZN6rp23503sys10usageFaultEv:
    movs  r2, #0x55
    b     __fault

.p2align 2
__fault:
    @ Stack will contain the following (ignoring possible FPU registers).
    @ Pushed by processor on exception dispatch:
    @   xPSR,
    @   faulting instruction PC,
    @   LR (R14),
    @   R12,
    @   R3 through R0
    @ Followed by these which we'll push below:
    @   fault type in R2 (from above),
    @   exception data (in LR),
    @   R11 through R4,
    @   SP during fault (MSP or PSP)

    push  {r2}
    push  {lr}
    push  {r11}
    push  {r10}
    push  {r9}
    push  {r8}
    push  {r7}
    push  {r6}
    push  {r5}
    push  {r4}
    mrs   r1, psp
    tst   lr, #4
    it    eq
    mrseq r1, msp           @ r1 <- either MSP or PSP depending on LR's bit 2
    push  {r1}
    mov   r0, sp            @ r0 points all this pushed stuff; see `PanicContext`
    @ ldr   r1, =__panic_sp   @ switch to our panic stack to ensure we have stack space
    @ mov   sp, r1
    bl    _ZN6rp23503sys12defaultPanicERKNS0_12PanicContextE
