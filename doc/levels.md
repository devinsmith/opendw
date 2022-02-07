Levels are a variable number of bytes.

Examples:

Purgatory (0x71)
================

First 4 bytes of level file:

0x22, 0x22, 0x1c, 0x64, these are loaded into gamestate [0x21 - 0x24]

Next bytes are variable, read until hitting >  0x80:

These are the resources we need for this level (& 0x7F += 0x6E):

0x00, 0x01, 0x02, 0x06, 0x09, 0x05, 0x7, 0x93
=============================================
0x6E, 0x6F, 0x70

Next bytes are set in pairs:

0x00, 0xd1
0x05, 0x81
0x00, 0xe1
0x80, 0x91

# Cached resources

Next bytes are loaded individually but are a maximum of 4 byte chunks

| Resources  | Resource Size    |
|------------|------------------|
|    0 - 3   | 0x1, 0x82        |
|    4 - 7   | 0x06, 0x82       |
|    8 - 11  | 0x04, 0x03, 0x87 |

Now offset to the title of the level:

0x16CF
