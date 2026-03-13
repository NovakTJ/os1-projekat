.global _ZN3TCB13contextSwitchEPNS_7ContextES1_
.type _ZN3TCB13contextSwitchEPNS_7ContextES1_, @function
_ZN3TCB13contextSwitchEPNS_7ContextES1_:
    # push s0-s11 onto stack (12 regs * 8 bytes = 96)
    addi sp, sp, -96
    sd s0,  0*8(sp)
    sd s1,  1*8(sp)
    sd s2,  2*8(sp)
    sd s3,  3*8(sp)
    sd s4,  4*8(sp)
    sd s5,  5*8(sp)
    sd s6,  6*8(sp)
    sd s7,  7*8(sp)
    sd s8,  8*8(sp)
    sd s9,  9*8(sp)
    sd s10, 10*8(sp)
    sd s11, 11*8(sp)

    # save ra and sp into old context
    sd ra, 0*8(a0)
    sd sp, 1*8(a0)

    # restore ra and sp from new context
    ld ra, 0*8(a1)
    ld sp, 1*8(a1)

    # pop s0-s11 from new thread's stack
    ld s0,  0*8(sp)
    ld s1,  1*8(sp)
    ld s2,  2*8(sp)
    ld s3,  3*8(sp)
    ld s4,  4*8(sp)
    ld s5,  5*8(sp)
    ld s6,  6*8(sp)
    ld s7,  7*8(sp)
    ld s8,  8*8(sp)
    ld s9,  9*8(sp)
    ld s10, 10*8(sp)
    ld s11, 11*8(sp)
    addi sp, sp, 96

    ret
