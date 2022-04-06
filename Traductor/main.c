#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "./parser.h"
#include <ctype.h>
#include <math.h>

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

const char *registers[10] = {
    "ds", "", "", "", "",
    "ip", "", "", "cc", "ac"
};

typedef struct Toperando {
    int tipo; //0=inmediato, 1=de registro, 2=directo
    int valor;
} Toperando;

int main(int argc, char *argv[])
{
    leerArchivo();
    return 0;
}

/*ATENCIÓN: C maneja los int automáticamente,
solamente hay que tener cuidado de poner el formato
adecuado cuando se hace printf y scanf*/

void leerArchivo(){
    //Estamos probando traducir fibonnacci sin los jumps en el codigo (WIP)
    FILE * arch = fopen("prueba.txt", "r+");
    char linea[100] = "";
    int instruccion;
    /*parsed[0] = rotulo, parsed[1] mnemonico SIEMPRE != NULL si no se ignora, parsed[2] y [3] operandos, parsed[4] comment*/
    if (arch != NULL) {
        while (fgets(linea,sizeof linea, arch)!=NULL){
            char **parsed = parseline(linea);
            if (parsed[1]) {
                instruccion = generaInstruccion(traduceMnemonico(parsed[1]), parsed[2], parsed[3]);
                printf("%08X %s\n", instruccion, linea);
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


int generaInstruccion(int codOp, char operando1[], char operando2[]){
    Toperando res1;
    Toperando res2;
    int instruccion = 0x00000000;
    if ((codOp & 0xF0000000) != 0xF0000000) { //operador con dos operandos
        traduceOperando(operando1, &res1);
        traduceOperando(operando2, &res2);
        //seteo instruccion
        instruccion = (codOp & 0xF0000000);
        //seteo tipo operando
        instruccion = instruccion | ((res1.tipo << 26) & 0b00001100000000000000000000000000);
        instruccion = instruccion | ((res2.tipo << 24) & 0b00000011000000000000000000000000);
        //seteo valores de operando
        if (res1.valor > pow(2,12)){
            printf("[WARNING] Operando 1 truncado\n");
        }
        if (res2.valor > pow(2,12)){
            printf("[WARNING] Operando 2 truncado\n");
        }
        instruccion = instruccion | ((res1.valor << 12) & 0x00FFF000);
        instruccion = instruccion | ((res2.valor) & 0x00000FFF);
    } else if ((codOp & 0xFF000000) != 0xFF000000){ //operador con un operando
        traduceOperando(operando1, &res1);
        //seteo instruccion
        instruccion = (codOp & 0xFF000000);
        //seteo tipo operando
        instruccion = instruccion | ((res1.tipo << 22) & 0b00000000110000000000000000000000);
        //seteo valor de operando
        if (res1.valor > pow(2,16)){
            printf("[WARNING] Operando Truncado\n");
        }
        instruccion = instruccion | ((res1.valor) & 0x0000FFFF);
    } else { //sin operandos
        return codOp & 0xFFF00000;
    }
    return instruccion;
}

/*esta funcion determina el tipo de operando y devuelve el numero convertido*/
void traduceOperando(char operando[], Toperando *input){ //LOS OPERANDOS SE CONVIERTEN DE UNO EN UNO
    Toperando resultado = *input;
    resultado.valor = -100;
    resultado.tipo = NULL;

    if (operando[0] == '[') { //OPERADOR DIRECTO
        char aux[10] = "";
        int k = 0;
        int i = 0; //recorremos entre corchetes
        while (i < sizeof(operando)) {
            if (operando[i] != '[' && operando != ']') {
                aux[k] = operando[i];
                k++;
            }
            i++;
        }

        resultado.tipo=2;
        resultado.valor=parserNumeros(aux);
    } else if (('a'<=operando[0] && operando[0]<='z') || ('A'<=operando[0] && operando[0]<='Z')) { //OPERADOR DE REGISTRO
        resultado.tipo = 1;

        char aux[strlen(operando)];
        strcpy(aux,operando);

        for(size_t l=0; l<strlen(aux); l++){
            aux[l] = aux[l] | 0x20; // convierto a minusculas caracter a caracter
        }


        if (strlen(aux) == 3){ //REGISTRO EXTENDIDO
            char local[3] = "%";
            local[1] = aux[1];

            resultado.valor = (parserNumeros(local) & 0b001111); //los dos bits mas significativos señalan el subregistro seleccionado
        } else {
            char local[3] = "%";
            local[1] = aux[0]; //el registro se referencia con el primer caracter si no es extendido
            if (aux[1] == 'x') {
                resultado.valor = (parserNumeros(local) & 0b001111) | (0b110000);
            } else if (aux[1] == 'l') {
                resultado.valor = (parserNumeros(local) & 0b001111) | (0b010000);
            } else if (aux[1] == 'h') {
                resultado.valor = (parserNumeros(local) & 0b001111) | (0b100000);
            } else {
                //CASOS DS, IP, CC, AC
                int i = 0;
                while (i < 10 && strcmp(aux, registers[i]) != 0) {
                    i++;
                }
                if (i<10) {
                    resultado.valor = (i & 0b001111);
                }
            }
        }
        if (resultado.valor == -100){
            printf("[WARNING] REGISTRO INVALIDO %s", operando);
        }
    } else { //OPERADOR INMEDIATO
        char aux[strlen(operando)];
        strcpy(aux, operando);
        resultado.tipo = 0;
        resultado.valor = parserNumeros(aux);
    }

    *input = resultado;
}

/* la idea es recibir un número en formato string
y convertirlo a decimal int basados en la especificación de la MV */
int parserNumeros(char num[]){
    char aux[100]= "";
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
Dado que es una longitud, el DS comienza en el valor de longitudCS. CONISDERAR, ademas,las posiciones del header*/

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

