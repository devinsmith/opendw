# Game script and virtual CPU

Much of the Dragon Wars game is implemented by various game scripts that execute on a virtual CPU. This
virtual CPU, can run in both 8 bit or 16 bit mode. The virtual CPU has at most 256 op codes. This virtual CPU
is implemented inside engine.c

Not much is known about the CPU but it may be somewhat similar to the 65C816 processor considering the game
was originally authored on the Apple II GS.

The initial game script is stored in the DATA1 file, in the very first section (section 0).

Known scripts:
DATA1: Section 0 (initial game script) 1148 bytes.
       Section 0 off: 1103, 45 bytes
       Section 0 off: 237 ?
       Section 0 off: 246, ? (Display's player name)

       Section 0 off: 1137, 11 bytes

       Section 71, offset: 3735
       Section 71, offset: 5764

       Section 22, offset 68

       Section 71, 5764
       Section 71, 3689

       Section 3, 1706 (encounter)
       Section 71, 0xe97 (Purgatory game start)

       Section ?, 395



The following op codes are documented. Arguments are designated with number and type (B = byte, W = word)

| OP | Arguments | Description                                  |
|----|-----------|----------------------------------------------|
| 00 | None      | Switches to 16 bit mode                      |
| 01 | None      | Switches to 8 bit mode                       |
| 02 | --        | Unknown                                      |
| 03 | None      | Pops stack, resets resource                  |
| 04 | None      | Pushes resource index on stack               |
| .. | ..        | ..                                           |
| 09 | 1 (B|W)   | Loads word\_3AE2 with argument               |
| 10 | 2 B       | Load gamestate (arg1), offset (arg2) word value |
| .. | ..        | ..                                           |
| 46 | None      | Jump if signed                               |
| .. | ..        | ..                                           |
| 58 | 1 B       | Load gamestate, check and set zero/sign flag |
| 66 | 1 B       | Load gamestate, check and set zero/sign flag |
