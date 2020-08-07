# Game script and virtual CPU

Much of the Dragon Wars game is implemented by various game scripts that execute on a virtual CPU. This
virtual CPU, can run in both 8 bit or 16 bit mode. The virtual CPU has at most 256 op codes. This virtual CPU
is implemented inside engine.c

Not much is known about the CPU but it may be somewhat similar to the 65C816 processor considering the game
was originally authored on the Apple II GS.

The initial game script is stored in the DATA1 file, in the very first section (section 0).

The following op codes are documented. Arguments are designated with number and type (B = byte, W = word)

| OP | Arguments | Description                                  |
|----|-----------|----------------------------------------------|
| 00 | None      | Switches to 16 bit mode                      |
| 01 | None      | Switches to 8 bit mode                       |
| 02 | --        | Unknown                                      |
| 03 | None      | Pops stack, resets resource                  |
| .. | ..        | ..                                           |
| 10 | 2 B       | Load gamestate (arg1), offset (arg2) word value |
| .. | ..        | ..                                           |
| 46 | None      | Jump if signed                               |
| .. | ..        | ..                                           |
| 66 | 1 B       | Load gamestate, check and set zero/sign flag |
