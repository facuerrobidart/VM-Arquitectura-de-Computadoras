/*DICCIONARIO DE MNEMONICOS: Son arrays que contienen los mnemonicos
ordenados según su código (respecto a la tabla de la especificación).
Estan separados por su cantidad de operandos.*/
const char *twoOp[] = {
    "mov","add","sub",
    "swap","mul","div","cmp",
    "shl","sht","and","or","xor"};
const char *oneOp[] = {
    "sys","jmp","jz","jp","jn",
    "jnz","jnp","jnn","ldl",
    "ldh","rnd","not"};
const char *noOp[] = {"stop"};

const char *registers[] = {
    "ds", "", "", "", "",
    "ip", "", "", "cc", "ac"
};
const char *standardRegisters[] = {
    "eax","ebx","ecx","edx","eex","efx",
    "ax","bx","cx","dx","ex","fx",
    "al","bl","cl","dl","el","fl",
    "ah","bh","ch","ch","eh","fh"
};

typedef struct Toperando {
    int tipo; //0=inmediato, 1=de registro, 2=directo
    int valor;
} Toperando;

typedef struct TRotulo {
    int nroLinea;
    char rotulo[30];
} TRotulo;
