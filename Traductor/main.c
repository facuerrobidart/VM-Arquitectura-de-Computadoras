#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "./parser.h"
#include <ctype.h>

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

int main(int argc, char *argv[])
{
    printf("%2X\n",traduceMnemonico("MUL"));
    printf("%2X\n",traduceMnemonico("JNN"));
    printf("%2X\n",traduceMnemonico("stop"));
    return 0;
}

void leerArchivo(int instructtiones[]){
    FILE * arch = fopen("Fibonacci.txt", "r");
    char linea[100];
    int nroLinea = 0;

    /*parsed[0] = rotulo, parsed[1] mnemonico SIEMPRE != NULL si no se ignora, parsed[2] y [3] operandos, parsed[4] comment*/
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

/*Esta funcion traduce los mnemonicos de char a binario
Recibe un mnemonico y compara contra la lista de mnemonicos
definida en las constantes.
Los índices de estas listas se corresponden con el código
de cada mnemonico especificado en la tabla*/

int traduceMnemonico(char instruccion[]){
    /*Pasamos a minusculas todas los mnemonicos
    para evitar errores de parseo por mayusculas y minusculas*/
    char aux[strlen(instruccion)];
    strcpy(aux, instruccion); //copio mnemonico
    for(size_t l=0; l<strlen(aux); l++){
        aux[l] = aux[l] | 0x20; // convierto a minusculas caracter a caracter
    }

    for(int i=0; i<12; i++){ //verifica si es de dos operandos
        if (strcmp(aux,twoOp[i])==0){
            return i << 28;
        }
    }
    for(int j=0; j<12; j++){ //verifica si es de un operando
        if (strcmp(aux,oneOp[j])== 0){
            return j << 24 | 0xF0000000;
        }
    }
    for(int k=1; k<2; k++){ //verifica si es sin operandos
        if (strcmp(aux, noOp[k-1])==0){
            return k << 20 | 0xFF000000;
        }
    }
    return 0xFFFFFFFF;
}

