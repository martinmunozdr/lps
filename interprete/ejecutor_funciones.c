#include"cabecera.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

void limpiar_expresion(struct Expresion *expre){
    if(expre->tipo=='O'){//ME PREVENGO DE VALORES DIFERENTES A NULL EN EXPRESIONES SIN INICIALIZAR
        if(expre->izquierda!=NULL)
            limpiar_expresion(expre->izquierda);
        if(expre->derecha!=NULL)
            limpiar_expresion(expre->derecha);
    }

    free(expre);
}

void limpiar_bloque_mientras(struct BloqueMientras *mientras){
    if(mientras->expre.izquierda!=NULL)
        limpiar_expresion(mientras->expre.izquierda);
    if(mientras->expre.derecha!=NULL)
        limpiar_expresion(mientras->expre.derecha);

    limpiar_sentencias(mientras->nodos);

    free(mientras);
}

void limpiar_bloque_si(struct BloqueSi *si){
    if(si->expre.izquierda!=NULL)
        limpiar_expresion(si->expre.izquierda);
    if(si->expre.derecha!=NULL)
        limpiar_expresion(si->expre.derecha);

    limpiar_sentencias(si->nodos);

    if(si->sino!=NULL){
        limpiar_sentencias(si->sino->nodos);
        free(si->sino);
    }

    struct NodoSinoSi *nodo_sinosi = si->sino_si;
    while(nodo_sinosi!=NULL){
        if(nodo_sinosi->sino_si.expre.izquierda!=NULL)
            limpiar_expresion(nodo_sinosi->sino_si.expre.izquierda);
        if(nodo_sinosi->sino_si.expre.derecha!=NULL)
            limpiar_expresion(nodo_sinosi->sino_si.expre.derecha);
        //limpiar_expresion(&nodo_sinosi->sino_si.expre); //esto provocaba error, lo reemplaze por las 4 lineas anteriores
        limpiar_sentencias(nodo_sinosi->sino_si.nodos);
        nodo_sinosi=nodo_sinosi->sgte;
    }

    free(si);
}

_Bool existe_funcion(struct ElementoTablaFunciones **TablaSimbolos,char* clave){
    int hash=mi_hash(clave);
    if(TablaSimbolos[hash]==NULL)
        return 0;//false, no existe la clave

    struct ElementoTablaFunciones *elem;
    elem = TablaSimbolos[hash];

    do{
        if(strcmp(clave,elem->nombre)==0)
            return 1; //true, existe clave
        elem=elem->sgte;
    }while(elem!=NULL);

    return 0;
}

void limpiar_funcion(struct BloqueFuncion funcion){
    if(funcion.parametros!=NULL){
        int i=0;
        for(;i<funcion.num_parametros;i++)
            free(funcion.parametros[i]);

        free(funcion.parametros);
    }

    if(funcion.clase_pertenece!=NULL)
        free(funcion.clase_pertenece);

    limpiar_sentencias(funcion.nodos);
}

void limpiar_bloque_funcion(struct BloqueFuncion *funcion){
    limpiar_funcion(*funcion);
    free(funcion);
}

void limpiar_sent_atributos(struct SentenciaAtributos *s_a){
    free(s_a->clase_pertenece);
    if(s_a->num_atributos>0)free(s_a->atributos);
    free(s_a);
}

void limpiar_sent_clase(struct SentenciaClase *s_c){
    if(s_c->num_padres>0)free(s_c->padres);
    free(s_c);
}


void limpiar_sentencias(struct NodoSentencia nodos){
    switch(nodos.sentencia.tipo){
        case ' ': case 'C': case 'R': break;
        case 'L': limpiar_sent_clase(nodos.sentencia.val.sent_clase); break;
        case 'D': if(nodos.sentencia.val.expre!=NULL)limpiar_expresion(nodos.sentencia.val.expre);break;
        case 'E': limpiar_expresion(nodos.sentencia.val.expre);break;
        case 'M': limpiar_bloque_mientras(nodos.sentencia.val.mientras);break;
        case 'S': limpiar_bloque_si(nodos.sentencia.val.si);break;
        case 'F': limpiar_bloque_funcion(nodos.sentencia.val.funcion);break;
        case 'A': limpiar_sent_atributos(nodos.sentencia.val.sent_atributos);break;
        case 'T': free(nodos.sentencia.val.sent_importar); break;
    }

    if(nodos.sgte!=NULL)
        limpiar_sentencias(*nodos.sgte);
}

void inicializar_tabla_funciones(struct ElementoTablaFunciones **TablaSimbolosFunciones){
    int i=0;
    for(;i<TAMANO_TABLA;i++)
        TablaSimbolosFunciones[i]=NULL;
}
void limpiar_elemento_tabla_funciones(struct ElementoTablaFunciones **elemento){
    free((*elemento)->nombre);

    //limpiar_funcion((*elemento)->funcion); //innecesario PORQUE se limpia al limpiar los elementos sintacticos (bloque central)

    if((*elemento)->sgte!=NULL)
        limpiar_elemento_tabla_funciones(&(*elemento)->sgte);

    free(*elemento);
}

int cantidad_funciones(struct ElementoTablaFunciones **TablaSimbolos){
    int cantidad=0,i=0;
    struct ElementoTablaFunciones *elem;

    for(;i<TAMANO_TABLA;i++)
        if(TablaSimbolos[i]!=NULL){
            cantidad++;
            elem = TablaSimbolos[i];
            while(elem->sgte!=NULL){cantidad++; elem=elem->sgte;}
        }

    return cantidad;
}

char** devolver_nombres_funciones(struct ElementoTablaFunciones **TablaSimbolos,int cantidad){
    int i=0,k=0;
    char **claves = (char**)malloc(sizeof(char*)*cantidad);
    struct ElementoTablaFunciones *elem;

    for(;i<TAMANO_TABLA;i++)
        if(TablaSimbolos[i]!=NULL){
            elem = TablaSimbolos[i];
            do{
                claves[k]=(char*)malloc(strlen(elem->nombre)+1);
                strcpy(claves[k++],elem->nombre);
                elem=elem->sgte;
            }while(elem!=NULL);
        }

    return claves;
}

void limpiar_tabla_funciones(struct ElementoTablaFunciones **TablaSimbolosFunciones){
    int i=0;
    for(;i<TAMANO_TABLA;i++)
        if(TablaSimbolosFunciones[i]!=NULL)
            limpiar_elemento_tabla_funciones(&TablaSimbolosFunciones[i]);
}

void llenar_elemento_funciones(char *nombre, struct BloqueFuncion funcion,struct ElementoTablaFunciones **elemento){
    *elemento =  (struct ElementoTablaFunciones *)malloc(sizeof(struct ElementoTablaFunciones));
    (*elemento)->nombre = (char *)malloc(strlen(nombre)+1);
    strcpy((*elemento)->nombre,nombre);
    (*elemento)->funcion=funcion;
    (*elemento)->sgte=NULL;
}

void asignar_elemento_funciones(char *nombre, struct BloqueFuncion funcion,struct ElementoTablaFunciones **elemento,int *error){
    if(*elemento==NULL)
        llenar_elemento_funciones(nombre,funcion,elemento);
    else if(strcmp((*elemento)->nombre,nombre)==0) //NO SE PUEDE SOBRE ESCRIBIR UNA FUNCION
        *error=-1;
    else
        asignar_elemento_funciones(nombre,funcion,&(*elemento)->sgte,error);
}

void asignar_elemento_tabla_simbolos_funciones(char *nombre, struct BloqueFuncion funcion,
        struct ElementoTablaFunciones **TablaSimbolos,int *error){
    int hash = mi_hash(nombre);
    asignar_elemento_funciones(nombre,funcion,&TablaSimbolos[hash],error);
}

struct BloqueFuncion recuperar_elemento_funciones(char *nombre, struct ElementoTablaFunciones *elemento,int *error){
    if(elemento==NULL){
        *error = -1; //HUBO ERROR, NO EXISTE EN LA TABLA DE SIMBOLOS
        printf("ERROR: No existe la SUBRUTINA: %s",nombre);
        struct BloqueFuncion valor_vacio;
        return valor_vacio;
    }

    if(strcmp(elemento->nombre,nombre)==0)
        return elemento->funcion;
    else
        return recuperar_elemento_funciones(nombre,elemento->sgte,error);
}

struct BloqueFuncion recuperar_elemento_tabla_simbolos_funciones(char *nombre, int *error,struct ElementoTablaFunciones **TablaSimbolos){
    int hash = mi_hash(nombre);
    return recuperar_elemento_funciones(nombre,TablaSimbolos[hash],error);
}
