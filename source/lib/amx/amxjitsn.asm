; AMXJITSN.ASM: Just-In-Time compiler for the Abstract Machine of the "Pawn"
; scripting language

;Copyright and license of use, please read
;-----------------------------------------
; (C) 1999-2000, Marc Peter; beta version; provided AS IS WITHOUT ANY WARRANTIES.
;
;Pertaining to condition 2 of the license (see below), please note that the
;original file has been substantially altered by G.W.M. Vissers and
;Thiadmer Riemersma. (See also the history of changes).
;
;Permission is hereby granted, without written agreement and without paid
;license or royalty fees, to use, copy, modify, and distribute this software
;and its documentation for any purpose, subject to the following conditions:
;
;1. The above copyright notice and this permission notice shall appear in all
;   copies or substantial portions of this software.
;
;2. Modifications of this software that do not originate from me (Marc Peter)
;   must be explicitly mentioned in a README file or another appropriate
;   place.
;
;The use of this software as a subsystem of a larger software product is
;explicitly allowed, regardless of whether that larger product is proprietary,
;gratis or commercially available.
;
;I (Marc Peter) specifically disclaim any warranties, including, but not
;limited to, the implied warranties of merchantability and fitness for a
;particular purpose. The software is provided on an "as is" basis,
;and I have no obligation to provide maintenance, support, updates,
;enhancements or modifications.
;
;I cannot be held liable for any damage or loss of profits that results
;from the use of the software (or part thereof), or from the inability to
;use it.
;
;
;
; I reached >155 million instr./sec on my AMD K6-2/366 with the Hanoi "bench"
; (27 disks, no output, DOS4/GW under Win95) with this implementation of the
; JIT compiler.

; NOTE 1:
; There is only one pass implemented in this version. This means there is no
; way of knowing the size of the compiled code before it has actually been com-
; piled. So the only chance the caller has, is to count the number of opcodes
; (in amx_BrowseRelocate()) and multiply this count with a "safe" factor to
; obtain a size value big enough to hold the entire code (and data, including
; the stack and heap, after adding their sizes). Afterwards it can realloc()
; this memory block to the actually needed smaller size.

; NOTE 2:
; The compiler destroys the opcode addresses of the given source by storing the
; respective compiled code's addresses there for the final address relocation
; step.

; NOTE 3:
; During execution of the compiled code with amx_exec_jit() the x86 processor's
; stack is switched into the data section of the abstract machine. This means
; that there should always be enough memory left between HEA and STK to provide
; stack space for occurring interrupts! (see the STACKRESERVE variable)

; NOTE 4:
; Although the Pawn compiler doesn't generate the LCTRL and SCTRL instructions,
; I have to tell that they don't work as expected in a JIT compiled program,
; because there is no easy way of transforming AMX code addresses and JIT
; translated ones. This might be fixed in a future version.

; NX ("No eXecute") and XD (eXecution Denied) bits
; ================================================
; (written by Thiadmer Riemersma)
;
; AMD defined a bit "No eXecute" for the page table entries (for its 64-bit
; processors) and Intel came with the same design, but calling it differently.
; The purpose is to make "buffer overrun" security holes impossible (or at least
; very, very difficult), by marking the stack and the heap as memory regions
; such that an attempt to execute processor instructions will cause a processor
; exception (of course, a buffer overrun that is not explictly handled will then
; crash the application --instead of executing the rogue code).
;
; For JIT compilers, this has the impact that you are not allowed to execute the
; code that the JIT has generated. To run the generated code, first you must
; adjust the attributes for the memory page. For Microsoft Windows, you can use
; VirtualAlloc() to allocate a memory block with the appropriate fags; on Linux,
; you would use vmalloc_exec(). Microsoft Windows also offers the function
; VirtualProtect() to change the page attributes of an existing memory block,
; but there are caveats in its use: if the block spans multiple pages, these
; pages must be consecutive, and if there are blocks of memory in a page
; unrelated to the JIT, their page attributes will change too.
;
; The JIT compiler itself requires only read-write access (this is the default
; for a memory block that you allocate). The execution of the JIT-compiled code
; requires full access to the memory block: read, write and execute. It needs
; write access, because the SYSREQ.C opcode is patched to SYSREQ.D after the
; first lookup (this is an optimization, look up the address of the native
; function only once). For processors that do not support the NX/XD bit,
; execution of code is implicitly supported if read access is supported.
;
; During compilation, the JIT compiler requires write-access to its own code
; segment: the JIT-compiler patches P-code parameters into its own code segment
; during compilation. This is handled in the support code for amx_InitJIT.
;
;
; CALLING CONVENTIONS
; ===================
; (written by Thiadmer Riemersma)
;
; This version is the JIT that uses the "stack calling convention". In the
; original implementation, this meant __cdecl; both for the calling convention
; for the _amx_jit_compile routine itself as for the callback functions.
; The current release supports __stdcall for the callback functions; to
; use it, you need to assemble the file with STDECL defined (Since STDCALL is
; a reserved word on the assembler, I had to choose a different name for the
; macro, hence STDECL.)

; Revision History
; ----------------
;  1 March 2010  by Thiadmer Riemersma
;       The instruction set has been reorganized: a minimal "core" instruction
;       set was picked and three supplemental instruction sets: overlay
;       instructions, macro instructions and packed instructions.
; 14 March 2009  by Thiadmer Riemersma
;       Addition of the relocation instructions (PUSHR_xxx).
; 26 august 2007  by Thiadmer Riemersma
;       Minor clean-up; removed unneeded parameter.
; 28 july 2005
;       Bug fix for the switch table, in the situation where only the default
;       case was present. Bug found by Bailopan.
; 17 february 2005  by Thiadmer Riemersma (TR)
;       Addition of the BREAK opcode, removal of the older debugging opcode
;       table. There should now be some debug support (if enabled during the
;       build of the JIT compiler), but not enough to run a debugger: the JIT
;       compiler does not keep a list that relates the code addresses of the
;       P-code versus the native code.
; 29 June 2004  by G.W.M. Vissers
;	Translated the thing into NASM. The actual generation of the code is
;	put into the data section because the code modifies itself whereas the
;	text section is usually read-only in the Unix ELF format.
;  6 march 2004  by Thiadmer Riemersma
;       Corrected a bug in OP_FILL, where a cell preceding the array would
;       be overwritten (zero'ed out). This bug was brought to my attention
;       by Robert Daniels.
; 22 december 2003  by Thiadmer Riemersma (TR)
;       Added the SYMTAG and SYSCALL.D opcodes (these are not really supported;
;       SYMTAG is a no-op).
;       Support __stdcall calling convention for for the native function "hook"
;       function (the __cdecl calling convention is also still supported).
; 14 October 2002 by Thiadmer Riemersma (TR)
;       Corrected the amx_s structure. The _hlw field was missing, which caused
;       errors for arguments to native functions that were passed by reference.
; 2002/08/05    TR
;   * store the status of the abstract machine in the AMX structure upon
;     return, so that the machine can be restarted (OP_SLEEP)
;   * added OP_NOP (for alignment, it is ignored by the JIT)
;   * make sure the JIT does not crash when we NULL is passed for the
;     return value
; 2000/03/03    MP
;       * _amx_opcodelist is equipped with an underscore, again 8-P
;       * added SRANGE as a no-op, so debugging info doesn't upset the JIT
;         compiler anymore
;       * added note about LCTRL, SCTRL and CALL.I
; 2000/03/02    MP
;       * made JIT support __cdecl calling conventions
;       * removed some rather unnecessary pops in the epilog of amx_exec_asm
;       * changed the template for CALL into a DB byte sequence (tasm 4.1
;         didn't like the immediate value)
; 1999/12/07    MP
;       * fixed crash caused by JIT compiler not saving registers
; 1999/08/06    MP - design change: closer to the "iron" with native stack
;       * The JIT compiler now generates relocatable code for case tables by
;         setting FORCERELOCATABLE = 1.
;       * removed all debug hook code
;       * exchanged meaning of ESP and ESI in asm_exec(): now low-level calls/
;         pushes/pops are possible
;       * removed the run-time functions for the CALL, CALL_PRI and RET op-codes,
;         they are now inline
;       * All these changes gained around 80% performance increase for the
;         hanoi bench.
; 1999/08/05    MP
;       * fixed OP_LINE in the case of NODBGCALLS==1, where no compiled address
;         was stored for the LINE byte code (i.e. SWITCH would jump to totally
;         wrong addresses). The same fix was applied to OP_FILL, OP_FILE and
;         OP_SCTRL (for the no-op case).
; 1999/08/04    MP
;       * updated with 4 new opcodes (SRANGE does nothing at the moment; 2dim.
;         arrays have not been tested.)
;       * hacked relocation code to support absoulute addresses for CASETBL
;         (This assumes that no generated address will be greater than
;         0x7fffffff. Bit no. 31 is used as flag for absolute addresses.)
;       * The run-time function for SWITCH uses a (hopefully) faster algorithm
;         to compute the destination address: It searches backwards now.
; 1999/07/08    MP - initial revision


;
; Support for the BREAK opcode (callback to the debugger): 0 = no, all other
; values = yes. Beware that the compiled code runs slower when this is enabled,
; and that debug support is still fairly minimal.
;
; GWMV: to generate BREAK opcodes, %define DEBUGSUPPORT
;
%undef DEBUGSUPPORT

;
; If this is set to 1 the JIT generates relocatable code for case tables, too.
; If set to 0, a faster variant for switch (using absolute addresses) is
; generated. I consider setting it to 0 a bad idea.
;
; GWMV: to use absolute addresses, %undef FORCERELOCATABLE
;
%define FORCERELOCATABLE

;
; Determines how much memory should be reserved for occurring interrupts.
; (If my memory serves me right, DOS4/G(W) provides a stack of 512 bytes
; for interrupts that occur in real mode and are promoted to protected mode.)
; This value _MUST_ be greater than 64 (for AMX needs) and should be at least
; 128 (to serve interrupts).
;
%define STACKRESERVE 256

;
; This variable controls the generation of memory range checks at run-time.
; You should set this to 0, only when you are sure that there are no range
; violations in your Pawn programs and you really need those 5% speed gain.
;
; GWMV: To disable runtime checks, %undef it, instread of setting it to zero
;
%define DORUNTIMECHECKS

%define JIT     1
%include "amxdefn.asm"

; GWMV:
; Nasm can't do the next as equivalence statements, since the value of
; esi is not determined at compile time
%define stk     [esi+32]    ; define some aliases to registers that will
%define alt     [esi+28]    ;   be stored on the stack when the code is
%define pri     [esi+24]    ;   actually beeing executed
%define code    [esi+20]
%define amx     [esi+16]
%define retval  [esi+12]
%define stp     [esi+8]
%define hea     [esi+4]
%define frm     [esi]       ; FRM is NOT stored in ebp, FRM+DAT is being held
                            ; in ebx instead.

;
; #define  PUSH(v)  ( stk-=sizeof(cell), *(cell *)(data+(int)stk)=v )
;
%macro _PUSH 1
	push	dword %1
%endmacro

;
; #define  POP(v)   ( v=*(cell *)(data+(int)stk), stk+=sizeof(cell) )
;
%macro _POP 1
	pop	dword %1
%endmacro


;
; For determining the biggest native code section generated for ONE Pawn
; opcode. (See the following macro and the macro CHECKCODESIZE below.)
;
%assign MAXCODESIZE      0

;
; This is the work horse of the whole JIT: It actually copies the code.
%macro GO_ON 2-3 4
        mov     esi, %1             ;get source address of JIT code
        mov     ecx,%2-%1           ;get number of bytes to copy
        mov     [ebx],edi           ;store address for jump-correction
        add     ebx,%3
        rep     movsb
        cmp     ebx,[end_code]
        jae     code_gen_done
        jmp     dword [ebx]         ;go on with the next opcode
%endmacro

; GWMV:
; Nasm can't handle the determination of the maximum code size as was done
; in the MASM/WASM implementation, since it only does two passes. This macro
; is called *after* the code for each Pawn instruction.
%macro CHECKCODESIZE 1
	%if MAXCODESIZE < $-%1
        %assign MAXCODESIZE $-%1
    %endif
%endmacro

;
; Modify the argument of an x86 instruction with the Pawn opcode's parameter
; before copying the code.
;
%macro putval 1
        mov     eax,[ebx+4]
        mov     dword [%1],eax
%endmacro

;
; Add an entry to the table of addresses which have to be relocated after the
; code compilation is done.
;
%macro RELOC 1
        mov     ebp,[reloc_num]
        mov     eax,[ebx+4]
        mov     [edx+ebp],eax           ; write jump target in P-code (absolute virtual address)
        lea     eax,[edi+%1]
        mov     [edx+ebp+4],eax         ; write address of jump operand (in JIT code)
        add     dword [reloc_num],8
%endmacro

%macro _DROPARGS 1              ; (TR) remove function arguments from the stack
    %ifndef STDECL               ; (for __cdecl calling convention only)
        add     esp,%1
    %endif
%endmacro


section .text


;----------------------------------------------------------------------------
; void amx_jit_compile( AMX *amx, JumpAddressArray *jumps, void *dest )
;----------------------------------------------------------------------------

; amx_jit_compile() assumes that the code of this module is allready browsed and
; relocated for the JIT compiler. It also assumes that both the jumps array and
; the dest memory block are large enough to hold all the data it has to write
; to them, as well as that the prefix (header) has already been copied to dest.

global  amx_jit_compile, _amx_jit_compile
amx_jit_compile:
_amx_jit_compile:
        push    ebp
        push    ebx
        push    edi
        push    esi

        mov     eax,[esp+20]            ; get amxh
        mov     edx,[esp+24]            ; get jumps array
        mov     ebx,[esp+28]            ; get destination

        mov     [amxhead],eax           ; save pointer to AMX_HEADER struct
        mov     ecx,[eax+_cod]          ; get offset of start of code
        mov     eax,[eax+_dat]          ; offset of start of data = end of code
        mov     edi,ecx
        add     ecx,[amxhead]           ; compute the real pointer
        add     eax,[amxhead]           ; dito
        add     edi,ebx                 ; get write pointer into EDI
        mov     [compiled_code],ebx
        mov     [end_code],eax          ; Store end-of-code address, so JIT
                                        ; compiler knows when to stop.
        mov     dword [reloc_num],0 ; init the index into the jumps array

        mov     ebx,ecx
        jmp     dword [ecx]         ; start compiling

        ; The compiler will jump back here when code generation is complete.

code_gen_done:                          ; Now copy the data section.
        mov     ebp,[amxhead]           ; get source AMX_HEADER start address
        add     edi,3                   ; DAT follows directly after COD
        and     edi,0fffffffch          ; align it on a DWORD boundary
        push    edi                     ; save data start pointer
        mov     esi,[end_code]          ; get start of data segment
        mov     ecx,[ebp+_h_hea]
        sub     ecx,[ebp+_dat]          ; compute length of array to copy
        rep movsb                       ; copy the data

        ; Now adjust the register values in the compiled AMX_HEADER.
        ; COD stays the same since the size of AMX_HEADER doesn't change in
        ; compiled mode.
        mov     ebx,[compiled_code]     ; get compiled AMX's header address
        pop     esi                     ; recall data start pointer
        sub     esi,ebx                 ; DAT = size of code + size of prefix
        mov     [ebx+_dat],esi          ; write corrected DAT register

        ;HEA and STP are already relative to DAT, so we don't need to fix them.

        ; Now the calls/jumps in the compiled code have to be relocated.
        sub     ecx,ecx         ; reset offset into relocation table
        cmp     ecx,[reloc_num]
        jae     reloc_code_done ; if there's nothing to fix, skip this part
    reloc_code_loop:
        mov     eax,[edx+ecx]   ; get jump target (patched in P-code)
        mov     edi,[edx+ecx+4] ; determine where to write the relocated value
        add     ecx,8           ; set pointer to next entry in relocation table
        add     edi,4           ; base address from where the offset is taken
%ifndef FORCERELOCATABLE
        ;MP: hack to suport absolute addresses for the CASETBL instruction
        test    eax,80000000h   ; check whether it is an absolute address
        pushf
        and     eax,7fffffffh   ; clear the flag bit for absolute addresses
        popf
        mov     eax,[eax]       ; translate into compiled absolute address
        jne     write_reloc     ; leave out the subtraction if absolute
%else
        mov     eax,[eax]       ; translate into compiled absolute address
%endif
        sub     eax,edi         ; make a relative offset
      write_reloc:
        mov     [edi-4],eax     ; write the relocated address
        cmp     ecx,[reloc_num]
        jb      reloc_code_loop

reloc_code_done:
        ; Relocate the addresses in the AMX_HEADER structure. (CIP and publics)
        add     ebp,[ebp+_cod]  ; make all addresses relative to COD, not base
        mov     eax,[ebx+_h_cip]
        add     eax,ebp         ; get absolute source CIP
        mov     eax,[eax]       ; translate CIP to compiled address
        sub     eax,ebx         ; make it relative to base
        sub     eax,[ebx+_cod]  ; and now relative to COD
        mov     [ebx+_h_cip],eax; store relocated CIP
        mov     edi,[ebx+_publics]
        sub     esi,esi
        mov     ecx,[ebx+_natives]
        sub     ecx,edi         ; ECX = _natives - _publics = public table size
        mov     si,[ebx+_defsize]
        or      ecx,ecx
        jz      reloc_done      ; If there are no publics, we are done.
    reloc_publics_loop:
        mov     eax,[ebx+edi]   ; get public function offset
        add     eax,ebp         ; make it a source address
        mov     eax,[eax]       ; translate to compiled address
        sub     eax,ebx         ; make it an offset relative to base
        sub     eax,[ebx+_cod]  ; and now relative to COD
        mov     [ebx+edi],eax   ; write corrected address back
        add     edi,esi         ; step to next public function entry
        sub     ecx,esi
        ja      reloc_publics_loop

reloc_done:
        mov     eax,0
        pop     esi
        pop     edi
        pop     ebx
        pop     ebp
        ret

OP_LOAD_PRI:
        putval  j_load_pri+2
        GO_ON   j_load_pri, OP_LOAD_ALT, 8
    j_load_pri:
        mov     eax,[edi+12345678h]
	CHECKCODESIZE j_load_pri

OP_LOAD_ALT:
        putval  j_load_alt+2
        GO_ON   j_load_alt, OP_LOAD_S_PRI, 8
    j_load_alt:
        mov     edx,[edi+12345678h]
	CHECKCODESIZE j_load_alt

OP_LOAD_S_PRI:
        putval  j_load_s_pri+2
        GO_ON   j_load_s_pri, OP_LOAD_S_ALT, 8
    j_load_s_pri:
        mov     eax,[ebx+12345678h]
	CHECKCODESIZE j_load_s_pri

OP_LOAD_S_ALT:
        putval  j_load_s_alt+2
        GO_ON   j_load_s_alt, OP_LOAD_I, 8
    j_load_s_alt:
        mov     edx,[ebx+12345678h]
	CHECKCODESIZE j_load_s_alt

OP_LOAD_I:
        GO_ON   j_load_i, OP_LODB_I
    j_load_i:
%ifdef DORUNTIMECHECKS
        call    [verify_adr_eax]
%endif
        mov     eax,[edi+eax]
	CHECKCODESIZE j_load_i

OP_LODB_I:
        mov     eax,[ebx+4]
        mov     eax,dword [(lodb_and-4)+eax*4]
        mov     dword [j_lodb_i_sm+1],eax   ;modify AND instruction
        GO_ON   j_lodb_i, OP_LREF_S_PRI, 8
    j_lodb_i:
%ifdef DORUNTIMECHECKS
        call    [verify_adr_eax]
%endif
        mov     eax,[edi+eax]           ;subject to misalignment stalls
    j_lodb_i_sm:
        and     eax,12345678h
	CHECKCODESIZE j_lodb_i

OP_LREF_S_PRI:
        putval  j_lref_s_pri+2
        GO_ON   j_lref_s_pri, OP_LREF_S_ALT, 8
    j_lref_s_pri:
        mov     eax,[ebx+12345678h]
        mov     eax,[edi+eax]
	CHECKCODESIZE j_lref_s_pri

OP_LREF_S_ALT:
        putval  j_lref_s_alt+2
        GO_ON   j_lref_s_alt, OP_CONST_PRI, 8
    j_lref_s_alt:
        mov     edx,[ebx+12345678h]
        mov     edx,[edi+edx]
	CHECKCODESIZE j_lref_s_alt

OP_CONST_PRI:
        putval  j_const_pri+1
        GO_ON   j_const_pri, OP_CONST_ALT, 8
    j_const_pri:
        mov     eax,12345678h
	CHECKCODESIZE j_const_pri

OP_CONST_ALT:
        putval  j_const_alt+1
        GO_ON   j_const_alt, OP_ADDR_PRI, 8
    j_const_alt:
        mov     edx,12345678h
	CHECKCODESIZE j_const_alt

OP_ADDR_PRI:
        putval  j_addr_pri+1
        GO_ON   j_addr_pri, OP_ADDR_ALT, 8
    j_addr_pri:
        mov     eax,12345678h
        add     eax,frm
	CHECKCODESIZE j_addr_pri

OP_ADDR_ALT:
        putval  j_addr_alt+1
        GO_ON   j_addr_alt, OP_STOR, 8
    j_addr_alt:
        mov     edx,12345678h
        add     edx,frm
	CHECKCODESIZE j_addr_alt

OP_STOR:
        putval  j_stor_pri+2
        GO_ON   j_stor_pri, OP_STOR_S, 8
    j_stor_pri:
        mov     [edi+12345678h],eax
	CHECKCODESIZE j_stor_pri

OP_STOR_S:
        putval  j_stor_s_pri+2
        GO_ON   j_stor_s_pri, OP_STOR_I, 8
    j_stor_s_pri:
        mov     [ebx+12345678h],eax
	CHECKCODESIZE j_stor_s_pri

OP_STOR_I:
        GO_ON   j_stor_i, OP_STRB_I
    j_stor_i:
%ifdef DORUNTIMECHECKS
        call    [verify_adr_edx]
%endif
        mov     [edi+edx],eax
	CHECKCODESIZE j_stor_i

OP_STRB_I:
        mov     eax,[ebx+4]
        cmp     eax,1
        jne     strb_not1byte
        GO_ON   j_strb_i_1b, strb_not1byte, 8
    j_strb_i_1b:
%ifdef DORUNTIMECHECKS
        call    [verify_adr_edx]
%endif
        mov     [edi+edx],al
        CHECKCODESIZE j_strb_i_1b

    strb_not1byte:
        cmp     eax,4
        je      strb_4byte
        GO_ON   j_strb_i_2b, strb_4byte, 8
    j_strb_i_2b:
%ifdef DORUNTIMECHECKS
        call    [verify_adr_edx]
%endif
        mov     [edi+edx],ax
        CHECKCODESIZE j_strb_i_2b

    strb_4byte:
        GO_ON   j_strb_i_4b, OP_SREF_S, 8
        j_strb_i_4b:
%ifdef DORUNTIMECHECKS
        call    [verify_adr_edx]
%endif
        mov     [edi+edx],eax
	CHECKCODESIZE j_strb_i_4b

OP_SREF_S:
        putval  j_sref_s_pri+2
        GO_ON   j_sref_s_pri, OP_ALIGN_PRI, 8
    j_sref_s_pri:
        mov     ebp,[ebx+12345678h]
        mov     [edi+ebp],eax
	CHECKCODESIZE j_sref_s_pri

OP_ALIGN_PRI:
        mov     eax,4
        sub     eax,[ebx+4]
        mov     dword [j_align_pri+1],eax
        GO_ON   j_align_pri, OP_LCTRL, 8
    j_align_pri:
        xor     eax,12345678h
	CHECKCODESIZE j_align_pri

OP_LCTRL:
        mov     eax,[ebx+4]
        cmp     eax,0
        jne     lctrl_1
        GO_ON   j_lctrl_0, lctrl_1, 8
    j_lctrl_0:
        mov     eax,code ; 1=COD
    	CHECKCODESIZE j_lctrl_0
    lctrl_1:
        cmp     eax,1
        jne     lctrl_2
        GO_ON   j_lctrl_1, lctrl_2, 8
    j_lctrl_1:
        mov     eax,edi  ; 1=DAT
    	CHECKCODESIZE j_lctrl_1
    lctrl_2:
        cmp     eax,2
        jne     lctrl_3
        GO_ON   j_lctrl_2, lctrl_3, 8
    j_lctrl_2:
        mov     eax,hea  ; 2=HEA
    	CHECKCODESIZE j_lctrl_2
    lctrl_3:
        cmp     eax,3
        jne     lctrl_4
        GO_ON   j_lctrl_3, lctrl_4, 8
    j_lctrl_3:
        mov     ebp,amx
        mov     eax,[ebp+_stp]
    	CHECKCODESIZE j_lctrl_3
    lctrl_4:
        cmp     eax,4
        jne     lctrl_5
        GO_ON   j_lctrl_4, lctrl_5, 8
    j_lctrl_4:
        mov     eax,esp         ; 4=STK
        sub     eax,edi
    	CHECKCODESIZE j_lctrl_4
    lctrl_5:
        cmp     eax,5
        jne     lctrl_6
        GO_ON   j_lctrl_5, lctrl_6, 8
    j_lctrl_5:
        mov     eax,frm         ; 5=FRM
    	CHECKCODESIZE j_lctrl_5
    lctrl_6:
        mov     dword [j_lctrl_6+1],edi
        GO_ON   j_lctrl_6, OP_SCTRL, 8
    j_lctrl_6:
        mov     eax,12345678h   ; 6=CIP
	CHECKCODESIZE j_lctrl_6


OP_SCTRL:
        mov     eax,[ebx+4]
        cmp     eax,2
        jne     sctrl_4
        GO_ON   j_sctrl_2, sctrl_4, 8
    j_sctrl_2:
        mov     hea,eax  ; 2=HEA
    	CHECKCODESIZE j_sctrl_2
    sctrl_4:
        cmp     eax,4
        jne     sctrl_5
        GO_ON   j_sctrl_4, sctrl_5, 8
    j_sctrl_4:
        ;mov     esp,eax  ; 4=STK
        ;add    esp,edi  ; relocate stack
        lea     esp,[eax+edi]
    	CHECKCODESIZE j_sctrl_4
    sctrl_5:
        cmp     eax,5
        jne     sctrl_ignore
        GO_ON   j_sctrl_5, sctrl_ignore, 8
    j_sctrl_5:
        mov     ebx,eax  ; 5=FRM
        mov     frm,eax
        add     ebx,edi  ; relocate frame
    	CHECKCODESIZE j_sctrl_5
    sctrl_ignore:
        mov     [ebx],edi
        add     ebx,8
        jmp     dword [ebx]

OP_XCHG:
        GO_ON   j_xchg, OP_PUSH_PRI
    j_xchg:                     ;one might use pushes/pops for pre-586's
        xchg    eax,edx
	CHECKCODESIZE j_xchg

OP_PUSH_PRI:
        GO_ON   j_push_pri, OP_PUSH_ALT
    j_push_pri:
        _PUSH   eax
	CHECKCODESIZE j_push_pri

OP_PUSH_ALT:
        GO_ON   j_push_alt, OP_PICK
    j_push_alt:
        _PUSH   edx
	CHECKCODESIZE j_push_alt

OP_PICK:
        putval  j_pick+2
        GO_ON   j_pick, OP_POP_PRI, 8
    j_pick:
        mov     eax,[esp+12345678h]
	CHECKCODESIZE j_pick

OP_POP_PRI:
        GO_ON   j_pop_pri, OP_POP_ALT
    j_pop_pri:
        _POP    eax
	CHECKCODESIZE j_pop_pri

OP_POP_ALT:
        GO_ON   j_pop_alt, OP_STACK
    j_pop_alt:
        _POP    edx
	CHECKCODESIZE j_pop_alt

OP_STACK:
        putval  j_stack+4
        GO_ON   j_stack, OP_HEAP, 8
    j_stack:
        mov     edx,esp
        add     esp,12345678h
        sub     edx,edi
%ifdef DORUNTIMECHECKS
        call    [chk_marginstack]
%endif
	CHECKCODESIZE j_stack

OP_HEAP:
        putval  j_heap_call-4
        GO_ON   j_heap, OP_PROC, 8
    j_heap:
        mov     edx,hea
        add     dword hea,12345678h
        j_heap_call:
%ifdef DORUNTIMECHECKS
        call    [chk_marginheap]
%endif
	CHECKCODESIZE j_heap

OP_PROC:
        GO_ON   j_proc, OP_RET
    j_proc:                     ;[STK] = FRM, STK = STK - cell size, FRM = STK
        _PUSH   frm             ; push old frame (for RET/RETN)
        mov     frm,esp         ; get new frame
        mov     ebx,esp         ; already relocated
        sub     frm,edi         ; relocate frame
	CHECKCODESIZE j_proc

OP_RET:
        GO_ON   j_ret, OP_RETN
    j_ret:
        _POP    ebx             ; pop frame
        mov     frm,ebx
        add     ebx,edi
        ret
        ;call   [jit_ret]
	CHECKCODESIZE j_ret

OP_RETN:
        GO_ON   j_retn, OP_CALL
    j_retn:
        jmp     [jit_retn]
	CHECKCODESIZE j_retn

OP_CALL:
        RELOC   1
        GO_ON   j_call, OP_JUMP, 8
    j_call:
        ;call   12345678h ; tasm chokes on this out of a sudden
        db      0e8h, 0, 0, 0, 0
	CHECKCODESIZE j_call

OP_JUMP:
        RELOC   1
        GO_ON   j_jump, OP_JZER, 8
    j_jump:
        db      0e9h, 0, 0, 0, 0
	CHECKCODESIZE j_jump

OP_JZER:
        RELOC   4
        GO_ON   j_jzer, OP_JNZ, 8
    j_jzer:
        or      eax,eax
        DB      0fh, 84h, 0, 0, 0, 0    ;jz NEAR 0      (tasm sucks a bit)
	CHECKCODESIZE j_jzer

OP_JNZ:
        RELOC   4
        GO_ON   j_jnz, OP_SHL, 8
    j_jnz:
        or      eax,eax
        DB      0fh, 85h, 0, 0, 0, 0    ;jnz NEAR 0
	CHECKCODESIZE j_jnz

OP_SHL:
        GO_ON   j_shl, OP_SHR
    j_shl:
        mov     ecx,edx         ; TODO: save ECX if used as special register
        shl     eax,cl
	CHECKCODESIZE j_shl

OP_SHR:
        GO_ON   j_shr, OP_SSHR
    j_shr:
        mov     ecx,edx         ; TODO: save ECX if used as special register
        shr     eax,cl
	CHECKCODESIZE j_shr

OP_SSHR:
        GO_ON   j_sshr, OP_SHL_C_PRI
    j_sshr:
        mov     ecx,edx         ; TODO: save ECX if used as special register
        sar     eax,cl
	CHECKCODESIZE j_sshr

OP_SHL_C_PRI:
        mov     al,[ebx+4]
        mov     byte [j_shl_c_pri+2],al
        GO_ON   j_shl_c_pri, OP_SHL_C_ALT, 8
    j_shl_c_pri:
        shl     eax,12h
	CHECKCODESIZE j_shl_c_pri

OP_SHL_C_ALT:
        mov     al,[ebx+4]
        mov     byte [j_shl_c_alt+2],al
        GO_ON   j_shl_c_alt, OP_SMUL, 8
    j_shl_c_alt:
        shl     edx,12h
	CHECKCODESIZE j_shl_c_alt

OP_SMUL:
        GO_ON   j_smul, OP_SDIV
    j_smul:
        push    edx
        imul    edx
        pop     edx
	CHECKCODESIZE j_smul

OP_SDIV:
        GO_ON   j_sdiv, OP_ADD
    j_sdiv:
        xchg    eax,edx
        call    [jit_sdiv]
	CHECKCODESIZE j_sdiv

OP_ADD:
        GO_ON   j_add, OP_SUB
    j_add:
        add     eax,edx
	CHECKCODESIZE j_add

OP_SUB:
        GO_ON   j_sub, OP_AND
    j_sub:
        neg     eax
        add     eax,edx
	CHECKCODESIZE j_sub

OP_AND:
        GO_ON   j_and, OP_OR
    j_and:
        and     eax,edx
	CHECKCODESIZE j_and

OP_OR:
        GO_ON   j_or, OP_XOR
    j_or:
        or      eax,edx
	CHECKCODESIZE j_or

OP_XOR:
        GO_ON   j_xor, OP_NOT
    j_xor:
        xor     eax,edx
	CHECKCODESIZE j_xor

OP_NOT:
        GO_ON   j_not, OP_NEG
    j_not:
        neg     eax             ; sets CF iff EAX != 0
        sbb     eax,eax         ; EAX == -1 iff CF set (zero otherwise)
        inc     eax             ; -1 => 0 and 0 => 1
	CHECKCODESIZE j_not

OP_NEG:
        GO_ON   j_neg, OP_INVERT
    j_neg:
        neg     eax
	CHECKCODESIZE j_neg

OP_INVERT:
        GO_ON   j_invert, OP_EQ
    j_invert:
        not     eax
	CHECKCODESIZE j_invert

OP_EQ:
        GO_ON   j_eq, OP_NEQ
    j_eq:
        cmp     eax,edx         ; PRI == ALT ?
        mov     eax,0
        sete    al
	CHECKCODESIZE j_eq

OP_NEQ:
        GO_ON   j_neq, OP_SLESS
    j_neq:
        cmp     eax,edx         ; PRI != ALT ?
        mov     eax,0
        setne   al

OP_SLESS:
        GO_ON   j_sless, OP_SLEQ
    j_sless:
        cmp     eax,edx         ; PRI < ALT ? (signed)
        mov     eax,0
        setl    al
	CHECKCODESIZE j_sless

OP_SLEQ:
        GO_ON   j_sleq, OP_SGRTR
    j_sleq:
        cmp     eax,edx         ; PRI <= ALT ? (signed)
        mov     eax,0
        setle   al
	CHECKCODESIZE j_sleq

OP_SGRTR:
        GO_ON   j_sgrtr, OP_SGEQ
    j_sgrtr:
        cmp     eax,edx         ; PRI > ALT ? (signed)
        mov     eax,0
        setg    al
	CHECKCODESIZE j_sgrtr

OP_SGEQ:
        GO_ON   j_sgeq, OP_INC_PRI
    j_sgeq:
        cmp     eax,edx         ; PRI >= ALT ? (signed)
        mov     eax,0
        setge   al
	CHECKCODESIZE j_sgeq

OP_INC_PRI:
        GO_ON   j_inc_pri, OP_INC_ALT
    j_inc_pri:
        inc     eax
	CHECKCODESIZE j_inc_pri

OP_INC_ALT:
        GO_ON   j_inc_alt, OP_INC_I
    j_inc_alt:
        inc     edx
	CHECKCODESIZE j_inc_alt

OP_INC_I:
        GO_ON   j_inc_i, OP_DEC_PRI
    j_inc_i:
        inc     dword [edi+eax]
	CHECKCODESIZE j_inc_i

OP_DEC_PRI:
        GO_ON   j_dec_pri, OP_DEC_ALT
    j_dec_pri:
        dec     eax
	CHECKCODESIZE j_dec_pri

OP_DEC_ALT:
        GO_ON   j_dec_alt, OP_DEC_I
    j_dec_alt:
        dec     edx
	CHECKCODESIZE j_dec_alt

OP_DEC_I:
        GO_ON   j_dec_i, OP_MOVS
    j_dec_i:
        dec     dword [edi+eax]
	CHECKCODESIZE j_dec_i

OP_MOVS:
        putval  j_movs+1
        GO_ON   j_movs, OP_CMPS, 8
    j_movs:
        mov     ecx,12345678h   ;TODO: save ECX if used as special register
        call    [jit_movs]
	CHECKCODESIZE j_movs

OP_CMPS:
        putval  j_cmps+1
        GO_ON   j_cmps, OP_FILL, 8
    j_cmps:
        mov     ecx,12345678h   ;TODO: save ECX if used as special register
        call    [jit_cmps]
	CHECKCODESIZE j_cmps

OP_FILL:
        putval  j_fill+1
        GO_ON   j_fill, OP_HALT, 8
    j_fill:
        mov     ecx,12345678h   ;TODO: save ECX if used as special register
        call    [jit_fill]
	CHECKCODESIZE j_fill

OP_HALT:
        putval  j_halt_sm+1
        GO_ON   j_halt, OP_BOUNDS, 8
    j_halt:
        cmp     dword retval,0
        je      j_halt_no_value
        mov     ebp,retval
        mov     [ebp],eax
    j_halt_no_value:
    j_halt_sm:
        mov     eax,12345678h
        jmp     [jit_return]
	CHECKCODESIZE j_halt

OP_BOUNDS:
        putval  j_bounds+1
        GO_ON   j_bounds, OP_SYSREQ, 8
    j_bounds:
        mov     ebp,12345678h
        call    [jit_bounds]
	CHECKCODESIZE j_bounds

OP_SYSREQ:
        putval  j_sysreq+1
        GO_ON   j_sysreq, OP_SWITCH, 8
    j_sysreq:
        mov     eax,12345678h   ; get function number
        call    [jit_sysreq]
	CHECKCODESIZE j_sysreq

OP_SWITCH:
        lea     eax,[edi+6]     ; The case table will be copied directly
        neg     eax             ; after the run-time call to [jit_switch].
        and     eax,3           ; We should align this table on a DWORD
        mov     ecx,eax         ; boundary.
        mov     al,90h          ; 90h = opcode of x86 NOP instruction
        rep  stosb              ; Write the right number of NOPs.
        mov     [ebx],edi       ; store address of SWITCH for relocation step
        mov     esi, j_switch
        mov     ecx,6
        rep  movsb              ; copy the call instruction
        mov     esi,[ebx+4]     ; get address of CASETBL instruction
        add     ebx,8           ; set instruction pointer to next opcode
        add     esi,4           ; point esi to first entry: (count, default adr)
        mov     ecx,[esi]       ; get number of cases (excluding default)
        inc     ecx
        mov     ebp,[reloc_num]
    j_case_loop:
        mov     eax,[esi]       ; get case value
        stosd                   ; write it
        mov     eax,[esi+4]     ; get destination address
%ifndef FORCERELOCATABLE
        or      eax,80000000h   ; add flag for "absolute address"
%endif
        mov     [edx+ebp],eax   ; write dest. adr. into relocation table
        mov     eax,[esi+4]     ; get destination address (again)
        add     esi,8           ; set ESI to next case
        mov     [edx+ebp+4],edi ; write adr. to patch into relocation table
        add     ebp,8           ; promote relocation pointer
        stosd                   ; write dest. adr.
        dec     ecx
        jnz     j_case_loop
        mov     dword [reloc_num],ebp       ; write back updated reloc_num

        jmp     [ebx]           ; GO_ON to next op-code

    j_switch:
        call    [jit_switch]

OP_CASETBL:                     ; compiles to nothing, SWITCH does all the work
        mov     eax,[ebx+4]     ; get count of cases
        lea     ebx,[ebx+8*eax+(8+4)]   ; adjust instruction pointer
        jmp     [ebx]           ; GO_ON with next op-code


OP_SWAP_PRI:                    ; TR
        GO_ON   j_swap_pri, OP_SWAP_ALT
    j_swap_pri:
        _POP    ebp
        _PUSH   eax
        mov     eax,ebp
	CHECKCODESIZE j_swap_pri


OP_SWAP_ALT:                    ; TR
        GO_ON   j_swap_alt, OP_PUSHR_PRI
    j_swap_alt:
        _POP    ebp
        _PUSH   edx
        mov     edx,ebp
	CHECKCODESIZE j_swap_alt


OP_PUSHR_PRI:                   ; TR
        GO_ON   j_pushr_pri, OP_NOP
    j_pushr_pri:
        mov     ebp,eax
        add     ebp,edi
        _PUSH   ebp

OP_NOP:                         ; TR
        GO_ON   j_nop, OP_BREAK
    j_nop:                      ; code alignment is ignored by the JIT
	CHECKCODESIZE j_nop


OP_BREAK:
%ifndef DEBUGSUPPORT
        mov     [ebx],edi               ; no line number support: ignore opcode
        add     ebx,4                   ; move on to next opcode
        cmp     ebx,[end_code]
        jae     code_gen_done
        jmp     DWORD [ebx]             ; go on with the next opcode
%else
        GO_ON   j_break, OP_INVALID
    j_break:
        mov     ebp,amx
        cmp     DWORD [ebp+_debug], 0
        je      $+4                     ; jump around the "call" statement
        call    [jit_break]
    CHECKCODESIZE j_break
%endif

OP_INVALID:                     ; break from the compiler with an error code
        mov     eax,AMX_ERR_INVINSTR
        pop     esi
        pop     edi
        pop     ecx
        pop     ebp
        ret


section .text

;----------------------------------------------------------------------------
;cell amx_jit_run( AMX *amx, cell *retval, char *data )
;----------------------------------------------------------------------------

global  amx_jit_run, _amx_jit_run
amx_jit_run:
_amx_jit_run:
        push    edi
        push    esi
        push    ebp
        push    ebx             ; due to __cdecl

        ; __cdecl overhead
        mov     eax, [esp+20]   ; get address of AMX structure
        mov     edx, [esp+24]   ; get address of retval
        mov     ebx, [esp+28]   ; get pointer to data section

        sub     esp,4*3         ; place for PRI, ALT & STK at SYSREQs

        push    DWORD [eax+_codeseg]    ; store pointer to code segment
        push    eax                     ; store pointer to AMX
        push    edx                     ; store address of retval
        push    DWORD [eax+_stp]        ; store STP
        push    DWORD [eax+_hea]        ; store HEA
        push    DWORD [eax+_frm]        ; store FRM

        mov     edi,ebx         ; get pointer to data segment
        mov     edx,[eax+_alt]  ; get ALT
        mov     ecx,[eax+_cip]  ; get CIP (N.B. different from ASM interpreter)
        mov     esi,[eax+_stk]  ; get STK (N.B. different from ASM interpreter)
        mov     ebx,[eax+_frm]  ; get FRM
        mov     eax,[eax+_pri]  ; get PRI
        add     ebx,edi         ; relocate frame

        add     esi,edi         ; ESP will contain DAT+STK
        xchg    esp,esi         ; switch to AMX stack

        add     stp,edi         ; make STP absolute address for run-time checks

        _POP    ebp             ; AMX pseudo-return address, ignored
        ; Call compiled code via CALL NEAR <address>
        add     ecx,code
        call    ecx

return_to_caller:
        cmp     dword retval,0
        je      return_to_caller_no_value
        mov     ebp,retval
        mov     [ebp],eax       ; provide return value

    return_to_caller_no_value:
        mov     eax,AMX_ERR_NONE
        jmp     _return

_return_popstack:
        add     esp,4           ; Correct ESP, because we just come from a
                                ; runtime error checking routine.
_return:
        ; store machine state
        mov     ecx,esp         ; get STK into ECX
        mov     ebp,amx         ; get amx into EBP

        sub     ecx,edi         ; correct STK
        mov     [ebp+_stk],ecx  ; store values in AMX structure: STK, ...
        mov     ecx,hea         ; ... HEA, ...
        mov     [ebp+_hea],ecx
        mov     ecx,ebx         ; ... and FRM
        sub     ecx,edi         ; (correct FRM)
        mov     [ebp+_frm],ecx
        mov     [ebp+_pri],eax  ; also store PRI, ...
        mov     [ebp+_alt],edx  ; ... and ALT

        ; return
        sub     stp,edi         ; make STP relative to DAT again
        xchg    esp,esi         ; switch back to caller's stack

        add     esp,4*9         ; remove temporary data

        pop     ebx             ; restore registers that have to be preserved
        pop     ebp             ; when using __cdecl convention
        pop     esi
        pop     edi

        ret


err_stack:
        mov     eax,AMX_ERR_STACKERR
        jmp     _return_popstack

err_stacklow:
        mov     eax,AMX_ERR_STACKLOW
        jmp     _return_popstack

_CHKMARGIN_STACK:               ; some run-time check routines
        cmp     esp,stp
        lea     ebp,[esp-STACKRESERVE]
        jg      err_stacklow
        sub     ebp,edi
        cmp     hea,ebp
        jg      err_stack
        ret

err_heaplow:
        mov     eax,AMX_ERR_HEAPLOW
        jmp     _return_popstack

_CHKMARGIN_HEAP:
        cmp     esp,stp
        jg      err_stacklow
        mov     ebp,amx
        mov     ebp,[ebp+_hlw]
        cmp     DWORD hea,ebp
        jl      err_heaplow
        ret

err_memaccess:
        mov     eax,AMX_ERR_MEMACCESS
        jmp     _return_popstack

_VERIFYADDRESS_eax:             ; used in load.i, store.i & lidx
        cmp     eax,stp
        jae     err_memaccess
        cmp     eax,hea
        jb      veax1
        lea     ebp,[eax+edi]
        cmp     ebp,esp
        jb      err_memaccess
    veax1:
        ret

_VERIFYADDRESS_edx:             ; used in load.i, store.i & lidx
        cmp     edx,stp
        jae     err_memaccess
        cmp     edx,hea
        jb      vedx1
        lea     ebp,[edx+edi]
        cmp     ebp,esp
        jb      err_memaccess
    vedx1:
        ret

JIT_OP_SDIV:
        mov     ebp,edx
        xor     edx,eax         ; Check signs of the operands.
        cdq
        js      sdiv_fiddle     ; If the signs of the operands are different
                                ; we'll have to fiddle around to achieve
                                ; proper rounding towards minus infinity.
        or      ebp,ebp         ; check for divide by zero
        jz      err_divide
        idiv    ebp             ; default behavior is right in the other cases
        ret

    sdiv_fiddle:
        or      ebp,ebp         ; check for divide by zero
        jz      err_divide
        idiv    ebp
        or      edx,edx
        jz      sdiv_goon       ; If there's no remainder the result is correct
        add     edx,ebp         ; else fix the result values.
        dec     eax             ; Amazing, how simple this is...
    sdiv_goon:
        ret

        ALIGN   4

JIT_OP_RETN:
        _POP    ebx             ; pop frame
        _POP    ecx             ; get return address

        mov     frm,ebx
        _POP    ebp

        add     ebx,edi
        add     esp,ebp         ; remove data from stack

        jmp     ecx


JIT_OP_MOVS:                    ;length of block to copy is already in ECX
        push    edi
        push    esi
        lea     esi,[edi+eax]
        lea     edi,[edi+edx]

        push    ecx             ; I hope the blocks to copy are properly
        shr     ecx,2           ; aligned, so I don't do anything about that.
        rep movsd
        pop     ecx
        and     ecx,3
        rep movsb

        pop     esi
        pop     edi
        ret

JIT_OP_CMPS:                    ;length of block to compare is already in ECX
        push    edi
        push    esi
        lea     esi,[edi+edx]
        lea     edi,[edi+eax]

        xor     eax,eax         ; This is surely not the fastest way to do this
        repe cmpsb              ; but the most simple one.
        je      cmps1
        sbb     eax,eax
        sbb     eax,0ffffffffh
    cmps1:
        pop     esi
        pop     edi
        ret


JIT_OP_FILL:                    ;length (in bytes) of block to fill is already in ECX
        push    edi
        lea     edi,[edi+edx]

        shr     ecx,2           ;length in 32-bit cells
        rep stosd               ;the value to use is already in EAX

        pop     edi
        ret

JIT_OP_BOUNDS:
        cmp     eax,0
        jl      err_bounds
        cmp     eax,ebp
        jg      err_bounds
        ret
err_bounds:
        mov     eax,AMX_ERR_BOUNDS
        jmp     _return_popstack

_CHKDIVIDEZERO:
        or      ebp,ebp         ; check for divide by zero
        jz      err_divide
        ret
err_divide:
        mov     eax,AMX_ERR_DIVIDE
        jmp     _return_popstack

JIT_OP_SYSREQ:
        mov     ecx,esp         ; get STK into ECX
        mov     ebp,amx         ; get amx into EBP

        sub     ecx,edi         ; correct STK
        mov     alt,edx         ; save ALT

        mov     [ebp+_stk],ecx  ; store values in AMX structure: STK,
        mov     ecx,hea         ; HEA,
        mov     ebx,frm         ; and FRM
        mov     [ebp+_hea],ecx
        mov     [ebp+_frm],ebx

        lea     ebx,pri         ; 3rd param: addr. of retval
        lea     ecx,[esp+4]     ; 4th param: parameter array

        xchg    esp,esi         ; switch to caller stack

        push    ecx
        push    ebx
        push    eax             ; 2nd param: function number
        push    ebp             ; 1st param: amx
        call    [ebp+_callback]
        _DROPARGS 16            ; remove args from stack

        xchg    esp,esi         ; switch back to AMX stack
        cmp     eax,AMX_ERR_NONE
        jne     _return_popstack; return error code, if any

        mov     eax,pri         ; get retval into eax (PRI)
        mov     edx,alt         ; restore ALT
        mov     ebx,frm         ; restore FRM
        add     ebx,edi         ; relocate frame
        ret


JIT_OP_BREAK:
%ifdef DEBUGSUPPORT
        mov     ecx,esp         ; get STK into ECX
        mov     ebp,amx         ; get amx into EBP

        sub     ecx,edi         ; correct STK

        mov     [ebp+_pri],eax  ; store values in AMX structure: PRI,
        mov     [ebp+_alt],edx  ; ALT,
        mov     [ebp+_stk],ecx  ; STK,
        mov     ecx,hea         ; HEA,
        mov     ebx,frm         ; and FRM
        mov     [ebp+_hea],ecx
        mov     [ebp+_frm],ebx  ; EBX & ECX are invalid by now
        ;??? storing CIP is not very useful, because the code changed (during JIT compile)

        xchg    esp,esi         ; switch to caller stack
        push    ebp             ; 1st param: amx
        call    [ebp+_debug]
        _DROPARGS 4             ; remove args from stack
        xchg    esp,esi         ; switch back to AMX stack
        cmp     eax,AMX_ERR_NONE
        jne     _return_popstack; return error code, if any

        mov     ebp,amx         ; get amx into EBP
        mov     eax,[ebp+_pri]  ; restore values
        mov     edx,[ebp+_alt]  ; ALT,
        mov     edx,alt         ; restore ALT
        mov     ebx,frm         ; restore FRM
        add     ebx,edi         ; relocate frame
%endif
        ret


JIT_OP_SWITCH:
        pop     ebp             ; pop return address = table address
        mov     ecx,[ebp]       ; ECX = number of records
        lea     ebp,[ebp+ecx*8+8] ; set pointer _after_ LAST case
        jecxz   op_switch_jump  ; special case: no cases at all
    op_switch_loop:
        cmp     eax,[ebp-8]     ; PRI == case label?
        je      op_switch_jump  ; found, jump
        sub     ebp,8           ; position to preceding case
        loop    op_switch_loop  ; check next case, or fall through
    op_switch_jump:
%ifndef FORCERELOCATABLE
        jmp     [ebp-4]         ; jump to the case instructions
%else
        add     ebp,[ebp-4]     ; add offset to make absolute adddress
        jmp     ebp
%endif


;----------------------------------------------------------------------------
;int amx_jit_list(const AMX *amx,const cell **opcodelist,int *numopcodes)
;----------------------------------------------------------------------------

global  amx_jit_list, _amx_jit_list
amx_jit_list:
_amx_jit_list:
        push    ebx
        ;mov     eax,[esp+08h]
        mov     edx,[esp+0ch]
        mov     ebx,[esp+10h]
        mov     eax,opcodelist
        mov     [edx],eax
        neg     eax
        add     eax,opcodelist_end   ; EAX = terminator - start
        shr     eax,2
        mov     [ebx],eax       ; store number of opcodes

        ; The caller of amx_jit_compile() can determine the maximum size of the
        ; compiled code by multiplying the result of this function by the number
        ; of opcodes in Pawn module.
        mov     eax,MAXCODESIZE

        pop     ebx
        ret


;============================================================================

section .data
        ALIGN   4       ; This is essential to avoid misalignment stalls.

end_code        DD  0   ; pointer to the end of the source code

compiled_code   DD  0   ; pointer to compiled code (including preamble)

amxhead         DD  0   ; pointer to the AMX_HEADER struct (arg #1 to runJIT)

reloc_num       DD  0   ; counts the addresses in the relocation table (jumps)

lodb_and        DD  0ffh, 0ffffh, 0, 0ffffffffh

;
; A list of the "run-time-library" functions that are called via indirect calls.
; So these calls don't have to be relocated. This gives also the possibility to
; replace some of these with shorter/faster non-debug or non-checking versions,
; without changing the compiled code. Instead this table could be changed...
;
verify_adr_eax  DD      _VERIFYADDRESS_eax
verify_adr_edx  DD      _VERIFYADDRESS_edx
chk_marginstack DD      _CHKMARGIN_STACK
chk_marginheap  DD      _CHKMARGIN_HEAP
chk_dividezero  DD      _CHKDIVIDEZERO
jit_return      DD      _return
jit_retn        DD      JIT_OP_RETN
jit_sdiv        DD      JIT_OP_SDIV
jit_movs        DD      JIT_OP_MOVS
jit_cmps        DD      JIT_OP_CMPS
jit_fill        DD      JIT_OP_FILL
jit_bounds      DD      JIT_OP_BOUNDS
jit_sysreq      DD      JIT_OP_SYSREQ
jit_break       DD      JIT_OP_BREAK
jit_switch      DD      JIT_OP_SWITCH

;
; The table for the browser/relocator function.
;

opcodelist:
        DD      OP_NOP
        DD      OP_LOAD_PRI
        DD      OP_LOAD_ALT
        DD      OP_LOAD_S_PRI
        DD      OP_LOAD_S_ALT
        DD      OP_LREF_S_PRI
        DD      OP_LREF_S_ALT
        DD      OP_LOAD_I
        DD      OP_LODB_I
        DD      OP_CONST_PRI
        DD      OP_CONST_ALT
        DD      OP_ADDR_PRI
        DD      OP_ADDR_ALT
        DD      OP_STOR
        DD      OP_STOR_S
        DD      OP_SREF_S
        DD      OP_STOR_I
        DD      OP_STRB_I
        DD      OP_ALIGN_PRI
        DD      OP_LCTRL
        DD      OP_SCTRL
        DD      OP_XCHG
        DD      OP_PUSH_PRI
        DD      OP_PUSH_ALT
        DD      OP_PUSHR_PRI
        DD      OP_POP_PRI
        DD      OP_POP_ALT
        DD      OP_PICK
        DD      OP_STACK
        DD      OP_HEAP
        DD      OP_PROC
        DD      OP_RET
        DD      OP_RETN
        DD      OP_CALL
        DD      OP_JUMP
        DD      OP_JZER
        DD      OP_JNZ
        DD      OP_SHL
        DD      OP_SHR
        DD      OP_SSHR
        DD      OP_SHL_C_PRI
        DD      OP_SHL_C_ALT
        DD      OP_SMUL
        DD      OP_SDIV
        DD      OP_ADD
        DD      OP_SUB
        DD      OP_AND
        DD      OP_OR
        DD      OP_XOR
        DD      OP_NOT
        DD      OP_NEG
        DD      OP_INVERT
        DD      OP_EQ
        DD      OP_NEQ
        DD      OP_SLESS
        DD      OP_SLEQ
        DD      OP_SGRTR
        DD      OP_SGEQ
        DD      OP_INC_PRI
        DD      OP_INC_ALT
        DD      OP_INC_I
        DD      OP_DEC_PRI
        DD      OP_DEC_ALT
        DD      OP_DEC_I
        DD      OP_MOVS
        DD      OP_CMPS
        DD      OP_FILL
        DD      OP_HALT
        DD      OP_BOUNDS
        DD      OP_SYSREQ
        DD      OP_SWITCH
        DD      OP_SWAP_PRI
        DD      OP_SWAP_ALT
        DD      OP_BREAK
        DD      OP_CASETBL
opcodelist_end:
