.data

PUBLIC GCONTEXT
PUBLIC GINDEX
PUBLIC GCCTX
PUBLIC GLOCK

GCONTEXT QWORD 0
GINDEX   QWORD 0
GCCTX    QWORD 0
GLOCK     QWORD 0

.code

GetReturn PROC

GetReturn ENDP

FirstCallback PROC
    mov GCONTEXT, rdx
    mov GINDEX, 0         ; reset GINDEX
    MOV GLOCK, 0
    jmp GenericCallback
    
FirstCallback ENDP

GenericCallback PROC
    ; Load GINDEX
    mov rax, GINDEX         ; rax = index
    mov rdx, 56             ; sizeof(WorkItemContext)
    imul rax, rdx           ; rax = index * 56

    ; Load GCONTEXT (base pointer)
    mov rdx, GCONTEXT       ; rdx = base address of workItems
    add rdx, rax            ; rdx = &workItems[index]

    ; rdx now points to WorkItemContext[index]
    mov GCCTX, rdx
    mov r10, rdx

    ; Load func ptr 
    mov rax, [rdx]          ; rax = workItems[index].func
    
    ; Load ARGC
    mov r11, [rdx + 010h]    ; argc (int)

    cmp r11, 4
    jg skip_call             ; current implementation only supports up to 4 args 

    ; Set RCX, RDX, R8, R9
    cmp r11, 0
    jle do_call              ; no args, just call
    
    mov rcx, [r10 + 018h]
    cmp r11, 1
    jle do_call

    mov rdx, [r10 + 020h]
    cmp r11, 2
    jle do_call

    mov r8, [r10 + 028h]
    cmp r11, 3
    jle do_call

    mov r9, [r10 + 030h]
    cmp r11, 4
    jle do_call

do_call:
    mov r11, GINDEX
    inc r11
    mov GINDEX, r11
    jmp rax

skip_call:
    mov r11, GINDEX
    inc r11
    mov GINDEX, r11
    ret

GenericCallback ENDP

LastCallback PROC frame
    ; Check if a lock is set, skip if true
    MOV rax, GLOCK
    cmp rax, 1
    je skip_call

    ; Set LOCK to avoid executing more than once
    MOV GLOCK, 1

    ; Load GINDEX
    mov rax, GINDEX         ; rax = index
    mov rdx, 56             ; sizeof(WorkItemContext)
    imul rax, rdx           ; rax = index * 56

    ; Load GCONTEXT (base pointer)
    mov rdx, GCONTEXT       ; rdx = base address of workItems
    add rdx, rax            ; rdx = &workItems[index]

    ; rdx now points to WorkItemContext[index]
    mov GCCTX, rdx
    mov r10, rdx

    ; Load func ptr 
    mov rax, [rdx]          ; rax = workItems[index].func
    mov rax, [rax]          ; rax = workItems[index].func

    mov r11, [r10 + 10h]
    
    cmp r11, 4
    jg skip_call             ; current implementation only supports up to 4 args 

    ; Set RCX, RDX, R8, R9
    cmp r11, 0
    jle do_call              ; no args, just call
    
    mov rcx, [r10 + 018h]
    cmp r11, 1
    jle do_call

    mov rdx, [r10 + 020h]
    cmp r11, 2
    jle do_call

    mov r8, [r10 + 028h]
    cmp r11, 3
    jle do_call

    mov r9, [r10 + 030h]
    cmp r11, 4
    jle do_call
    

do_call:
    mov r10, [r10 + 08h]   ; addressToPush
    
    .pushreg rbx
    push rbx
    .allocstack 20h
    sub rsp, 20h           ; this is for the spoofed/swapped frame
    .endprolog
    push r10
    mov rbx, GCONTEXT      ; we need this structure in RBX
    add rbx, 8             ; we use the address of the return value, unused for generic callbacks 

    jmp rax

skip_call:
    mov rax, 0
    ret


LastCallback ENDP

END