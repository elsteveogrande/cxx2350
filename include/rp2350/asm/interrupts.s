.syntax unified
.cpu    cortex-m33
.fpu    fpv5-sp-d16

.text

.section .systext
.globl  _ZN6rp23507__panicEv
.p2align 2
.thumb_func
_ZN6rp23507__panicEv:
    @ Save registers into a `PanicContext` which we'll create on the stack
    @ and populate by pushing (in reverse field order).
    @
    @ Stack already contains the following:
    @ Pushed by processor on exception dispatch:
    @   xPSR,
    @   faulting instruction PC,
    @   LR (R14),
    @   R12,
    @   R3 through R0
    @
    @ Followed by these which we'll push below:
    @   fault type in R2 (from above),
    @   exception data (in LR),
    @   R11 through R4,
    @   SP during fault (MSP or PSP)
    @
    @ (Note: no FPU regs saved)

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
    mrseq r1, msp
    push  {r1}
    mov   r0, sp
    ldr   r1, =__panic_sp   @ switch to our panic stack to ensure space
    mov   sp, r1
    @ rp2350::panic(PanicContext const&)
    bl    _ZN6rp23505panicERKNS0_12PanicContextE
