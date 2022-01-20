# Game resources

Resource data in data1. It is unknown how data2 is used at this point.

| Num  | Size | Compressed | Description                         |
|------|------|------------|-------------------------------------|
|    0 | 1148 | N          | Initial game script                 |
|    7 | 5632 | N          | Character data.                     |
|   29 |      | Y          | Title screen (320x200)              |
|   71 | 5846 | Y          | Purgatory level                     |
|  110 |11860 | Y          | Castle wall (viewport)              |
|  111 | 7050 | Y          | Sky portion (viewport)              |
|  112 | 5054 | Y          | Red clay road portion (viewport)    |
|  116 |  936 | Y          | Water puddle (viewport)             |

# Resource 7 (Character data)

The first 3584 (7 * 512) are individual player records.
The next 256 bytes are gamestate data
The final 0x700 bytes are loaded into D760.

