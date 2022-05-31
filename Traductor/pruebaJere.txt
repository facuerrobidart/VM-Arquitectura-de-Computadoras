BUFFER EQU 0

DISK_O EQU 0
DISK_D EQU 1

MSG_SECTOR EQU "Ingrese el sector inicial: "
MSG_COUNT EQU "Ingrese la cantidad de sectores: "

MSG_SUCCESS EQU "Disco clonado con exito!"
MSG_ERROR EQU "Error de clonacion!"

          LDH 3
          LDL MSG_SECTOR
          MOV EDX, AC
          MOV AX, %900
          SYS %4

          MOV EDX, 0
          MOV CX, 1
          MOV AX, %801
          SYS %1

          LDH 3
          LDL MSG_COUNT
          MOV EDX, AC
          MOV AX, %900
          SYS %4

          MOV EDX, 1
          MOV CX, 1
          MOV AX, %801
          SYS %1

          MOV EDX, [0]
          MOV ECX, [1]

          PUSH DISK_O ; disco origen
          PUSH EDX ; sector inicial
          PUSH ECX ; cant sectores
          CALL GENERATE
          ADD SP, 3
          
          CMP EAX, 0
          JNZ ERROR

          PUSH DISK_O ; disco origen
          PUSH DISK_D ; disco destino
          PUSH EDX ; sector inicial
          PUSH ECX ; cant sectores
          CALL CLONE
          ADD SP, 4
          
          CMP EAX, 0
          JNZ ERROR

          PUSH DISK_O ; disco origen
          PUSH EDX ; sector inicial
          PUSH ECX ; cant sectores
          CALL HASH
          ADD SP, 3
          
          CMP EAX, 0
          JNZ ERROR

          MOV EFX, EBX

          PUSH DISK_D ; disco destino
          PUSH EDX ; sector inicial
          PUSH ECX ; cant sectores
          CALL HASH
          ADD SP, 3
          
          CMP EAX, 0
          JNZ ERROR

          MOV [0], EFX
          MOV [1], EBX

          MOV AX, %801
          MOV CX, 2
          MOV EDX, 0
          SYS %2

          CMP [0], [1]
          JNZ ERROR

          LDH 3
          LDL MSG_SUCCESS
          MOV EDX, AC
          MOV AX, %800
          SYS %4

          STOP

ERROR:    LDH 3
          LDL MSG_ERROR
          MOV EDX, AC
          MOV AX, %800
          SYS %4

          STOP

PARAMS:   PUSH BP
          MOV BP, SP
          PUSH EBX
          PUSH EEX
          PUSH AC

          MOV DL, [BP+3] ; disco
          MOV AH, %08 ; params
          SYS %D

          MOV EAX, AH
          CMP EAX, 0
          JNZ END_P

          MOV EBX, [BP+2] ; sector inicial
          MOV EEX, DH ; cant sectores
          AND EEX, %0FF
          DIV EBX, EEX
          MOV DH, AC ; sector
          MOV EEX, CL ; cant cabezas
          AND EEX, %0FF
          DIV EBX, EEX
          MOV CL, AC ; cabeza
          MOV CH, EBX ; cilindro

END_P:    POP AC
          POP EEX
          POP EBX
          MOV SP, BP
          POP BP
          RET

READ:     PUSH BP
          MOV BP, SP
          PUSH EBX
          PUSH ECX
          PUSH EDX

          PUSH [BP+5] ; disco
          PUSH [BP+4] ; sector inicial
          CALL PARAMS
          ADD SP, 2
          
          CMP EAX, 0
          JNZ END_R

          MOV AL, [BP+3] ; cant sectores
          MOV AH, %02 ; leer
          MOV EBX, [BP+2] ; buffer
          SYS %D
          
          MOV EAX, AH

END_R:    POP EDX
          POP ECX
          POP EBX
          MOV SP, BP
          POP BP
          RET

WRITE:    PUSH BP
          MOV BP, SP
          PUSH EBX
          PUSH ECX
          PUSH EDX

          PUSH [BP+5] ; disco
          PUSH [BP+4] ; sector inicial
          CALL PARAMS
          ADD SP, 2
          
          MOV EAX, AH
          CMP EAX, 0
          JNZ END_W

          MOV AL, [BP+3] ; cant sectores
          MOV AH, %03 ; escribir
          MOV EBX, [BP+2] ; buffer
          SYS %D
          
          MOV EAX, AH

END_W:    POP EDX
          POP ECX
          POP EBX
          MOV SP, BP
          POP BP
          RET

GENERATE: PUSH BP
          MOV BP, SP
          PUSH EBX
          PUSH ECX
          PUSH EDX
          PUSH AC

          MOV ECX, [BP+2] ; cant sectores
          MUL ECX, 512 ; cant bytes
          DIV ECX, 4 ; cant celdas
          MOV EDX, BUFFER
          
          LDH %7FFF
          LDL %FFFF
          MOV EBX, AC

WHILE_G:  CMP ECX, EDX
          JNP END_G
          RND EBX
          MOV [EDX], AC
          ADD EDX, 1
          JMP WHILE_G

END_G:    PUSH [BP+4] ; disco
          PUSH [BP+3] ; sector inicial
          PUSH [BP+2] ; cant sectores
          PUSH BUFFER
          CALL WRITE
          ADD SP, 4

          POP AC
          POP EDX
          POP ECX
          POP EBX
          MOV SP, BP
          POP BP
          RET

CLONE:    PUSH BP
          MOV BP, SP

          PUSH [BP+5] ; disco origen
          PUSH [BP+3] ; sector inicial
          PUSH [BP+2] ; cant sectores
          PUSH BUFFER
          CALL READ
          ADD SP, 4
          
          CMP EAX, 0
          JNZ END_C

          PUSH [BP+4] ; disco destino
          PUSH [BP+3] ; sector inicial
          PUSH [BP+2] ; cant sectores
          PUSH BUFFER
          CALL WRITE
          ADD SP, 4

END_C:    MOV SP, BP
          POP BP
          RET

HASH:     PUSH BP
          MOV BP, SP
          PUSH ECX
          PUSH EDX
          PUSH AC

          PUSH [BP+4] ; disco
          PUSH [BP+3] ; sector inicial
          PUSH [BP+2] ; cant sectores
          PUSH BUFFER
          CALL READ
          ADD SP, 4
          
          CMP EAX, 0
          JNZ END_H

          MOV EBX, 0 ; hash
          MOV ECX, [BP+2] ; cant sectores
          MUL ECX, 512 ; cant bytes
          DIV ECX, 4 ; cant celdas
          MOV EDX, BUFFER

WHILE_H:  CMP ECX, EDX
          JNP END_H
          XOR EBX, [EDX]
          ADD EDX, 1
          JMP WHILE_H

END_H:    POP AC
          POP EDX
          POP ECX
          MOV SP, BP
          POP BP
          RET