
#ifdef __i386__




	.file	"sha1-586.s"
.text
.globl	sha1_block_asm_data_order
.type	sha1_block_asm_data_order,@function
.align	16
sha1_block_asm_data_order:
	movl	12(%esp),	%ecx
	pushl	%esi
	sall	$6,		%ecx
	movl	12(%esp),	%esi
	pushl	%ebp
	addl	%esi,		%ecx
	pushl	%ebx
	movl	16(%esp),	%ebp
	pushl	%edi
	movl	12(%ebp),	%edx
	subl	$108,		%esp
	movl	16(%ebp),	%edi
	movl	8(%ebp),	%ebx
	movl	%ecx,		68(%esp)

.L000start:

	movl	(%esi),		%eax
	movl	4(%esi),	%ecx
.byte 15
.byte 200	
.byte 15
.byte 201	
	movl	%eax,		(%esp)
	movl	%ecx,		4(%esp)
	movl	8(%esi),	%eax
	movl	12(%esi),	%ecx
.byte 15
.byte 200	
.byte 15
.byte 201	
	movl	%eax,		8(%esp)
	movl	%ecx,		12(%esp)
	movl	16(%esi),	%eax
	movl	20(%esi),	%ecx
.byte 15
.byte 200	
.byte 15
.byte 201	
	movl	%eax,		16(%esp)
	movl	%ecx,		20(%esp)
	movl	24(%esi),	%eax
	movl	28(%esi),	%ecx
.byte 15
.byte 200	
.byte 15
.byte 201	
	movl	%eax,		24(%esp)
	movl	%ecx,		28(%esp)
	movl	32(%esi),	%eax
	movl	36(%esi),	%ecx
.byte 15
.byte 200	
.byte 15
.byte 201	
	movl	%eax,		32(%esp)
	movl	%ecx,		36(%esp)
	movl	40(%esi),	%eax
	movl	44(%esi),	%ecx
.byte 15
.byte 200	
.byte 15
.byte 201	
	movl	%eax,		40(%esp)
	movl	%ecx,		44(%esp)
	movl	48(%esi),	%eax
	movl	52(%esi),	%ecx
.byte 15
.byte 200	
.byte 15
.byte 201	
	movl	%eax,		48(%esp)
	movl	%ecx,		52(%esp)
	movl	56(%esi),	%eax
	movl	60(%esi),	%ecx
.byte 15
.byte 200	
.byte 15
.byte 201	
	movl	%eax,		56(%esp)
	movl	%ecx,		60(%esp)


	movl	%esi,		132(%esp)
.L001shortcut:


	movl	(%ebp),		%eax
	movl	4(%ebp),	%ecx

	movl	%ebx,		%esi
	movl	%eax,		%ebp
	roll	$5,		%ebp
	xorl	%edx,		%esi
	andl	%ecx,		%esi
	addl	%edi,		%ebp
	movl	(%esp),		%edi
	xorl	%edx,		%esi
	rorl	$2,		%ecx
	leal	1518500249(%ebp,%edi,1),%ebp
	addl	%esi,		%ebp

	movl	%ecx,		%edi
	movl	%ebp,		%esi
	roll	$5,		%ebp
	xorl	%ebx,		%edi
	andl	%eax,		%edi
	addl	%edx,		%ebp
	movl	4(%esp),	%edx
	xorl	%ebx,		%edi
	rorl	$2,		%eax
	leal	1518500249(%ebp,%edx,1),%ebp
	addl	%edi,		%ebp

	movl	%eax,		%edx
	movl	%ebp,		%edi
	roll	$5,		%ebp
	xorl	%ecx,		%edx
	andl	%esi,		%edx
	addl	%ebx,		%ebp
	movl	8(%esp),	%ebx
	xorl	%ecx,		%edx
	rorl	$2,		%esi
	leal	1518500249(%ebp,%ebx,1),%ebp
	addl	%edx,		%ebp

	movl	%esi,		%ebx
	movl	%ebp,		%edx
	roll	$5,		%ebp
	xorl	%eax,		%ebx
	andl	%edi,		%ebx
	addl	%ecx,		%ebp
	movl	12(%esp),	%ecx
	xorl	%eax,		%ebx
	rorl	$2,		%edi
	leal	1518500249(%ebp,%ecx,1),%ebp
	addl	%ebx,		%ebp

	movl	%edi,		%ecx
	movl	%ebp,		%ebx
	roll	$5,		%ebp
	xorl	%esi,		%ecx
	andl	%edx,		%ecx
	addl	%eax,		%ebp
	movl	16(%esp),	%eax
	xorl	%esi,		%ecx
	rorl	$2,		%edx
	leal	1518500249(%ebp,%eax,1),%ebp
	addl	%ecx,		%ebp

	movl	%edx,		%eax
	movl	%ebp,		%ecx
	roll	$5,		%ebp
	xorl	%edi,		%eax
	andl	%ebx,		%eax
	addl	%esi,		%ebp
	movl	20(%esp),	%esi
	xorl	%edi,		%eax
	rorl	$2,		%ebx
	leal	1518500249(%ebp,%esi,1),%ebp
	addl	%eax,		%ebp

	movl	%ebx,		%esi
	movl	%ebp,		%eax
	roll	$5,		%ebp
	xorl	%edx,		%esi
	andl	%ecx,		%esi
	addl	%edi,		%ebp
	movl	24(%esp),	%edi
	xorl	%edx,		%esi
	rorl	$2,		%ecx
	leal	1518500249(%ebp,%edi,1),%ebp
	addl	%esi,		%ebp

	movl	%ecx,		%edi
	movl	%ebp,		%esi
	roll	$5,		%ebp
	xorl	%ebx,		%edi
	andl	%eax,		%edi
	addl	%edx,		%ebp
	movl	28(%esp),	%edx
	xorl	%ebx,		%edi
	rorl	$2,		%eax
	leal	1518500249(%ebp,%edx,1),%ebp
	addl	%edi,		%ebp

	movl	%eax,		%edx
	movl	%ebp,		%edi
	roll	$5,		%ebp
	xorl	%ecx,		%edx
	andl	%esi,		%edx
	addl	%ebx,		%ebp
	movl	32(%esp),	%ebx
	xorl	%ecx,		%edx
	rorl	$2,		%esi
	leal	1518500249(%ebp,%ebx,1),%ebp
	addl	%edx,		%ebp

	movl	%esi,		%ebx
	movl	%ebp,		%edx
	roll	$5,		%ebp
	xorl	%eax,		%ebx
	andl	%edi,		%ebx
	addl	%ecx,		%ebp
	movl	36(%esp),	%ecx
	xorl	%eax,		%ebx
	rorl	$2,		%edi
	leal	1518500249(%ebp,%ecx,1),%ebp
	addl	%ebx,		%ebp

	movl	%edi,		%ecx
	movl	%ebp,		%ebx
	roll	$5,		%ebp
	xorl	%esi,		%ecx
	andl	%edx,		%ecx
	addl	%eax,		%ebp
	movl	40(%esp),	%eax
	xorl	%esi,		%ecx
	rorl	$2,		%edx
	leal	1518500249(%ebp,%eax,1),%ebp
	addl	%ecx,		%ebp

	movl	%edx,		%eax
	movl	%ebp,		%ecx
	roll	$5,		%ebp
	xorl	%edi,		%eax
	andl	%ebx,		%eax
	addl	%esi,		%ebp
	movl	44(%esp),	%esi
	xorl	%edi,		%eax
	rorl	$2,		%ebx
	leal	1518500249(%ebp,%esi,1),%ebp
	addl	%eax,		%ebp

	movl	%ebx,		%esi
	movl	%ebp,		%eax
	roll	$5,		%ebp
	xorl	%edx,		%esi
	andl	%ecx,		%esi
	addl	%edi,		%ebp
	movl	48(%esp),	%edi
	xorl	%edx,		%esi
	rorl	$2,		%ecx
	leal	1518500249(%ebp,%edi,1),%ebp
	addl	%esi,		%ebp

	movl	%ecx,		%edi
	movl	%ebp,		%esi
	roll	$5,		%ebp
	xorl	%ebx,		%edi
	andl	%eax,		%edi
	addl	%edx,		%ebp
	movl	52(%esp),	%edx
	xorl	%ebx,		%edi
	rorl	$2,		%eax
	leal	1518500249(%ebp,%edx,1),%ebp
	addl	%edi,		%ebp

	movl	%eax,		%edx
	movl	%ebp,		%edi
	roll	$5,		%ebp
	xorl	%ecx,		%edx
	andl	%esi,		%edx
	addl	%ebx,		%ebp
	movl	56(%esp),	%ebx
	xorl	%ecx,		%edx
	rorl	$2,		%esi
	leal	1518500249(%ebp,%ebx,1),%ebp
	addl	%edx,		%ebp

	movl	%esi,		%ebx
	movl	%ebp,		%edx
	roll	$5,		%ebp
	xorl	%eax,		%ebx
	andl	%edi,		%ebx
	addl	%ecx,		%ebp
	movl	60(%esp),	%ecx
	xorl	%eax,		%ebx
	rorl	$2,		%edi
	leal	1518500249(%ebp,%ecx,1),%ebp
	addl	%ebp,		%ebx

	movl	8(%esp),	%ecx
	movl	%edi,		%ebp
	xorl	(%esp),		%ecx
	xorl	%esi,		%ebp
	xorl	32(%esp),	%ecx
	andl	%edx,		%ebp
	rorl	$2,		%edx
	xorl	52(%esp),	%ecx
.byte 209
.byte 193	
	xorl	%esi,		%ebp
	movl	%ecx,		(%esp)
	leal	1518500249(%ecx,%eax,1),%ecx
	movl	%ebx,		%eax
	roll	$5,		%eax
	addl	%ebp,		%ecx
	addl	%eax,		%ecx

	movl	12(%esp),	%eax
	movl	%edx,		%ebp
	xorl	4(%esp),	%eax
	xorl	%edi,		%ebp
	xorl	36(%esp),	%eax
	andl	%ebx,		%ebp
	rorl	$2,		%ebx
	xorl	56(%esp),	%eax
.byte 209
.byte 192	
	xorl	%edi,		%ebp
	movl	%eax,		4(%esp)
	leal	1518500249(%eax,%esi,1),%eax
	movl	%ecx,		%esi
	roll	$5,		%esi
	addl	%ebp,		%eax
	addl	%esi,		%eax

	movl	16(%esp),	%esi
	movl	%ebx,		%ebp
	xorl	8(%esp),	%esi
	xorl	%edx,		%ebp
	xorl	40(%esp),	%esi
	andl	%ecx,		%ebp
	rorl	$2,		%ecx
	xorl	60(%esp),	%esi
.byte 209
.byte 198	
	xorl	%edx,		%ebp
	movl	%esi,		8(%esp)
	leal	1518500249(%esi,%edi,1),%esi
	movl	%eax,		%edi
	roll	$5,		%edi
	addl	%ebp,		%esi
	addl	%edi,		%esi

	movl	20(%esp),	%edi
	movl	%ecx,		%ebp
	xorl	12(%esp),	%edi
	xorl	%ebx,		%ebp
	xorl	44(%esp),	%edi
	andl	%eax,		%ebp
	rorl	$2,		%eax
	xorl	(%esp),		%edi
.byte 209
.byte 199	
	xorl	%ebx,		%ebp
	movl	%edi,		12(%esp)
	leal	1518500249(%edi,%edx,1),%edi
	movl	%esi,		%edx
	roll	$5,		%edx
	addl	%ebp,		%edi
	addl	%edx,		%edi

	movl	%esi,		%ebp
	movl	16(%esp),	%edx
	rorl	$2,		%esi
	xorl	24(%esp),	%edx
	xorl	%eax,		%ebp
	xorl	48(%esp),	%edx
	xorl	%ecx,		%ebp
	xorl	4(%esp),	%edx
.byte 209
.byte 194	
	addl	%ebx,		%ebp
	movl	%edx,		16(%esp)
	movl	%edi,		%ebx
	roll	$5,		%ebx
	leal	1859775393(%edx,%ebp,1),%edx
	addl	%ebx,		%edx

	movl	%edi,		%ebp
	movl	20(%esp),	%ebx
	rorl	$2,		%edi
	xorl	28(%esp),	%ebx
	xorl	%esi,		%ebp
	xorl	52(%esp),	%ebx
	xorl	%eax,		%ebp
	xorl	8(%esp),	%ebx
.byte 209
.byte 195	
	addl	%ecx,		%ebp
	movl	%ebx,		20(%esp)
	movl	%edx,		%ecx
	roll	$5,		%ecx
	leal	1859775393(%ebx,%ebp,1),%ebx
	addl	%ecx,		%ebx

	movl	%edx,		%ebp
	movl	24(%esp),	%ecx
	rorl	$2,		%edx
	xorl	32(%esp),	%ecx
	xorl	%edi,		%ebp
	xorl	56(%esp),	%ecx
	xorl	%esi,		%ebp
	xorl	12(%esp),	%ecx
.byte 209
.byte 193	
	addl	%eax,		%ebp
	movl	%ecx,		24(%esp)
	movl	%ebx,		%eax
	roll	$5,		%eax
	leal	1859775393(%ecx,%ebp,1),%ecx
	addl	%eax,		%ecx

	movl	%ebx,		%ebp
	movl	28(%esp),	%eax
	rorl	$2,		%ebx
	xorl	36(%esp),	%eax
	xorl	%edx,		%ebp
	xorl	60(%esp),	%eax
	xorl	%edi,		%ebp
	xorl	16(%esp),	%eax
.byte 209
.byte 192	
	addl	%esi,		%ebp
	movl	%eax,		28(%esp)
	movl	%ecx,		%esi
	roll	$5,		%esi
	leal	1859775393(%eax,%ebp,1),%eax
	addl	%esi,		%eax

	movl	%ecx,		%ebp
	movl	32(%esp),	%esi
	rorl	$2,		%ecx
	xorl	40(%esp),	%esi
	xorl	%ebx,		%ebp
	xorl	(%esp),		%esi
	xorl	%edx,		%ebp
	xorl	20(%esp),	%esi
.byte 209
.byte 198	
	addl	%edi,		%ebp
	movl	%esi,		32(%esp)
	movl	%eax,		%edi
	roll	$5,		%edi
	leal	1859775393(%esi,%ebp,1),%esi
	addl	%edi,		%esi

	movl	%eax,		%ebp
	movl	36(%esp),	%edi
	rorl	$2,		%eax
	xorl	44(%esp),	%edi
	xorl	%ecx,		%ebp
	xorl	4(%esp),	%edi
	xorl	%ebx,		%ebp
	xorl	24(%esp),	%edi
.byte 209
.byte 199	
	addl	%edx,		%ebp
	movl	%edi,		36(%esp)
	movl	%esi,		%edx
	roll	$5,		%edx
	leal	1859775393(%edi,%ebp,1),%edi
	addl	%edx,		%edi

	movl	%esi,		%ebp
	movl	40(%esp),	%edx
	rorl	$2,		%esi
	xorl	48(%esp),	%edx
	xorl	%eax,		%ebp
	xorl	8(%esp),	%edx
	xorl	%ecx,		%ebp
	xorl	28(%esp),	%edx
.byte 209
.byte 194	
	addl	%ebx,		%ebp
	movl	%edx,		40(%esp)
	movl	%edi,		%ebx
	roll	$5,		%ebx
	leal	1859775393(%edx,%ebp,1),%edx
	addl	%ebx,		%edx

	movl	%edi,		%ebp
	movl	44(%esp),	%ebx
	rorl	$2,		%edi
	xorl	52(%esp),	%ebx
	xorl	%esi,		%ebp
	xorl	12(%esp),	%ebx
	xorl	%eax,		%ebp
	xorl	32(%esp),	%ebx
.byte 209
.byte 195	
	addl	%ecx,		%ebp
	movl	%ebx,		44(%esp)
	movl	%edx,		%ecx
	roll	$5,		%ecx
	leal	1859775393(%ebx,%ebp,1),%ebx
	addl	%ecx,		%ebx

	movl	%edx,		%ebp
	movl	48(%esp),	%ecx
	rorl	$2,		%edx
	xorl	56(%esp),	%ecx
	xorl	%edi,		%ebp
	xorl	16(%esp),	%ecx
	xorl	%esi,		%ebp
	xorl	36(%esp),	%ecx
.byte 209
.byte 193	
	addl	%eax,		%ebp
	movl	%ecx,		48(%esp)
	movl	%ebx,		%eax
	roll	$5,		%eax
	leal	1859775393(%ecx,%ebp,1),%ecx
	addl	%eax,		%ecx

	movl	%ebx,		%ebp
	movl	52(%esp),	%eax
	rorl	$2,		%ebx
	xorl	60(%esp),	%eax
	xorl	%edx,		%ebp
	xorl	20(%esp),	%eax
	xorl	%edi,		%ebp
	xorl	40(%esp),	%eax
.byte 209
.byte 192	
	addl	%esi,		%ebp
	movl	%eax,		52(%esp)
	movl	%ecx,		%esi
	roll	$5,		%esi
	leal	1859775393(%eax,%ebp,1),%eax
	addl	%esi,		%eax

	movl	%ecx,		%ebp
	movl	56(%esp),	%esi
	rorl	$2,		%ecx
	xorl	(%esp),		%esi
	xorl	%ebx,		%ebp
	xorl	24(%esp),	%esi
	xorl	%edx,		%ebp
	xorl	44(%esp),	%esi
.byte 209
.byte 198	
	addl	%edi,		%ebp
	movl	%esi,		56(%esp)
	movl	%eax,		%edi
	roll	$5,		%edi
	leal	1859775393(%esi,%ebp,1),%esi
	addl	%edi,		%esi

	movl	%eax,		%ebp
	movl	60(%esp),	%edi
	rorl	$2,		%eax
	xorl	4(%esp),	%edi
	xorl	%ecx,		%ebp
	xorl	28(%esp),	%edi
	xorl	%ebx,		%ebp
	xorl	48(%esp),	%edi
.byte 209
.byte 199	
	addl	%edx,		%ebp
	movl	%edi,		60(%esp)
	movl	%esi,		%edx
	roll	$5,		%edx
	leal	1859775393(%edi,%ebp,1),%edi
	addl	%edx,		%edi

	movl	%esi,		%ebp
	movl	(%esp),		%edx
	rorl	$2,		%esi
	xorl	8(%esp),	%edx
	xorl	%eax,		%ebp
	xorl	32(%esp),	%edx
	xorl	%ecx,		%ebp
	xorl	52(%esp),	%edx
.byte 209
.byte 194	
	addl	%ebx,		%ebp
	movl	%edx,		(%esp)
	movl	%edi,		%ebx
	roll	$5,		%ebx
	leal	1859775393(%edx,%ebp,1),%edx
	addl	%ebx,		%edx

	movl	%edi,		%ebp
	movl	4(%esp),	%ebx
	rorl	$2,		%edi
	xorl	12(%esp),	%ebx
	xorl	%esi,		%ebp
	xorl	36(%esp),	%ebx
	xorl	%eax,		%ebp
	xorl	56(%esp),	%ebx
.byte 209
.byte 195	
	addl	%ecx,		%ebp
	movl	%ebx,		4(%esp)
	movl	%edx,		%ecx
	roll	$5,		%ecx
	leal	1859775393(%ebx,%ebp,1),%ebx
	addl	%ecx,		%ebx

	movl	%edx,		%ebp
	movl	8(%esp),	%ecx
	rorl	$2,		%edx
	xorl	16(%esp),	%ecx
	xorl	%edi,		%ebp
	xorl	40(%esp),	%ecx
	xorl	%esi,		%ebp
	xorl	60(%esp),	%ecx
.byte 209
.byte 193	
	addl	%eax,		%ebp
	movl	%ecx,		8(%esp)
	movl	%ebx,		%eax
	roll	$5,		%eax
	leal	1859775393(%ecx,%ebp,1),%ecx
	addl	%eax,		%ecx

	movl	%ebx,		%ebp
	movl	12(%esp),	%eax
	rorl	$2,		%ebx
	xorl	20(%esp),	%eax
	xorl	%edx,		%ebp
	xorl	44(%esp),	%eax
	xorl	%edi,		%ebp
	xorl	(%esp),		%eax
.byte 209
.byte 192	
	addl	%esi,		%ebp
	movl	%eax,		12(%esp)
	movl	%ecx,		%esi
	roll	$5,		%esi
	leal	1859775393(%eax,%ebp,1),%eax
	addl	%esi,		%eax

	movl	%ecx,		%ebp
	movl	16(%esp),	%esi
	rorl	$2,		%ecx
	xorl	24(%esp),	%esi
	xorl	%ebx,		%ebp
	xorl	48(%esp),	%esi
	xorl	%edx,		%ebp
	xorl	4(%esp),	%esi
.byte 209
.byte 198	
	addl	%edi,		%ebp
	movl	%esi,		16(%esp)
	movl	%eax,		%edi
	roll	$5,		%edi
	leal	1859775393(%esi,%ebp,1),%esi
	addl	%edi,		%esi

	movl	%eax,		%ebp
	movl	20(%esp),	%edi
	rorl	$2,		%eax
	xorl	28(%esp),	%edi
	xorl	%ecx,		%ebp
	xorl	52(%esp),	%edi
	xorl	%ebx,		%ebp
	xorl	8(%esp),	%edi
.byte 209
.byte 199	
	addl	%edx,		%ebp
	movl	%edi,		20(%esp)
	movl	%esi,		%edx
	roll	$5,		%edx
	leal	1859775393(%edi,%ebp,1),%edi
	addl	%edx,		%edi

	movl	%esi,		%ebp
	movl	24(%esp),	%edx
	rorl	$2,		%esi
	xorl	32(%esp),	%edx
	xorl	%eax,		%ebp
	xorl	56(%esp),	%edx
	xorl	%ecx,		%ebp
	xorl	12(%esp),	%edx
.byte 209
.byte 194	
	addl	%ebx,		%ebp
	movl	%edx,		24(%esp)
	movl	%edi,		%ebx
	roll	$5,		%ebx
	leal	1859775393(%edx,%ebp,1),%edx
	addl	%ebx,		%edx

	movl	%edi,		%ebp
	movl	28(%esp),	%ebx
	rorl	$2,		%edi
	xorl	36(%esp),	%ebx
	xorl	%esi,		%ebp
	xorl	60(%esp),	%ebx
	xorl	%eax,		%ebp
	xorl	16(%esp),	%ebx
.byte 209
.byte 195	
	addl	%ecx,		%ebp
	movl	%ebx,		28(%esp)
	movl	%edx,		%ecx
	roll	$5,		%ecx
	leal	1859775393(%ebx,%ebp,1),%ebx
	addl	%ecx,		%ebx

	movl	32(%esp),	%ecx
	movl	40(%esp),	%ebp
	xorl	%ebp,		%ecx
	movl	(%esp),		%ebp
	xorl	%ebp,		%ecx
	movl	20(%esp),	%ebp
	xorl	%ebp,		%ecx
	movl	%edx,		%ebp
.byte 209
.byte 193	
	orl	%edi,		%ebp
	movl	%ecx,		32(%esp)
	andl	%esi,		%ebp
	leal	2400959708(%ecx,%eax,1),%ecx
	movl	%edx,		%eax
	rorl	$2,		%edx
	andl	%edi,		%eax
	orl	%eax,		%ebp
	movl	%ebx,		%eax
	roll	$5,		%eax
	addl	%ebp,		%ecx
	addl	%eax,		%ecx

	movl	36(%esp),	%eax
	movl	44(%esp),	%ebp
	xorl	%ebp,		%eax
	movl	4(%esp),	%ebp
	xorl	%ebp,		%eax
	movl	24(%esp),	%ebp
	xorl	%ebp,		%eax
	movl	%ebx,		%ebp
.byte 209
.byte 192	
	orl	%edx,		%ebp
	movl	%eax,		36(%esp)
	andl	%edi,		%ebp
	leal	2400959708(%eax,%esi,1),%eax
	movl	%ebx,		%esi
	rorl	$2,		%ebx
	andl	%edx,		%esi
	orl	%esi,		%ebp
	movl	%ecx,		%esi
	roll	$5,		%esi
	addl	%ebp,		%eax
	addl	%esi,		%eax

	movl	40(%esp),	%esi
	movl	48(%esp),	%ebp
	xorl	%ebp,		%esi
	movl	8(%esp),	%ebp
	xorl	%ebp,		%esi
	movl	28(%esp),	%ebp
	xorl	%ebp,		%esi
	movl	%ecx,		%ebp
.byte 209
.byte 198	
	orl	%ebx,		%ebp
	movl	%esi,		40(%esp)
	andl	%edx,		%ebp
	leal	2400959708(%esi,%edi,1),%esi
	movl	%ecx,		%edi
	rorl	$2,		%ecx
	andl	%ebx,		%edi
	orl	%edi,		%ebp
	movl	%eax,		%edi
	roll	$5,		%edi
	addl	%ebp,		%esi
	addl	%edi,		%esi

	movl	44(%esp),	%edi
	movl	52(%esp),	%ebp
	xorl	%ebp,		%edi
	movl	12(%esp),	%ebp
	xorl	%ebp,		%edi
	movl	32(%esp),	%ebp
	xorl	%ebp,		%edi
	movl	%eax,		%ebp
.byte 209
.byte 199	
	orl	%ecx,		%ebp
	movl	%edi,		44(%esp)
	andl	%ebx,		%ebp
	leal	2400959708(%edi,%edx,1),%edi
	movl	%eax,		%edx
	rorl	$2,		%eax
	andl	%ecx,		%edx
	orl	%edx,		%ebp
	movl	%esi,		%edx
	roll	$5,		%edx
	addl	%ebp,		%edi
	addl	%edx,		%edi

	movl	48(%esp),	%edx
	movl	56(%esp),	%ebp
	xorl	%ebp,		%edx
	movl	16(%esp),	%ebp
	xorl	%ebp,		%edx
	movl	36(%esp),	%ebp
	xorl	%ebp,		%edx
	movl	%esi,		%ebp
.byte 209
.byte 194	
	orl	%eax,		%ebp
	movl	%edx,		48(%esp)
	andl	%ecx,		%ebp
	leal	2400959708(%edx,%ebx,1),%edx
	movl	%esi,		%ebx
	rorl	$2,		%esi
	andl	%eax,		%ebx
	orl	%ebx,		%ebp
	movl	%edi,		%ebx
	roll	$5,		%ebx
	addl	%ebp,		%edx
	addl	%ebx,		%edx

	movl	52(%esp),	%ebx
	movl	60(%esp),	%ebp
	xorl	%ebp,		%ebx
	movl	20(%esp),	%ebp
	xorl	%ebp,		%ebx
	movl	40(%esp),	%ebp
	xorl	%ebp,		%ebx
	movl	%edi,		%ebp
.byte 209
.byte 195	
	orl	%esi,		%ebp
	movl	%ebx,		52(%esp)
	andl	%eax,		%ebp
	leal	2400959708(%ebx,%ecx,1),%ebx
	movl	%edi,		%ecx
	rorl	$2,		%edi
	andl	%esi,		%ecx
	orl	%ecx,		%ebp
	movl	%edx,		%ecx
	roll	$5,		%ecx
	addl	%ebp,		%ebx
	addl	%ecx,		%ebx

	movl	56(%esp),	%ecx
	movl	(%esp),		%ebp
	xorl	%ebp,		%ecx
	movl	24(%esp),	%ebp
	xorl	%ebp,		%ecx
	movl	44(%esp),	%ebp
	xorl	%ebp,		%ecx
	movl	%edx,		%ebp
.byte 209
.byte 193	
	orl	%edi,		%ebp
	movl	%ecx,		56(%esp)
	andl	%esi,		%ebp
	leal	2400959708(%ecx,%eax,1),%ecx
	movl	%edx,		%eax
	rorl	$2,		%edx
	andl	%edi,		%eax
	orl	%eax,		%ebp
	movl	%ebx,		%eax
	roll	$5,		%eax
	addl	%ebp,		%ecx
	addl	%eax,		%ecx

	movl	60(%esp),	%eax
	movl	4(%esp),	%ebp
	xorl	%ebp,		%eax
	movl	28(%esp),	%ebp
	xorl	%ebp,		%eax
	movl	48(%esp),	%ebp
	xorl	%ebp,		%eax
	movl	%ebx,		%ebp
.byte 209
.byte 192	
	orl	%edx,		%ebp
	movl	%eax,		60(%esp)
	andl	%edi,		%ebp
	leal	2400959708(%eax,%esi,1),%eax
	movl	%ebx,		%esi
	rorl	$2,		%ebx
	andl	%edx,		%esi
	orl	%esi,		%ebp
	movl	%ecx,		%esi
	roll	$5,		%esi
	addl	%ebp,		%eax
	addl	%esi,		%eax

	movl	(%esp),		%esi
	movl	8(%esp),	%ebp
	xorl	%ebp,		%esi
	movl	32(%esp),	%ebp
	xorl	%ebp,		%esi
	movl	52(%esp),	%ebp
	xorl	%ebp,		%esi
	movl	%ecx,		%ebp
.byte 209
.byte 198	
	orl	%ebx,		%ebp
	movl	%esi,		(%esp)
	andl	%edx,		%ebp
	leal	2400959708(%esi,%edi,1),%esi
	movl	%ecx,		%edi
	rorl	$2,		%ecx
	andl	%ebx,		%edi
	orl	%edi,		%ebp
	movl	%eax,		%edi
	roll	$5,		%edi
	addl	%ebp,		%esi
	addl	%edi,		%esi

	movl	4(%esp),	%edi
	movl	12(%esp),	%ebp
	xorl	%ebp,		%edi
	movl	36(%esp),	%ebp
	xorl	%ebp,		%edi
	movl	56(%esp),	%ebp
	xorl	%ebp,		%edi
	movl	%eax,		%ebp
.byte 209
.byte 199	
	orl	%ecx,		%ebp
	movl	%edi,		4(%esp)
	andl	%ebx,		%ebp
	leal	2400959708(%edi,%edx,1),%edi
	movl	%eax,		%edx
	rorl	$2,		%eax
	andl	%ecx,		%edx
	orl	%edx,		%ebp
	movl	%esi,		%edx
	roll	$5,		%edx
	addl	%ebp,		%edi
	addl	%edx,		%edi

	movl	8(%esp),	%edx
	movl	16(%esp),	%ebp
	xorl	%ebp,		%edx
	movl	40(%esp),	%ebp
	xorl	%ebp,		%edx
	movl	60(%esp),	%ebp
	xorl	%ebp,		%edx
	movl	%esi,		%ebp
.byte 209
.byte 194	
	orl	%eax,		%ebp
	movl	%edx,		8(%esp)
	andl	%ecx,		%ebp
	leal	2400959708(%edx,%ebx,1),%edx
	movl	%esi,		%ebx
	rorl	$2,		%esi
	andl	%eax,		%ebx
	orl	%ebx,		%ebp
	movl	%edi,		%ebx
	roll	$5,		%ebx
	addl	%ebp,		%edx
	addl	%ebx,		%edx

	movl	12(%esp),	%ebx
	movl	20(%esp),	%ebp
	xorl	%ebp,		%ebx
	movl	44(%esp),	%ebp
	xorl	%ebp,		%ebx
	movl	(%esp),		%ebp
	xorl	%ebp,		%ebx
	movl	%edi,		%ebp
.byte 209
.byte 195	
	orl	%esi,		%ebp
	movl	%ebx,		12(%esp)
	andl	%eax,		%ebp
	leal	2400959708(%ebx,%ecx,1),%ebx
	movl	%edi,		%ecx
	rorl	$2,		%edi
	andl	%esi,		%ecx
	orl	%ecx,		%ebp
	movl	%edx,		%ecx
	roll	$5,		%ecx
	addl	%ebp,		%ebx
	addl	%ecx,		%ebx

	movl	16(%esp),	%ecx
	movl	24(%esp),	%ebp
	xorl	%ebp,		%ecx
	movl	48(%esp),	%ebp
	xorl	%ebp,		%ecx
	movl	4(%esp),	%ebp
	xorl	%ebp,		%ecx
	movl	%edx,		%ebp
.byte 209
.byte 193	
	orl	%edi,		%ebp
	movl	%ecx,		16(%esp)
	andl	%esi,		%ebp
	leal	2400959708(%ecx,%eax,1),%ecx
	movl	%edx,		%eax
	rorl	$2,		%edx
	andl	%edi,		%eax
	orl	%eax,		%ebp
	movl	%ebx,		%eax
	roll	$5,		%eax
	addl	%ebp,		%ecx
	addl	%eax,		%ecx

	movl	20(%esp),	%eax
	movl	28(%esp),	%ebp
	xorl	%ebp,		%eax
	movl	52(%esp),	%ebp
	xorl	%ebp,		%eax
	movl	8(%esp),	%ebp
	xorl	%ebp,		%eax
	movl	%ebx,		%ebp
.byte 209
.byte 192	
	orl	%edx,		%ebp
	movl	%eax,		20(%esp)
	andl	%edi,		%ebp
	leal	2400959708(%eax,%esi,1),%eax
	movl	%ebx,		%esi
	rorl	$2,		%ebx
	andl	%edx,		%esi
	orl	%esi,		%ebp
	movl	%ecx,		%esi
	roll	$5,		%esi
	addl	%ebp,		%eax
	addl	%esi,		%eax

	movl	24(%esp),	%esi
	movl	32(%esp),	%ebp
	xorl	%ebp,		%esi
	movl	56(%esp),	%ebp
	xorl	%ebp,		%esi
	movl	12(%esp),	%ebp
	xorl	%ebp,		%esi
	movl	%ecx,		%ebp
.byte 209
.byte 198	
	orl	%ebx,		%ebp
	movl	%esi,		24(%esp)
	andl	%edx,		%ebp
	leal	2400959708(%esi,%edi,1),%esi
	movl	%ecx,		%edi
	rorl	$2,		%ecx
	andl	%ebx,		%edi
	orl	%edi,		%ebp
	movl	%eax,		%edi
	roll	$5,		%edi
	addl	%ebp,		%esi
	addl	%edi,		%esi

	movl	28(%esp),	%edi
	movl	36(%esp),	%ebp
	xorl	%ebp,		%edi
	movl	60(%esp),	%ebp
	xorl	%ebp,		%edi
	movl	16(%esp),	%ebp
	xorl	%ebp,		%edi
	movl	%eax,		%ebp
.byte 209
.byte 199	
	orl	%ecx,		%ebp
	movl	%edi,		28(%esp)
	andl	%ebx,		%ebp
	leal	2400959708(%edi,%edx,1),%edi
	movl	%eax,		%edx
	rorl	$2,		%eax
	andl	%ecx,		%edx
	orl	%edx,		%ebp
	movl	%esi,		%edx
	roll	$5,		%edx
	addl	%ebp,		%edi
	addl	%edx,		%edi

	movl	32(%esp),	%edx
	movl	40(%esp),	%ebp
	xorl	%ebp,		%edx
	movl	(%esp),		%ebp
	xorl	%ebp,		%edx
	movl	20(%esp),	%ebp
	xorl	%ebp,		%edx
	movl	%esi,		%ebp
.byte 209
.byte 194	
	orl	%eax,		%ebp
	movl	%edx,		32(%esp)
	andl	%ecx,		%ebp
	leal	2400959708(%edx,%ebx,1),%edx
	movl	%esi,		%ebx
	rorl	$2,		%esi
	andl	%eax,		%ebx
	orl	%ebx,		%ebp
	movl	%edi,		%ebx
	roll	$5,		%ebx
	addl	%ebp,		%edx
	addl	%ebx,		%edx

	movl	36(%esp),	%ebx
	movl	44(%esp),	%ebp
	xorl	%ebp,		%ebx
	movl	4(%esp),	%ebp
	xorl	%ebp,		%ebx
	movl	24(%esp),	%ebp
	xorl	%ebp,		%ebx
	movl	%edi,		%ebp
.byte 209
.byte 195	
	orl	%esi,		%ebp
	movl	%ebx,		36(%esp)
	andl	%eax,		%ebp
	leal	2400959708(%ebx,%ecx,1),%ebx
	movl	%edi,		%ecx
	rorl	$2,		%edi
	andl	%esi,		%ecx
	orl	%ecx,		%ebp
	movl	%edx,		%ecx
	roll	$5,		%ecx
	addl	%ebp,		%ebx
	addl	%ecx,		%ebx

	movl	40(%esp),	%ecx
	movl	48(%esp),	%ebp
	xorl	%ebp,		%ecx
	movl	8(%esp),	%ebp
	xorl	%ebp,		%ecx
	movl	28(%esp),	%ebp
	xorl	%ebp,		%ecx
	movl	%edx,		%ebp
.byte 209
.byte 193	
	orl	%edi,		%ebp
	movl	%ecx,		40(%esp)
	andl	%esi,		%ebp
	leal	2400959708(%ecx,%eax,1),%ecx
	movl	%edx,		%eax
	rorl	$2,		%edx
	andl	%edi,		%eax
	orl	%eax,		%ebp
	movl	%ebx,		%eax
	roll	$5,		%eax
	addl	%ebp,		%ecx
	addl	%eax,		%ecx

	movl	44(%esp),	%eax
	movl	52(%esp),	%ebp
	xorl	%ebp,		%eax
	movl	12(%esp),	%ebp
	xorl	%ebp,		%eax
	movl	32(%esp),	%ebp
	xorl	%ebp,		%eax
	movl	%ebx,		%ebp
.byte 209
.byte 192	
	orl	%edx,		%ebp
	movl	%eax,		44(%esp)
	andl	%edi,		%ebp
	leal	2400959708(%eax,%esi,1),%eax
	movl	%ebx,		%esi
	rorl	$2,		%ebx
	andl	%edx,		%esi
	orl	%esi,		%ebp
	movl	%ecx,		%esi
	roll	$5,		%esi
	addl	%ebp,		%eax
	addl	%esi,		%eax

	movl	%ecx,		%ebp
	movl	48(%esp),	%esi
	rorl	$2,		%ecx
	xorl	56(%esp),	%esi
	xorl	%ebx,		%ebp
	xorl	16(%esp),	%esi
	xorl	%edx,		%ebp
	xorl	36(%esp),	%esi
.byte 209
.byte 198	
	addl	%edi,		%ebp
	movl	%esi,		48(%esp)
	movl	%eax,		%edi
	roll	$5,		%edi
	leal	3395469782(%esi,%ebp,1),%esi
	addl	%edi,		%esi

	movl	%eax,		%ebp
	movl	52(%esp),	%edi
	rorl	$2,		%eax
	xorl	60(%esp),	%edi
	xorl	%ecx,		%ebp
	xorl	20(%esp),	%edi
	xorl	%ebx,		%ebp
	xorl	40(%esp),	%edi
.byte 209
.byte 199	
	addl	%edx,		%ebp
	movl	%edi,		52(%esp)
	movl	%esi,		%edx
	roll	$5,		%edx
	leal	3395469782(%edi,%ebp,1),%edi
	addl	%edx,		%edi

	movl	%esi,		%ebp
	movl	56(%esp),	%edx
	rorl	$2,		%esi
	xorl	(%esp),		%edx
	xorl	%eax,		%ebp
	xorl	24(%esp),	%edx
	xorl	%ecx,		%ebp
	xorl	44(%esp),	%edx
.byte 209
.byte 194	
	addl	%ebx,		%ebp
	movl	%edx,		56(%esp)
	movl	%edi,		%ebx
	roll	$5,		%ebx
	leal	3395469782(%edx,%ebp,1),%edx
	addl	%ebx,		%edx

	movl	%edi,		%ebp
	movl	60(%esp),	%ebx
	rorl	$2,		%edi
	xorl	4(%esp),	%ebx
	xorl	%esi,		%ebp
	xorl	28(%esp),	%ebx
	xorl	%eax,		%ebp
	xorl	48(%esp),	%ebx
.byte 209
.byte 195	
	addl	%ecx,		%ebp
	movl	%ebx,		60(%esp)
	movl	%edx,		%ecx
	roll	$5,		%ecx
	leal	3395469782(%ebx,%ebp,1),%ebx
	addl	%ecx,		%ebx

	movl	%edx,		%ebp
	movl	(%esp),		%ecx
	rorl	$2,		%edx
	xorl	8(%esp),	%ecx
	xorl	%edi,		%ebp
	xorl	32(%esp),	%ecx
	xorl	%esi,		%ebp
	xorl	52(%esp),	%ecx
.byte 209
.byte 193	
	addl	%eax,		%ebp
	movl	%ecx,		(%esp)
	movl	%ebx,		%eax
	roll	$5,		%eax
	leal	3395469782(%ecx,%ebp,1),%ecx
	addl	%eax,		%ecx

	movl	%ebx,		%ebp
	movl	4(%esp),	%eax
	rorl	$2,		%ebx
	xorl	12(%esp),	%eax
	xorl	%edx,		%ebp
	xorl	36(%esp),	%eax
	xorl	%edi,		%ebp
	xorl	56(%esp),	%eax
.byte 209
.byte 192	
	addl	%esi,		%ebp
	movl	%eax,		4(%esp)
	movl	%ecx,		%esi
	roll	$5,		%esi
	leal	3395469782(%eax,%ebp,1),%eax
	addl	%esi,		%eax

	movl	%ecx,		%ebp
	movl	8(%esp),	%esi
	rorl	$2,		%ecx
	xorl	16(%esp),	%esi
	xorl	%ebx,		%ebp
	xorl	40(%esp),	%esi
	xorl	%edx,		%ebp
	xorl	60(%esp),	%esi
.byte 209
.byte 198	
	addl	%edi,		%ebp
	movl	%esi,		8(%esp)
	movl	%eax,		%edi
	roll	$5,		%edi
	leal	3395469782(%esi,%ebp,1),%esi
	addl	%edi,		%esi

	movl	%eax,		%ebp
	movl	12(%esp),	%edi
	rorl	$2,		%eax
	xorl	20(%esp),	%edi
	xorl	%ecx,		%ebp
	xorl	44(%esp),	%edi
	xorl	%ebx,		%ebp
	xorl	(%esp),		%edi
.byte 209
.byte 199	
	addl	%edx,		%ebp
	movl	%edi,		12(%esp)
	movl	%esi,		%edx
	roll	$5,		%edx
	leal	3395469782(%edi,%ebp,1),%edi
	addl	%edx,		%edi

	movl	%esi,		%ebp
	movl	16(%esp),	%edx
	rorl	$2,		%esi
	xorl	24(%esp),	%edx
	xorl	%eax,		%ebp
	xorl	48(%esp),	%edx
	xorl	%ecx,		%ebp
	xorl	4(%esp),	%edx
.byte 209
.byte 194	
	addl	%ebx,		%ebp
	movl	%edx,		16(%esp)
	movl	%edi,		%ebx
	roll	$5,		%ebx
	leal	3395469782(%edx,%ebp,1),%edx
	addl	%ebx,		%edx

	movl	%edi,		%ebp
	movl	20(%esp),	%ebx
	rorl	$2,		%edi
	xorl	28(%esp),	%ebx
	xorl	%esi,		%ebp
	xorl	52(%esp),	%ebx
	xorl	%eax,		%ebp
	xorl	8(%esp),	%ebx
.byte 209
.byte 195	
	addl	%ecx,		%ebp
	movl	%ebx,		20(%esp)
	movl	%edx,		%ecx
	roll	$5,		%ecx
	leal	3395469782(%ebx,%ebp,1),%ebx
	addl	%ecx,		%ebx

	movl	%edx,		%ebp
	movl	24(%esp),	%ecx
	rorl	$2,		%edx
	xorl	32(%esp),	%ecx
	xorl	%edi,		%ebp
	xorl	56(%esp),	%ecx
	xorl	%esi,		%ebp
	xorl	12(%esp),	%ecx
.byte 209
.byte 193	
	addl	%eax,		%ebp
	movl	%ecx,		24(%esp)
	movl	%ebx,		%eax
	roll	$5,		%eax
	leal	3395469782(%ecx,%ebp,1),%ecx
	addl	%eax,		%ecx

	movl	%ebx,		%ebp
	movl	28(%esp),	%eax
	rorl	$2,		%ebx
	xorl	36(%esp),	%eax
	xorl	%edx,		%ebp
	xorl	60(%esp),	%eax
	xorl	%edi,		%ebp
	xorl	16(%esp),	%eax
.byte 209
.byte 192	
	addl	%esi,		%ebp
	movl	%eax,		28(%esp)
	movl	%ecx,		%esi
	roll	$5,		%esi
	leal	3395469782(%eax,%ebp,1),%eax
	addl	%esi,		%eax

	movl	%ecx,		%ebp
	movl	32(%esp),	%esi
	rorl	$2,		%ecx
	xorl	40(%esp),	%esi
	xorl	%ebx,		%ebp
	xorl	(%esp),		%esi
	xorl	%edx,		%ebp
	xorl	20(%esp),	%esi
.byte 209
.byte 198	
	addl	%edi,		%ebp
	movl	%esi,		32(%esp)
	movl	%eax,		%edi
	roll	$5,		%edi
	leal	3395469782(%esi,%ebp,1),%esi
	addl	%edi,		%esi

	movl	%eax,		%ebp
	movl	36(%esp),	%edi
	rorl	$2,		%eax
	xorl	44(%esp),	%edi
	xorl	%ecx,		%ebp
	xorl	4(%esp),	%edi
	xorl	%ebx,		%ebp
	xorl	24(%esp),	%edi
.byte 209
.byte 199	
	addl	%edx,		%ebp
	movl	%edi,		36(%esp)
	movl	%esi,		%edx
	roll	$5,		%edx
	leal	3395469782(%edi,%ebp,1),%edi
	addl	%edx,		%edi

	movl	%esi,		%ebp
	movl	40(%esp),	%edx
	rorl	$2,		%esi
	xorl	48(%esp),	%edx
	xorl	%eax,		%ebp
	xorl	8(%esp),	%edx
	xorl	%ecx,		%ebp
	xorl	28(%esp),	%edx
.byte 209
.byte 194	
	addl	%ebx,		%ebp
	movl	%edx,		40(%esp)
	movl	%edi,		%ebx
	roll	$5,		%ebx
	leal	3395469782(%edx,%ebp,1),%edx
	addl	%ebx,		%edx

	movl	%edi,		%ebp
	movl	44(%esp),	%ebx
	rorl	$2,		%edi
	xorl	52(%esp),	%ebx
	xorl	%esi,		%ebp
	xorl	12(%esp),	%ebx
	xorl	%eax,		%ebp
	xorl	32(%esp),	%ebx
.byte 209
.byte 195	
	addl	%ecx,		%ebp
	movl	%ebx,		44(%esp)
	movl	%edx,		%ecx
	roll	$5,		%ecx
	leal	3395469782(%ebx,%ebp,1),%ebx
	addl	%ecx,		%ebx

	movl	%edx,		%ebp
	movl	48(%esp),	%ecx
	rorl	$2,		%edx
	xorl	56(%esp),	%ecx
	xorl	%edi,		%ebp
	xorl	16(%esp),	%ecx
	xorl	%esi,		%ebp
	xorl	36(%esp),	%ecx
.byte 209
.byte 193	
	addl	%eax,		%ebp
	movl	%ecx,		48(%esp)
	movl	%ebx,		%eax
	roll	$5,		%eax
	leal	3395469782(%ecx,%ebp,1),%ecx
	addl	%eax,		%ecx

	movl	%ebx,		%ebp
	movl	52(%esp),	%eax
	rorl	$2,		%ebx
	xorl	60(%esp),	%eax
	xorl	%edx,		%ebp
	xorl	20(%esp),	%eax
	xorl	%edi,		%ebp
	xorl	40(%esp),	%eax
.byte 209
.byte 192	
	addl	%esi,		%ebp
	movl	%eax,		52(%esp)
	movl	%ecx,		%esi
	roll	$5,		%esi
	leal	3395469782(%eax,%ebp,1),%eax
	addl	%esi,		%eax

	movl	%ecx,		%ebp
	movl	56(%esp),	%esi
	rorl	$2,		%ecx
	xorl	(%esp),		%esi
	xorl	%ebx,		%ebp
	xorl	24(%esp),	%esi
	xorl	%edx,		%ebp
	xorl	44(%esp),	%esi
.byte 209
.byte 198	
	addl	%edi,		%ebp
	movl	%esi,		56(%esp)
	movl	%eax,		%edi
	roll	$5,		%edi
	leal	3395469782(%esi,%ebp,1),%esi
	addl	%edi,		%esi

	movl	%eax,		%ebp
	movl	60(%esp),	%edi
	rorl	$2,		%eax
	xorl	4(%esp),	%edi
	xorl	%ecx,		%ebp
	xorl	28(%esp),	%edi
	xorl	%ebx,		%ebp
	xorl	48(%esp),	%edi
.byte 209
.byte 199	
	addl	%edx,		%ebp
	movl	%edi,		60(%esp)
	movl	%esi,		%edx
	roll	$5,		%edx
	leal	3395469782(%edi,%ebp,1),%edi
	addl	%edx,		%edi


	movl	128(%esp),	%ebp
	movl	12(%ebp),	%edx
	addl	%ecx,		%edx
	movl	4(%ebp),	%ecx
	addl	%esi,		%ecx
	movl	%eax,		%esi
	movl	(%ebp),		%eax
	movl	%edx,		12(%ebp)
	addl	%edi,		%eax
	movl	16(%ebp),	%edi
	addl	%ebx,		%edi
	movl	8(%ebp),	%ebx
	addl	%esi,		%ebx
	movl	%eax,		(%ebp)
	movl	132(%esp),	%esi
	movl	%ebx,		8(%ebp)
	addl	$64,		%esi
	movl	68(%esp),	%eax
	movl	%edi,		16(%ebp)
	cmpl	%eax,		%esi
	movl	%ecx,		4(%ebp)
	jb	.L000start
	addl	$108,		%esp
	popl	%edi
	popl	%ebx
	popl	%ebp
	popl	%esi
	ret
.L_sha1_block_asm_data_order_end:
.size	sha1_block_asm_data_order,.L_sha1_block_asm_data_order_end-sha1_block_asm_data_order
.ident	"sha1_block_asm_data_order"
.text
.globl	sha1_block_asm_host_order
.type	sha1_block_asm_host_order,@function
.align	16
sha1_block_asm_host_order:
	movl	12(%esp),	%ecx
	pushl	%esi
	sall	$6,		%ecx
	movl	12(%esp),	%esi
	pushl	%ebp
	addl	%esi,		%ecx
	pushl	%ebx
	movl	16(%esp),	%ebp
	pushl	%edi
	movl	12(%ebp),	%edx
	subl	$108,		%esp
	movl	16(%ebp),	%edi
	movl	8(%ebp),	%ebx
	movl	%ecx,		68(%esp)

	movl	(%esi),		%eax
	movl	4(%esi),	%ecx
	movl	%eax,		(%esp)
	movl	%ecx,		4(%esp)
	movl	8(%esi),	%eax
	movl	12(%esi),	%ecx
	movl	%eax,		8(%esp)
	movl	%ecx,		12(%esp)
	movl	16(%esi),	%eax
	movl	20(%esi),	%ecx
	movl	%eax,		16(%esp)
	movl	%ecx,		20(%esp)
	movl	24(%esi),	%eax
	movl	28(%esi),	%ecx
	movl	%eax,		24(%esp)
	movl	%ecx,		28(%esp)
	movl	32(%esi),	%eax
	movl	36(%esi),	%ecx
	movl	%eax,		32(%esp)
	movl	%ecx,		36(%esp)
	movl	40(%esi),	%eax
	movl	44(%esi),	%ecx
	movl	%eax,		40(%esp)
	movl	%ecx,		44(%esp)
	movl	48(%esi),	%eax
	movl	52(%esi),	%ecx
	movl	%eax,		48(%esp)
	movl	%ecx,		52(%esp)
	movl	56(%esi),	%eax
	movl	60(%esi),	%ecx
	movl	%eax,		56(%esp)
	movl	%ecx,		60(%esp)
	jmp	.L001shortcut
.L_sha1_block_asm_host_order_end:
.size	sha1_block_asm_host_order,.L_sha1_block_asm_host_order_end-sha1_block_asm_host_order
.ident	"sha1_block_asm_host_order"

#endif
