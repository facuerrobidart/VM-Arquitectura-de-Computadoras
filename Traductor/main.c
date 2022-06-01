#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "parser.h"
#include "pattern.h"
#include <ctype.h>
#include <math.h>
#include "diccionario.h"


int calculaCS(char nombreArchivo[],
              TRotulo rotulos[],
              TEquNumber equs[],
              TEquString equsString[],
              int *nRotulos, int *nEqus, int *nEquString,int *es, int *ss, int *ds, int *csSinEqu,int *yaExiste);
char *stringBinario(int num);

int main(int argc, char *argv[])
{
    if (argv[1] && argv[2]) {
        leerArchivo(argv[1], argv[2],(argv[3] == NULL || strcmp("-o", argv[3]) != 0));
    } else {
        printf("[ERROR] Faltan argumentos\n");
    }
    return 0;
}

/*ATENCI�N: C maneja los int autom�ticamente,
solamente hay que tener cuidado de poner el formato
adecuado cuando se hace printf y scanf*/

void leerArchivo(char nombreArchivo[], char nombreOutput[], int mostrar){
    TRotulo rotulos[100];
    TEquNumber equsNumber[100];
    TEquString equsString[100];
    int cantidadRotulos = 0, nEqusNumber = 0, nEqusString = 0, ss = 0, es = 0, ds = 0, csSinEqus = 0,yaExisteEqu=0,borrar=0;

    if (!strstr(nombreOutput, ".mv2")) { //si no nos pasan la extension en los argumentos del programa
        strcat(nombreOutput, ".mv2"); //le agregamos la extension
    }

    int longCS = calculaCS(nombreArchivo, rotulos, equsNumber, equsString, &cantidadRotulos, &nEqusNumber, &nEqusString, &es, &ss, &ds, &csSinEqus,&yaExisteEqu);
    FILE * output = fopen(nombreOutput, "w+");

    //HEADERS DEL BINARIO
    fprintf(output, "%s\n", "01001101010101100010110100110010"); //MV-2
    fprintf(output, PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(ds)); //longitudes de segments
    fprintf(output, PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(ss));
    fprintf(output, PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(es));
    fprintf(output, PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(longCS));
    fprintf(output, "%s\n", "01010110001011100011001000110010"); //V.22
    //FIN HEADERS

    FILE * arch = fopen(nombreArchivo, "r+");
    char linea[400] = "";
    int instruccion;
    int nroLinea = 0;
    /*parsed[0] = rotulo, parsed[1] mnemonico SIEMPRE != NULL si no se ignora, parsed[2] y [3] operandos, parsed[4] comment*/
    while (fgets(linea,sizeof linea, arch) != NULL){
        char **parsed = parseline(linea);
        if (parsed) {
            if (parsed[1]) {
                int traducido = traduceMnemonico(parsed[1],&borrar);
                instruccion = generaInstruccion(traducido, parsed[2], parsed[3], rotulos, cantidadRotulos,
                                                equsNumber, nEqusNumber,
                                                equsString, nEqusString,
                                                csSinEqus,&borrar);
                if (mostrar) {
                    printf("[%04d] %08X %s\n", nroLinea, instruccion, linea);
                }
                fprintf(output, PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(instruccion));
                nroLinea++;
             } else {
                if (!parsed[2] && !parsed[3]){
                    printf("%s\n", linea);
                } else {
                    printf("[ERROR] Operandos presentes en una linea sin instruccion");
                }
            }
        }
    }
    fclose(arch);

    for (int i = 0; i < nEqusString; i++) {
        for (size_t k = 0; k < strlen(equsString[i].valor); k++) { //pasamos char a char los string definidos con directivas EQU
            fprintf(output, PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32((int) equsString[i].valor[k]));
        }
        fprintf(output, PRINTF_BINARY_PATTERN_INT32"\n", PRINTF_BYTE_TO_BINARY_INT32(0));
    }
     fclose(output);
     if(yaExisteEqu || borrar)//si hay al menos un EQU repetido, no se genera archivo de salida
        remove(nombreOutput);
}

int esRegistro(char registro[]){
    for(int i=0; i < 24; i++){
        if (strcmp(registro, standardRegisters[i]) == 0){
            return 1;
        }
    }
    for(int j=0; j<10; j++){
        if (strcmp(registro, registers[j]) == 0){
            return 1;
        }
    }
    return 0;
}

/*Esta funcion traduce los mnemonicos de char a binario
Recibe un mnemonico y compara contra la lista de mnemonicos
definida en las constantes.
Los �ndices de estas listas se corresponden con el c�digo
de cada mnemonico especificado en la tabla*/

int traduceMnemonico(char instruccion[],int* borra){
    /*Pasamos a minusculas todas los mnemonicos
    para evitar errores de parseo por mayusculas y minusculas*/
    char aux[strlen(instruccion)];
    strcpy(aux, instruccion); //copio mnemonico
    for(size_t l=0; l<strlen(aux); l++){
        aux[l] = aux[l] | 0x20; // convierto a minusculas caracter a caracter
    }

    for(int i=0; i< 15; i++){ //verifica si es de dos operandos
        if (strcmp(aux,twoOp[i])==0){
            return i << 28;
        }
    }
    for(int j=0; j<15; j++){ //verifica si es de un operando
        if (strcmp(aux,oneOp[j])== 0){
            return j << 24 | 0xF0000000;
        }
    }
    for(int k=0; k<2; k++){ //verifica si es sin operandos
        if (strcmp(aux, noOp[k])==0){
            return k << 20 | 0xFF000000;
        }
    }
     *borra=1;
     printf("\n[ERROR] mnemonico desconocido: '%s'\n",aux);
    return 0xFFFFFFFF; // mnemonico inexistente
}


int generaInstruccion(int codOp, char operando1[], char operando2[],
                      TRotulo rotulos[], int cantRotulos,
                      TEquNumber equsNumber[], int nEqusNumber,
                      TEquString equsString[], int nEqusString, int csSinEqus,int *borrar){
    Toperando res1;
    Toperando res2;
    int instruccion = 0x00000000;
    if ((codOp & 0xF0000000) != 0xF0000000) { //operador con dos operandos
        traduceOperando(operando1, &res1, rotulos, cantRotulos, equsNumber, nEqusNumber, equsString, nEqusString, csSinEqus,borrar);
        traduceOperando(operando2, &res2, rotulos, cantRotulos, equsNumber, nEqusNumber, equsString, nEqusString, csSinEqus,borrar);
        //seteo instruccion
        instruccion = (codOp & 0xF0000000);
        //seteo tipo operando
        instruccion = instruccion | ((res1.tipo << 26) & 0b00001100000000000000000000000000);
        instruccion = instruccion | ((res2.tipo << 24) & 0b00000011000000000000000000000000);
        //seteo valores de operando
        if (abs(res1.valor) > pow(2,11)){
            printf("[WARNING] Operando 1 truncado\n");
        }
        if (abs(res2.valor) > pow(2,11)){
            printf("[WARNING] Operando 2 truncado\n");
        }
        instruccion = instruccion | ((res1.valor << 12) & 0x00FFF000);
        instruccion = instruccion | ((res2.valor) & 0x00000FFF);
    } else if ((codOp & 0xFF000000) != 0xFF000000){ //operador con un operando
        traduceOperando(operando1, &res1, rotulos, cantRotulos, equsNumber, nEqusNumber, equsString, nEqusString, csSinEqus,borrar);
        //seteo instruccion
        instruccion = (codOp & 0xFF000000);
        //seteo tipo operando
        instruccion = instruccion | ((res1.tipo << 22) & 0b00000000110000000000000000000000);
        //seteo valor de operando
        if (abs(res1.valor) > pow(2,15)){
            printf("[WARNING] Operando Truncado\n");
        }
        instruccion = instruccion | ((res1.valor) & 0x0000FFFF);
    } else { //sin operandos
        return codOp & 0xFFF00000;
    }
    return instruccion;
}

int equNumerico(char *simbolo, TEquNumber equsNumber[], int nEqusNumber, int *existeEqu){
    int idx = 0;

    while (idx < nEqusNumber && !(*existeEqu)){
        if (strcmp(simbolo, equsNumber[idx].nombre) == 0){ //si el operando estaba definido en un EQU, lo trato como un operando inmediato
            *existeEqu = 1;
            return equsNumber[idx].valor;
        }
        idx++;
    }
    return 0;
}



int equString(char *simbolo, TEquString equsString[], int nEqusString, int *existeEqu){
     int idx=0;
    while (idx < nEqusString && !(*existeEqu)){
        if (strcmp(simbolo, equsString[idx].nombre) == 0){ //si el operando estaba definido en un EQU, lo trato como un operando inmediato
            *existeEqu = 1;
             return equsString[idx].offset;
        }
        idx++;
    }
    return 0;

}

/*esta funcion determina el tipo de operando y devuelve el numero convertido*/
void traduceOperando(char operando[], Toperando *input, TRotulo rotulos[], int cantRotulos,
                     TEquNumber equsNumber[], int nEqusNumber,
                     TEquString equsString[], int nEqusString, int csSinEqus,int *borrar){ ///LOS OPERANDOS SE CONVIERTEN DE UNO EN UNO

    Toperando resultado = *input;
    resultado.valor = -100;
    resultado.tipo = NULL;
    int offsetNegativo = 0;


    if (operando[0] == '[') { ///OPERADOR DIRECTO
        char aux[30] = "";
        int k = 0;
        size_t i = 0; //recorremos entre corchetes

        while (i < strlen(operando)) {
            if (operando[i] != '[' && operando[i] != ']') {
                aux[k] = operando[i];
                k++;
            }
            i++;
        }

        if (('a'<= aux[0] && aux[0] <= 'z') || ('A' <= aux[0] && aux[0] <= 'Z')) {///OPERANDO INDIRECTO
                size_t longitud = 0;
                char op1[4] = "";
                char op2[11] = "";
                int idx = 0;

                for(size_t l=0; l<strlen(aux); l++){
                    if (aux[l] != '+' && aux[l] != '-')
                        aux[l] = aux[l] | 0x20; // convierto a minusculas caracter a caracter
                }

                while (longitud < strlen(aux) && aux[longitud] != '+' && aux[longitud] != '-'  ){
                    op1[idx] = aux[longitud];
                    idx++; longitud++;
                }

                if (aux[longitud] == '+' || aux[longitud] == '-'){ //si es un signo positivo lo omito
                    if(aux[longitud] == '-')
                        offsetNegativo=1;
                   // if(aux[longitud]=='+')
                        longitud++;
                }

                int p=0;


                while (longitud < sizeof(aux) && aux[longitud]!= NULL){
                    op2[p] = aux[longitud];
                    p++; longitud++;
                }

                resultado.tipo = 3; //OPERANDO INDIRECTO

                 if(strlen(op1)==2 ){
                    //CASOS DS, IP, CC, AC
                    int indice = 0;

                    while (indice < 10 && strcmp(op1, registers[indice]) != 0) {
                        indice++;
                    }
                    if (indice<10) {
                        resultado.valor = (indice & 0x00F);
                    }
                }else {
                    char opAux[3]="%";
                    opAux[1]=op1[1];
                    resultado.valor = parserNumeros(opAux);
                }
                if (strcmp("", op2) != 0){ //implica la existencia de un offset
                    int encontreEq = 0;
                    int res = equNumerico(op2, equsNumber, nEqusNumber, &encontreEq);
                    if(offsetNegativo)
                        res*=(-1);
                    int preVal;
                    if (encontreEq) {
                        resultado.valor = (resultado.valor | (res << 4) );
                    }
                    else {
                        res=equString(op2, equsString, nEqusString, &encontreEq);
                        if (encontreEq) {
                          resultado.valor = (resultado.valor | ( ((csSinEqus+res)&0xFF) << 4));
                          //resultado.tipo=3;
                        } else if (('a'<= op2[0] && op2[0] <= 'z')){
                           printf("\n[ERROR] simbolo inexistente : '%s'\n",op2);
                        } else {//si es un numero
                            int parseo=parserNumeros(op2);
                            //printf("%d\n",parseo);
                            //printf("%d\n",resultado.valor);
                            if(offsetNegativo)
                               parseo*=(-1);
                            resultado.valor = resultado.valor | (parseo << 4 );
                        }
                    }
                }
                else{//caso [EQU]
                      int findEqu = 0;
                      int res = equNumerico(op1, equsNumber, nEqusNumber, &findEqu);

                      if (findEqu) {
                          resultado.tipo = 2;
                          resultado.valor = res;
                      }
                      else {

                           res=equString(op1, equsString, nEqusString, &findEqu);
                           if (findEqu) {
                             resultado.tipo = 2;//marco el string como inmediato
                             resultado.valor = (csSinEqus+res) & 0xFF;
                           }
                      }
                }
        } else {
            resultado.tipo=2;
            resultado.valor=parserNumeros(aux);
        }
    } else if (('a'<=operando[0] && operando[0]<='z') || ('A'<=operando[0] && operando[0]<='Z')) {//CASO REGISTROS
        int encontreEqu = 0;
        char aux[strlen(operando)];
        int idx = 0;
        strcpy(aux,operando);

        for(size_t l=0; l<strlen(aux); l++){
            aux[l] = aux[l] | 0x20; // convierto a minusculas caracter a caracter
        }


        int valor = equNumerico(aux, equsNumber, nEqusNumber, &encontreEqu);
        if (encontreEqu) {
            resultado.tipo = 0;
            resultado.valor = valor;
        }else{
            valor = equString(aux, equsString, nEqusString, &encontreEqu);///FACU FIJATE QUE ACA ES DONDE DA RARO. DEBERIA DEVOLVER 10 Y DEVUELVE
            if (encontreEqu) {
                resultado.tipo = 0;
                resultado.valor = csSinEqus + valor;
                //printf("%d\n", resultado.valor);
            }
        }
        idx = 0;
        while (idx < nEqusString && !encontreEqu) {
            if (strcmp(aux, equsString[idx].nombre) == 0){
                resultado.tipo = 2; //OPERANDO DIRECTO a la posicion del 1er caracter en el CS
                resultado.valor = csSinEqus + equsString[idx].offset;
                encontreEqu = 1;
            }
            idx++;
        }

        if (!encontreEqu) {
            resultado.tipo = 1;
            if (esRegistro(aux)) { ///OPERADOR DE REGISTRO
                if (strlen(aux) == 3){ //REGISTRO EXTENDIDO
                    char local[3] = "%";
                    local[1] = aux[1];

                    resultado.valor = (parserNumeros(local) & 0b001111); //los dos bits mas significativos se�alan el subregistro seleccionado
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
            } else { ///tenemos que buscar el rotulo al cual saltar
                resultado.tipo=0;
                int i = 0;
                while (i<cantRotulos && strcmp(operando, rotulos[i].rotulo) != 0) {
                    i++;
                }
                if (i < cantRotulos){
                    resultado.valor = rotulos[i].nroLinea;
                } else{
                    printf("[ERROR] simbolo no encontrado: %s\n", operando);//contempla tanto el caso de rotulos como de EQU
                    *borrar=1;

                }
            }
        }


    } else { ///OPERADOR INMEDIATO
        char aux[strlen(operando)];
        strcpy(aux, operando);
        resultado.tipo = 0;
        resultado.valor = parserNumeros(aux);
    }

    *input = resultado;
}



/* la idea es recibir un n�mero en formato string
y convertirlo a decimal int basados en la especificaci�n de la MV */
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
    } else if (num[0] == '\''){ //CARACTER ASCII
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


void simboloRepetido(char *nombre,TEquNumber equs[], TEquString equsString[], TRotulo rotulos[],int nEqus, int nEquString, int nRotulos, int *resultado){
  int i=0;

  while(!(*resultado) && i<nEquString){
    if(strcmp(nombre,equsString[i].nombre)==0){
      *resultado = 1;
    }
    i++;
  }

  i=0;

  while(!(*resultado) && i<nEqus){
    if(strcmp(nombre,equs[i].nombre)==0){
      *resultado = 1;
    }
    i++;
  }

  i=0;

  while (!(*resultado) && i < nRotulos){
    if (strcmp(nombre, rotulos[i].rotulo) == 0){
      *resultado = 1;
    }
    i++;
  }
}


/*Devuelve la longitud del code segment, toma en cuenta las lineas
que poseen mnemonico, ya sea valido o no ya que todos son traducidos igualmente
Dado que es una longitud, el DS comienza en el valor de longitudCS. CONISDERAR, ademas,las posiciones del header*/

int calculaCS(char nombreArchivo[],
              TRotulo rotulos[],
              TEquNumber equs[],
              TEquString equsString[],
              int *nRotulos, int *nEqus, int *nEquString, int *es, int *ss, int *ds, int *csSinEqu,int *yaExisteEqu){
    FILE * arch = fopen(nombreArchivo, "r+");
    char linea[200];
    int longitudCS = 0;
    int cantRotulos = 0;
    int offset = 0;

    *es = 1024; *ss = 1024; *ds = 1024; *csSinEqu = 0;

    if (arch != NULL) {
        while (fgets(linea,sizeof linea, arch)!=NULL) {

            char **parsed = parseline(linea);
            if (parsed[0]){ //chequea rotulo
                simboloRepetido(parsed[0],equs,equsString,rotulos, *nEqus, *nEquString, cantRotulos, yaExisteEqu);
                if(*yaExisteEqu){
                    printf("\n [ERROR] Simbolo repetido : %s \n", parsed[0]);
                }

                strcpy(rotulos[cantRotulos].rotulo, parsed[0]);
                rotulos[cantRotulos].nroLinea = (*csSinEqu);
                cantRotulos++;
            }

            if (parsed[1]) {
                longitudCS++;
                (*csSinEqu)++;
            }


            if (parsed[5]){ //chequea las directivas
                if (strcmp(parsed[5], "DATA") == 0)
                    *ds = parserNumeros(parsed[6]);
                else
                    if (strcmp(parsed[5], "EXTRA") == 0)
                        *es = parserNumeros(parsed[6]);
                    else
                        if (strcmp(parsed[5], "STACK") == 0)
                            *ss = parserNumeros(parsed[6]);
            }

            if (parsed[7]) { //chequea un EQU
                char *nombre =  parsed[7];

                for(size_t l=0; l<strlen(nombre); l++){
                    nombre[l] = nombre[l] | 0x20; // convierto a minusculas caracter a caracter
                }
                simboloRepetido(nombre,equs,equsString,rotulos, *nEqus, *nEquString, cantRotulos, yaExisteEqu);

                if(*yaExisteEqu){
                    printf("\n [ERROR] Simbolo repetido : %s \n",nombre);
                }

                if (parsed[8][0] != '"') { //si no es un string, el valor es un numero
                    strcpy(equs[*nEqus].nombre, nombre); //copio nombre de equ
                    equs[*nEqus].valor = parserNumeros(parsed[8]);
                    (*nEqus)++;
                } else {
                    strcpy(equsString[*nEquString].nombre, nombre);
                    strcpy(equsString[*nEquString].valor, "");

                    for (size_t i = 1; i < strlen(parsed[8]) - 1 ; i++){
                        equsString[*nEquString].valor[i-1] = parsed[8][i];
                    }

                    equsString[*nEquString].offset = offset;
                    //printf(" %d      ",equsString[*nEquString].offset);
                    offset += strlen(equsString[*nEquString].valor) + 1; //tenemos que contar el caracter terminator
                    longitudCS += strlen(parsed[8]) - 1;
                    (*nEquString)++;
                }
            }
        }

        fclose(arch);
        *nRotulos = cantRotulos;
    } else {
        printf("[ERROR] El archivo no existe\n");
    }

    return longitudCS;
}

char *stringBinario(int num){
    char resultado[33];
    itoa(num, resultado, 2);
    return resultado;
}
