#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "./parser.h"

const char *twoOp[] = {
    "mov","add","sub",
    "swap","mul","div","cmp",
    "shl","sht","and","or","xor"};
const char *oneoOp[] = {
    "sys","jmp","jz","jp","jn",
    "jnz","jnp","jnn","ldl",
    "ldh","rnd","not"};
const char *noOp[] = {"stop"};

int main(int argc, char *argv[])
{
    leerArchivo();
    return 0;
}

void leerArchivo(int instructtiones[]){
    FILE * arch = fopen("Fibonacci.txt", "r");
    char linea[100];
    int nroLinea = 0;

    /*
    parsed[0] = rotulo, parsed[1] mnemonico SIEMPRE != NULL si no se ignora, parsed[2] y [3] operandos, parsed[4] comment*/
    if (arch != NULL) {
        while (fgets(linea,sizeof linea, arch)!=NULL){
            char **parsed = parseline(linea);
            if (parsed[1]) {
            } else {
                if (!parsed[2] && !parsed[3] && parsed[4]){
                    printf(parsed[4]);
                } else {
                    printf("[ERROR] Operandos presentes en una línea sin instrucción");
                }
            }
        }
        fclose(arch);
    } else {
        printf("[ERROR] El archivo no existe");
    }
    nroLinea++;
}

int traduceMnemonico(char instruccion[]){
    for(int i=0; i<12; i++){
        if (strcmp(instruccion,twoOp[i])==0){
            return 0xi << 28;
        }
    }
    for(int j=0; j<12; j++){
        if (strcmp(instruccion,oneOp[j])== 0){
            return 0xFj << 24;
        }
    }
    for(int k=1; k<2; k++){
        if (strcmp(instruccion, noOp[k-1])==0){
            return 0xFFk << 20;
        }
    }
    return 0xFFFFFFFF;
}

