	.file	"dot_product_benchmarks.c"
	.text
	.globl	dot_product_C
	.type	dot_product_C, @function
dot_product_C:
.LFB4741:
	.cfi_startproc
	testq	%rdi, %rdi
	jle	.L4
	movl	$0, %ecx
	movl	$0, %eax
.L3:
	movslq	%ecx, %r9
	movzwl	(%rsi,%r9,2), %r8d
	movzwl	(%rdx,%r9,2), %r9d
	imull	%r9d, %r8d
	addl	%r8d, %eax
	addl	$1, %ecx
	movslq	%ecx, %r8
	cmpq	%rdi, %r8
	jl	.L3
	ret
.L4:
	movl	$0, %eax
	ret
	.cfi_endproc
.LFE4741:
	.size	dot_product_C, .-dot_product_C
	.globl	dot_product_AVX
	.type	dot_product_AVX, @function
dot_product_AVX:
.LFB4742:
	.cfi_startproc
	leaq	8(%rsp), %r10
	.cfi_def_cfa 10, 0
	andq	$-32, %rsp
	pushq	-8(%r10)
	pushq	%rbp
	.cfi_escape 0x10,0x6,0x2,0x76,0
	movq	%rsp, %rbp
	pushq	%r10
	.cfi_escape 0xf,0x3,0x76,0x78,0x6
	testq	%rdi, %rdi
	jle	.L10
	movl	$0, %eax
	vpxor	%xmm1, %xmm1, %xmm1
.L8:
	movslq	%eax, %rcx
	vpmovzxwd	(%rsi,%rcx,2), %ymm2
	vpmovzxwd	(%rdx,%rcx,2), %ymm0
	vpmulld	%ymm2, %ymm0, %ymm0
	vpaddd	%ymm1, %ymm0, %ymm1
	addl	$8, %eax
	movslq	%eax, %rcx
	cmpq	%rdi, %rcx
	jl	.L8
.L7:
	vmovdqa	%ymm1, -48(%rbp)
	leaq	-48(%rbp), %rdx
	leaq	-16(%rbp), %rcx
	movl	$0, %eax
.L9:
	addl	(%rdx), %eax
	addq	$4, %rdx
	cmpq	%rdx, %rcx
	jne	.L9
	popq	%r10
	.cfi_remember_state
	.cfi_def_cfa 10, 0
	popq	%rbp
	leaq	-8(%r10), %rsp
	.cfi_def_cfa 7, 8
	ret
.L10:
	.cfi_restore_state
	vpxor	%xmm1, %xmm1, %xmm1
	jmp	.L7
	.cfi_endproc
.LFE4742:
	.size	dot_product_AVX, .-dot_product_AVX
	.globl	functions
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"C (local)"
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC1:
	.string	"C (compiled with GCC7.2 -O3 -mavx2)"
	.data
	.align 32
	.type	functions, @object
	.size	functions, 48
functions:
	.quad	dot_product_C
	.quad	.LC0
	.quad	dot_product_gcc7_O3
	.quad	.LC1
	.quad	0
	.quad	0
	.ident	"GCC: (GNU) 7.1.0"
	.section	.note.GNU-stack,"",@progbits
