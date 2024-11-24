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

VGA_BASE_ADDRESS EQU 0A000h
CGA_BASE_ADDRESS EQU 0B800h

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
loc_1B5:
  push dx
  call sub_A5A
  call sub_311

loc_1BC:
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
  mov bx, 1dh ; TITLE3
  call sub_5A5 ; Clears screen?
  push ax ; picture memory reference???
  call sub_49E ; Picture should be fully drawn here.
  call sub_5C3B ; PIT timers
  mov ax, 0001h
  call sub_5ECA
  pop ax
  call sub_1270 ; free's title seqence.
  call waitkey ; reads key?
  call waitkey_or_mouse ; plays music (actually title loop)

  jmp loc_5C60

; 0x3A7
; not sure who calls this.
; Is this the ending game screen?
sub_3A7:
  mov bx, 18h ; TITLE0
  call sub_5A5 ; Clear screen?
  push ax
  call sub_49E
  call sub_5C3B
  mov ax, 1
  call sub_5ECA
  pop ax
  call sub_1270

sub_3BE:
  mov bx, 19h
  call sub_5A5
  push ax
  mov ax, word ptr [dword_4BB]
  mov word ptr [word_4bf], ax
  mov ax, word ptr [dword_4BB + 2]
  mov word ptr [word_4C1], ax
  mov word ptr [tag_lookup_item], 0ffffh
  mov bx, 7d00h
  mov al, 1
  call sub_12E6
  push ax
  mov word ptr [dword_4BB], bx
  mov word ptr [dword_4BB+2], cx
  call waitkey_or_mouse
  xor bx, bx
.loc_3ED:
  push bx
  call sub_435
  pop bx
  push bx
  call sub_443
  call sub_49E
  mov word ptr [word_4C4A], 12h
  xor ax, ax
.loc_401:
  cmp word ptr [timer_unknown4], ax
  jnz .loc_401
  pop bx
  inc bx
  cmp bx, byte ptr +4
  jc .loc_3ED
  pop ax
  call sub_1270
  pop ax
  call sub_1270
  mov bx, 1Ah
.loc_419:
  push bx
  call sub_5A5
  push ax
  call waitkey_or_mouse
  call sub_49E
  pop ax
  call sub_1270
  pop bx
  inc bx
  cmp bx, byte ptr 1dh
  jc .loc_419
  call waitkey_or_mouse
  jmp loc_5C60

sub_435:
  push es
  les di, dword ptr [dword_4BB]
  xor ax, ax
  mov cx, 3e80h
  rep stosw
  pop es
  ret

sub_443:
  push es
  push ds
  shl bx, 1
  mov ax, bx
  shl bx, 1
  add bx, ax
  mov bp, [bx+477h]
  mov dx, [bx+475h]
  mov si, [bx+473h]
  mov es, word ptr [dword_4BB + 2]
  mov ds, word ptr [word_4C1]
.loc_461:
  mov di, si
  push si
  mov cx, dx
  rep movsb
  pop si
  add si, 0a0h
  dec bp
  jnz .loc_461
  pop ds
  pop es
  ret

unknown_473: db 73h, 0Eh, 34h, 0, 67h, 0, 4Eh, 32h, 22h
             db 0, 36h, 0, 76h, 4Bh, 0Ch, 0, 13h, 0
             db 60h, 63h, 0A0h, 0, 22h, 0

; 0x48B
waitkey_or_mouse:
  call sub_2D0B
  js .loc_497
  call poll_mouse
  cmp al, 80h ; did mouse button get clicked.
  jnz waitkey_or_mouse

.loc_497:
  retn

; 0x498
waitkey:
  call sub_2D0B
  js waitkey
  ret

sub_49E:
  push es
  push ds
  mov bx, word ptr [graphics_mode]
  lds si, dword ptr [dword_4BB]
  call word ptr cs:[bx+drawing_handlers]
  pop ds
  pop es
  jmp waitkey

; 0x4B1
drawing_handlers: dw draw_cga_picture   ; 0x0000 (CGA RGB monitor)
                  dw draw_cga_picture   ; 0x0002 (CGA composite monitor)
                  dw draw_tandy_picture ; 0x0004 (Tandy 16 color)
                  dw draw_ega_picture   ; 0x0006 (EGA 16 color)
                  dw draw_vga_picture   ; 0x0008 (VGA/MCGA 16 color)


dword_4BB: dd 0
word_4BF: dw 0
word_4C1: dw 0

; draws a picture stored at ds:si using CGA RGB.
; 0x4C3
draw_cga_picture:
  mov ax, 0b800h
  mov es, ax
  xor di, di
  mov bx, di
  mov cx, 0c8h
.loc_4CF:
  push cx
  push di
  mov di, cs:[di-55FEh]
  mov cx, 50h
.loc_4D9:
  lodsw
  mov bl, al
  mov al, cs:[bx-47AEh]
  mov bl, ah
  or al, cs:[bx-46AEh]
  stosb
  loop .loc_4D9
  pop di
  pop cx
  inc di
  inc di
  loop .loc_4CF
  ret

; 0x4F2
draw_tandy_picture:
  mov ax, 0b800h
  mov es, ax
  xor bx, bx
  mov dx, 0c8h
.loc_4FC:
  mov di, cs:[bx-52deh]
  mov cx, 50h
  rep movsw
  inc bx
  inc bx
  dec dx
  jnz .loc_4FC
  ret

; 0x50C
draw_ega_picture:
  xor di, di
  mov dx, 3c4h
  mov al, 2
  out dx, al
  mov cx, 0c8h
.loc_517:
  push cx
  mov bp, 28h
  xor ah, ah
.loc_51D:
  mov es, word ptr cs:[ptr1]
  lodsb
  mov bx, ax
  shl bx, 1
  mov cx, word ptr es:[bx+2A80h]
  mov dx, word ptr es:[bx+2C80h]
  lodsb
  mov bx, ax
  shl bx, 1
  or cx, word ptr es:[bx+2E80h]
  or dx, word ptr es:[bx+3080h]
  lodsb
  mov bx, ax
  shl bx, 1
  or cx, word ptr es:[bx+3280h]
  or dx, word ptr es:[bx+3480h]
  lodsb
  mov bx, ax
  shl bx, 1
  or cx, word ptr es:[bx+3680h]
  or dx, word ptr es:[bx+3880h]
  mov bx, dx
  mov dx, 0a000h
  mov es, dx
  mov dx, 3c5h
  mov al, 8
  out dx, al
  mov byte ptr es:[di], cl
  shr al, 1
  out dx, al
  mov byte ptr es:[di], ch
  shr al, 1
  out dx, al
  mov byte ptr es:[di], bl
  shr al, 1
  out dx, al
  mov byte ptr es:[di], bh
  inc di
  dec bp
  jnz .loc_51D
  pop cx
  loop .loc_517
  mov al, 0fh
  out dx, al
  ret

; draws a picture stored at ds:si in VGA/MCGA 16 color.
; 0x58B
draw_vga_picture:
  mov ax, VGA_BASE_ADDRESS
  mov es, ax
  xor di, di
  mov cx, 7D00h; 32k (word at a time)

  ; Lookup pixel key data from 0x45AE to get actual pictures.
  ; BB56 ?
  ; Pixel table is ??
.loc_595:
  lodsb  ; si++
  ; al populated from ds:si
  xor bx, bx
  mov bl, al
  shl bx, 1   ; bx = bx << 1
  mov ax, word ptr cs:[bx-45AEh]
  stosw ; Store ax at es:di,  di += 2
  loop .loc_595
  retn


; Inputs BX, others?
; Clears screen?
sub_5A5:
  mov al, 1

  ; determines bx and cx (not sure yet what ax does)
  call sub_2EB0

  push ax ; saves ax for later after xor operation.
  mov word ptr [dword_4BB], bx ; bx likely 0, cx likely 0x1887
  mov word ptr [dword_4BB+2], cx ; setting picture data segment.

  ; Indices into bx.
  mov si, bx
  mov di, bx

  add di, 0A0h
  mov es, cx
  mov ds, cx
  mov cx, 3E30h

  ; Half of the title segment is XOR'd into itself. ?
.loc_5C2:
  lodsw   ; set AX = DS:SI, SI += 2
  xor ax, [si+009Eh] ; ax ^ 
  stosw   ; puts AX into ES:DI,  DI += 2;
  loop .loc_5C2

  push cs
  push cs
  pop ds
  pop es
  pop ax
  ret

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
  mov dx, offset timer_proc_handler ; should be 0x4b10
  int 21h

  ; DOS - 2+ - SET INTERRUPT VECTOR
  ; 24 - CRITICAL ERROR HANDLER
  ; http://www.ctyme.com/intr/int-24.htm
  mov ax, 2524h
  mov dx, offset critical_error_handler ; should be 0x616
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

  ; In DOS the command line arguments are stored in the PSP.
  ; at 0x80 is the argument to the program.
  mov bx, 80h ; PSP for command line arguments
  mov al, [bx]
  or al, al
  jnz loc_63D
  retn

loc_638:
  xor ax, ax
  mov [graphics_mode], ax

; start of configuration ?
loc_63D:
  mov ax, 3
  int 10h ; Set Video mode (al = mode)
  ; https://en.wikipedia.org/wiki/Box-drawing_character
  mov al, 0c9h ; character for little left corner angle.
  call sub_9F0
  ; Draw 0xCD 0x16 times...
  mov cx, 0016h
  call sub_A0E
  call sub_A16

menu_header_str db 10h, " Dragon Wars Configure Menu V1.1", 11h, 0

  ; This code starts at 0x673

  ; Draw 0xCD 0x15 times...
  mov cx, 0015h
  call sub_A0E
  mov al, 0BBh ; character for little right corner angle.
  call sub_9F0

  mov dh, 1
  mov al, 0BAh

.local1:
  xor dl, dl
  call set_cursor_to_dh_dl
  call sub_9F0
  mov dl, 4Eh
  call set_cursor_to_dh_dl
  call sub_9F0
  inc dh
  cmp dh, 18h
  jb .local1

  xor dl, dl
  call set_cursor_to_dh_dl

; 0x69E
  mov al, 0C8h
  call sub_9F0

  ; Draw 0xCD 0x16 times...
  mov cx, 0016h
  call sub_A0E
  call sub_A16

; raw data text
menu_footer_str db 10h, " Copyright 1989, 1990 Interplay ", 11h, 0

; 0x6CF
  mov cx, 0015h
  call sub_A0E
  mov al, 0BCh
  call sub_9F0
  xor di, di
.loc_6DC:
  mov dx, [di+cursor_pos_array]
  call set_cursor_to_dh_dl

  ;DOS - PRINT String terminated by "$"
  ; in address DS:DX
  mov dx, word ptr [di+menu_text_ptrs]
  mov ah, 9
  int 21h;

  inc di;
  inc di;
  cmp di, 0016h ; should be # items in cursor_pos_array * 2
  jb .loc_6DC

.loc_6F2:
  call sub_8A6

; Read key loop, sub_2D0B will return key code in AL.
.loc_6F5:
  call sub_2D0B

  ; Keys: 1 (0xB1), 2 (0xB2)
  ; 'b' 0xE2

  jns .loc_6F5
  ; down arrow is 0x8A
  cmp al, 8Dh ; Enter key
  jne .loc_6FF
  retn

.loc_6FF:
  cmp al, 9Bh
  jne .loc_70A

  ; Escape (0x9B) was pressed.
  ; ax is address of empty string.
  mov ax, offset empty_string
  push ax
  jmp loc_1BC

  ; Checking for key presses '1' and '2'
.loc_70A:
  xor bx, bx
  cmp al, 0B2h
  je .loc_716
  not bx
  cmp al, 0B1h
  jne .loc_71C

.loc_716:
  mov [mouse_configured], bx ; set [test2] to 0xFFFF (mouse on)
                             ; or 0x0000 (mouse off)
  jmp .loc_6F2

  ; Checking letters (a-e)
.loc_71C:
  and ax, 0DFh
  cmp al, 0C1h
  jb .loc_730
  cmp al, 0C6h
  jnb .loc_730
  sub al, 0C1h
  shl al, 1

.loc_72B:
  mov [graphics_mode], ax
  jmp .loc_6F2

.loc_730:
  cmp al, 0d3h  ; 'Check for 's' key'
  jne .loc_6F5
  call sub_8F3

.loc_737:
  call waitkey_or_mouse
  jmp loc_63D

; Cursor positions (starting at 0x73D, every 2 bytes)
cursor_pos_array dw 061Ch, 071Ch, 081Ch, 091Ch, 0A1Ch, 1020h, 1120h
                 dw 0312h, 1408h, 1517h, 0D0Fh

; menu text pointers (starting at 0x753, every 2 bytes)
menu_text_ptrs dw cga_rgb_option_text
               dw cga_comp_option_text
               dw tandy_option_text
               dw ega_option_text
               dw vga_option_text
               dw mouse_on_text
               dw mouse_off_text
               dw select_screen_text
               dw start_or_save_text
               dw escape_game_text
               dw mouse_instr_text

; menu options.
; 0x753 (why is there a bunch of garbage before this string?)
cga_rgb_option_text db  "A. CGA RGB monitor$"
cga_comp_option_text db "B. CGA composite monitor$"
tandy_option_text   db  "C. Tandy 16 color$"
ega_option_text     db  "D. EGA 16 color$"
vga_option_text     db  "E. VGA/MCGA 16 color$"
mouse_on_text       db  "1. Mouse On$"
mouse_off_text      db  "2. Mouse Off$"
select_screen_text  db  "Select a screen format by typing its letter.$"
start_or_save_text  db  "Press ", 11h, 0C4h, 0D9h, ' to begin the game or press "S" to save configuration$'
escape_game_text    db  "or press ESC to return to MS-DOS.$"
mouse_instr_text    db  "Press 1 or 2 for enabling/disabling mouse support.$"

; 0x8A6
; possibly responsible to for the selection item
; What does this do?
sub_8A6:
  xor di, di

.loc_8A8:
  mov dx, [di+cursor_pos_array]
  sub dl, 2   ; move 2 cels in front of text.
  call set_cursor_to_dh_dl

  mov si, di
  mov bx, 0102h
  cmp di, 000Ah
  jb .loc_8CC
  mov ax, [mouse_configured]
  cmp si, 000Ah
  jnz .loc_8C6
  not ax

.loc_8C6:
  or ax, ax
  jz .loc_8D2
  jnz .loc_8DE

.loc_8CC:
  cmp si, [graphics_mode]
  jnz .loc_8DE

.loc_8D2:
  ; print the cursor
  mov al, 11h
  call sub_9F0
  mov al, 10h
  call sub_9F0
  jmp short .loc_8E6

.loc_8DE:
  ; erase cursor
  mov al, 20h
  call sub_9F0
  call sub_9F0

.loc_8E6:
  inc di
  inc di
  cmp di, 0eh
  jb .loc_8A8
  mov dx, 1538h
  jmp set_cursor_to_dh_dl

sub_8F3:
  ; Draws some type of box
  call sub_958
  call sub_A16

; raw data text
saving_state_str db "Saving game state.", 0

; 0x90C
  call sub_9AB

  pushf
  push ax
  call sub_958
  pop ax
  popf

  ; was there a write error?
  jnc loc_942

  or ax, ax
  jz loc_92D
  call sub_A16
drive_error_str db "Drive error.", 0
  ret

loc_92D:
  call sub_A16
write_protected_str db "Write protected.", 0
  ret

loc_942:
  call sub_A16
game_state_saved_str db "Game state saved.", 0
; 0x957
  ret

; Draws a box
; at 0x958
sub_958:
  mov dx, 0E1Bh
  call set_cursor_to_dh_dl
  mov al, 0C9h
  call sub_9F0
  mov cx, 18h

.loc_966:
  call sub_A0E
  mov al, 0BBh
  call sub_9F0
.lov_96E:
  mov dh, 00Fh
  mov al, 0BAh
.loc_972:
  mov dl, 1Bh
  call set_cursor_to_dh_dl
  mov al, 0BAh
  call sub_9F0
  mov cx, 18h
  mov al, 20h
  call sub_A10
  mov al, 0BAh
  call sub_9F0
.loc_989:
  inc dh
.loc_98B:
  cmp dh, 012h
  jb .loc_972
  mov dl, 01Bh
  call set_cursor_to_dh_dl
  mov al, 0C8h
.loc_997:
  call sub_9F0
.loc_99A:
  mov cx, 018h
  call sub_A0E
.loc_9A0:
  mov al, 0BCh
  call sub_9F0
  mov dx, 1120h
  jmp set_cursor_to_dh_dl

;unknown_chunk db 90h

sub_9AB:
  ; Open dragon.com
  mov dx, offset dragon_com_str

  ; DOS 2+ - OPEN - OPEN EXISTING FILE
  ; AH = 0x3D
  ; AL = access mode  (02 = read/write)
  ; DS:DX = path to file to open (ASCIZ)
  mov ax, 3D02h
  int 21h

  jc .loc_9D4 ; any error ?

  mov [dragon_com_file_handle], ax

  ; DOS 2+ - WRITE - WRITE TO FILE OR DEVICE
  ; AH = 40h
  ; BX = file handle
  ; CX = number of bytes to write
  ; DS:DX -> data to write
  mov ah, 40h
  mov bx, [dragon_com_file_handle]
  mov cx, 0006h
  mov dx, offset start
  int 21h

  ; save flag of writes.
  pushf
  push ax

  ; DOS 2+ - CLOSE - CLOSE FILE
  ; AH = 3Eh
  ; BX = file handle
  mov ah, 3Eh
  mov bx, [dragon_com_file_handle]
  int 21h

  jc .loc_9D5 ; any error ?

  pop ax
  popf

.loc_9D4:
  ret

.loc_9D5:
  pop dx
  pop dx
  ret

; at 0x9D8
dragon_com_file_handle dw 0
; at 0x9DA
dragon_com_str db 'DRAGON.COM', 0

; 0x9E5
; Sets the video hardware cursor to whatever values are in DH, DL
set_cursor_to_dh_dl:
  push bx
  push ax

  ; VIDEO - SET Cursor position.
  ; DH,DL = row, column (0, 0 = upper left)
  ; BH = page number
  xor bh, bh ; bh = 0
  mov ah, 2
  int 10h

  pop ax
  pop bx
  retn

; Should be at 0x9F0 (kinda like a putchar)
sub_9F0:
  push ax
  push bx
  push cx
  push dx

  ; VIDEO - Write attributes/characters at cursor position (ah = 9)
  ; AL = character (passed in)
  ; BH = display page (0)
  ; BL = Attributes of character (alpha modes) or color (graphics modes)
  ; CX = number of times to write character (here 1)
  mov bx, 7
  mov ah, 9
  mov cx, 1
  int 10h

  ; VIDEO - Read Cursor Position (ah = 3)
  ; BH = page number (here 0)
  ; Returns DH, DL = row, column,  CH = cursor start line, CL = cursor end line
  mov ah, 3
  xor bh, bh
  int 10h

  ; move cursor forward
  inc dx

  ; VIDEO - Set cursor position (ah = 2)
  ; BH = page number (here 0)
  ; DH, DL = row, column (0, 0 = upper left)
  mov ah, 2
  int 10h

  pop dx
  pop cx
  pop bx
  pop ax
  retn

; Should be at 0xA0E
; Just sets al to 0xCD ?
sub_A0E:
  mov al, 0CDh

sub_A10:
  call sub_9F0
  loop sub_A10
  retn

sub_A16:
  pop bx
.loc_A17:
  mov al, [bx]
  inc bx
  or al, al
  jz .loc_A23
  call sub_9F0
  jmp .loc_A17
.loc_A23:
  jmp bx


; 0x0A25
setup_memory:
  mov ax, 0DE60h ; # of bytes to allocate.
  call determine_paragraphs
  ; AX now holds # of paragraphs representing 0xDE60

  ; DOS - 2+ - Adjust memory block size (setblock)
  ; ES = segment address of block to change
  ; BX = new size in paragraphs.
  mov bx, ax
  mov ah, 4Ah
  int 21h
  ; AX now holds adjusted block size.

  mov bx, 02A8h
  cmp word ptr [graphics_mode], 6 ; EGA 16 color?
  jne .loc_A3E
  mov bx, 03A8h ; Guess EGA needs more memory?

.loc_A3E:
  ; DOS - 2+ - ALLOCATE MEMORY
  ; BX = number of 16-byte paragraphs desired.
  mov ah, 48h
  int 21h
  mov word ptr [ptr1], ax

  ; allocate memory
  mov bx, 2A8h
  mov ah, 48h
  int 21h
  mov word ptr [ptr2], ax

  mov bx, 370h
  mov ah, 48h
  int 21h
  mov word ptr [ptr3], ax

  retn


; 0x0A5A
; Cleanup memory.
; es clobbered, saved and restored.
sub_A5A:
  push es

  mov es, word ptr [ptr3]
  mov ah, 49h ; DOS - 2+ - FREE MEMORY
  int 21h ; ES = segment address of area to be freed

  mov es, word ptr [ptr1]
           ; DOS - 2+ - FREE MEMORY
  int 21h ; ES = segment address of area to be freed

  mov es, word ptr [ptr2]
           ; DOS - 2+ - FREE MEMORY
  int 21h ; ES = segment address of area to be freed

  ; Loop through memory alloc list and free pointers.
  mov si, 2
  mov di, 4
.loc_A75:
  xor al, al   ; al = 0
  cmp byte ptr [si+memory_alloc_list], al
  jz .loc_A89
  mov byte ptr [si+memory_alloc_list], al
  mov es, word ptr [di+memory_alloc_ptrs]
  mov ah, 49h ; DOS - 2+ - FREE MEMORY
  int 21h ; ES = segment address of area to be freed
.loc_A89:
  inc di
  inc di
  inc si
  cmp si, 80h
  jb .loc_A75
  pop es
  retn

; Zeros from 0xA94-0xCA0 (524 bytes)
zeros_A94: db 524 dup (0)

; Unknown, populating memory with some bytes
; Draws GUI i think
sub_CA0:
  push ds
  mov ds, word ptr [ptr1] ; 0'd pointer.
  xor di, di
  mov cx, 0088h ; number of vertical lines
  mov ax, 0F00Fh

  ; loads ds:di (0-0x2a80) with values (0x88 * 0x50) 10880 bytes
  ; validates that nothing is higher than 0xF0 and 0x0F
.loc_CAD:
  and [di], al
  and [di+4Fh], ah
  add di, 0050h
  loop .loc_CAD

  pop ds
  mov byte ptr [zero_104E], cl ; this would be 0 due to above loop.
  mov bx, 000Ch ; signifies number of viewport things.

.loc_CBF:
  push bx
  mov ax, word ptr [bx+viewport_dimensions] ; what is this?
  mov word ptr [dword_104F], ax
  mov word ptr [dword_104F + 2], cs
  xor ah, ah
  mov al, byte ptr [bx+viewport_dimensions + 2] ; argument? "xpos"
  mov word ptr [arg1_36C0], ax
  mov al, byte ptr [bx+viewport_dimensions + 3] ; "ypos"
  mov word ptr [arg2_36C4], ax ; argment 2 ?

  ; 98, 7b
  ; 0, 7b
  ; 98, 0
  ; 0, 0

  call sub_CF8

; 0xCDE
  pop bx
  sub bx, 4
  jns .loc_CBF

  ; the actual drawing is done here
  jmp draw_viewport

; 0xCE7
  push ds
  lds di, dword ptr [dword_104F]
  mov ax, [bx+di]
  pop ds
  test ax, ax
  jnz .loc_CF4
  ret

.loc_CF4:
  add word ptr [dword_104F], ax

; Unknown function, unknown arguments or responses.
; Reads from dword_104F
; dl is used to test for various things.
sub_CF8:
  push ds
  push es
  les si, dword ptr [dword_104F] ; si = [dword_104F] (contents of)
  lods byte ptr es:[si] ;es lodsb ; load byte at es:si into al (inc si by 1)
  mov byte ptr [word_1048], al ; basically always 4 ?
  lods byte ptr es:[si] ;es lodsb ; load byte at es:si into al (inc si by 1)
  mov byte ptr [counter_104D], al ; counter?
  mov dx, 4080h
  lods byte ptr es:[si] ; es lodsb ; load byte at es:si into al (inc si by 1)
  cbw ; ax = sign extend al   ax = offset?

  ; make sure that ax is positive ?
  test byte ptr [zero_104E], dl ; is greater than 0x80 ?
  je .loc_D16
  neg ax
.loc_D16:
  add word ptr [arg1_36C0], ax
  test byte ptr [word_1048], dl ; is greater than 0x80 ?
  je .loc_D2A
  test byte ptr [zero_104E], dl
  je .loc_D2A
  dec word ptr [arg1_36C0]
.loc_D2A:
  and byte ptr [word_1048], 7Fh ; cap color by making sure it's lower than 0x7F
  lods byte ptr es:[si] ; es lodsb ; offset #2
  cbw
  test byte ptr [zero_104E], dh ; is grater than 0x40
  je .loc_D3A
  neg ax
.loc_D3A:
  add word ptr [arg2_36C4], ax
  mov bx, word ptr [arg1_36C0]
  and bx, word ptr 1 ;  and bx, strict word 0x1
  shl bx, 1
  test byte ptr [arg1_36C0+1], dl
  je .loc_D52
  or bx, word ptr 4 ;or bx, strict word 0x4

.loc_D52:
  test byte ptr [zero_104E], dl
  je .loc_D5C
  or bx, word ptr 8  ;or bx, strict word 0x8
.loc_D5C:
  mov ax, word ptr [word_1053] ; 0x50 ? what is this?
  test byte ptr [zero_104E], dh
  je .loc_D67
  neg ax
.loc_D67:
  mov word ptr [word_1055], ax ; resave what we did at 0xD5C

  push es
  mov es, word ptr [ptr1]
  pop ds
  ; populates the actual pointer.
  call word ptr cs:[bx+function_ptr_tbl]
  pop es
  pop ds
sub_D77:
  ret

function_ptr_tbl: dw sub_D88
  dw sub_DEB
  dw sub_E6D
  dw sub_EC5
  dw sub_F3D
  dw sub_FB2
  dw sub_D77
  dw sub_D77

; 0x98, 0x7B => 0x4C, 0xF6
sub_D88:
  sar word ptr cs:[arg1_36C0], 1 ; divide by 2 (shift right 1)
  mov ax, word ptr cs:[word_1048] ; 0x04
  mov word ptr cs:[word_104A], ax
  add ax, word ptr cs:[arg1_36C0] ; ax = ax + 4 (likely 4)
  sub ax, word ptr cs:[word_1053] ; ax = ax - 0x50
  jbe .loc_DA8
  sub word ptr cs:[word_104A], ax
  jna .loc_DEA
.loc_DA8:
  mov bx, word ptr cs:[arg2_36C4]
  shl bx, 1 ; bx = bx * 2;
  mov dx, word ptr cs:[arg1_36C0]
  add dx, word ptr cs:[bx-4FBEh] ; 0xB042 - 0xB141 pixel table? (0xb138)
  xor bh, bh
.loc_DBB:
  mov cx, word ptr cs:[word_104A] ; counter

  ; save si to bp, so we can skip ahead.
  mov bp, si ; set source and destination pointers
  mov di, dx

  ; memcpy from ? to ptr1
.loc_DC4:
  lodsb ; Load byte at address DS:SI into AL   si++
  mov bl, al
  mov al, es:[di]

  ; We now AND with some table (see 0xB252-0xB351)
  ; Most of the time this value will be 0'd out.
  ; But there are some magic values that work.
  ; Values ending with 0x06 (0x16, 0x26, 0x36, etc) are AND'd with 0x0F.
  ; Values in 0x60-0x65 are AND'd with 0xF0
  ; Value 0x66 is a magic value that AND's with 0xFF
  ; Values 0x67-0x6F are AND'd with 0xF0
  and al, byte ptr cs:[bx - 4DAEh] ; 0xB252 - 0xB351

  or al, byte ptr cs:[bx - 4CAEh] ; 0xB352-0xB451
  stosb   ; al -> es:di di++
  loop .loc_DC4
  mov si, bp ; restore position.
  add si, word ptr cs:[word_1048] ; number bytes to skip
  add dx, word ptr cs:[word_1055] ; skipping by 0x50 ?
  dec byte ptr cs:[counter_104D]
  jnz .loc_DBB
.loc_DEA:
  ret

; Somewhat similar to D88, see notes there.
sub_DEB:
  sar word ptr cs:[arg1_36C0], 1 ; divide by 2 (shift right 1)
  xor dl, dl
  mov ax, word ptr cs:[word_1048]
  mov word ptr cs:[word_104A], ax
  add ax, word ptr cs:[arg1_36C0]
  sub ax, word ptr cs:[word_1053] ; ax = ax - 0x50
  jc .loc_E0F
  sub word ptr cs:[word_104A],ax
  jna .loc_E6C
  dec dl
.loc_E0F:
  mov byte ptr cs:[byte_104C],dl
  mov bx, word ptr cs:[arg2_36C4]
  shl bx, 1
  mov di, word ptr cs:[arg1_36C0]
  add di, word ptr cs:[bx-4FBEh] ; 0xB042 - 0xB141 pixel table? (0xb138)
  xor ah, ah
.loc_E27:
  mov cx, word ptr cs:[word_104A]
  push di
  mov bp, si
  jmp short .loc_E35

  ; loop
.loc_E31:
  mov es:[di], dx
  inc di

.loc_E35:
  lodsb
  mov bx, ax
  shl bx, 1
  mov dx, es:[di]
  and dx, cs:[bx-4BAEh]
  or dx, cs:[bx-49AEh]
  loop .loc_E31

  mov es:[di], dl
  test byte ptr cs:[byte_104C], 80h
  jnz .loc_E58
  inc di
  mov es:[di], dh
.loc_E58:
  mov si, bp
  pop di
  add si, word ptr cs:[word_1048]
  add di, word ptr cs:[word_1055]
  dec byte ptr cs:[counter_104D]
  jnz .loc_E27

.loc_E6C:
  ret

sub_E6D:
  mov ax, word ptr cs:[arg1_36C0]
  neg ax
  sar al, 1
  mov bx, word ptr cs:[word_1048]
  sub bx, ax
  mov word ptr cs:[word_104A], bx
  jna .loc_EC4

  xor ah, ah
  add si, ax
  mov bx, word ptr cs:[arg2_36C4]
  shl bx, 1
  mov dx, word ptr cs:[bx-4fbeh]
  xor ah, ah
.loc_E95:
  mov cx, word ptr cs:[word_104A]
  mov di, dx
  mov bp, si
.loc_E9E:
  lodsb
  mov bx, ax
  mov al, es:[di]
  and al, cs:[bx-4daeh]
  or al, cs:[bx-4caeh]
  stosb
  loop .loc_E9E
  mov si, bp
  add si, word ptr cs:[word_1048]
  add dx, word ptr cs:[word_1055]
  dec byte ptr cs:[counter_104D]
  jnz .loc_E95
.loc_EC4:
  ret

sub_EC5:
  mov ax, word ptr cs:[arg1_36C0]
  neg ax
  sar ax, 1
  mov bx, word ptr cs:[word_1048]
  sub bx, ax
  mov word ptr cs:[word_104A], bx
  jna .loc_F3C

  xor ah, ah
  add si, ax

  mov bx, word ptr cs:[arg2_36C4]
  shl bx,1
  mov di, cs:[bx-4fbeh]
  dec di
  xor ah,ah
.loc_EEE:
  mov cx, word ptr cs:[word_104A]
  push di
  mov bp,si
  lodsb
  mov bx,ax
  shl bx,1
  mov dx, es:[di]
  and dx, cs:[bx-4baeh]
  or dx, cs:[bx-49aeh]
  inc di
  mov es:[di], dh
  loop .loc_F10
  jmp short .loc_F28
.loc_F10:
  lodsb
  mov bx,ax
  shl bx,1
  mov dx, es:[di]
  and dx, cs:[bx-4baeh]
  or dx, cs:[bx-49aeh]
  mov es:[di], dx
  inc di
  loop .loc_F10
.loc_F28:
  mov si,bp
  pop di
  add si, word ptr cs:[word_1048]
  add di, word ptr cs:[word_1055]
  dec byte ptr cs:[counter_104D]
  jnz .loc_EEE
.loc_F3C:
  ret

sub_F3D:
  sar word ptr cs:[arg1_36C0], 1
  mov ax, word ptr cs:[word_1048]
  mov word ptr cs:[word_104a], ax
  add ax, word ptr cs:[arg1_36C0]
  sub ax, word ptr cs:[word_1053]
  jna .loc_F64
  mov bx, word ptr cs:[word_1048]
  sub bx,ax
  mov word ptr cs:[word_104a], bx
  jna .loc_FB1
.loc_F64:
  mov bx, word ptr cs:[arg2_36C4]
  shl bx,1
  mov dx, word ptr cs:[arg1_36c0]
  add dx, cs:[bx-4fbeh]
  add si, word ptr cs:[word_1048]
  dec si
  xor bh,bh
.loc_F7D:
  mov bp,si
  mov di,dx
  mov cx, word ptr cs:[word_104a]
.loc_F86:
  mov bl,[si]
  dec si
  mov bl, cs:[bx-4eaeh]
  mov al, es:[di]
  and al, cs:[bx-4daeh]
  or al, cs:[bx-4caeh]
  stosb
  loop .loc_F86
  mov si,bp
  add si, word ptr cs:[word_1048]
  add dx, word ptr cs:[word_1055]
  dec byte ptr cs:[counter_104d]
  jnz .loc_F7D
.loc_FB1:
  ret

sub_FB2:
  sar word ptr cs:[arg1_36C0], 1
  xor dl, dl
  mov ax, word ptr cs:[word_1048]
  mov word ptr cs:[word_104a], ax
  add ax, word ptr cs:[arg1_36C0]
  sub ax, word ptr cs:[word_1053]
  jc .loc_FDD
  mov bx, word ptr cs:[word_1048]
  sub bx, ax
  mov word ptr cs:[word_104A], bx
  jna .loc_1047
  dec dl
.loc_FDD:
  mov byte ptr cs:[byte_104c], dl
  mov bx, word ptr cs:[arg2_36C4]
  shl bx, 1
  mov dx, word ptr cs:[arg1_36C0]
  add dx, cs:[bx-4fbeh]
  add si, word ptr cs:[word_1048]
  dec si
  xor bh,bh
.loc_FFB:
  mov cx, word ptr cs:[word_104A]
  mov di, dx
  mov bp, si
  jmp short .loc_100A
.loc_1006:
  mov es:[di], ax
  inc di
.loc_100A:
  mov bl,[si]
  dec si
  xor bh,bh
  mov bl, cs:[bx-4eaeh]
  shl bx,1
  mov ax, es:[di]
  and ax, cs:[bx-4baeh]
  or ax, cs:[bx-49aeh]
  loop .loc_1006
  mov es:[di], al
  test byte ptr cs:[byte_104c], 80h
  jnz .loc_1034
  inc di
  mov es:[di], ah
.loc_1034:
  mov si,bp
  add si, word ptr cs:[word_1048]
  add dx, word ptr cs:[word_1055]
  dec byte ptr cs:[counter_104d]
  jnz .loc_FFB
.loc_1047:
  ret

word_1048: dw 0
word_104A: dw 0
byte_104C: db 0
counter_104D: db 0
zero_104E: db 0

dword_104F: dd 0 ; gets set with 67E8 ; function pointer?
                 ; gets set with CS (0x1DD)  (ptr to code segment?)
word_1053: dw 0
word_1055: dw 0
word_1057: dw 0 ; Unknown 1057/1058
word_1059: dw 0 ; Unknown 1059/105A
word_105B: dw 0 ; 105B/105C
word_105D: dw 0 ; 105D/105E
byte_105F: db 0 ; 105F

; 0x1060
; The draw something sub.
; Seems to draw specific data (in ptr1) ?
; May be only for viewport data?
draw_viewport:
  mov al, 00ah
  call sub_2752 ; What does input 0x0a mean?
  jnc .loc_1068 ; jump if cf = 0, if sub_2752 didn't do anything
  ret

.loc_1068:
  ; unknown what sub_1F54 does at this time.
  mov al, 0ah
  call sub_1F54

  ; XXX Document values of SI
  ; SI is always 0
  xor di, di
loc_106F:
  mov si, [di-4FBEh] ; B042 ; table of 80s (0)

  ; Initial offset index.
  mov ax, word ptr [initial_offset]
  add ax, 8
  shl ax, 1 ; ax *= 2 (since it's the index into a word table)
  mov di, ax ; di = ax

  ; 
  mov bp, word ptr [vp_height] ; 0x88 - height
  sub bp, word ptr [initial_offset] ; subtraction
  mov dx, word ptr [vp_width] ; 0x50 - width
  mov bx, word ptr [graphics_mode]
  push es
  push ds
  mov ds, word ptr [ptr1]
  ; draw ds:si -> es:di 
  call word ptr cs:[bx+draw_handlers2]
  pop ds
  pop es
  ret

; 0x109B
draw_handlers2:
  dw sub_10A5 ; 0x0000 (CGA RGB drawing routine)
  dw sub_10A5 ; 0x0002 (CGA composite monitor)
  dw sub_10D8 ; 0x0004 (Tandy 16 color)
  dw sub_10F3 ; 0x0006 (EGA 16 color)
  dw sub_1175 ; 0x0008 (VGA/MCGA 16 color)

; drawing routine for CGA
; inputs: dx = count
sub_10A5:
  mov ax, CGA_BASE_ADDRESS
  mov es, ax
  xor bh, bh
  shr dx, 1
.loc_10AE:
  push di    ; save di
  mov di, cs:[di-55feh]
  add di, 4
  mov cx,dx
.loc_10B9:
  lodsw
  mov bl,al
  mov al, cs:[bx-47aeh]
  mov bl,ah
  or al, cs:[bx-46aeh]
  stosb
  loop .loc_10B9
  pop di
  inc di
  inc di
  dec bp
  jnz .loc_10AE
  ret

; 0x10D2
initial_offset: dw 0
; 0x10D4
vp_height: dw 88h
; 0x10D6
vp_width: dw 50h

; Tandy drawing
sub_10D8:
  mov ax, CGA_BASE_ADDRESS
  mov es, ax
  mov bx, di
  shr dx, 1
.loc_10E1:
  mov di, cs:[bx-52deh]
  add di, 8
  mov cx, dx
  rep movsw
  inc bx
  inc bx
  dec bp
  jnz .loc_10E1
  ret

; ega
sub_10F3:
  mov ax, 0a000h
  mov es, ax
  mov di, cs:[di-546eh]
  inc di
  inc di
  mov cx, bp
  shr dx, 1
  shr dx, 1
  mov bp, dx
  mov dx, 3C4h
  mov ax, 102h
  out dx, ax
  inc dx
.loc_110F:
  push cx
  push di
  push bp
  xor ah, ah
.loc_1114:
  lodsb
  mov bx, ax
  shl bx, 1
  mov cx, [bx+2a80h]
  mov dx, [bx+2c80h]
  lodsb
  mov bx, ax
  shl bx, 1
  or cx, [bx+2e80h]
  or dx, [bx+3080h]
  lodsb
  mov bx, ax
  shl bx, 1
  or cx, [bx+3280h]
  or dx, [bx+3480h]
  lodsb
  mov bx, ax
  shl bx, 1
  or cx, [bx+3680h]
  or dx, [bx+3880h]
  mov bx, dx
  mov dx, 3c5h
  mov al, 8
  out dx, al
  mov es:[di], cl
  shr al, 1
  out dx, al
  mov es:[di], ch
  shr al, 1
  out dx, al
  mov es:[di], bl
  shr al, 1
  out dx, al
  mov es:[di], bh
  inc di
  dec bp
  jnz .loc_1114
  pop bp
  pop di
  pop cx
  add di, 28h
  loop .loc_110f
  mov al, 0fh
  out dx, al
  ret

; Drawing routine! (this is a function ptr) used for MCGA 256 color.
; DX is passed in as count.
; SI passed in as source buffer for pixel lookups.
; di passed as destination pixel start.

; ds = source picture.?
; dx = 0x50 0x50  = 80
; bx = 0x08 0x00 = 8
; cx = 0x00 0x00 
; di = 0x10 0x10
; bp = 0x88 0x88
; si = 0x00 0x00
sub_1175:
  mov ax, VGA_BASE_ADDRESS
  mov es, ax
.loc_117A:
  push di
  mov di, cs:[di-514eh] ; 0xAEB2-
  add di, 10h   ; inset - di = 0xa10  line 8, 10  (8, 10)
  mov cx, dx
.loc_1185:
  lodsb ; load byte from DS:SI into al (bump si by 1)
  mov bl, al ; save
  xor bh, bh
  shl bx, 1; bx = bx * 2;
  mov ax, cs:[bx-45AEh];   // bba6  ; pixel data
  stosw ; store ax into es:di   (di += 2)
  loop .loc_1185
  pop di
  inc di
  inc di
  dec bp
  jne .loc_117A
  ret

word_119B: dw 0
word_119D: dw 0
byte_119F: db 0

sub_11A0:
  mov word ptr [word_11C4], 0
sub_11A6:
  mov ax, word ptr [word_11C2]
  mul word ptr [word_11C0]
  mov word ptr [word_11C6], ax
  mov word ptr [word_11C8], dx
  mov ax, word ptr [word_11C4]
  mul word ptr [word_11C0]
  add word ptr [word_11C8], ax
  ret

word_11C0: dw 0
word_11C2: dw 0
word_11C4: dw 0
word_11C6: dw 0
word_11C8: dw 0

word_11CA: dw 0
word_11CC: dw 0

; 11CE
sub_11CE:
  mov word ptr [word_11CA], 0
  mov word ptr [word_11CC], 0
  mov cx, 20h
.loc_11DD:
  shl word ptr [word_11C6], 1
  rcl word ptr [word_11C8], 1
  rcl word ptr [word_11CA], 1
  rcl word ptr [word_11CC], 1
  mov ax, word ptr [word_11CA]
  sub ax, word ptr [word_11C0]
  mov bx, word ptr [word_11CC]
  sbb bx, 0
  jc .loc_1208
  mov word ptr [word_11CA], ax
  mov word ptr [word_11CC], bx
  inc word ptr [word_11C6]
.loc_1208:
  loop .loc_11DD
  ret

;120B
sub_120B:
  mov word ptr [word_11C2], 0
  mov word ptr [word_11C4], 0
  mov word ptr [word_11C0], 0ah
  mov bx, 0ffffh
.loc_1220:
  inc bx
  mov al, [bx+3926h]
  and al, 7fh
  cmp al, 20h
  jz .loc_1220
.loc_122B:
  mov al, [bx+3926h]
  inc bx
  and ax, 7fh
  xor al, 30h
  cmp al, 0ah
  ja .loc_125D

  push ax
  push bx
  call sub_11A6
  pop bx
  pop ax
  add ax, word ptr [word_11C6]
  mov word ptr [word_11C2], ax
  mov ax, word ptr [word_11C8]
  adc ax, 0
  mov word ptr [word_11C4], ax
  jnc .loc_122B
  mov cx, 4
  mov di, 3897h
  mov al, 0ffh
  rep stosb
  ret

.loc_125D:
  mov cx, 4
  mov si, 11c2h
  mov di, 3897h
  rep movsb
  ret

word_1269: dw 0
word_126B: dw 0
word_126D: dw 0
byte_126F: db 0

; Input AL. free's title sequence.
sub_1270:
  cmp al, 0FFh
  je .loc_128C
  cmp al, 2
  jb .loc_128C

  ; do actual memory free.
  xor ah, ah
  mov di, ax
  mov byte ptr [di+memory_alloc_list], ah
  shl di, 1
  push es
  mov es, word ptr [di+memory_alloc_ptrs]

  ; DOS - 2+ - FREE MEMORY
  ; ES = segment address of area to be freed.
  mov ah, 49h
  int 21h
  pop es
.loc_128C:
  ret

; sets memory alloc list item to '2'.
; al = index of memory to mark.
sub_128D:
  mov bl, 2 ; marking the memory alloc list with this.
  jmp short loc_1297

  mov bl, 1
  jmp short loc_1297
  mov bl, 0FFh

loc_1297:
  cmp al, 0FFh
  jz .loc_12A7
  cmp al, 2
  jb .loc_12A7
  xor ah, ah
  mov si, ax
  mov byte ptr [si + memory_alloc_list], bl

.loc_12A7:
  ret


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

; 0x12B5
; Get bytes extracted.
; Outputs: BX - bytes of resource
sub_12B5:
  xor ah, ah
  shl ax, 1
  mov si, ax
  mov bx, word ptr [si+bytes_allocated_list]
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

; Inputs al, bx
sub_12E6:
  mov byte ptr [mem_alloc_list_marker], al
  mov word ptr [last_allocated_byte_size], bx
  mov ax, bx ; AX now controls number of bytes to allocate.
  call determine_paragraphs
  mov word ptr [mem_paragraphs_to_alloc], ax
  mov ax, word ptr [tag_lookup_item]
  or ax, ax
  js .loc_1301
  call find_index_by_tag
  jnc .loc_1316

.loc_1301:
  call game_mem_alloc
  jnc .loc_1325
  call sub_138E ; memory failure?
  jnc .loc_1301

  call game_mem_alloc
  jnc .loc_1325
  mov dx, 132bh
  jmp loc_1B5

.loc_1316:
  mov al, byte ptr [mem_alloc_list_marker]
  mov byte ptr [si + memory_alloc_list], al
  mov ax, si
  push ax
  call get_indexed_memory
  pop ax
  stc
.loc_1325:
  ret


; 0x1326 (a value of 0 means this location is free, a non value of 0 means
;         this location is probably not free)
mem_alloc_list_marker: db 0

; 0x1327
last_allocated_byte_size: dw 0
; 0x1329
mem_paragraphs_to_alloc: dw 0

fatal_error: db 'Fatal error : Out of memory.$'

; Allocates memory from a memory block list.
; BX = Number of 16 byte paragraphs desired.
; Returns:
;   Index into table (AX)
;   Allocated memory (CX)
; at 0x1348
game_mem_alloc:
  mov di, offset memory_alloc_list
  mov cx, 80h ; size of memory allocation list.
  nop

  ; Search for a free location.
  ; We can do 128 allocations, but no more.
  ; A value of 0 means that we can allocate, a value of non zero
  ; means the memory is already allocated.
  xor ax, ax
  repne scasb   ; essentially searching ES:DI for AX (0), EDI++
  jnz .loc_138C ; not found. jump to error.

  ; found a free spot, prepare to allocate.
  ; di is the index into the memory_alloc_list.
  sub di, offset memory_alloc_list + 1
  push di
  mov bx, word ptr [mem_paragraphs_to_alloc]

  ; DOS - 2+ - ALLOCATE MEMORY
  ; BX = number of 16-byte paragraphs desired.

  ; Returns:
  ; CF - clear if successful.
  ;    AX = segment of allocated block;
  ; CF - set on error
  ;    AX = error code
  ;    BX = size of largest available block.
  mov ah, 48h
  int 21h

  pop di ; Index into memory allocation list.
  jc .loc_138C ; error

  ; Mark the memory_allocation list as used with a non-zero value.
  mov bl, byte ptr [mem_alloc_list_marker]
  mov byte ptr [di+memory_alloc_list], bl

  ; Save the memory allocated in a list.
  shl di, 1
  mov word ptr [di+memory_alloc_ptrs], ax

  mov ax, word ptr [last_allocated_byte_size]
  mov word ptr [di+bytes_allocated_list], ax
  mov ax, word ptr [tag_lookup_item]
  mov word ptr [di+memory_tag_list], ax

  mov ax, di
  shr ax, 1
  push ax
  call get_indexed_memory
  ; Memory will be in CX register.
  pop ax
  clc
  ret
.loc_138C:
  ; Error occurred during allocation process.
  stc
  ret

sub_138E:
  mov cx, 80h
  nop
  mov al, 2
  mov si, word ptr [word_13B5]
.loc_1398:
  inc si
  cmp si, 80h
  jb .loc_13A1
  xor si, si
.loc_13A1:
  cmp byte ptr [si+memory_alloc_list], al
  jz .loc_13AB
  loop .loc_1398
  stc
  ret

.loc_13AB:
  mov ax, si
  mov word ptr [word_13B5], ax
  call sub_1270
  clc
  ret

word_13B5: dw 0


; Given ax, determine the number of paragraphs (16 byte units)
; to allocate. If we don't end on a 16 byte boundary, increment.
; Inputs: AX (# of bytes)
; Outputs: AX (# of paragraphs)
; Clobbers BX.
; Originally at CS:13B7
determine_paragraphs:
  ; AX is passed in
  ; AX >> 4
  ; if (AX & 0x000f) AX++
  mov bx, ax
  shr ax, 1
  shr ax, 1
  shr ax, 1
  shr ax, 1
  test bx, 000fh
  jz .locret_13c8
  inc ax
.locret_13c8:
  retn

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

word_174A: dw 0
word_174B: dw 0
word_174D: dw 0
byte_174F: db 0

; Mini map
sub_1750:
  mov bx, offset data_17D9
  mov cx, cs
  call sub_25E0
  mov al, byte ptr [game_state]
  mov byte ptr [byte_1960], al
  mov al, byte ptr [game_state + 1]
  mov byte ptr [byte_1961], al

  call sub_17E2
  call sub_59A6
loc_176A:
  call sub_17F7

  mov bx, offset mini_map_inputs
  mov cx, cs
  call sub_28B0
  jmp bx

; 0x1777
mini_map_inputs: db 80h, 80h ; flags (0x8080) - Disable mouse
  db 09Bh, 09Bh, 017h  ; ESC -> 0x179B
  db 088h, 0A7h, 017h  ; Left -> 0x17A7
  db 0CAh, 0A7h, 017h  ; 'J' -> 0x17A7
  db 095h, 0B3h, 017h  ; Right -> 0x17B3
  db 0CCh, 0B3h, 017h  ; 'L' -> 0x17B3
  db 08Ah, 0C0h, 017h  ; Down -> 0x17C0
  db 0DAh, 0C0h, 017h  ; 'Z' -> 0x17C0
  db 0CBh, 0C0h, 017h  ; 'K' -> 0x17C0
  db 08Bh, 0CCh, 017h  ; Up -> 0x17CC
  db 0C1h, 0CCh, 017h  ; 'A' -> 0x17CC
  db 0C9h, 0CCh, 017h  ; 'I' -> 0x17CC
  db 0FFh ;

; 179B
mini_map_escape:
  call init_offsets
  call sub_37C8
  call sub_587E
  jmp sub_26B8

; 17A7
mini_map_left:
  mov al, byte ptr [byte_1961]
  dec al
  js loc_176A
loc_17AE:
  mov byte ptr [byte_1961], al
  jmp short loc_176A

; 17B3
mini_map_right:
  mov al, byte ptr [byte_1961]
  inc al
  cmp al, byte ptr [game_state + 33]
  jc loc_17AE
  jmp short loc_176A

; 17C0
mini_map_down:
  mov al, byte ptr [byte_1960]
  dec al
  js loc_176A
loc_17C7:
  mov byte ptr [byte_1960], al
  jmp short loc_176A

; 17CC
mini_map_up:
  mov al, byte ptr [byte_1960]
  inc al
  cmp al, byte ptr [game_state + 34]
  jc loc_17C7
  jmp short loc_176A

data_17D9: db 01h, 00h, 27h, 0C0h

; sets up table to be sums of 0x50, 0x88 times.
; 0x00, 0x00, 0x50, 0x00, 0xA0, 0x00, 0xF0, 0x00, 0x40, 0x01 ...
; 0x17DD
init_offsets:
  mov dx, 50h
  jmp short loc_17E5
sub_17E2:
  mov dx, 90h
loc_17E5:
  xor ax, ax
  mov word ptr [word_1053], dx
  mov di, offset table_of_80
  mov cx, 0088h
.loc_17F1:
  ; Fill ES:DI with AX.  DI += 2
  stosw
  add ax, dx
  loop .loc_17F1 ; loops until cx = 0
  ret

; 0x17F7
; Draw the mini map
sub_17F7:
  mov byte ptr [byte_1964], 0
  mov byte ptr [zero_104E], 0
  jmp short loc_1806
.loc_1803:
  call sub_184B

loc_1806:
  xor al, al
.loc_1808:
  mov byte ptr [byte_1962], al

  call sub_1861
  mov al, byte ptr [byte_1962]
  inc al
  cmp al, 9
  jc .loc_1808
  call sub_1967

  ; Get state of key press buffer
  mov ah, 1
  int 16h
  jz .loc_1837

  ; got a key
  mov cx, 4
  mov di, offset data_1843
  test al,al

  jz .loc_1831
  mov di, offset data_1847
  and al, 0dfh
  mov ah,al
.loc_1831:
  mov al,ah
  repne scasb

  jz .loc_1842
.loc_1837:
  ; No key, or key pressing done.
  inc byte ptr [byte_1964]
  mov al, byte ptr [byte_1964]
  cmp al, 8
  jc .loc_1803
.loc_1842:
  ret

data_1843: db 04bh, 04dh, 050h, 048h  ; Arrow keys
data_1847: db 04ch, 04ah, 04bh, 049h

; Viewport move memory
; 0x184B
sub_184B:
  push es
  push ds
  mov si, 0d80h
  xor di,di
  mov cx, 06c0h
  mov ax, word ptr [ptr1]
  mov es,ax
  mov ds,ax
  rep movsw ; mov word ds:si to es:di (si, di += 2), repeat 0x06C0 times.
  pop ds
  pop es
  ret

sub_1861:
  call sub_194A
  xor al, al

  cmp bl, byte ptr [game_state]
  jnz .loc_1874
  cmp dl, byte ptr [game_state + 1]
  jnz .loc_1874
  not al
.loc_1874:
  mov byte ptr [byte_1966],al
  call sub_54D8

  test byte ptr [word_11C6+1], 8   ; 11C7 (should it be a byte instead of word)
  jz .loc_18E4
  mov bl, byte ptr [word_11C6 + 1]
  shr bl, 1
  shr bl, 1
  shr bl, 1
  shr bl, 1
  and bx, word ptr 3

  mov al, byte ptr [bx + data_56E5 + 4] ; ?
  xor di,di
  call sub_19C7
  mov bl, byte ptr [word_11C6]
  shr bl, 1
  shr bl, 1
  shr bl, 1
  shr bl, 1
  and bx, word ptr 0fh

  jz .loc_18B6
  mov al, byte ptr [bx + data_56C6]
  mov di, 6
  call sub_19C7
.loc_18B6:
  mov bl, byte ptr [word_11C6]
  and bx, word ptr 0fh
  jz .loc_18CA
  mov al, byte ptr [bx + data_56C6]
  mov di, 0ch
  call sub_19C7
.loc_18CA:
  test byte ptr [byte_1966], 80h
  jnz .loc_1945

  mov bl, byte ptr [word_11C6 + 1]
  and bx, word ptr 7
  jz .loc_1948
  mov al, byte ptr [bx + data_56e5 + 7]
  xor di,di
  jmp sub_19C7

.loc_18E4:
  mov bx,695ch
  call sub_1A13
  mov al, byte ptr [word_11C6]
  mov byte ptr [byte_1949], al
  call sub_194A
  inc bl
  call sub_54D8
  test byte ptr [word_11C6 + 1],8
  jz .loc_191B

  mov bl, byte ptr [byte_1949]
  shr bl, 1
  shr bl, 1
  shr bl, 1
  shr bl, 1
  and bx, word ptr 0fh
  jz .loc_191B

  mov al, byte ptr [bx + data_56C6]
  mov di, 6
  call sub_19C7
.loc_191B:
  call sub_194A
  dec dl
  call sub_54D8
  test byte ptr [word_11C6 + 1],8
  jz .loc_193E
  mov bl, byte ptr [byte_1949]
  and bx, word ptr 0fh
  jz .loc_193E
  mov al, byte ptr [bx + data_56C6]
  mov di, 0ch
  call sub_19C7
.loc_193E:
  test byte ptr [byte_1966], 80h
  jz .loc_1948

.loc_1945:
  jmp sub_1A10
.loc_1948:
  ret


byte_1949: db 0

sub_194A:
  mov bl, 3
  sub bl, byte ptr [byte_1964]
  add bl, byte ptr [byte_1960]

  mov dl, byte ptr [byte_1961]
  sub dl, 4
  add dl, byte ptr [byte_1962]
  ret


byte_1960: db 0
byte_1961: db 0
byte_1962: db 0
byte_1963: db 0
byte_1964: db 0
byte_1965: db 0
byte_1966: db 0

; minimap?
sub_1967:
  mov byte ptr [vp_width], 90h
  mov bx, word ptr [byte_1964]
  shl bx, 1
  mov ax, word ptr [bx+data_1997]
  mov byte ptr [initial_offset], al
  mov ax, [bx+19A7h]
  mov byte ptr [vp_height], al
  mov di, [bx+19B7h]
  call loc_106F ; 1984
  mov byte ptr [initial_offset], 0
  mov byte ptr [vp_height], 88h
  mov byte ptr [vp_width], 50h
  ret

; 1997 (viewport offsets)
data_1997:
  dw 0, 10h, 28h, 40h, 58h, 70h, 88h, 0a0h

sub_19C7:

sub_1A10:

sub_1A13:

loc_1F53:
  ret

; input is al
; unknown what this does at this time.
sub_1F54:
  cmp byte ptr [byte_2476], 0
  je loc_1F53

  xor ah, ah
  shl ax, 1
  shl ax, 1
  mov bx, ax
  mov ax, word ptr [word_2472]

sub_1F8F:
  cmp byte ptr [byte_2476], 0
  jz loc_1F53

word_2472: dw 0
word_2474: dw 0FFFFh
byte_2476: db 0
byte_2477: db 0
byte_2478: db 0

; 0x25E0
; inputs cx:bx (script)
sub_25E0:
  push ds


; 0x268E
ui_drawn_yet: db 0
word_2697: dw 0
word_2699: dw 0
word_269B: dw 0
word_269D: dw 0

; 0x26B8
sub_26B8:

; counter WTF?
; 0x2720
; NO Inputs, no outputs
counters_increment:
  ; word_269D += 8;
  mov al, byte ptr [word_269D]
  add al, 08h
  mov byte ptr [word_269D], al

  ; word_2699 -= 8;
  mov al, byte ptr [word_2699]
  sub al, 08h
  mov byte ptr [word_2699], al

  ; word_269B++;
  inc byte ptr [word_269B]
  ; word_2697--;
  dec byte ptr [word_2697]
  ret

; 0x2739
; NO Inputs, no outputs
counters_decrement:
  mov al, byte ptr [word_2699]
  add al, 08h
  mov byte ptr [word_2699], al
  mov al, byte ptr [word_269D]
  sub al, 08h
  mov byte ptr [word_269D], al
  dec byte ptr [word_269B]
  inc byte ptr [word_2697]
  ret


; Inputs: AL
sub_2752:
  cmp byte ptr [ui_drawn_yet], 0
  je loc_2792

; Inputs: AL
sub_2759:
  xor ah, ah
  shl ax, 1
  shl ax, 1
  mov si, ax
  call counters_increment
  mov al, [si+2794h]
  cmp al, byte ptr [word_269B]
  ; XXX 2018-09-03
  jnc .loc_278F
  mov al, [si+2795h]
  cmp al, byte ptr [word_269D]
  jnc .loc_278F
  mov al, byte ptr [word_2697]
  cmp al, [si+2796h]
  jnc .loc_278F
  mov al, byte ptr [word_2699]
  cmp al, [si+2797h]
  jnc .loc_278F
  call counters_decrement
  stc ; error condition?
  ret
.loc_278F:
  call counters_decrement

loc_2792:
  clc ; carry flag = 0
  ret

sub_28B0:

; Gets keycode ?
; Returns keycode in AL.
; Also sets sign flag ?
; At 0x2D0B
sub_2D0B:
  mov bx, word ptr [unknown_2dd9]
  or bx, bx
  js .loc_2D31
  ; 0x2D13
  mov al, byte ptr [bx+unknown_2ddb]
  or al, al
  jz .loc_2D2B
  inc bx
  test bx, 0fh
  jnz .loc_2D25
  mov bx, 0ffffh

.loc_2D25:
  mov word ptr [unknown_2dd9], bx
  jmp .loc_2D4B

.loc_2D2B: ; AL was 0.
  mov word ptr [unknown_2dd9], 0ffffh ; Save?

.loc_2D31:

  ; KEYBOARD - CHECK BUFFER, DO NOT CLEAR
  ; Return: ZF clear if character in buffer
  ; AH = scan code, AL = character
  ; ZF set if no character in buffer
  mov ah, 1
  int 16h
  jz .loc_2D6F

  ; key is in buffer.
  ; KEYBOARD - READ CHAR FROM BUFFER, WAIT IF EMPTY
  ; Return: AH = BIOS scan code, AL = ASCII character
  xor ah, ah
  int 16h

  ; check_keystroke might change al to something else.
  ; For example the '1' key al goes in at 0x31 and comes out as 0xB1
  ; Involved in remapping values for arrow keys.
  call check_keystroke

  jns .loc_2D72
  ; Here we deal with numbers > 0x7F

  ; As far as I can tell this is unreachable code.
  ; The only way that AL ends up being 0x93 is if AL is 0x13 and sub_2E7B
  ; ORs it with 0x80.
  ; Possibly handles control-S ?
  cmp al, 93h
  jne .loc_2D4B

  ; XXX: NEVER REACHED?
  xor byte ptr [byte_107], 40h
  jmp .loc_2D31

.loc_2D4B:
  mov bx, word ptr [unknown_2dd7]
  or bx, bx
  js .loc_2D6C
  mov byte ptr [bx+unknown_2ddb], al
  inc bx
  test bx, 0fh
  jnz .loc_2D63
  mov bx, 0ffffh
  jmp .loc_2D68

.loc_2D63:
  mov byte ptr [bx+unknown_2ddb], 0

.loc_2D68:
  mov word ptr [unknown_2dd7], bx

.loc_2D6C:
  or al, al
  retn

.loc_2D6F: ; No key was pressed.
  xor al, al
  retn

; dealing with values of al between 0 and 0x7f
.loc_2D72:
  cmp al, 3Bh
  jb .loc_2D6F
  cmp ah, 44h
  ja .loc_2D9E
  sub ah, 3Bh
  shl ah, 1
  shl ah, 1
  shl ah, 1
  shl ah, 1
  mov bx, word ptr [unknown_2dd7]
  and bx, 0f0h
  cmp bl, ah
  jz .loc_2DC3
  mov bl, ah
  xor bh, bh
  mov word ptr [unknown_2dd9], bx
  jmp sub_2D0B

.loc_2D9E:
  cmp ah, 68h
  jb .loc_2D6F
  cmp ah, 71h
  ja .loc_2D6F
  sub ah, 68h
  shl ah, 1
  shl ah, 1
  shl ah, 1
  shl ah, 1
  mov bx, word ptr [unknown_2dd7]
  or bx, bx
  js .loc_2DCC
  and bx, 0f0h
  cmp bl, ah
  jnz .loc_2DCC

.loc_2DC3:
  mov word ptr [unknown_2dd7], 0ffffh
  jmp sub_2D0B

.loc_2DCC:
  mov bl, ah
  xor bh, bh
  mov word ptr [unknown_2dd7], bx
  jmp sub_2D0B

; 0x2DD7
unknown_2dd7: dw 0ffffh
; 2DD9
unknown_2dd9: dw 0ffffh
unknown_2ddb: db 30 dup(0)

; Check keystroke
; Inputs are AH = BIOS scan code, AL = ASCII character
; Returns AL (remapped keystroke)
; CS:0x2E7B
check_keystroke:
  or al, al
  jnz .loc_2E9E

  ; al is 0.

  ; map some BIOS scan codes to ascii characters
  ; which get put into al.
  mov al, 88h
  cmp ah, 4bh ; LEFT key.
  je .loc_2EA6

  mov al, 95h
  cmp ah, 4dh ; RIGHT key.
  je .loc_2EA6

  mov al, 8Ah
  cmp ah, 50h ; DOWN key
  je .loc_2EA6

  mov al, 8Bh
  cmp ah, 48h ; UP key.
  je .loc_2EA6

  xor al, al ; None of these keys.
  retn

.loc_2E9E:
  ; AL is non-zero.
  or al, 80h
  cmp al, 0ffh
  jne .loc_2EA6

  ; default to 0x88 (which probably means unhandled)
  mov al, 88h ; AL was 0xFF, but we return 0x88 (which is the code for
               ;  left key)

.loc_2EA6:
  or al, al
  retn

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
  ; al = method: offset from begining of file.
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
; 0x313E
ptr3: dw 0

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

; 36C0
arg1_36C0: dw 0
word_36C2: dw 0
; 36C4
arg2_36C4: dw 0

; 0x37C8
sub_37C8:

loc_37F0:

; 3824
poll_mouse:
  cmp byte ptr [g_mouse_configured], 0
  jz loc_3846

  ; MS - MOUSE - RETURN POSITION AND BUTTON STATUS
  ; Return: BX = button status, CX = column, DX = row
  mov ax, 3
  int 33h

  mov word ptr [mouse_last_xpos], cx ; 0-639
  mov word ptr [mouse_last_ypos], dx ; 0-199

  ; bx = 1 if left button pressed
  ; bx = 2 if right button pressed
  ; bx = 3 if both buttons pressed
  ; since Dragon wars does not use the right mouse button
  ; as far as I can tell, bx will usually be 0 or 1.
  cmp bx, 1

  ; if left mouse button pressed, CF will be 0
  ; else, carry flag will be 1
  cmc ; CF = ~CF  (or flip the above)

  ; Push the carry into the most significant bit, stores the last
  ; 8 clicks.
  rcr byte ptr [byte_3854], 1

sub_3840:
  mov al, byte ptr [byte_3854]
  and al, 0C0h ; only keep the last 2 clicks.

  ret

loc_3846:
  xor ax, ax
  mov word ptr [mouse_last_xpos], ax
  mov word ptr [mouse_last_ypos], ax
  mov byte ptr [byte_3854], al
  and al, 0C0h
  ret


byte_3854: db 0

; 0x3855
g_mouse_configured: db 0
; 0x3856
mouse_last_xpos: dw 0
; 0x3858
mouse_last_ypos: dw 0

; 0x3860 - 0x38E0
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

; 0x4C4A
word_4C4A: dw 0


; 0x4F0F
byte_4F0F: db 0
byte_4F10: db 0
; 0x4F11
ptr1: dw 0
; 0x4F13
ptr2: dw 0

sub_54D8:

data_56C6: db 0
data_56E5: db 0

sub_587E:


sub_59A6:
  ; Implemented in engine.c


byte_5A70: db 0
byte_5A71: db 0
byte_5A72: db 0
word_5A74: dw 0

word_5A76: dw 0
word_5A78: dw 0  ; Holds 5A9D
word_5A7A: dw 0  ; Holds 5C1D


word_5C35: dw 0
; 0x5C37
old_pit_isr: dd 0

; 0x5C3B
; PIT timers
sub_5C3B:
  push es

  ; Get System timer (PIT) routine.
  mov ax, 3508h
  int 21h     ; DOS - 2+ - GET INTERRUPT VECTOR
              ; AL = interrupt number
              ; Return: ES:BX = value of interrupt vector
  mov word ptr [old_pit_isr], bx
  mov word ptr [old_pit_isr+2], es
  pop es

  mov dx, offset sub_5C7B ; Needs to be replaced with vector function.
  mov ax, 2508h
  int 21h    ; DOS - SET	INTERRUPT VECTOR
             ; AL = interrupt number
             ; DS:DX = new vector to be used for	specified interrupt

  ; Why are the below bytes written to different i/o ports?
  ; PIT timers.
  ; SET pit timers for PC speaker playing.
  mov  al, 0B6h
  out 43h, al   ; Timer 8253-5 (AT:	8254.2).

  ; Set to 274.1687 hz.
  ; 1193182 / 0x1100 = 274.
  mov ax, 1100h
  out 40h, al   ; Timer 8253-5 (AT:	8254.2).
  mov al, ah
  out 40h, al   ; Timer 8253-5 (AT:	8254.2).
  retn

loc_5C60:
  mov ax, 0FFFFh
  out 40h, al
  mov al, ah
  out 40h, al

  push ds
  lds dx, dword ptr [old_pit_isr]

  mov ax, 2508h
  int 21h    ; DOS - SET INTERRUPT VECTOR
             ; AL = interrupt number
             ; DS:DX = new vector to be used for	specified interrupt

  in al, 61h
  and al, 0FCh
  out 61h, al

  ret

; New timer routine, invoked 18.2 times every second.
; 0x5C7B
sub_5C7B:
  sti
  push ax
  push bx
  push dx
  push ds
  push si
  push cs
  pop ds

  ; seems to be a volatile bool that controls
  ; whether sub_5CB6 should be run.
  cmp byte ptr [byte_5A71], 0
  jnz .loc_5C97
  mov byte ptr [byte_5A71], 1
  call sub_5CB6
  mov byte ptr [byte_5A71], 0

.loc_5C97:
  dec byte ptr [byte_5A70]
  jnz .loc_5CAC
  mov byte ptr [byte_5A70], 0Dh
  pop si
  pop ds
  pop dx
  pop bx
  pop ax
  jmp [old_pit_isr]

.loc_5CAC:
  mov al, 20h
  out 20h, al
  pop si
  pop ds
  pop dx
  pop bx
  pop ax
  iret

sub_5CB6:
  mov bx, word ptr [word_5A74] ; Starts as 1 ?
  or bx, bx
  jz .loc_5D14 ; done playing music?
  mov word ptr [word_5C35], 0
  mov byte ptr [byte_5A72], 4
  mov si, 5C1Dh
  mov word ptr [word_5A7A], si
  mov si, 5A9Dh
  mov word ptr [word_5A78], si

.loc_5CD7:
  cmp word ptr [si], 0
  je .loc_5CF4
  call sub_5D1D
  xor ax, ax
  cmp word ptr [word_5C35], ax
  jne .loc_5CF4
  cmp [si+0Ah], ax
  je .loc_5CF4


.loc_5CF4:
  add si, 30h
  dec byte ptr [byte_5A72]
  jnz .loc_5CD7

  ; Music playing (frequency)
  mov ax, [bx+08h]
  out 42h, al ; low byte
  mov al, ah
  out 42h, al ; high byte

  mov bx, [bx+0ah]
  and bl, 03h
.loc_5D14:
  in al, 61h ; turn on note (get value from port 61h)
  and al, 0fch ; get ready to set bits 0 and 1
  or al, bl
  out 61h, al ; set bits 0 and 1, (can turn on or off the music)
  ret

sub_5D1D:
  push cx

; Seems to initialize 4 records to 0,
; Possibly player records.
sub_5ECA:
  mov word ptr [word_5A74], ax
  mov si, ax
  shl si, 1
  shl si, 1
  shl si, 1
  mov byte ptr [byte_5A71], 1
  mov byte ptr [byte_5A72], 4
  mov di, 5A9Dh

.loc_5EE2:
  mov ax, [si+5F10h]
  cmp ax, 0
  jz .loc_5EFE
  mov bx, 002Eh

.loc_5EEE:
  mov word ptr [bx+di], 0000h
  sub bx, 2
  jge .loc_5EEE
  mov [di+2], ax
  mov word ptr [di], 1

.loc_5EFE:
  add di, 30h
  inc si
  inc si
  dec byte ptr [byte_5A72]
  jnz .loc_5EE2
  mov byte ptr [byte_5A71], 0
  ret

; 0x6748 ; another set of function pointers?
; First line is a pointer to the data
; followed by X and Y bytes.
viewport_dimensions:
  dw data_6758
  db 0, 0
  dw data_6784
  db 98h, 0
  dw data_67B0
  db 0, 7Bh
  dw data_67E8
  db 98h, 7Bh

; 1st data point will be anded with 0x80 and 0x7F
; second data point is a counter
; Viewport pixel data starts on 5th
data_6758: db 004h, 00Ah, 000h, 000h
           db 0AAh, 0AAh, 0AAh, 006h ; 0x00 (0x675C-0x675F)
           db 0AAh, 0AAh, 0AAh, 006h ; 0x01 (0x6760-0x6763)
           db 0AAh, 0AEh, 0E2h, 006h ; 0x02 (0x6764-0x6767)
           db 000h, 000h, 000h, 066h ; 0x03 (0x6768-0x676B)
           db 022h, 022h, 020h, 066h ; 0x04 (0x676C-0x676F)
           db 00Eh, 0AAh, 0A0h, 066h ; 0x05 (0x6770-0x6773)
           db 00Eh, 0AEh, 006h, 066h ; 0x06 (0x6774-0x6777)
           db 00Eh, 0E0h, 066h, 066h ; 0x07 (0x6778-0x677B)
           db 00Ah, 006h, 066h, 066h ; 0x08 (0x677C-0x677F)
           db 006h, 066h, 066h, 066h ; 0x09 (0x6780-0x6783)

data_6784: db 004h, 00Ah, 000h, 000h
           db 060h, 0AAh, 0AEh, 0EAh ; 0x00 (0x6788-0x678B)
           db 060h, 0AEh, 0EEh, 0AEh ; 0x01 (0x678C-0x678F)
           db 060h, 02Eh, 0EEh, 0E2h ; 0x02 (0x6790-0x6793)
           db 066h, 000h, 000h, 000h ; 0x03 (0x6794-0x6797)
           db 066h, 002h, 022h, 022h ; 0x04 (0x6798-0x679B)
           db 066h, 002h, 0EEh, 0E0h ; 0x05 (0x679C-0x679F)
           db 066h, 060h, 02Eh, 0E0h ; 0x06 (0x67A0-0x67A3)
           db 066h, 066h, 062h, 0E0h ; 0x07 (0x67A4-0x67A7)
           db 066h, 066h, 066h, 0A0h ; 0x08 (0x67A8-0x67AB)
           db 066h, 066h, 066h, 060h ; 0x09 (0x67AC-0x67AF)

data_67B0: db 004h, 00Dh, 000h, 000h
           db 000h, 006h, 066h, 066h ; 0x00 (0x67B4-0x67B7)
           db 0AEh, 0E0h, 006h, 066h ; 0x01 (0x67B8-0x67BB)
           db 0AAh, 0EAh, 0E0h, 066h ; 0x02 (0x67BC-0x67BF)
           db 0AEh, 0AEh, 02Eh, 006h ; 0x03 (0x67C0-0x67C3)
           db 0AEh, 0E2h, 02Ah, 006h ; 0x04 (0x67C4-0x67C7)
           db 022h, 022h, 02Ah, 006h ; 0x05 (0x67C8-0x67CB)
           db 022h, 022h, 02Ah, 006h ; 0x06 (0x67CC-0x67CF)
           db 022h, 022h, 02Ah, 006h ; 0x07 (0x67D0-0x67D3)
           db 022h, 022h, 02Ah, 006h ; 0x08 (0x67D4-0x67D7)
           db 022h, 022h, 02Ah, 006h ; 0x09 (0x67D8-0x67DB)
           db 022h, 022h, 02Ah, 006h ; 0x0A (0x67DC-0x67DF)
           db 022h, 022h, 020h, 006h ; 0x0B (0x67E0-0x67E3)
           db 022h, 022h, 000h, 096h ; 0x0C (0x67E4-0x67E7)

; ?
data_67E8: db 004h, 00Dh, 000h, 000h
           db 066h, 066h, 060h, 000h ; 0x00 (0x67EC-0x67EF)
           db 066h, 060h, 00Eh, 0EAh ; 0x01 (0x67F0-0x67F3)
           db 066h, 00Eh, 0AEh, 0AAh ; 0x02 (0x67F4-0x67F7)
           db 060h, 0E2h, 0EAh, 0EAh ; 0x03 (0x67F8-0x67FB)
           db 060h, 0A2h, 02Eh, 0EAh ; 0x04 (0x67FC-0x67FF)
           db 060h, 0A2h, 022h, 022h ; 0x05 (0x6800-0x6803)
           db 060h, 0A2h, 022h, 022h ; 0x06 (0x6804-0x6807)
           db 060h, 0A2h, 022h, 02Ah ; 0x07 (0x6808-0x680B)
           db 060h, 0A2h, 022h, 02Ah ; 0x08 (0x680C-0x680F)
           db 060h, 0A2h, 022h, 02Ah ; 0x09 (0x6810-0x6813)
           db 060h, 0A2h, 022h, 02Ah ; 0x0A (0x6814-0x6817)
           db 000h, 002h, 022h, 02Ah ; 0x0B (0x6818-0x681B)
           db 091h, 000h, 002h, 02Ah ; 0x0C (0x681C-0x681F)

; 0xB042 - 0xB151 (0x88 * 2)
; stores offsets to destination pointers.
table_of_80: dw 88h dup(0)


; 0xBC52
; used by 2F2A
; Contains the first 768 bytes of DATA1
data1_header: dw 180h DUP(0)

_TEXT ENDS
END start
