;DIRECCIONAMIENTO INDIRECTO
\\DATA 11 
ant		EQU 	-1
sig		EQU		1
;inicializo los registros
		mov 	eax, 1
		mov		ebx, 5
        mov     ecx, 2        
        mov     edx, 8        
;relleno la memoria de 1 a 10
otro:	cmp		eax, 7
		jz		sigue
		mov		[eax], eax
		add		eax, 1
		jmp 	otro
;recupero registros y multiplico x10
sigue:	mul 	[ecx+ant],10
		mul 	[ecx],10
		mul 	[ecx+sig],10
		mul 	[ebx-1],10
		mul 	[ebx],10
		mul 	[ebx-ant],10
        mov     [eax],[ecx-sig]
		push 	[eax]
		push 	[eax]
		mov 	BP, SP
		push 	[eax]
		add		[BP], 1
		add		[BP+1], 2
		add		[BP-1], 3
		mov		[edx], [bp]
		mov		[edx+sig], [bp+1]
		mov		[edx+2], [bp-1]
;imprime todo 
		mov 	eax, %1        
		mov		ecx, 10
		mov		edx, 1
		sys 	%2				
		sys		%F
		stop
		stop
		stop
		stop
		stop