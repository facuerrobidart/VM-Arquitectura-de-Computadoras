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

typedef struct Toperando {
    int tipo; //0=inmediato, 1=de registro, 2=directo
    int valor;
} Toperando;

int main(int argc, char *argv[])
{
    printf("%8X", parserNumeros("@-2"));
    return 0;
}

/*ATENCIÓN: C maneja los int automáticamente,
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
                    printf("[ERROR] Operandos presentes en una línea sin instrucción");
                }
            }
        }
        fclose(arch);
    } else {
        printf("[ERROR] El archivo no existe");
    }
}


/*validamos que los registros de operadores de registro
 tengan un nombre valido*/
int validaRegistro(char reg[]){
    return (strlen(reg) == 3 && reg[0] == 'e' && reg[1]>='a' && reg[1]<='e' && reg[2] == 'x')
    || (strlen(reg) == 2 && reg[0]>='a' && reg[0]<='e' && (reg[1] == 'h' || reg[1] =='l' || reg[1] =='x'));
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
Toperando convierteOperando(char operando[]){ //LOS OPERANDOS SE CONVIERTEN DE UNO EN UNO
    Toperando resultado;

    if (operando[0] == '[') { //OPERADOR DIRECTO
        char aux[strlen(operando)-2];
        int k = 0;
        for(size_t i=1; i < strlen(aux) - 1; i++){ //recorremos entre corchetes
            aux[k] = operando[i];
            k++;
        }
        resultado.tipo=2;
        resultado.valor=parserNumeros(aux);
    } else if ((65<=operando[0] && operando[0]<=90) || (97<=operando[0] && operando[0]<=122)) { //OPERADOR DE REGISTRO
        resultado.tipo = 1;

        char aux[strlen(operando)];
        strcpy(aux,operando);

        for(size_t l=0; l<strlen(aux); l++){
            aux[l] = aux[l] | 0x20; // convierto a minusculas caracter a caracter
        }

        if (!validaRegistro(aux)){
                printf("[WARNING] REGISTRO INVALIDO");
        }

        if (strlen(aux) == 3){ //REGISTRO EXTENDIDO
            char local[3] = "%";
            strcat(local, aux[1]);

            resultado.valor = (parserNumeros(local) & 0b001111); //los dos bits mas significativos señalan el subregistro seleccionado
        } else {
            char local[3] = "%";
            strcat(local, aux[0]); //el registro se referencia con el primer caracter si no es extendido
            if (aux[1] == 'x') {
                resultado.valor = (parserNumeros(local) & 0b111111);
            } else if (aux[1] == 'l') {
                resultado.valor = (parserNumeros(local) & 0b011111);
            } else {
                resultado.valor = (parserNumeros(local) & 0b101111);
            }
        }
    } else { //OPERADOR INMEDIATO
        resultado.tipo = 0;
        resultado.valor = parserNumeros(aux);
    }

    return resultado; //devuelve valor del operando y su tipo
}

/* la idea es recibir un número en formato string
y convertirlo a decimal int basados en la especificación de la MV */
int parserNumeros(char num[]){
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
    } else if (num[0] == '‘'){ //CARACTER ASCII
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
    if (arch != NULL) {
        while (fgets(linea,sizeof linea, arch)!=NULL){
            char **parsed = parseline(linea);
            if (parsed[1]) {
                longitudCS++;
            }
        }
        fclose(arch);
    } else {
        printf("[ERROR] El archivo no existe");
    }

    return longitudCS;
}

