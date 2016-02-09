#include"cabecera.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>


inline int ishex(int x)
{
	return	(x >= '0' && x <= '9')	||
		(x >= 'a' && x <= 'f')	||
		(x >= 'A' && x <= 'F');
}

int decode(const char *s, char *dec)
{
	char *o;
	const char *end = s + strlen(s);
	int c;
 
	for (o = dec; s <= end; o++) {
		c = *s++;
		if (c == '+') c = ' ';
		else if (c == '%' && (	!ishex(*s++)	||
					!ishex(*s++)	||
					!sscanf(s - 2, "%2x", &c)))
			return -1;
 
		if (dec) *o = c;
	}
 
	return o - dec;
}

int obtener_par(char *cadena,struct Par *pares,int *cant_pares,int *contador){
    if(cadena[*contador]=='\0')
        return 0;

    if(cadena[*contador]=='&')
        (*contador)++;

    char nombre[10000],dato[10000];

    int k=0;
    while(cadena[*contador]!='='){
        nombre[k++] = cadena[*contador];
        (*contador)++;
    }
    nombre[k]='\0';

    (*contador)++; //salto el '='

    k=0;
    while(cadena[*contador]!='&' && cadena[*contador]!='\0'){
        dato[k++] = cadena[*contador];
        (*contador)++;
    }
    dato[k]='\0';

    pares[*cant_pares].nombre = (char*)malloc(strlen(nombre)+1);
    pares[*cant_pares].valor = (char*)malloc(strlen(dato)+1);
    strcpy(pares[*cant_pares].nombre,nombre);
    decode(dato,pares[*cant_pares].valor);

    (*cant_pares)++;

    return 1;
}


int obtener_datos_post(struct Par *pares, int *cant_pares){
    if(getenv("REQUEST_METHOD")==NULL || strcmp(getenv("REQUEST_METHOD"),"POST")) {
        //printf("ERROR: Este programa debe ser llamado con METHOD POST.\n");
        return 1;
    }
    if(getenv("CONTENT_TYPE")==NULL ||strcmp(getenv("CONTENT_TYPE"),"application/x-www-form-urlencoded")) {
        //printf("ERROR: Este programa solo puede recibir datos de formularios. \n");
        return 1;
    }

    char cadena[10000];
    scanf("%s",cadena);

    //decode(cadena,data);
    //struct Par pares[MAXPARES];
    int /*cant_pares=0,*/contador=0;

    while(obtener_par(cadena,pares,cant_pares,&contador));

    return 0;
}

int obtener_datos_get(struct Par *pares, int *cant_pares){
    char *cadena;
    cadena = getenv("QUERY_STRING");

    if(cadena == NULL)
        return 1;

    int contador=0;

    while(obtener_par(cadena,pares,cant_pares,&contador));

    return 0;
}
