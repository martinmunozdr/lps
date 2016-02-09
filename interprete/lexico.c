#include"cabecera.h"
#include<stdio.h>
#include<ctype.h>
#include<string.h>

int es_texto(char *linea, struct Token *token, int *contador){
    if(linea[*contador]!='\"')
        return 2;

    token->caracter = *contador;
    (*contador)++;
    int conta_contenido=0;

    while(linea[*contador]!='\"'){
        if(linea[*contador]=='\0'){
            printf("\nSe encontró cadena sin cerrar\n");
            return 1;
        }

        token->contenido[conta_contenido++]=linea[(*contador)++];
    }

    token->contenido[conta_contenido++]='\0';
    (*contador)++;

    return 0;
}

int es_numero(char *linea, struct Token *token, int *contador){
    if(!isdigit(linea[*contador]))
        return 2;

    int conta_contenido=0;
    int cant_puntos = 0;
    token->caracter = *contador;

    while(isdigit(linea[*contador]) || linea[*contador]=='.'){
        if(linea[*contador]=='.')
            cant_puntos++;

        if(cant_puntos>1){
            printf("\nSe encontró más de un punto en un número, en el caracter %d\n",*contador + 1);
            return 1;
        }

        token->contenido[conta_contenido++]=linea[(*contador)++];
    }

    token->contenido[conta_contenido++]='\0';

    return 0;
}

int es_operador(char *linea, struct Token *token, int *contador){
    token->caracter = *contador;

    if(linea[*contador]=='=' && linea[*contador + 1]=='='){
        strcpy(token->contenido,"==");
        (*contador)+=2;
        return 0;
    }

    if(linea[*contador]=='>' && linea[*contador + 1]=='='){
        strcpy(token->contenido,">=");
        (*contador)+=2;
        return 0;
    }

    if(linea[*contador]=='<' && linea[*contador + 1]=='='){
        strcpy(token->contenido,"<=");
        (*contador)+=2;
        return 0;
    }

    if(linea[*contador]=='!' && linea[*contador + 1]=='='){
        strcpy(token->contenido,"!=");
        (*contador)+=2;
        return 0;
    }

    if(linea[*contador]=='='){
        strcpy(token->contenido,"=");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]=='>'){
        strcpy(token->contenido,">");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]=='<'){
        strcpy(token->contenido,"<");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]=='+'){
        strcpy(token->contenido,"+");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]=='-'){
        strcpy(token->contenido,"-");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]=='*'){
        strcpy(token->contenido,"*");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]=='/'){
        strcpy(token->contenido,"/");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]=='^'){
        strcpy(token->contenido,"^");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]==','){
        strcpy(token->contenido,",");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]==';'){
        strcpy(token->contenido,";");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]=='('){
        strcpy(token->contenido,"(");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]==')'){
        strcpy(token->contenido,")");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]=='['){
        strcpy(token->contenido,"[");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]==']'){
        strcpy(token->contenido,"]");
        (*contador)++;
        return 0;
    }

    if(linea[*contador]=='.'){
        strcpy(token->contenido,".");
        (*contador)++;
        return 0;
    }

    return 2;
}

int es_id_o_reservada(char *linea, struct Token *token, int *contador){
    if(!isalpha(linea[*contador]))
        return 2;

    int conta_contenido=0;
    token->caracter = *contador;

    while(isdigit(linea[*contador]) || isalpha(linea[*contador]) || linea[*contador]=='_'){
        token->contenido[conta_contenido++]=linea[(*contador)++];
    }

    token->contenido[conta_contenido++]='\0';
    token->tipo = 'I';

    if(strcmp(token->contenido,"CLASE")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"METODO")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"FIN_METODO")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"ATRIBUTOS")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"IMPORTAR")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"SUBRUTINA")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"FIN_SUBRUTINA")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"SI")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"MIENTRAS")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"SINO")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"SINO_SI")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"CONTINUAR")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"ROMPER")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"DEVOLVER")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"FIN_SI")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"FIN_MIENTRAS")==0){
        token->tipo='R';
    }
    else if(strcmp(token->contenido,"NEGACION")==0){
        token->tipo='O';
    }
    else if(strcmp(token->contenido,"RESTO")==0){
        token->tipo='O';
    }
    else if(strcmp(token->contenido,"DIV_ENTERA")==0){
        token->tipo='O';
    }
    else if(strcmp(token->contenido,"Y")==0){
        token->tipo='O';
    }
    else if(strcmp(token->contenido,"O")==0){
        token->tipo='O';
    }

    return 0;
}

int analisis_lexico(char *linea, struct Token *tokens, int *cant_tokens){
    int contador = 0, rpta;
    struct Token token;
    *cant_tokens = 0;

    while(linea[contador]!='\0'){
        while(linea[contador]==' ')
            contador++;
        if(linea[contador]=='\0')
            break;

        rpta = es_texto(linea,&token,&contador);
        if(rpta==0){ //Se encontró texto
            token.tipo = 'T';
            tokens[(*cant_tokens)++] = token;
            continue;
        }
        else if(rpta==1){ //Se encontró error
            return 1;
        }

        while(linea[contador]==' ')
            contador++;
        if(linea[contador]=='\0')
            break;

        rpta = es_numero(linea,&token,&contador);
        if(rpta==0){ //Se encontró numero
            token.tipo = 'N';
            tokens[(*cant_tokens)++] = token;
            continue;
        }
        else if(rpta==1){ //Se encontró error
            return 1;
        }

        while(linea[contador]==' ')
            contador++;
        if(linea[contador]=='\0')
            break;

        rpta = es_operador(linea,&token,&contador);
        if(rpta==0){ //Se encontró operador
            token.tipo = 'O';
            tokens[(*cant_tokens)++] = token;
            continue;
        }
        else if(rpta==1){ //Se encontró error
            return 1;
        }

        while(linea[contador]==' ')
            contador++;
        if(linea[contador]=='\0')
            break;

        rpta = es_id_o_reservada(linea,&token,&contador);
        if(rpta==0){ //Se encontró operador
            tokens[(*cant_tokens)++] = token;
            continue;
        }
        else if(rpta==1){ //Se encontró error
            return 1;
        }

        if(linea[contador]=='#')
            return 0;

        printf("\nERROR. Se encontró caracter inesperado en caracter %d\n",contador+1);
        return 1;
    }   

    return 0;
}
