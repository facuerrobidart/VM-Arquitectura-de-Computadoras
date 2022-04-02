#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "./parser.h"
#include <ctype.h>

/*DICCIONARIO DE MNEMONICOS: Son arrays que contienen los mnemonicos
ordenados seg�n su c�digo (respecto a la tabla de la especificaci�n).
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
    printf("%d", parserNumeros("�A"));
    return 0;
}

/*ATENCI�N: C maneja los int autom�ticamente,
solamente hay que tener cuidado de poner el formato
adecuado cuando se hace printf y scanf*/

void leerArchivo(int instructtiones[]){
    FILE * arch = fopen("Fibonacci.txt", 'r');
    char linea[100];
    int instruccion = 0x00000000;

    /*parsed[0] = rotulo, parsed[1] mnemonico SIEMPRE != NULL si no se ignora, parsed[2] y [3] operandos, parsed[4] comment*/
    if (arch != NULL) {
        while (fgets(linea,sizeof linea, arch)!=NULL){
            char **parsed = parseline(linea);
            if (parsed[1]) {
                instruccion = instruccion | traduceMnemonico(parsed[1]);
            } else {
                if (!parsed[2] && !parsed[3] && parsed[4]){
                    printf(parsed[4]);
                } else {
                    printf("[ERROR] Operandos presentes en una l�nea sin instrucci�n");
                }
            }
        }
        fclose(arch);
    } else {
        printf("[ERROR] El archivo no existe");
    }
}

/*Esta funcion traduce los mnemonicos de char a binario
Recibe un mnemonico y compara contra la lista de mnemonicos
definida en las constantes.
Los �ndices de estas listas se corresponden con el c�digo
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
    return 0xFFFFFFFF; // mnemonico inexistente
}

int traduceOperando(int codOp, char operando1[], char operando2[]){
    /*if (codOp & 0xF0000000 != 0xF0000000) { //operador con dos operandos

    } else if (codOp & 0xFF000000 != 0xFF000000){ //operador con un operando

    } else { //sin operandos
        return
    }*/
}

/*esta funcion determina el tipo de operando y devuelve el numero convertido*/
int convierteOperando(char operando[]){
    if (operando[0] == '[') { //OPERADOR DIRECTO
        char aux[];
        int k = 0;
        for(size_t i=1; i < strlen(aux) - 1; i++){ //recorremos entre corchetes
            aux[k] = operando[i];
            k++
        }
        return parserNumeros(aux);
    }
}

int parserNumeros(char num[]){
    /* la idea es recibir un n�mero en formato string
    y convertirlo a decimal int basados en la especificaci�n de la MV */
    char aux[100];
    int k = 0;
    if (num[0] == '@'){ //OCTAL
        for(size_t i=1; i<strlen(num); i++){
            aux[k] = num[i];
            k++;
        }
        return (int) strtol(aux, NULL, 8);
    } else if (num[0] == '%'){ //HEXA
        for(size_t i=1; i<strlen(num); i++){
            aux[k] = num[i];
            k++;
        }
        return (int) strtol(aux, NULL, 16);
    } else if (num[0] == '�'){ //CARACTER ASCII
        return (int) num[1];
    } else { //DECIMAL
        size_t startValue = num[0] == '#' ? 1 : 0;
        for(size_t i = startValue; i<strlen(num); i++){
            aux[k] = num[i];
            k++;
        }
        return (int) strtol(aux, NULL, 10);
    }
}

/*Devuelve la longitud del code segment, toma en cuenta las lineas
que poseen mnemonico, ya sea valido o no ya que todos son traducidos igualmente
Dado que es una longitud, el DS comienza en el valor de longitudCS*/

int calculaCS (char nombreArchivo[]){
    FILE * arch = fopen(nombreArchivo, "r+");
    char linea[200];
    int longitudCS = 0;
    /*parsed[0] = rotulo, parsed[1] mnemonico SIEMPRE != NULL si no se ignora, parsed[2] y [3] operandos, parsed[4] comment*/
    if (arch != NULL) {
        while (fgets(linea,sizeof linea, arch)!=NULL){
            char **parsed = parseline(linea);
            if (parsed[1]) {
                instruccion = instruccion | traduceMnemonico(parsed[1]);
                cs++;
            }
        }
        fclose(arch);
    } else {
        printf("[ERROR] El archivo no existe");
    }

    return longitudCS;
}

