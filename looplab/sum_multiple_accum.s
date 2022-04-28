// This is assembly is roughly equivalent to the following C code:
// unsigned short sum_C(long size, unsigned short * a) {
//    unsigned short sum = 0;
//    for (int i = 0; i < size; ++i) {
//        sum += a[i];
//    }
//    return sum;
//}

// This implementation follows the Linux x86-64 calling convention:
//    %rdi contains the size
//    %rsi contains the pointer a
// and
//    %ax needs to contain the result when the function returns
// in addition, this code uses
//    %rcx to store i

// the '.global' directive indicates to the assembler to make this symbol
// available to other files.
.global sum_multiple_accum
sum_multiple_accum:
    // set sum (%ax) to 0
    xor %eax, %eax
    // return immediately; special case if size (%rdi) == 0
    test %rdi, %rdi
    je .L_done
    // store i = 0 in rcx
    movq $0, %rcx
    // r12w through r15w are accumulators
    // obey the calling convention!
    pushq %r12w
    pushq %r13w
    pushq %r14w
    pushq %r15w
    // store 0 in r12w
    movq $0, %r12
    // store 0 in r13w
    movq $0, %r13
    // store 0 in r14w
    movq $0, %r14
    // store 0 in r15w
    movq $0, %r15
// labels starting with '.L' are local to this file
.L_loop:
    // sum (%r12w) += a[i]
    addw (%rsi,%rcx,2), %r12w
    // sum (%r13w) += a[i+1]
    addw 2(%rsi,%rcx,2), %r13w
    // sum (%r14w) += a[i+2]
    addw 4(%rsi,%rcx,2), %r14w
    // sum (%r15w) += a[i+3]
    addw 6(%rsi,%rcx,2), %r15w
    // i += 4
    addq $4, %rcx
    // i < end?
    cmpq %rdi, %rcx
    jl .L_loop
.L_dest:
    addw %r12w, %ax
    addw %r13w, %ax
    addw %r14w, %ax
    addw %r15w, %ax
.L_done:
    // restore accumulators
    popq %r15w
    popq %r14w
    popq %r13w
    popq %r12w
    retq
