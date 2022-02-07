Viewport drawing
================

1. The sky tile is drawn first

2. Then components of the ground (this is drawn in 9 sprites)

3. Then other components (there are 24 possible sprites that can be overlaid)


The components of the ground and other components are defined by data\_5A56
which is re-loaded based on x,y position and direction.

When drawing ground components we ignore the lower 4 bits of 5A56,

When drawing other components we ignore the lower 4 bits only if the
value is greater than 0x80.

This allows bytes in 5A56 to serve as a dual purpose.

