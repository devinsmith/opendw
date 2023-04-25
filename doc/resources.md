# Game resources

Resource data in data1. It is unknown how data2 is used at this point.

| Num  | Size | Compressed | Description                         |
|------|------|------------|-------------------------------------|
|    0 | 1148 | N          | Initial game script                 |
|    7 | 5632 | N          | Character data.                     |
|   29 |      | Y          | Title screen (320x200)              |
|   31 | 2177 | Y          | String data for monsters?           |
|   71 | 5846 | Y          | Purgatory level                     |
|  110 |11860 | Y          | Castle wall (viewport)              |
|  111 | 7050 | Y          | Sky portion (viewport)              |
|  112 | 5054 | Y          | Red clay road portion (viewport)    |
|  116 |  936 | Y          | Water puddle (viewport)             |
|  261 |12452 | Y          | Scream (PCM audio)                  |

There are also resources in the game executable:

| Num  | Size | Description                         |
|------|------|-------------------------------------|
|    0 | 2560 | Bottom bricks (msg window)          |
|    1 |  128 | Lower left bricks (msg)             |
|    2 |  128 | Lower right bricks (msg)            |
|    3 | 1280 | Top bricks (msg)                    |
|    4 |  576 | Right character border              |
|    5 | 1536 | Character banner (Dragon Wars)      |
|    6 | 1152 | Left green pillar                   |
|    7 |   64 | Left brick pillar connector         |
|    8 |   64 | Right brick pillar connector        |
|    9 | 2880 | Right green pillar                  |
|   10 |  364 | North arrow                         |
|   11 |  364 | East arrow                          |
|   12 |  364 | South arrow                         |
|   13 |  364 | West arrow                          |


# Resource 7 (Character data)

The first 3584 (7 * 512) are individual player records.
The next 256 bytes are gamestate data
The final 0x700 bytes are loaded into D760.

