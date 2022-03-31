# Interrupt handling in Dragon Wars

There are typically no "threads" in 16 bit DOS programs, but DOS does
provide interrupts that can be hooked (like a signal) that are invoked on
certain conditions. These are also called interrupt service routines (ISRs).
These are the interrupts handled by Dragon Wars.

## DOS Interrupts
### Critical Error handler: 0x0616 (INT 0x24)

Appears to abort the program (dragon.com) on error, but it's unused
by dosbox emulation (as far as I can tell).

## Hardware Interrupts
### System timer handler (title screen) 0x5C7B (INT 0x08)

Invoked on hardware PIT interrupt (0x08). Responsible for playing
opening music during the title screen.

## Software Interrupts
### Tick handler: 0x4B10 (INT 0x1C)

First invokes existing tick handler then stores values into
timer variables.
