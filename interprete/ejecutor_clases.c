#include"cabecera.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>


int copiar_atributos_vacios(struct ElementoTabla **TablaSimbolos,struct ElementoTabla **TablaSimbolosOrig){
    int cant=cantidad_claves(TablaSimbolosOrig);
    if(cant==0)
       return 0;

    char **claves = devolver_claves(TablaSimbolosOrig,cant);
    struct Valor vacio;
    vacio.tipo='V';
    vacio.referencia=NULL;
    int i=0;
    for(i=0;i<cant;i++)
        asignar_elemento_tabla_simbolos(claves[i],vacio,TablaSimbolos);

    free(claves);

    return 0;
}

int copiar_metodos(struct ElementoTablaFunciones **TablaSimbolos,struct ElementoTablaFunciones **TablaSimbolosOrig){
    int cant=cantidad_funciones(TablaSimbolosOrig),error=0;
    if(cant==0)
       return 0;

    char **claves = devolver_nombres_funciones(TablaSimbolosOrig,cant);

    int i=0;
    for(i=0;i<cant;i++){
        asignar_elemento_tabla_simbolos_funciones(claves[i],
           recuperar_elemento_tabla_simbolos_funciones(claves[i], &error,TablaSimbolosOrig)
           ,TablaSimbolos,&error
        );
        if(error<0)return error;
    }

    free(claves);

    return 0;
}


void llenar_referencia_objeto(struct ElementoTablaFunciones **TablaSimbolos,struct Clase *clase){
    int i=0;
    struct ElementoTablaFunciones *elem;

    for(i=0;i<TAMANO_TABLA;i++)
        if(TablaSimbolos[i]!=NULL){
            elem = TablaSimbolos[i];
            elem->funcion.objeto_pertenece = clase;
            while(elem->sgte!=NULL){elem=elem->sgte; elem->funcion.objeto_pertenece = clase;}
        }
}

void inicializar_tabla_clase(struct ElementoTablaClases **TablaSimbolos){
    int i=0;
    for(;i<TAMANO_TABLA;i++)
        TablaSimbolos[i]=NULL;
}

void limpiar_clase(struct Clase clase){
    free(clase.nombre);
    limpiar_tabla(clase.atributos);
    limpiar_tabla_funciones(clase.metodos);
}

void limpiar_elemento_tabla_clases(struct ElementoTablaClases **elemento){
    free((*elemento)->nombre);

    limpiar_clase((*elemento)->clase);

    if((*elemento)->sgte!=NULL)
        limpiar_elemento_tabla_clases(&(*elemento)->sgte);

    free(*elemento);
}

void limpiar_tabla_clases(struct ElementoTablaClases **TablaSimbolos){
    int i=0;
    for(;i<TAMANO_TABLA;i++)
        if(TablaSimbolos[i]!=NULL)
            limpiar_elemento_tabla_clases(&TablaSimbolos[i]);
}

void llenar_elemento_clases(char *nombre, struct Clase clase,struct ElementoTablaClases **elemento){
    *elemento =  (struct ElementoTablaClases *)malloc(sizeof(struct ElementoTablaClases));
    (*elemento)->nombre = (char *)malloc(strlen(nombre)+1);
    strcpy((*elemento)->nombre,nombre);
    (*elemento)->clase=clase;
    (*elemento)->sgte=NULL;
}

void asignar_elemento_clases(char *nombre, struct Clase clase,struct ElementoTablaClases **elemento,int *error){
    if(*elemento==NULL)
        llenar_elemento_clases(nombre,clase,elemento);
    else if(strcmp((*elemento)->nombre,nombre)==0) //NO SE PUEDE SOBRE ESCRIBIR UNA CLASE
        *error=-1;
    else
        asignar_elemento_clases(nombre,clase,&(*elemento)->sgte,error);
}

void asignar_elemento_tabla_simbolos_clases(char *nombre, struct Clase clase,
        struct ElementoTablaClases **TablaSimbolos,int *error){
    int hash = mi_hash(nombre);
    asignar_elemento_clases(nombre,clase,&TablaSimbolos[hash],error);
}

_Bool existe_clase(struct ElementoTablaClases **TablaSimbolos,char* clave){
    int hash=mi_hash(clave);
    if(TablaSimbolos[hash]==NULL)
        return 0;//false, no existe la clave

    struct ElementoTablaClases *elem;
    elem = TablaSimbolos[hash];

    do{
        if(strcmp(clave,elem->nombre)==0)
            return 1; //true, existe clave
        elem=elem->sgte;
    }while(elem!=NULL);

    return 0;
}

struct Clase recuperar_elemento_clases(char *nombre, struct ElementoTablaClases *elemento,int *error){
    if(elemento==NULL){
        *error = -1; //HUBO ERROR, NO EXISTE EN LA TABLA DE SIMBOLOS
        printf("ERROR: No existe la CLASE: %s\n",nombre);
        struct Clase valor_vacio;
        return valor_vacio;
    }

    if(strcmp(elemento->nombre,nombre)==0)
        return elemento->clase;
    else
        return recuperar_elemento_clases(nombre,elemento->sgte,error);
}

struct Clase recuperar_elemento_tabla_simbolos_clases(char *nombre, int *error,struct ElementoTablaClases **TablaSimbolos){
    int hash = mi_hash(nombre);
    return recuperar_elemento_clases(nombre,TablaSimbolos[hash],error);
}

struct Clase* recuperar_ref_elemento_clases(char *nombre, struct ElementoTablaClases *elemento,int *error){
    if(elemento==NULL){
        *error = -1; //HUBO ERROR, NO EXISTE EN LA TABLA DE SIMBOLOS
        printf("ERROR: No existe la CLASE: %s\n",nombre);
        return NULL;
    }

    if(strcmp(elemento->nombre,nombre)==0)
        return &elemento->clase;
    else
        return recuperar_ref_elemento_clases(nombre,elemento->sgte,error);
}

struct Clase* recuperar_ref_elemento_tabla_simbolos_clases(char *nombre, int *error,struct ElementoTablaClases **TablaSimbolos){
    int hash = mi_hash(nombre);
    return recuperar_ref_elemento_clases(nombre,TablaSimbolos[hash],error);
}
