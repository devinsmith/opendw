; This is a disassembly of the Interplay DragonWars executable that can
; hopefully be byte for byte compatible with the original game.
;
; The game was originally written by Rebecca Ann Heineman.
;
; Disassembly by Devin Smith <devin@devinsmith.net>
;

; This is a COM file, so everything uses the same segment.
      TITLE DRAGON
_TEXT SEGMENT
      ASSUME CS:_TEXT,DS:_TEXT,ES:_TEXT,SS:_TEXT
      ORG 100H

start:
  jmp short begin

; word at 0x102
; indicates graphics mode.
; 0x0000 (CGA RGB monitor)
; 0x0002 (CGA composite monitor)
; 0x0004 (Tandy 16 color)
; 0x0006 (EGA 16 color)
; 0x0008 (VGA/MCGA 16 color)
; 0xffff (not configured)
graphics_mode dw 0008h

; if the mouse is ON, this value should be 0xffff
; if the mouse is OFF, this value should be 0x0000
mouse_configured dw 0FFFFh ; (indicates whether the mouse is on or off)

unknown db 0
; Always 0xc0 (maybe a constant?)
byte_107 db 0c0h

apple2gs db 'Created on an Apple ][ GS. Apple ][ Forever!'

begin:
  ; Setup segments
  cli
  mov ax, cs
  mov sp, 0C94h ; Stack size?
  mov ss, ax
  mov es, ax
  mov ds, ax
  sti

  call setup_interrupts
  call check_config
  call setup_memory
  call init_graphics

  ; memset/bzero (pointer at 0x3860)
  ; Fill ES:DI with AX, rep CX times.
  mov di, offset game_state
  xor ax, ax
  mov cx, 80h
  rep stosw

  ; memset (pointer at 0xBC52h)
  ; actually fill it with 0xFFFF
  ; Fill ES:DI with AX, rep CX times.
  mov di, offset data1_header
  dec ax
  mov cx, 180h
  rep stosw

  call init_mm
  call init_offsets

  mov al, 0FFh ; value of true ?
  mov byte ptr [byte_4F0F], al
  mov byte ptr [game_state + 57h], al
  mov byte ptr [game_state + 5Bh], al
  mov byte ptr [game_state + 56h], al
  mov byte ptr [game_state + 5Ah], al
  mov byte ptr [byte_4F10], al
  mov byte ptr [game_state + 8], al

  inc ax ; al becomes 0.
  mov byte ptr [byte_393C], al ; value of false (0)
  call sub_3578
  call sub_387 ; Run title sequence and block for keypress.
  call sub_37C8 ; draws viewport.

  ; Important parts of the game state?
  mov byte ptr [word_2697], 1
  mov byte ptr [word_269B], 27h
  mov byte ptr [word_2699], 8
  mov byte ptr [word_269D], 0B8h
  mov byte ptr [ui_drawn_yet], 0FFh ; indicates that UI should be drawn

  call sub_26B8 ; draws rest of UI

  ; 0x1A6 - reads tag item 1.
  xor bx, bx
  mov al, 1
  call sub_2EB0
  xor bx, bx
  call sub_3AA0 ; This is the game loop

  ; 1B2
  mov dx, offset empty_string
  push dx
  call sub_A5A
  call sub_311

  call restore_interrupts
  call close_file

  ; Restore video mode.
  mov ax, 3
  int 10h

  ; print terminating message
  pop dx
  mov ah, 9
  int 21h


  int 20h ; Terminate to DOS

empty_string: db '$'

; Initialize memory block lists.
; 0x01CF
init_mm:
  mov si, 007fh
  mov di, 00FEh
  xor ax, ax

  ; zero out memory lists
loc_1D7:
  mov word ptr [di+memory_alloc_ptrs], ax
  mov word ptr [di+bytes_allocated_list], ax
  mov byte ptr [si+memory_alloc_list], al
  mov word ptr [di+memory_tag_list], ax
  dec di
  dec di
  dec si
  jns loc_1D7

  ; 0x01EC
  mov ax, 0C960h
  shr ax, 1
  shr ax, 1
  shr ax, 1
  shr ax, 1
  mov bx, cs
  add ax, bx

  mov word ptr [memory_alloc_ptrs], ax
  mov word ptr [memory_alloc_ptrs + 2], ax

  mov ax, 0FFFFh
  mov word ptr [memory_tag_list], ax
  mov word ptr [memory_tag_list + 2], ax
  mov byte ptr [memory_alloc_list], al
  mov byte ptr [memory_alloc_list + 1], al
  mov word ptr [bytes_allocated_list], 0001
  mov word ptr [bytes_allocated_list + 2], 0E00h
  ret

; 0x21D
init_graphics:
  mov word ptr [unknown_2FA], 0
  mov bx, word ptr [graphics_mode]
  call word ptr [graphics_handlers + bx]
  jmp loc_37F0

graphics_handlers: dw init_cga_rgb ; 0x0000 (CGA RGB monitor)
                   dw init_cga_composite ; 0x0002 (CGA composite monitor)
                   dw init_tandy16 ; 0x0004 (Tandy 16 color)
                   dw init_ega16   ; 0x0006 (EGA 16 color)
                   dw init_vga16   ; 0x0008 (VGA/MCGA 16 color)

; CGA RGB
; 0x238
init_cga_rgb:
  mov ax, 4
  int 10h
  mov si, offset cga_init_chunk
  jmp loc_253
  ;db 90h  ; BUG in MASM ?

; CGA chunk (palette?)
; 0x243
cga_init_chunk: db 0, 0AAh, 55h, 55h, 0AAh, 0AAh, 55h, 55h
                db 0AAh, 0AAh, 55h, 55h, 0AAh, 0AAh, 055h, 0FFh

loc_253:
  mov dx, 0C030h
  mov di, 0FFh
loc_259:
  mov bx, di
  shr bx, 1
  shr bx, 1
  shr bx, 1
  shr bx, 1
  mov al, byte ptr [bx+si]
  and al, dh
  mov bx, di
  and bx, 0fh ; and bx,byte +0xf
  mov ah, byte ptr [bx+si]
  and ah, dl
  or al, ah
  mov byte ptr [di-47AEh], al
  dec di
  jnz loc_259
  mov dx, 0C03h
  mov di, 0FFh


init_cga_composite:

init_tandy16:

init_ega16:

init_vga16:

unknown_2FA dw 0

; 0x311
sub_311:

; 0x387
sub_387:

; should be at 0x5D0
setup_interrupts:
  push es

  ; DOS - 2+ - GET INTERRUPT VECTOR
  ; 1C - TIME - SYSTEM TIMER TICK
  ; http://www.ctyme.com/intr/rb-2443.htm
  mov ax, 351ch
  int 21h
  mov word ptr [save_tic_vec], bx
  mov word ptr [save_tic_vec+2], es

  ; DOS - 2+ - GET INTERRUPT VECTOR
  ; 24 - CRITICAL ERROR HANDLER
  ; http://www.ctyme.com/intr/int-24.htm
  mov ax, 3524h
  int 21h
  mov word ptr [save_critical_vec], bx
  mov word ptr [save_critical_vec+2], es

  ; DOS - 2+ - SET INTERRUPT VECTOR
  ; 1C - TIME - SYSTEM TIMER TICK
  ; http://www.ctyme.com/intr/rb-2443.htm
  mov ax, 251ch
  mov dx, word ptr [timer_proc_handler] ; should be 0x4b10
  int 21h

  ; DOS - 2+ - SET INTERRUPT VECTOR
  ; 24 - CRITICAL ERROR HANDLER
  ; http://www.ctyme.com/intr/int-24.htm
  mov ax, 2524h
  mov dx, word ptr [critical_error_handler] ; should be 0x616
  int 21h

  pop es
  retn

; At 0x5fd
; Restores interrupt vectors on exit.
restore_interrupts:
  push ds

  ; DOS - 2+ - SET INTERRUPT VECTOR
  ; 1C - TIME - SYSTEM TIMER TICK
  ; http://www.ctyme.com/intr/rb-2443.htm
  mov ax, 251Ch
  lds dx, dword ptr [save_tic_vec]
  int 21h

  ; DOS - 2+ - SET INTERRUPT VECTOR
  ; 24 - CRITICAL ERROR HANDLER
  ; http://www.ctyme.com/intr/int-24.htm
  mov ax, 2524h
  lds dx, dword ptr [save_critical_vec]
  int 21h

  pop ds
  retn


; Variables
save_critical_vec: dd 0


; Whenever there is an I/O error, this interrupt is invoked.
; Upon entry into this int 0x24 service routine, the stack will contain the
; following data
;
; DOS pushes the following registers for our handler.
; ES, DS, BP, DI, SI, DX, CX, BX, AX
;
; 0x616
critical_error_handler:
  add sp, 8
  mov ax, di ; What type of block device error?
  pop bx
  pop cx
  pop dx
  pop si
  pop di
  pop bp
  pop ds
  pop es
  stc ; set carry flag.
  ;; Indicates we should terminate program?
  retf 02h ; should this be iret ?


; 0x0627
check_config:
  cmp word ptr [graphics_mode], 0ffffh
  jz loc_638
  mov bx, 80h
  mov al, [bx]
  or al, al
  jnz loc_63D
  retn

loc_638:
  xor ax, ax
  mov [graphics_mode], ax

; start of configuration ?
loc_63D:

; 0x0A25
setup_memory:

; 0x0A5A
sub_A5A:

; MEMORY USAGE:
; Memory is tracked in the following ways:
;   Pointers are stored in the memory_alloc_ptrs list.
;   # of bytes are stored in bytes_allocated_list
;   Reference count? is stored in the memory_alloc_list.
;
; As an example, lets say you want to allocate 128 bytes
; Your pointer would be stored in memory_alloc_ptrs
; 128 (0x80) would be stored in bytes_allocated_list
; the value of 1 would be stored in memory_alloc_list.

; Works together with memory_alloc_list to store allocated pointers.
; at 0x13C9 (up to 0x14C8)
memory_alloc_ptrs: dw 128 dup (0)

; Stores the number of allocated bytes each pointer should contain.
; at 0x14C9 (up to 0x15C8)
bytes_allocated_list: dw 128 dup (0)

; at 0x15C9 (up to 0x1648)
; Indicates if each pointer in the above list is in use.
; XXX: This could also be a reference counter?
; Since values can take on anything from 0 - 0xFF
;   and values of 0, 1, 2, and 0xFF seem to be in use.
memory_alloc_list: db 128 dup (0)

; at 0x1649 - 0x1749
; Controls lookups by some sort of tag?
memory_tag_list: dw 80h dup (0)


; sets up table to be sums of 0x50, 0x88 times.
; 0x00, 0x00, 0x50, 0x00, 0xA0, 0x00, 0xF0, 0x00, 0x40, 0x01 ...
; 0x17DD
init_offsets:
  mov dx, 50h

; 0x268E
ui_drawn_yet: db 0
word_2697: dw 0
word_2699: dw 0
word_269B: dw 0
word_269D: dw 0

; 0x26B8
sub_26B8:

; 0x2EB0
sub_2EB0:

; 0x3116
close_file:

; 0x3578
sub_3578:

; 0x37C8
sub_37C8:

loc_37F0:

; 0x3860
game_state dw 80h DUP (0)

byte_393C: db 0

; 0x3AA0
sub_3AA0:

; Invoked every clock tick on x86
; 0x4b10
timer_proc_handler:
  push ds
  push ax
  push cs
  pop ds
  pushf
  ; call existing procedure (if there is one)
  ;call far word ptr [save_tic_vec]
  call dword ptr [save_tic_vec]

  xor ax, ax
  cmp al, byte ptr [timer_unknown2] ; seconds?
  jz .check1
  dec byte ptr [timer_unknown2]
.check1:
  cmp al, byte ptr [timer_unknown0] ; minutes?
  jz .check2
  dec byte ptr [timer_unknown0]
.check2:
  cmp al, byte ptr [timer_unknown1] ; hours ?
  jz .check3
  dec byte ptr [timer_unknown1]
.check3:
  cmp al, byte ptr [timer_unknown5] ; days ?
  jnz .check4
  cmp al, byte ptr [game_state + 90h] ; 0x3895
  jnz .check4
  cmp ax, word ptr [timer_unknown3]
  jz .check4
  dec word ptr [timer_unknown3]
.check4:
  cmp ax, word ptr [timer_unknown4]
  jz .local5
  dec word ptr [timer_unknown4]
  pop ax
.local5:
  pop ds
  iret


; 0x4B5C
save_tic_vec: dd 0

; 0x4C35
timer_unknown0: db 1
; 0x4C36
timer_unknown1: db 1
; 0X4C37
timer_unknown2: db 1
; 0x4C38
timer_unknown3: dw 1
; 0x4C3A
timer_unknown4: dw 1

; 0x4C3C
timer_unknown5: db 0

; 0x4F0F
byte_4F0F: db 0
byte_4F10: db 0

; 0xBC52
; used by 2F2A
; Contains the first 768 bytes of DATA1
data1_header: dw 180h DUP(0)

_TEXT ENDS
END start
