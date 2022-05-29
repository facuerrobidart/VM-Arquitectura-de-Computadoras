/*DICCIONARIO DE MNEMONICOS: Son arrays que contienen los mnemonicos
ordenados según su código (respecto a la tabla de la especificación).
Estan separados por su cantidad de operandos.*/
const char *twoOp[] = {
    "mov","add","sub",
    "swap","mul","div","cmp",
    "shl","shr","and","or","xor","slen","smov","scmp"};
const char *oneOp[] = {
    "sys","jmp","jz","jp","jn",
    "jnz","jnp","jnn","ldl",
    "ldh","rnd","not","push","pop","call"};
const char *noOp[] = {"ret","stop"};

const char *registers[] = {
    "ds", "ss", "es", "cs", "hp",
    "ip", "sp", "bp", "cc", "ac"
};
const char *standardRegisters[] = {
    "eax","ebx","ecx","edx","eex","efx",
    "ax","bx","cx","dx","ex","fx",
    "al","bl","cl","dl","el","fl",
    "ah","bh","ch","dh","eh","fh"
};

typedef struct Toperando {
    int tipo; //0=inmediato, 1=de registro, 2=directo
    int valor;
} Toperando;

typedef struct TRotulo {
    int nroLinea;
    char rotulo[30];
} TRotulo;

typedef struct TEquNumber {
    char nombre[11];
    int valor;
} TEquNumber;

typedef struct TEquString {
    char nombre[11];
    char valor[30];
    int offset;
} TEquString;
