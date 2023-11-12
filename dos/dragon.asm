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
  and bx, word ptr 0fh ; and bx,0xf
  mov ah, byte ptr [bx+si]
  and ah, dl
  or al, ah
  mov byte ptr [di-47AEh], al
  dec di
  jns loc_259
  mov dx, 0C03h
  mov di, 0FFh

loc_280:
  mov bx, di
  shr bx, 1
  shr bx, 1
  shr bx, 1
  shr bx, 1
  mov al, [bx+si]
  and al, dh
  mov bx, di
  and bx, word ptr 0fh ; and bx,0xf
  mov ah, [bx+si]
  and ah, dl
  or al, ah
  mov [di-46AEh], al
  dec di
  jns loc_280
  retn

init_cga_composite:
  mov ax, 4
  int 10h

  mov si, offset cga_init_comp_chunk
  jmp loc_253

cga_init_comp_chunk: db 0, 0AAh, 22h, 99h, 88h, 66h, 0DDh, 0EEh, 0BBh
                     db 0AAh, 11h, 55h, 0CCh, 0AAh, 0DDh, 0FFh


; Tandy initialization
; 0x2BC
init_tandy16:
  mov ax, 9
  int 10h
  push ds
  mov ax, 40h
  mov ds, ax
  mov di, 13h
  mov cx, [di]
  shl cx, 1
  shl cx, 1
  shl cx, 1
  shl cx, 1
  shl cx, 1
  shl cx, 1
  sub cx, 400h
  call sub_2FC
  mov bx, [di]
  add bx, ax
  inc bx
  cmp bx, cx
  jna loc_2F8
  cmp ax, cx
  jnc loc_2F8
  sub bx, cx
  mov cs:[unknown_2FA], bx
  sub cx, ax
  dec cx
  mov [di], cx
loc_2F8:
  pop ds
  ret

unknown_2FA dw 0


; Tandy cleanup routine?
sub_2FC:
  xor si, si
  mov di, 3
  mov ax, cs
  dec ax
loc_304:
  mov ds, ax
  cmp byte ptr [si], 5ah
  je loc_310
  add ax, [di]
  inc ax
  jmp loc_304
loc_310:
  ret

; TODO
; 0x311
sub_311:
  ; check for tandy graphics.
  cmp byte ptr [graphics_mode], 4
  jne loc_323

  ; Tandy cleanup?
  push ds
  call sub_2FC
  mov ax, cs:[unknown_2FA]
  add [di], ax
  pop ds
loc_323:
  ret

init_ega16:
  mov ax, 0dh
  int 10h
  push ds
  mov ds, word ptr [ptr1]
  mov di, 1FEh
loc_331:
  xor bx, bx
  mov dx, bx
  mov ax, di
  shr ax, 1
  call sub_36D
  mov [di+3680h], bx
  mov [di+3880h], dx
  call sub_36B
  mov [di+3280h], bx
  mov [di+3480h], dx
  call sub_36B
  mov [di+2E80h], bx
  mov [di+3080h], dx
  call sub_36B
  mov [di+2a80h], bx
  mov [di+2c80h], dx
  dec di
  dec di
  jns loc_331
  pop ds
  ret

sub_36B:
  xor ax, ax

sub_36D:
  call sub_370

sub_370:
  shl al, 1
  rcl bl, 1
  shl al, 1
  rcl bh, 1
  shl al, 1
  rcl dl, 1
  shl al, 1
  rcl dh, 1
  ret


init_vga16:
  ; VIDEO - SET VIDEO MODE
  ; AL = mode (mode 13)
  mov ax, 13h
  int 10h
  ret


; title sequence
; No inputs, no outputs.
; 0x387
sub_387:
  mov bx, 1dh

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

; Inputs: AL
; Outputs: CX
;     BX set to 0.
; 0x12A8
get_indexed_memory:
  xor ah, ah
  shl ax, 1
  mov si, ax
  mov cx, word ptr [si+memory_alloc_ptrs]
  xor bx, bx
  ret


; Inputs are unknown.
; Outputs are unknown.
; Operates on tag_lookup_item;
; Searches memory_tag_list (0x80 words length) for contents of
; [tag_lookup_item]
; some kind of search.
;   Sets carry flag if we couldn't find this tag.
;   Clears carry flag if we found the tag..
; At 0x12C0
find_index_by_tag:
  mov di, offset memory_tag_list
  mov cx, 80h
  nop
  mov ax, word ptr [tag_lookup_item] ; global variable shared across several subroutines.

  ; Essentially searching ES:DI for data in [tag_lookup_item]
.loc_12CA:
  jcxz .loc_12D0 ; exhausted CX, set carry, we didn't find it.
  repne scasw  ; Compare AX with word at ES:DI, set flags, DI += 2;
  je .loc_12D2 ; Found!

.loc_12D0:
  stc ; set carry, we did not find [tag_lookup_item] in ES:DI.
  ret

.loc_12D2:
  ; Found the tag, make sure it's still allocated.
  mov si, di
  sub si, offset memory_tag_list+2
  shr si, 1
  mov bl, byte ptr [si+memory_alloc_list]
  or bl, bl
  jz .loc_12CA ; if bl == 0. (NOT ALLOCATED), search again.

  mov ax, si
  clc
  ret


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

; at 0x2F49
read_file:
  mov al, 3Fh ; 0x3F = Read file.
  jmp loc_2F4F

unknown_2F4D: db 0B0h, 40h

loc_2F4F:
  mov byte ptr [file_read_mode], al
  mov word ptr [save_buffer], bx ; save location of where to read file to.
  mov word ptr [save_buffer+2], cx
loc_2F5A:
  mov bx, 300h
  xor dx, dx
  mov si, offset data1_header
  mov cx, word ptr [tag_lookup_item] ; 1D?
  jcxz .loc_2F75

.loc_2F68:
  lodsw ; Load word at address DS:(E)SI into AX. SI += 2
  cmp ah, 0FFh
  je .loc_2F73

  add bx, ax
  adc dx, 0000h

.loc_2F73:
  loop .loc_2F68

.loc_2F75:
  mov word ptr [file_offset], bx
  mov word ptr [file_offset+2], dx
  lodsw ; Load word at address DS:(E)SI into AX. SI += 2

  mov word ptr [bytes_to_read], ax
  or ax, ax
  stc
  je loc_2FB6

  cmp ah, 0FFh
  jne .loc_2F92

  not ax
  call sub_2FBC
  jmp loc_2F5A

.loc_2F92:
  mov al, byte ptr [file_read_mode]
  mov bx, word ptr [save_buffer]
  mov cx, word ptr [save_buffer+2]
  mov word ptr [dword_3134], bx
  mov word ptr [dword_3134+2], cx
  call read_write_DATA1_file

  jc loc_2FB7
  cmp byte ptr [file_read_mode], 40h ; is this a write?
  jne loc_2FB6

  call close_file
  jc loc_2FB7

loc_2FB6:
  ret

loc_2FB7:
  call loc_2FF0
  jmp loc_2F5A

; Input AL (likely, al to be 0)
sub_2FBC:
  mov byte ptr [byte_393F], al
  mov byte ptr [byte_3045], al
  add al, 31h
  mov byte ptr [byte_3144], al
.loc_2FC7:
  xor ax, ax
  mov word ptr [file_offset], ax
  mov word ptr [file_offset+2], ax

  mov word ptr [bytes_to_read], 300h ; bytes to read?
  mov word ptr [dword_3134], offset data1_header ; position to read into buffer
  mov word ptr [dword_3134+2], cs ; segment
  call open_data1 ; Opens DATA1
  ; Did file open fail?
  jc .loc_2FEB
  mov al, 3Fh ; read DATA1
  call read_write_DATA1_file
  jnc loc_2FB6 ; JNC (no error) we are done.

  ; Potential error handler
.loc_2FEB:
  call loc_2FF0
  jmp .loc_2FC7

loc_2FF0:
  mov bl, byte ptr [ui_drawn_yet]

byte_3045: db 0

open_data1:
  call close_file

  ; DOS - GET DEFAULT DISK NUMBER
  mov ah, 19h
  int 21h
  mov byte ptr [default_drive], al

.loc_3054:
  mov dl, al
  mov byte ptr [byte_30B3], al

  ; DOS - SELECT DISK
  ; DL = new default drive number
  ; Return: AL = number of logical drives
  mov ah, 0Eh
  int 21h
  mov byte ptr [num_logical_drives], al ; DosBox returns 26 (0x1A) (A-Z)

  ; DOS - 2+ - OPEN DISK FILE WITH HANDLE
  ; DS:DX -> ASCIZ filename
  ; AL = access mode (2 = read & write)
  mov dx, offset DATA1_STRZ
  mov ax, 3D02h
  int 21h

  ; Return:
  ; CF clear if successful, set on error
  ; AX = file handle
  ; AX = error code (01h,02h,03h,04h,05h,0Ch,56h) (see #01680 at AH=59h)

  jc .loc_3073
  mov word ptr [file_handle], ax
  mov byte ptr [is_file_open], 0FFh
  ret

  ; Error code handling if DATA1 couldn't be opened.
.loc_3073:
  mov al, byte ptr [byte_30B3]
  inc al
  cmp al, byte ptr [num_logical_drives]



; 0x30B2
default_drive: db 0
; 0x30B3
byte_30B3: db 0
; 0x30B4
num_logical_drives: db 0

; Handles reading or writing files files.
; Inputs:
;    AL (0x3F read file / 0x40 write file)
;    dword_3134 - File buffer. In read mode, this buffer is populated.
;                 In write mode this buffer is written to the file.
;    file_offset   - Seek/offset position from start of file. 32 bit.
;    bytes_to_read -  Number of bytes to read.
;
; Originally at 0x30B5
read_write_DATA1_file:
  mov byte ptr [file_rw_op], al
  cmp byte ptr [is_file_open], 0
  jnz .loc_30D1


  ; DOS - 2+ - OPEN DISK FILE WITH HANDLE
  ; DS:DX -> ASCIZ filename
  ; AL = access mode (2 = read & write)
  mov dx, offset DATA1_STRZ
  mov ax, 3D02h
  int 21h

  ; Return:
  ; CF clear if successful, set on error
  ; AX = file handle
  ; AX = error code (01h,02h,03h,04h,05h,0Ch,56h) (see #01680 at AH=59h)

  jc .loc_310B

  mov word ptr [file_handle], ax
  mov byte ptr [is_file_open], 0FFh
.loc_30D1:

  mov ax, 4200h
  ; DOS - 2+ - MOVE FILE READ/WRITE POINTER (LSEEK)
  ; al = method: offset from beginenning of file.
  ; AH = 42h
  ; AL = origin of move
  ;   00h start of file
  ;   01h current file position
  ;   02h end of file
  ; BX = file handle
  ; CX:DX = (signed) offset from origin of new file position

  ; Return:
  ; CF clear if successful
  ; DX:AX = new file position in bytes from start of file
  ; CF set on error
  ; AX = error code (01h,06h) (see #01680 at AH=59h/BX=0000h)
  mov bx, word ptr [file_handle]
  mov dx, word ptr [file_offset]
  mov cx, word ptr [file_offset+2]
  int 21h

  jc .loc_3102 ; error

  ; Read or write file contents into/from buffer at DS:DX
  ; AH = 3F ?
  ; CX = Number of bytes to read.
  ; DS:DX Buffer to read into.
  mov ah, byte ptr [file_rw_op]
  mov bx, word ptr [file_handle]
  mov cx, word ptr [bytes_to_read]
  push ds
  lds dx, dword ptr [dword_3134] ; 0xBC52
  int 21h

  pop ds
  jc .loc_3102 ; Any error reading or writing?

  cmp byte ptr [file_rw_op], 40h
  clc
  jne .loc_310B ; was this a write?

.loc_3102:
  pushf
  push ax
  call close_file
  jb .loc_310C
  pop ax
  popf

.loc_310B:
  ret

.loc_310C:
  pop dx
  popf
  jnb .loc_3112
  mov ax, dx

.loc_3112:
  stc
  ret


; 0x3114
file_rw_op: db 0
; 0x3115
is_file_open: db 0
; 0x3130
save_buffer: dd 0
dword_3134: dd 0
; 0x3138
file_offset: dd 0
; 0x313C
file_handle: dw 0
; 0x3140
DATA1_STRZ: db 'DATA1', 0
byte_3144: db 0

; 0x3116
close_file:

; 0x312B
tag_lookup_item: dw 0
; 0x312D
bytes_to_read: dw 0
; 0x312F
file_read_mode: db 0

; 0x3578
sub_3578:

; 0x37C8
sub_37C8:

loc_37F0:

; 0x3860
game_state dw 80h DUP (0)

byte_393C: db 0
byte_393F: db 0

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
; 0x4F11
ptr1: dw 0

; 0xBC52
; used by 2F2A
; Contains the first 768 bytes of DATA1
data1_header: dw 180h DUP(0)

_TEXT ENDS
END start
