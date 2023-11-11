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

  int 20h ; Terminate to DOS

; 0x21D
init_graphics:
  mov word ptr [unknown_2FA], 0

unknown_2FA dw 0

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

; 0x3860
game_state dw 80h DUP (0)

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


_TEXT ENDS
END start
