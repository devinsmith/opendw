DOS Version of Dragon Wars
==========================

This is a DOS version of Dragon Wars that we hope to be byte for byte
compatible with the Interplay version. It uses MASM 5.0 and a makefile for
generation of the executable.

To generate, use DOS (or dosbox) with an installation of MASM 5.0

```
MAKE DRAGON
DRAGON.COM
```

A `DRAGON.EXE` is also generated, but it does not work. This is because MASM
5.0 cannot generate COM files directly and `EXE2BIN` is used to convert the
EXE to a COM file.

