#include"cabecera.h"
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
//#include<stdbool.h>
#include <sqlite3.h>    // Para trabajar con SQLite
#include <time.h>

#define COMPROBAR_ERROR_EXPRESION  if(*error<0)return rpta;

//extern struct ElementoTabla *TablaSimbolos[TAMANO_TABLA];
extern struct ElementoTablaFunciones *TablaSimbolosFunciones[TAMANO_TABLA];
extern struct ElementoTablaClases *TablaSimbolosClases[TAMANO_TABLA];

//CGI
//struct Par pares[MAXPARES];
//int cant_pares=0:
///////////////

////SQLITE
  sqlite3 *db=NULL;          // Definimos un puntero a la base de datos
  char *errMsg = 0;     // Variable para el mensaje de error
  int rc;               // Variable para el retorno de la sentencia
  sqlite3_stmt *result=NULL; // Puntero a la respuesta de la consulta

////////////////////////////////////////
//LOG SQL INJECTION

void milog(char *ruta_log, char *message){
    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    char fecha[13],hora[16],nomb_arch[200];
    strftime(fecha,13,"%d_%m_%y",tlocal);
    strftime(hora,16,"%H:%M:%S",tlocal);

    sprintf(nomb_arch,"%slog_%s",ruta_log,fecha);

    FILE *fp;
    fp = fopen ( nomb_arch, "a" );

    if(fp==NULL){
        printf("\nNO SE PUDO ESCRIBIR LOG EN LA RUTA ELEGIDA EN LA SUBRUTINA"
               " PREDEFINIDA LOG_SI_DETECTO_SQL_INJECTION, REVISE SUS PERMISOS\n");
        return;
    }

    fprintf(fp, "%s: %s", hora, message);
    fclose ( fp );
}

void stringToUpper(char *cad){
    int i=0;
    while(cad[i]){
        if(cad[i]>=97)
            cad[i] -= 32;
        i++;
    }
}

void agregar_mensaje(char **message, char *temp){
    if(*message==NULL){
        *message = (char*)malloc(strlen(temp)+1);
        strcpy(*message,temp);
    }else{
        char *buffer_temp=(char*)malloc(strlen(*message)+1);
        strcpy(buffer_temp, *message);
        free(*message);
        *message = (char*)malloc(strlen(buffer_temp)
                    +strlen(temp)+1);
        strcpy(*message, buffer_temp);
        strcat(*message, temp);
        free(buffer_temp);
    }
}

/* *message debe venir inicializado en null*/
_Bool detectar_sql_injection(char *sql, char **message){
    _Bool rpta=0;
    int contador1=0,contador2=0, contador_comillas=0,contador_puntoycomas=0;
    char cad[31],temp[250];
    cad[0]='\0';

    while(contador1<strlen(sql)){
        if(isalpha(sql[contador1]) || (isdigit(sql[contador1]) && contador2>0)){
            if(contador2<30){
                cad[contador2]=sql[contador1];
                cad[contador2+1]='\0';
                contador2++;
            }
        }
        else{
            stringToUpper(cad);
            if(strlen(cad)>0){
                if(strcmp(cad,"UNION")==0){
                    rpta=1;
                    strcpy(temp,"\nSE DETECTO POSIBLE INTENTO DE SQL INJECTION, SE ENCONTRO LA PALABRA: UNION \n");
                    agregar_mensaje(message, temp);
                }else if(strcmp(cad,"HAVING")==0){
                    rpta=1;
                    strcpy(temp,"\nSE DETECTO POSIBLE INTENTO DE SQL INJECTION, SE ENCONTRO LA PALABRA: HAVING \n");
                    agregar_mensaje(message, temp);
                }else if(strcmp(cad,"SELECT")==0){
                    rpta=1;
                    strcpy(temp,"\nSE DETECTO POSIBLE INTENTO DE SQL INJECTION, SE ENCONTRO LA PALABRA: SELECT \n");
                    agregar_mensaje(message, temp);
                }else if(strcmp(cad,"AND")==0){
                    rpta=1;
                    strcpy(temp,"\nSE DETECTO POSIBLE INTENTO DE SQL INJECTION, SE ENCONTRO LA PALABRA: AND \n");
                    agregar_mensaje(message, temp);
                }else if(strcmp(cad,"ORDER")==0){
                    rpta=1;
                    strcpy(temp,"\nSE DETECTO POSIBLE INTENTO DE SQL INJECTION, SE ENCONTRO LA PALABRA: ORDER \n");
                    agregar_mensaje(message, temp);
                }else if(strcmp(cad,"GROUP")==0){
                    rpta=1;
                    strcpy(temp,"\nSE DETECTO POSIBLE INTENTO DE SQL INJECTION, SE ENCONTRO LA PALABRA: GROUP \n");
                    agregar_mensaje(message, temp);
                }else if(strcmp(cad,"OR")==0){
                    rpta=1;
                    strcpy(temp,"\nSE DETECTO POSIBLE INTENTO DE SQL INJECTION, SE ENCONTRO LA PALABRA: OR \n");
                    agregar_mensaje(message, temp);
                }else if(strcmp(cad,"INSERT")==0){
                    rpta=1;
                    strcpy(temp,"\nSE DETECTO POSIBLE INTENTO DE SQL INJECTION, SE ENCONTRO LA PALABRA: INSERT \n");
                    agregar_mensaje(message, temp);
                }else if(strcmp(cad,"DROP")==0){
                    rpta=1;
                    strcpy(temp,"\nSE DETECTO POSIBLE INTENTO DE SQL INJECTION, SE ENCONTRO LA PALABRA: DROP \n");
                    agregar_mensaje(message, temp);
                }else if(strcmp(cad,"CREATE")==0){
                    rpta=1;
                    strcpy(temp,"\nSE DETECTO POSIBLE INTENTO DE SQL INJECTION, SE ENCONTRO LA PALABRA: CREATE \n");
                    agregar_mensaje(message, temp);
                }
            }
            contador2=0;
            cad[0]='\0';            
        }

        contador1++;
    }

    contador1=0;
    while(contador1<strlen(sql)){
        if(sql[contador1]==';'){
            contador_puntoycomas++;
        }else if(sql[contador1]=='\''){
            contador_comillas++;             
        }

        contador1++;
    }

    if(contador_puntoycomas){
                    rpta=1;
                    sprintf(temp,"\nSE DETECTO POSIBLE INTENTO DE SQL INJECTION, SE ENCONTRO %d PUNTO Y COMA: ; \n ",contador_puntoycomas);
                    agregar_mensaje(message, temp);
    }

    if(contador_comillas){
                    rpta=1;
                    sprintf(temp,"\nSE DETECTO POSIBLE INTENTO DE SQL INJECTION, SE ENCONTRO %d COMILLA: ' \n",contador_comillas);
                    agregar_mensaje(message, temp);
    }

    return rpta;
}

////////////////////////////////////////


_Bool creacion_clase(char *nombre,struct Valor *rpta,int *error){
    if(!existe_clase(TablaSimbolosClases,nombre))
        return 0;

    rpta->tipo='C';
    rpta->val.clase = (struct Clase*)malloc(sizeof(struct Clase));
    *rpta->val.clase = copiar_clase(
          recuperar_elemento_tabla_simbolos_clases(nombre, error,TablaSimbolosClases)
    );

    llenar_referencia_objeto(rpta->val.clase->metodos,rpta->val.clase);

    return 1;
}

_Bool es_funcion_predefinida(char *nombre){
    if(strcmp(nombre,"ESCRIBIR")==0){
        return 1;
    }else if(strcmp(nombre,"TIPO_DE")==0){
        return 1;
    }else if(strcmp(nombre,"CLASE_DE")==0){
        return 1;
    }else if(strcmp(nombre,"LEER_NUMERO")==0){
        return 1;
    }else if(strcmp(nombre,"LEER_ENTERO")==0){
        return 1;
    }else if(strcmp(nombre,"LEER_TEXTO")==0){
        return 1;
    }else if(strcmp(nombre,"DICCIONARIO")==0){
        return 1;
    }else if(strcmp(nombre,"EXISTE_CLAVE")==0){
        return 1;
    }else if(strcmp(nombre,"INDICE_EN_LISTA")==0){
        return 1;
    }else if(strcmp(nombre,"INDICE_EN_ARREGLO")==0){
        return 1;
    }else if(strcmp(nombre,"CLAVES")==0){
        return 1;
    }else if(strcmp(nombre,"ARREGLO_A_LISTA")==0){
        return 1;
    }else if(strcmp(nombre,"LISTA_A_ARREGLO")==0){
        return 1;
    }else if(strcmp(nombre,"LISTA")==0){
        return 1;
    }else if(strcmp(nombre,"ARREGLO")==0){
        return 1;
    }else if(strcmp(nombre,"TXT_A_NUM")==0){
        return 1;
    }else if(strcmp(nombre,"NUM_A_TXT")==0){
        return 1;
    }else if(strcmp(nombre,"TXT_A_ENT")==0){
        return 1;
    }else if(strcmp(nombre,"ENT_A_TXT")==0){
        return 1;
    }else if(strcmp(nombre,"ENT_A_NUM")==0){
        return 1;
    }else if(strcmp(nombre,"NUM_A_ENT")==0){
        return 1;
    }else if(strcmp(nombre,"LONGITUD")==0){
        return 1;
    }else if(strcmp(nombre,"OBTENER_POST")==0){
        return 1;
    }else if(strcmp(nombre,"OBTENER_GET")==0){
        return 1;
    }else if(strcmp(nombre,"SQLITE3_OPEN")==0){
        return 1;
    }else if(strcmp(nombre,"SQLITE3_PREPARE")==0){
        return 1;
    }else if(strcmp(nombre,"SQLITE3_STEP")==0){
        return 1;
    }else if(strcmp(nombre,"SQLITE3_COLUMN_INT")==0){
        return 1;
    }else if(strcmp(nombre,"SQLITE3_COLUMN_DOUBLE")==0){
        return 1;
    }else if(strcmp(nombre,"SQLITE3_COLUMN_TEXT")==0){
        return 1;
    }else if(strcmp(nombre,"SQLITE3_FINALIZE")==0){
        return 1;
    }else if(strcmp(nombre,"SQLITE3_CLOSE")==0){
        return 1;
    }else if(strcmp(nombre,"SQLITE3_BIND_TEXT")==0){
        return 1;
    }else if(strcmp(nombre,"SQLITE3_BIND_INT")==0){
        return 1;
    }else if(strcmp(nombre,"SQLITE3_BIND_DOUBLE")==0){
        return 1;
    }else if(strcmp(nombre,"ESCRIBIR_TOKEN_CSRF")==0){
        return 1;
    }else if(strcmp(nombre,"LOG_SI_DETECTO_SQL_INJECTION")==0){
        return 1;
    }

    return 0;
}

char* texto_tipo(char tipo){
    switch(tipo){
        case 'N': return "NUMERO CON DECIMALES";
        case 'T': return "TEXTO";
        case 'L': return "LISTA";
        case 'B': return "VALOR DE VERDAD";
        case 'D': return "DICCIONARIO";
        case 'A': return "ARREGLO";
        case 'V': return "VACIO";
        case 'F': return "SUBRUTINA O METODO";
        case 'C': return "OBJETO";
        case 'E': return "ENTERO";
    }

    return "(ERROR INESPERADO) tipo indeterminado.";
}

int mi_hash(char *cadena){
    int i=0,rpta=0;
    for(;i<strlen(cadena);i++)
        rpta += (int)cadena[i];

    return rpta % TAMANO_TABLA;
}

//funciones diccionario
int cantidad_claves(struct ElementoTabla **TablaSimbolos){
    int cantidad=0,i=0;
    struct ElementoTabla *elem;

    for(;i<TAMANO_TABLA;i++)
        if(TablaSimbolos[i]!=NULL){
            cantidad++;
            elem = TablaSimbolos[i];
            while(elem->sgte!=NULL){cantidad++; elem=elem->sgte;}
        }

    return cantidad;
}

//cantidad debe tener la cantidad total de claves
char** devolver_claves(struct ElementoTabla **TablaSimbolos,int cantidad){
    int i=0,k=0;
    char **claves = (char**)malloc(sizeof(char*)*cantidad);
    struct ElementoTabla *elem;

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

_Bool existe_clave(struct ElementoTabla **TablaSimbolos,char* clave){
    int hash=mi_hash(clave);
    if(TablaSimbolos[hash]==NULL)
        return 0;//false, no existe la clave

    struct ElementoTabla *elem;
    elem = TablaSimbolos[hash];

    do{
        if(strcmp(clave,elem->nombre)==0)
            return 1; //true, existe clave
        elem=elem->sgte;
    }while(elem!=NULL);

    return 0;
}
///////////////////////////////
void inicializar_tabla(struct ElementoTabla **TablaSimbolos){
    int i=0;
    for(;i<TAMANO_TABLA;i++)
        TablaSimbolos[i]=NULL;
}

void limpiar_nodo(struct Nodo *nodo){
    if(nodo->sgte!=NULL)
        limpiar_nodo(nodo->sgte);

    free(nodo);
}

void limpiar_lista(struct Lista *lista){
    if(lista->primer_elem.sgte!=NULL)
        limpiar_nodo(lista->primer_elem.sgte);

    free(lista);
}

struct Nodo copiar_nodo(struct Nodo original){
    struct Nodo rpta;

    rpta.valor=copiar_valor(original.valor);

    if(original.sgte==NULL)
        rpta.sgte=NULL;
    else{
        rpta.sgte = (struct Nodo*)malloc(sizeof(struct Nodo));
        *rpta.sgte = copiar_nodo(*original.sgte);
    }

    return rpta;
}

struct Lista copiar_lista(struct Lista original){
    struct Lista rpta;

    rpta.num_elems = original.num_elems;
    rpta.primer_elem = copiar_nodo(original.primer_elem);

    return rpta;
}

struct Diccionario copiar_diccionario(struct Diccionario original){
    struct Diccionario rpta;

    inicializar_tabla(rpta.datos);

    int error=0,i=0,cantidad = cantidad_claves(original.datos);
    if(cantidad==0)
        return rpta;

    char ** claves = devolver_claves(original.datos,cantidad);

    for(;i<cantidad;i++){
        asignar_elemento_tabla_simbolos(claves[i], 
            copiar_valor(recuperar_elemento_tabla_simbolos(claves[i], &error,original.datos)),
            rpta.datos
        );
    }

    free(claves);
    return rpta;
}

struct Clase copiar_clase(struct Clase original){
    struct Clase rpta;

    rpta.nombre = (char*)malloc(strlen(original.nombre)+1);
    strcpy(rpta.nombre, original.nombre);

    inicializar_tabla_funciones(rpta.metodos);

    int i=0,error=0,cantidad = cantidad_funciones(original.metodos);
    char **claves;
    if(cantidad>0){
        claves = devolver_nombres_funciones(original.metodos,cantidad);

        for(i=0;i<cantidad;i++){
            asignar_elemento_tabla_simbolos_funciones(claves[i], 
                recuperar_elemento_tabla_simbolos_funciones(claves[i], &error,original.metodos),
                rpta.metodos,&error
            );
        }

        free(claves);
    }

    inicializar_tabla(rpta.atributos);

    error=0,cantidad = cantidad_claves(original.atributos);
    if(cantidad==0)
        return rpta;

    claves = devolver_claves(original.atributos,cantidad);

    for(i=0;i<cantidad;i++){
        asignar_elemento_tabla_simbolos(claves[i], 
            copiar_valor(recuperar_elemento_tabla_simbolos(claves[i], &error,original.atributos)),
            rpta.atributos
        );
    }

    free(claves);
    return rpta;
}

struct Arreglo copiar_arreglo(struct Arreglo original){
    struct Arreglo rpta;

    rpta.num_elems=original.num_elems;
    rpta.elems=(struct Valor*)malloc(rpta.num_elems*sizeof(struct Valor));

    int i=0;
    for(;i<rpta.num_elems;i++)
        rpta.elems[i] = copiar_valor(original.elems[i]);

    return rpta;
}

struct Valor copiar_valor(struct Valor valor){
    struct Valor rpta;
    rpta.referencia=NULL;//la copia nunca es referencia

    switch(valor.tipo){
        case 'N':
            rpta.tipo='N';
            rpta.val.numero = (double*)malloc(sizeof(double));
            *rpta.val.numero = *valor.val.numero;

            break;
        case 'E':
            rpta.tipo='E';
            rpta.val.entero = (int*)malloc(sizeof(int));
            *rpta.val.entero = *valor.val.entero;

            break;
        case 'B':
            rpta.tipo='B';
            rpta.val.booleano = (_Bool*)malloc(sizeof(_Bool));
            *rpta.val.booleano = *valor.val.booleano;

            break;
        case 'T':
            rpta.tipo='T';
            rpta.val.texto = (char*)malloc(strlen(valor.val.texto)+1);
            strcpy(rpta.val.texto,valor.val.texto);

            break;
        case 'L':
            rpta.tipo='L';
            rpta.val.lista = (struct Lista*)malloc(sizeof(struct Lista));
            *rpta.val.lista = copiar_lista(*valor.val.lista);

            break;
        case 'D':
            rpta.tipo='D';
            rpta.val.diccionario = (struct Diccionario*)malloc(sizeof(struct Diccionario));
            *rpta.val.diccionario = copiar_diccionario(*valor.val.diccionario);

            break;
        case 'V':
            rpta.tipo='V';
            break;
        case 'A':
            rpta.tipo='A';
            rpta.val.arreglo = (struct Arreglo*)malloc(sizeof(struct Arreglo));
            *rpta.val.arreglo = copiar_arreglo(*valor.val.arreglo);
            break;
        case 'C':
            rpta.tipo='C';
            rpta.val.clase = (struct Clase*)malloc(sizeof(struct Clase));
            *rpta.val.clase = copiar_clase(*valor.val.clase);
            break;
        case 'F':
            rpta.tipo = 'F';
            rpta.val.funcion = (struct BloqueFuncion*)malloc(sizeof(struct BloqueFuncion));
            *rpta.val.funcion = *valor.val.funcion;
            break;
    }

    return rpta;
}

void limpiar_diccionario(struct Diccionario *dic){
    limpiar_tabla(dic->datos);
    free(dic);
}

void limpiar_arreglo(struct Arreglo *arr){
    free(arr->elems);
    free(arr);
}

void limpiar_valor(struct Valor valor){
    if(valor.tipo=='T')
        free(valor.val.texto);
    else if(valor.tipo=='L')
        limpiar_lista(valor.val.lista);
    else if(valor.tipo=='D')
        limpiar_diccionario(valor.val.diccionario);
    else if(valor.tipo=='N')
        free(valor.val.numero);
    else if(valor.tipo=='E')
        free(valor.val.entero);
    else if(valor.tipo=='B')
        free(valor.val.booleano);
    else if(valor.tipo=='A')
        limpiar_arreglo(valor.val.arreglo);
    else if(valor.tipo=='C'){
        limpiar_clase(*valor.val.clase);
        free(valor.val.clase);
    }
    else if(valor.tipo=='F')
        free(valor.val.funcion);
}

void limpiar_elemento_tabla(struct ElementoTabla **elemento){
    if(strcmp((*elemento)->nombre,"ESTE")!=0)
        limpiar_valor((*elemento)->valor);

    free((*elemento)->nombre);

    if((*elemento)->sgte!=NULL)
        limpiar_elemento_tabla(&(*elemento)->sgte);

    free(*elemento);
}

void limpiar_tabla(struct ElementoTabla **TablaSimbolos){
    int i=0;
    for(;i<TAMANO_TABLA;i++)
        if(TablaSimbolos[i]!=NULL)
            limpiar_elemento_tabla(&TablaSimbolos[i]);
}

void llenar_elemento(char *nombre, struct Valor valor,struct ElementoTabla **elemento){
    *elemento =  (struct ElementoTabla *)malloc(sizeof(struct ElementoTabla));
    (*elemento)->nombre = (char *)malloc(strlen(nombre)+1);
    strcpy((*elemento)->nombre,nombre);
    (*elemento)->valor=valor;
    (*elemento)->sgte=NULL;
}

void cambiar_valor_elemento(struct Valor valor,struct ElementoTabla **elemento){
    limpiar_valor((*elemento)->valor);
    (*elemento)->valor=valor;
}

//SI NO EXISTE LO CREA, SI EXISTE LO REEMPLAZA
void asignar_elemento(char *nombre, struct Valor valor,struct ElementoTabla **elemento){
    if(*elemento==NULL){
        llenar_elemento(nombre,valor,elemento);}
    else if(strcmp((*elemento)->nombre,nombre)==0){
        cambiar_valor_elemento(valor,elemento);}
    else{
        asignar_elemento(nombre,valor,&(*elemento)->sgte);}
}

void asignar_elemento_tabla_simbolos(char *nombre, struct Valor valor,struct ElementoTabla **TablaSimbolos){
    int hash = mi_hash(nombre);
    asignar_elemento(nombre,valor,&TablaSimbolos[hash]);
}

struct Valor recuperar_elemento(char *nombre, struct ElementoTabla *elemento,int *error){
    if(elemento==NULL){
        *error = -1; //HUBO ERROR, NO EXISTE EN LA TABLA DE SIMBOLOS
        printf("ERROR: No existe el identificador: %s",nombre);
        struct Valor valor_vacio;
        valor_vacio.tipo='V';
        valor_vacio.referencia=NULL;
        return valor_vacio;
    }

    if(strcmp(elemento->nombre,nombre)==0){
        struct Valor rpta = elemento->valor;
        rpta.referencia=&elemento->valor;
        return rpta;
    }else
        return recuperar_elemento(nombre,elemento->sgte,error);
}

struct Valor recuperar_elemento_tabla_simbolos(char *nombre, int *error,struct ElementoTabla **TablaSimbolos){
    int hash = mi_hash(nombre);
    return recuperar_elemento(nombre,TablaSimbolos[hash],error);
}

/////FUNCIONES PARA CONCATENAR NODOS, LISTAS
void concatenar_nodos(struct Nodo *nodo, struct Nodo nuevo_nodo){
    if(nodo->sgte==NULL) {
        nodo->sgte=(struct Nodo*)malloc(sizeof(struct Nodo));
        *(nodo->sgte) = nuevo_nodo;
    }
    else{
        concatenar_nodos(nodo->sgte,nuevo_nodo);
    }
}

void concatenar_listas(struct Lista *lista1, struct Lista lista2){
    concatenar_nodos(& lista1->primer_elem, lista2.primer_elem);
}
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
///FUNCIONES PARA RECUPERAR ELEMENTOS DE UNA LISTA
/////////////////////////////////////////////////////////////////////////

///RECIBE COMO PRIMER PARAMETRO EL PRIMER ELEMENTO DE UNA LISTA Y COMO SEGUNDO EL INDICE A RECUPERAR(indices empiezan de 1)
struct Valor recuperar_elemento_lista(struct Nodo *nodo, int indice, int *error,_Bool referencia){
    if(indice==1){
        struct Valor rpta;
        if(referencia)
            rpta = nodo->valor;
        else
            rpta = copiar_valor(nodo->valor);
        rpta.referencia=&nodo->valor;
        return rpta;
    }
    else
        return recuperar_elemento_lista(nodo->sgte,indice-1,error,referencia);
}

struct Lista recuperar_elementos_lista(struct Lista lista_orig, int indice_ini,int indice_fin, int *error){
    struct Lista lista;

    lista.num_elems=indice_fin-indice_ini+1;
    lista.primer_elem.valor = recuperar_elemento_lista(&lista_orig.primer_elem, indice_ini, error,0);
    lista.primer_elem.sgte=NULL;

    struct Nodo nuevo_nodo;
    int i=indice_ini+1;
    for(;i<=indice_fin;i++){
        nuevo_nodo.valor=recuperar_elemento_lista(&lista_orig.primer_elem, i, error,0);
        nuevo_nodo.sgte=NULL;
        concatenar_nodos(&lista.primer_elem,nuevo_nodo);
    }

    return lista;
}

//////////////////VALORES PREDEFINIDOS DEL LENGUAJE

_Bool valor_predefinido(char* nombre,struct Valor *rpta){
    if(strcmp(nombre,"VERDADERO")==0){
        rpta->tipo='B';
        rpta->val.booleano=(_Bool*)malloc(sizeof(_Bool));
        *rpta->val.booleano=1;
        return 1;
    }else if(strcmp(nombre,"FALSO")==0){
        rpta->tipo='B';
        rpta->val.booleano=(_Bool*)malloc(sizeof(_Bool));
        *rpta->val.booleano=0;
        return 1;
    }else if(strcmp(nombre,"NL")==0){
        rpta->tipo='T';
        rpta->val.texto=(char*)malloc(2);
        rpta->val.texto[0]='\n';
        rpta->val.texto[1]='\0';
        return 1;
    }else if(strcmp(nombre,"TAB")==0){
        rpta->tipo='T';
        rpta->val.texto=(char*)malloc(2);
        rpta->val.texto[0]='\t';
        rpta->val.texto[1]='\0';
        return 1;
    }else if(strcmp(nombre,"VACIO")==0){
        rpta->tipo='V';
        return 1;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////
///RESUELVE EXPRESIONES
///SI HAY ERROR el parametro error pasa a valer -1 y el valor de retorno es indeterminado
/////////////////////////////////////////////////////////////////////////
struct Valor resolver_expresion(struct Expresion expresion, int *error,struct ElementoTabla **TablaSimbolos,_Bool referencia){
    struct Valor rpta,izq,der;
    //INICIALIZO EL VALOR DE RESPUESTA POR SI OCURRE ALGUN ERROR INESPERADO (ESTE VALOR NO DEBERIA LLEGAR AL USUARIO FINAL)
    rpta.tipo='V';
    rpta.referencia=NULL;
    ////////////////
    if(expresion.tipo=='N'){
        if(strchr(expresion.texto,'.')==NULL){
            rpta.tipo='E';
            rpta.val.entero=(int*)malloc(sizeof(int));
            sscanf(expresion.texto,"%d",rpta.val.entero);
        }else{
            rpta.tipo='N';
            rpta.val.numero=(double*)malloc(sizeof(double));
            sscanf(expresion.texto,"%lf",rpta.val.numero);
        }
    }
    else if(expresion.tipo=='T'){
        rpta.tipo='T';
        rpta.val.texto=(char*)malloc(strlen(expresion.texto)+1);
        strcpy(rpta.val.texto,expresion.texto);

    }
    else if(expresion.tipo=='I'){
        if(valor_predefinido(expresion.texto,&rpta)){
            return rpta;
        }
        if(existe_clave(TablaSimbolos,expresion.texto)){
            if(referencia)
                rpta = recuperar_elemento_tabla_simbolos(expresion.texto, error,TablaSimbolos);
            else
                rpta = copiar_valor(recuperar_elemento_tabla_simbolos(expresion.texto, error,TablaSimbolos));
        }else if(existe_funcion(TablaSimbolosFunciones,expresion.texto)){
            rpta.tipo='F';
            rpta.val.funcion = (struct BloqueFuncion*)malloc(sizeof(struct BloqueFuncion));
            *rpta.val.funcion = recuperar_elemento_tabla_simbolos_funciones(expresion.texto,error,
                                             TablaSimbolosFunciones);
        }else{
            *error=-1;
            printf("\nERROR: NO EXISTE EL IDENTIFICADOR: %s\n",expresion.texto);
            return rpta;
        }
        COMPROBAR_ERROR_EXPRESION
    }
    else if(expresion.tipo=='O'){
        if(strcmp(expresion.operacion,"+")==0 && expresion.izquierda!=NULL){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(izq.tipo!=der.tipo){
                *error=-1;
                printf("\nERROR: Está intentando sumar valores de distinto tipo\n");
                return rpta;
            }

            switch(izq.tipo){
                case 'N':
                    rpta.tipo='N';
                    rpta.val.numero=(double*)malloc(sizeof(double));
                    *rpta.val.numero = *izq.val.numero + *der.val.numero;

                    limpiar_valor(izq);
                    limpiar_valor(der);
                    break;
                case 'E':
                    rpta.tipo='E';
                    rpta.val.entero=(int*)malloc(sizeof(int));
                    *rpta.val.entero = *izq.val.entero + *der.val.entero;

                    limpiar_valor(izq);
                    limpiar_valor(der);
                    break;
                case 'T':
                    rpta.tipo='T';
                    rpta.val.texto = (char*)malloc(strlen(izq.val.texto)+strlen(der.val.texto)+1);
                    strcpy(rpta.val.texto, izq.val.texto);
                    strcat(rpta.val.texto, der.val.texto);

                    limpiar_valor(izq);
                    limpiar_valor(der);
                    break;
                case 'L':
                    rpta.tipo='L';
                    rpta.val.lista = (struct Lista*)malloc(sizeof(struct Lista));
                    rpta.val.lista->num_elems = izq.val.lista->num_elems + der.val.lista->num_elems;

                    if(izq.val.lista->num_elems>0){
                        rpta.val.lista->primer_elem = izq.val.lista->primer_elem;
                        concatenar_listas(rpta.val.lista,*der.val.lista);
                    }else{
                        rpta.val.lista->primer_elem = der.val.lista->primer_elem;
                    }

                    free(izq.val.lista);//NO USO limpiar_lista porque esto eliminaría también los nodos
                    free(der.val.lista);
                    break;
                default:
                    *error=-1;
                    printf("\nERROR: EL OPERADOR (+) SUMA NO SE APLICA AL TIPO: %s\n",texto_tipo(izq.tipo));
                    return rpta;
            }
        }
        else if(strcmp(expresion.operacion,",")==0){
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            rpta.tipo = 'L';
            rpta.val.lista = (struct Lista*)malloc(sizeof(struct Lista));

            if(izq.tipo!='L'){
                rpta.val.lista->num_elems=2;
                rpta.val.lista->primer_elem.valor=izq;
                rpta.val.lista->primer_elem.sgte = (struct Nodo*)malloc(sizeof(struct Nodo));
                rpta.val.lista->primer_elem.sgte->valor=der;
                rpta.val.lista->primer_elem.sgte->sgte=NULL;

            }
            else{
                /*if(der.tipo=='L'){
                    rpta.val.lista->num_elems = izq.val.lista->num_elems + der.val.lista->num_elems;
                    rpta.val.lista->primer_elem = izq.val.lista->primer_elem;
                    concatenar_listas(rpta.val.lista,*der.val.lista);
                    rpta.referencia=NULL; //no es referencia
                }
                else{*/
                    struct Nodo nuevo_nodo;
                    nuevo_nodo.sgte=NULL;
                    nuevo_nodo.valor=der;

                    rpta.val.lista->num_elems = izq.val.lista->num_elems + 1;
                    if(izq.val.lista->num_elems==0){
                        rpta.val.lista->primer_elem=nuevo_nodo;
                    }else{
                        rpta.val.lista->primer_elem = izq.val.lista->primer_elem;
                        concatenar_nodos(&rpta.val.lista->primer_elem,nuevo_nodo);
                    }
                    free(izq.val.lista);

                //}
            }
        }
        else if(strcmp(expresion.operacion,";")==0){
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            rpta.tipo = 'L';
            rpta.val.lista = (struct Lista*)malloc(sizeof(struct Lista));

            if(izq.tipo!='L'){
                rpta.val.lista->num_elems=2;
                rpta.val.lista->primer_elem.valor=izq;
                rpta.val.lista->primer_elem.sgte = (struct Nodo*)malloc(sizeof(struct Nodo));
                rpta.val.lista->primer_elem.sgte->valor=der;
                rpta.val.lista->primer_elem.sgte->sgte=NULL;

            }
            else{
                /*if(der.tipo=='L'){
                    rpta.val.lista->num_elems = izq.val.lista->num_elems + der.val.lista->num_elems;
                    rpta.val.lista->primer_elem = izq.val.lista->primer_elem;
                    concatenar_listas(rpta.val.lista,*der.val.lista);
                    rpta.referencia=NULL; //no es referencia
                }
                else{*/
                    struct Nodo nuevo_nodo;
                    nuevo_nodo.sgte=NULL;
                    nuevo_nodo.valor=der;

                    rpta.val.lista->num_elems = izq.val.lista->num_elems + 1;
                    if(izq.val.lista->num_elems==0){
                        rpta.val.lista->primer_elem=nuevo_nodo;
                    }else{
                        rpta.val.lista->primer_elem = izq.val.lista->primer_elem;
                        concatenar_nodos(&rpta.val.lista->primer_elem,nuevo_nodo);
                    }
                    free(izq.val.lista);

                //}
            }
        }
        else if(strcmp(expresion.operacion,"*")==0){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(izq.tipo!=der.tipo){
                *error=-1;
                printf("\nERROR: Está intentando multiplicar valores de distinto tipo"
                       " (ambos deben ser enteros o numeros con decimales).\n"
                       "Si desea multiplicar por ejemplo entero por número con decimales utilice la"
                       " subrutina ENT_A_NUM O NUM_A_ENT\n");
                return rpta;
            }

            if(!(izq.tipo=='N' && der.tipo=='N') && !(izq.tipo=='E' && der.tipo=='E')){
                *error=-1;
                printf("\nERROR: Está intentando multiplicar valores no numéricos.\n");
                return rpta;
            }

            if(izq.tipo=='N'){
                rpta.tipo='N';
                rpta.val.numero=(double*)malloc(sizeof(double));
                *rpta.val.numero = *izq.val.numero * *der.val.numero;
            }else{
                rpta.tipo='E';
                rpta.val.entero=(int*)malloc(sizeof(int));
                *rpta.val.entero = *izq.val.entero * *der.val.entero;
            }

            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"/")==0){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(izq.tipo!=der.tipo){
                *error=-1;
                printf("\nERROR: Está intentando dividir valores de distinto tipo"
                       " (ambos deben ser enteros o numeros con decimales).\n"
                       "Si desea dividir por ejemplo entero por número con decimales utilice la"
                       " subrutina ENT_A_NUM O NUM_A_ENT\n");
                return rpta;
            }

            if(!(izq.tipo=='N' && der.tipo=='N') && !(izq.tipo=='E' && der.tipo=='E')){
                *error=-1;
                printf("\nERROR: Está intentando dividir valores no numéricos.\n");
                return rpta;
            }
            else if((der.tipo=='N'&&*der.val.numero==0)||(der.tipo=='E'&&*der.val.entero==0)){
                *error=-1;
                printf("\nERROR: DIVISION ENTRE CERO.\n");
                return rpta;
            }

            if(izq.tipo=='N'){
                rpta.tipo='N';
                rpta.val.numero=(double*)malloc(sizeof(double));
                *rpta.val.numero = *izq.val.numero / *der.val.numero;
            }else{
                rpta.tipo='E';
                rpta.val.entero=(int*)malloc(sizeof(int));
                *rpta.val.entero = *izq.val.entero / *der.val.entero;
            }

            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"-")==0 && expresion.izquierda!=NULL){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(izq.tipo!=der.tipo){
                *error=-1;
                printf("\nERROR: Está intentando restar valores de distinto tipo"
                       " (ambos deben ser enteros o numeros con decimales).\n"
                       "Si desea restar por ejemplo entero con número con decimales utilice la"
                       " subrutina ENT_A_NUM O NUM_A_ENT\n");
                return rpta;
            }

            if(izq.tipo!='N' && izq.tipo!='E'){
                *error=-1;
                printf("\nERROR: Está intentando restar valores no numéricos.\n");
                return rpta;
            }

            if(izq.tipo=='N'){
                rpta.tipo='N';
                rpta.val.numero=(double*)malloc(sizeof(double));
                *rpta.val.numero = *izq.val.numero - *der.val.numero;
            }else{
                rpta.tipo='E';
                rpta.val.entero=(int*)malloc(sizeof(int));
                *rpta.val.entero = *izq.val.entero - *der.val.entero;
            }

            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"DIV_ENTERA")==0){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(izq.tipo!=der.tipo){
                *error=-1;
                printf("\nERROR: Está intentando dividir valores de distinto tipo"
                       " (ambos deben ser enteros o numeros con decimales).\n"
                       "Si desea dividir por ejemplo entero por número con decimales utilice la"
                       " subrutina ENT_A_NUM O NUM_A_ENT\n");
                return rpta;
            }

            if(izq.tipo!='N' && izq.tipo!='E'){
                *error=-1;
                printf("\nERROR: Está intentando dividir valores no numéricos.\n");
                return rpta;
            }else if((der.tipo=='N'&&*der.val.numero==0)||(der.tipo=='E'&&*der.val.entero==0)){
                *error=-1;
                printf("\nERROR: DIVISION ENTRE CERO.\n");
                return rpta;
            }

            if(izq.tipo=='N'){
                rpta.tipo='N';
                rpta.val.numero=(double*)malloc(sizeof(double));
                *rpta.val.numero = (int)(*izq.val.numero / *der.val.numero);
            }else{
                rpta.tipo='E';
                rpta.val.entero=(int*)malloc(sizeof(int));
                *rpta.val.entero = (int)(*izq.val.entero / *der.val.entero);
            }

            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"RESTO")==0){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(izq.tipo!=der.tipo){
                *error=-1;
                printf("\nERROR: Está intentando hallar el resto con valores de distinto tipo"
                       " (ambos deben ser enteros o numeros con decimales).\n");
                return rpta;
            }

            if(izq.tipo!='N' && izq.tipo!='E'){
                *error=-1;
                printf("\nERROR: Está intentando hallar el resto de valores no numéricos.\n");
                return rpta;
            }
            else if((der.tipo=='N'&&*der.val.numero==0)||(der.tipo=='E'&&*der.val.entero==0)){
                *error=-1;
                printf("\nERROR: DIVISION ENTRE CERO.\n");
                return rpta;
            }

            if(izq.tipo=='N'){
                rpta.tipo='N';
                rpta.val.numero=(double*)malloc(sizeof(double));
                *rpta.val.numero = (int)*izq.val.numero % (int)*der.val.numero;
            }else{
                rpta.tipo='E';
                rpta.val.entero=(int*)malloc(sizeof(int));
                *rpta.val.entero = *izq.val.entero % *der.val.entero;
            }

            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"^")==0){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(izq.tipo!=der.tipo){
                *error=-1;
                printf("\nERROR: Está intentando exponenciar valores de distinto tipo"
                       " (ambos deben ser enteros o numeros con decimales).\n");
                return rpta;
            }

            if(izq.tipo!='N' && izq.tipo!='E'){
                *error=-1;
                printf("\nERROR: Está intentando exponenciar valores no numéricos.\n");
                return rpta;
            }
            else if((der.tipo=='N'&&*der.val.numero<0)||(der.tipo=='E'&&*der.val.entero<0)){
                *error=-1;
                printf("\nERROR: NO PUEDE ELEVAR A UN NUMERO MENOR QUE CERO.\n");
                return rpta;
            }

            if(izq.tipo=='N'){
                rpta.tipo='N';
                rpta.val.numero=(double*)malloc(sizeof(double));
                *rpta.val.numero = pow(*izq.val.numero, *der.val.numero);
            }else{
                rpta.tipo='E';
                rpta.val.entero=(int*)malloc(sizeof(int));
                *rpta.val.entero = pow(*izq.val.entero, *der.val.entero);
            }

            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"+")==0 && expresion.izquierda==NULL){ //+ unario
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(der.tipo!='N' && der.tipo!='E'){
                *error=-1;
                printf("\nERROR: Está intentando ponerle signo mas a un valor no numérico.\n");
                return rpta;
            }

            if(der.tipo=='N'){
                rpta.tipo='N';
                rpta.val.numero=(double*)malloc(sizeof(double));
                *rpta.val.numero = *der.val.numero;
            }else{
                rpta.tipo='E';
                rpta.val.entero=(int*)malloc(sizeof(int));
                *rpta.val.entero = *der.val.entero;
            }

            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"-")==0 && expresion.izquierda==NULL){ //- unario
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(der.tipo!='N' && der.tipo!='E'){
                *error=-1;
                printf("\nERROR: Está intentando ponerle signo menos a un valor no numérico.\n");
                return rpta;
            }

            if(der.tipo=='N'){
                rpta.tipo='N';
                rpta.val.numero=(double*)malloc(sizeof(double));
                *rpta.val.numero = -*der.val.numero;
            }else{
                rpta.tipo='E';
                rpta.val.entero=(int*)malloc(sizeof(int));
                *rpta.val.entero = -*der.val.entero;
            }

            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,">")==0){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if((izq.tipo!='N' && izq.tipo!='E') || (der.tipo!='N' && der.tipo!='E')){
                *error=-1;
                printf("\nERROR: Está intentando determinar cual es mayor en valores no numéricos.\n");
                return rpta;
            }

            rpta.tipo='B';
            rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
            *rpta.val.booleano = (izq.tipo=='N'?*izq.val.numero:*izq.val.entero) > (der.tipo=='N'?*der.val.numero:*der.val.entero);

            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"<")==0){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if((izq.tipo!='N' && izq.tipo!='E') || (der.tipo!='N' && der.tipo!='E')){
                *error=-1;
                printf("\nERROR: Está intentando determinar cual es menor en valores no numéricos.\n");
                return rpta;
            }

            rpta.tipo='B';
            rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
            *rpta.val.booleano = (izq.tipo=='N'?*izq.val.numero:*izq.val.entero) < (der.tipo=='N'?*der.val.numero:*der.val.entero);

            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,">=")==0){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if((izq.tipo!='N' && izq.tipo!='E') || (der.tipo!='N' && der.tipo!='E')){
                *error=-1;
                printf("\nERROR: Está intentando determinar cual es mayor o igual en valores no numéricos.\n");
                return rpta;
            }

            rpta.tipo='B';
            rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
            *rpta.val.booleano = (izq.tipo=='N'?*izq.val.numero:*izq.val.entero) >= (der.tipo=='N'?*der.val.numero:*der.val.entero);

            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"<=")==0){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            if((izq.tipo!='N' && izq.tipo!='E') || (der.tipo!='N' && der.tipo!='E')){
                *error=-1;
                printf("\nERROR: Está intentando determinar cual es menor o igual en valores no numéricos.\n");
                return rpta;
            }

            rpta.tipo='B';
            rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
            *rpta.val.booleano = (izq.tipo=='N'?*izq.val.numero:*izq.val.entero) <= (der.tipo=='N'?*der.val.numero:*der.val.entero);

            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"==")==0){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(izq.tipo!=der.tipo){
                *error=-1;
                printf("\nERROR: Está intentando determinar igualdad en valores de distinto tipo.\n");
                return rpta;
            }
            else if(izq.tipo=='L'){
                *error=-1;
                printf("\nERROR: No se puede determinar igualdad en el caso de listas.\n");
                return rpta;
            }
            else if(izq.tipo=='D'){
                *error=-1;
                printf("\nERROR: No se puede determinar igualdad en el caso de diccionarios.\n");
                return rpta;
            }
            else if(izq.tipo=='A'){
                *error=-1;
                printf("\nERROR: No se puede determinar igualdad en el caso de arreglos.\n");
                return rpta;
            }
            else if(izq.tipo=='C'){
                *error=-1;
                printf("\nERROR: No se puede determinar igualdad en el caso de objetos.\n");
                return rpta;
            }

            switch(izq.tipo){
                case 'N':
                    rpta.tipo='B';
                    rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                    *rpta.val.booleano = *izq.val.numero == *der.val.numero;

                    break;
                case 'E':
                    rpta.tipo='B';
                    rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                    *rpta.val.booleano = *izq.val.entero == *der.val.entero;

                    break;
                case 'B':
                    rpta.tipo='B';
                    rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                    *rpta.val.booleano = *izq.val.booleano == *der.val.booleano;

                    break;
                case 'T':
                    rpta.tipo='B';
                    rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                    *rpta.val.booleano = strcmp(izq.val.texto,der.val.texto)==0;

                    break;
                case 'V':
                    rpta.tipo='B';
                    rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                    *rpta.val.booleano = 1;//SI LOS DOS SON VACIO ES VERDADERO

                    break;
                case 'F':
                    rpta.tipo='B';
                    rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                    *rpta.val.booleano = izq.val.funcion==der.val.funcion;//SI APUNTAN A LA MISMA SUBRUTINA O METODO ES VERDADERO

                    break;
            }
            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"!=")==0){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            if(izq.tipo!=der.tipo){
                *error=-1;
                printf("\nERROR: Está intentando diferenciar en valores de distinto tipo.\n");
                return rpta;
            }
            else if(izq.tipo=='L'){
                *error=-1;
                printf("\nERROR: No se puede utilizar operador diferencia (!=) en el caso de listas.\n");
                return rpta;
            }
            else if(izq.tipo=='D'){
                *error=-1;
                printf("\nERROR: No se puede utilizar operador diferencia (!=) en el caso de diccionarios.\n");
                return rpta;
            }
            else if(izq.tipo=='A'){
                *error=-1;
                printf("\nERROR: No se puede utilizar operador diferencia (!=) en el caso de arreglos.\n");
                return rpta;
            }
            else if(izq.tipo=='C'){
                *error=-1;
                printf("\nERROR: No se puede utilizar operador diferencia (!=) en el caso de objetos.\n");
                return rpta;
            }

            switch(izq.tipo){
                case 'N':
                    rpta.tipo='B';
                    rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                    *rpta.val.booleano = *izq.val.numero != *der.val.numero;

                    break;
                case 'E':
                    rpta.tipo='B';
                    rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                    *rpta.val.booleano = *izq.val.entero != *der.val.entero;

                    break;
                case 'B':
                    rpta.tipo='B';
                    rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                    *rpta.val.booleano = *izq.val.booleano != *der.val.booleano;

                    break;
                case 'T':
                    rpta.tipo='B';
                    rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                    *rpta.val.booleano = strcmp(izq.val.texto,der.val.texto)!=0;

                    break;
                case 'V':
                    rpta.tipo='B';
                    rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                    *rpta.val.booleano = 0;//SI LOS DOS SON VACIO ES FALSO

                    break;
                case 'F':
                    rpta.tipo='B';
                    rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                    *rpta.val.booleano = izq.val.funcion!=der.val.funcion;//SI APUNTAN A DISTINTA SUBRUTINA O METODO ES VERDADERO

                    break;
            }
            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"Y")==0){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(izq.tipo!='B' || der.tipo!='B'){
                *error=-1;
                printf("\nERROR: Está intentando usar Y lógico en valores que no son de tipo verdadero o falso.\n");
                return rpta;
            }

            rpta.tipo='B';
            rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
            *rpta.val.booleano = *izq.val.booleano && *der.val.booleano;

            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"O")==0){ //+ binario
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(izq.tipo!='B' || der.tipo!='B'){
                *error=-1;
                printf("\nERROR: Está intentando usar O lógico en valores que no son de tipo verdadero o falso.\n");
                return rpta;
            }

            rpta.tipo='B';
            rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
            *rpta.val.booleano = *izq.val.booleano || *der.val.booleano;

            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"NEGACION")==0){ //Unario
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(der.tipo!='B'){
                *error=-1;
                printf("\nERROR: Está intentando usar NEGACION lógica en un valor que no es del tipo verdadero o falso.\n");
                return rpta;
            }

            rpta.tipo='B';
            rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
            *rpta.val.booleano = !*der.val.booleano;

            limpiar_valor(izq);
            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"=")==0){ //binario
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(expresion.izquierda->tipo=='I'){
                if(strcmp(expresion.izquierda->texto,"ESTE")==0){
                    limpiar_valor(der);
                    printf("\nEL IDENTIFICADOR ESTE ESTA RESERVADO, NO PUEDE UTILIZARLO\n");
                    *error=-1;
                    return rpta;
                }
                asignar_elemento_tabla_simbolos(expresion.izquierda->texto, der,TablaSimbolos);
            }else{
                izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,1);COMPROBAR_ERROR_EXPRESION

                if(izq.referencia==NULL){
                    limpiar_valor(izq);
                    limpiar_valor(der);

                    *error=-1;
                    printf("\nERROR: Lo que se encuentra a la izquierda del operador de asignación =\n"
                           " No es apto para asignarle un valor.\n"
                           " Debería ser un identificador (variable). o un identificador"
                           " (de lista, arreglo o diccionario) indexado con corchetes.\n"
                           " NO SE PUEDE asignar un valor a una sublista o subarreglo (obtenidos indexando con dos índices).\n"  );
                    return rpta;
                }

                limpiar_valor(*izq.referencia);
                *izq.referencia=der;
            }

            rpta = copiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"IN")==0){ //binario INDEXACION
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,referencia);COMPROBAR_ERROR_EXPRESION
            der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION

            if(der.tipo=='E'){
                if(izq.tipo=='L'){
                    if((int)*der.val.entero<1){
                        printf("\nERROR: No se permiten índices menores a 1\n");
                        *error=-1;
                        return rpta;
                    }
                    if((int)*der.val.entero>izq.val.lista->num_elems){
                        printf("\nERROR: Indice fuera de rango, la lista contiene solo %d"
                               " elementos y usted está intentando acceder al elemento %d\n",
                               izq.val.lista->num_elems,(int)*der.val.entero);
                        *error=-1;
                        return rpta;
                    }

                    rpta=recuperar_elemento_lista(&izq.val.lista->primer_elem,
                          (int)*der.val.entero, error,referencia);COMPROBAR_ERROR_EXPRESION
                }
                else if(izq.tipo=='T'){
                    if((int)*der.val.entero<1){
                        printf("\nERROR: No se permiten índices menores a 1\n");
                        *error=-1;
                        return rpta;
                    }
                    if((int)*der.val.entero>strlen(izq.val.texto)){
                        printf("\nERROR: Indice fuera de rango, el texto contiene solo %d"
                               " caracteres y usted está intentando acceder al caracter %d\n",
                               strlen(izq.val.texto),(int)*der.val.entero);
                        *error=-1;
                        return rpta;
                    }

                    rpta.tipo='T';
                    rpta.val.texto=(char*)malloc(2);
                    rpta.val.texto[0]=izq.val.texto[(int)*der.val.entero-1];
                    rpta.val.texto[1]='\0';
                }
                else if(izq.tipo=='A'){
                    if((int)*der.val.entero<1){
                        printf("\nERROR: No se permiten índices menores a 1\n");
                        *error=-1;
                        return rpta;
                    }
                    if((int)*der.val.entero>izq.val.arreglo->num_elems){
                        printf("\nERROR: Indice fuera de rango, el arreglo contiene solo %d"
                               " elementos y usted está intentando acceder al elemento %d\n",
                               izq.val.arreglo->num_elems,(int)*der.val.entero);
                        *error=-1;
                        return rpta;
                    }

                    if(referencia){
                        rpta=izq.val.arreglo->elems[(int)*der.val.entero-1];
                        rpta.referencia = &izq.val.arreglo->elems[(int)*der.val.entero-1];
                    }
                    else
                        rpta=copiar_valor(izq.val.arreglo->elems[(int)*der.val.entero-1]);
                }
                else{
                    printf("\nERROR: Solo se pueden indexar con enteros: Textos, Arreglos y Listas\n");
                    *error=-1;
                    return rpta;
                }
            }
            else if(der.tipo=='L'){
                if(izq.tipo=='L'){
                    if(der.val.lista->num_elems!=2){
                        printf("\nERROR: Solo se pueden indexar una lista con 1 o 2 indices,"
                               "en el segundo caso el primero sería el indice de inicio y el segundo el indice de fin.\n"
                               "Está tratando de indexarlo con %d indices.",der.val.lista->num_elems);
                        *error=-1;
                        return rpta;
                    }
                    if(der.val.lista->primer_elem.valor.tipo!='E'){
                        printf("\nERROR: Los índices deben ser enteros, "
                               "el índice inicial no lo es, es de tipo %s.",texto_tipo(der.val.lista->primer_elem.valor.tipo));
                        *error=-1;
                        return rpta;
                    }
                    if(der.val.lista->primer_elem.sgte->valor.tipo!='E'){
                        printf("\nERROR: Los índices deben ser enteros, "
                               "el índice final no lo es, es de tipo %s.",texto_tipo(der.val.lista->primer_elem.sgte->valor.tipo));
                        *error=-1;
                        return rpta;
                    }
                    int indice_ini = (int)*der.val.lista->primer_elem.valor.val.entero,
                        indice_fin = (int)*der.val.lista->primer_elem.sgte->valor.val.entero;
                    
                    if(indice_ini<1){
                        printf("\nERROR: Primer índice menor a 1, No se permiten índices menores a 1\n");
                        *error=-1;
                        return rpta;
                    }
                    if(indice_ini>izq.val.lista->num_elems){
                        printf("\nERROR: Primer Indice fuera de rango, la lista contiene solo %d"
                               " elementos y usted está intentando acceder al elemento %d\n",
                               izq.val.lista->num_elems,indice_ini);
                        *error=-1;
                        return rpta;
                    }
                    if(indice_ini>=indice_fin){
                        printf("\nERROR: El segundo Indice debe ser mayor que el primero, esta usando %d"
                               " como primer índice y %d como segundo.\n",
                               indice_ini,indice_fin);
                        *error=-1;
                        return rpta;
                    }
                    if(indice_fin>izq.val.lista->num_elems){
                        printf("\nERROR: Segundo Indice fuera de rango, la lista contiene solo %d"
                               " elementos y usted está intentando acceder al elemento %d\n",
                               izq.val.lista->num_elems,indice_fin);
                        *error=-1;
                        return rpta;
                    }

                    rpta.tipo='L';
                    rpta.val.lista = (struct Lista*)malloc(sizeof(struct Lista));
                    *rpta.val.lista = recuperar_elementos_lista(*izq.val.lista, indice_ini, indice_fin, error);COMPROBAR_ERROR_EXPRESION

                }
                else if(izq.tipo=='T'){
                    if(der.val.lista->num_elems!=2){
                        printf("\nERROR: Solo se pueden indexar un texto con 1 o 2 indices,"
                               "en el segundo caso el primero sería el indice de inicio y el segundo el indice de fin.\n"
                               "Está tratando de indexarlo con %d indices.",der.val.lista->num_elems);
                        *error=-1;
                        return rpta;
                    }
                    if(der.val.lista->primer_elem.valor.tipo!='E'){
                        printf("\nERROR: Los índices deben ser enteros, "
                               "el índice inicial no lo es, es de tipo %s.",texto_tipo(der.val.lista->primer_elem.valor.tipo));
                        *error=-1;
                        return rpta;
                    }
                    if(der.val.lista->primer_elem.sgte->valor.tipo!='E'){
                        printf("\nERROR: Los índices deben ser enteros, "
                               "el índice final no lo es, es de tipo %s.",texto_tipo(der.val.lista->primer_elem.sgte->valor.tipo));
                        *error=-1;
                        return rpta;
                    }
                    int indice_ini = (int)*der.val.lista->primer_elem.valor.val.entero,
                        indice_fin = (int)*der.val.lista->primer_elem.sgte->valor.val.entero;
                    
                    if(indice_ini<1){
                        printf("\nERROR: Primer índice menor a 1, No se permiten índices menores a 1\n");
                        *error=-1;
                        return rpta;
                    }
                    if(indice_ini>strlen(izq.val.texto)){
                        printf("\nERROR: Primer Indice fuera de rango, el texto contiene solo %d"
                               " caracteres y usted está intentando acceder al caracter %d\n",
                               strlen(izq.val.texto),indice_ini);
                        *error=-1;
                        return rpta;
                    }
                    if(indice_ini>=indice_fin){
                        printf("\nERROR: El segundo Indice debe ser mayor que el primero, esta usando %d"
                               " como primer índice y %d como segundo.\n",
                               indice_ini,indice_fin);
                        *error=-1;
                        return rpta;
                    }
                    if(indice_fin>strlen(izq.val.texto)){
                        printf("\nERROR: Segundo Indice fuera de rango, el texto contiene solo %d"
                               " caracteres y usted está intentando acceder al caracter %d.\n",
                               strlen(izq.val.texto),indice_fin);
                        *error=-1;
                        return rpta;
                    }

                    rpta.tipo='T';
                    rpta.val.texto = (char*)malloc(indice_fin-indice_ini+2);

                    int j=indice_ini;
                    for(;j<=indice_fin;j++)
                        rpta.val.texto[j-indice_ini]=izq.val.texto[j-1];
                    rpta.val.texto[indice_fin-indice_ini+1]='\0';

                }else if(izq.tipo=='A'){
                    if(der.val.lista->num_elems!=2){
                        printf("\nERROR: Solo se pueden indexar un texto con 1 o 2 indices,"
                               "en el segundo caso el primero sería el indice de inicio y el segundo el indice de fin.\n"
                               "Está tratando de indexarlo con %d indices.",der.val.lista->num_elems);
                        *error=-1;
                        return rpta;
                    }
                    if(der.val.lista->primer_elem.valor.tipo!='E'){
                        printf("\nERROR: Los índices deben ser enteros, "
                               "el índice inicial no lo es, es de tipo %s.",texto_tipo(der.val.lista->primer_elem.valor.tipo));
                        *error=-1;
                        return rpta;
                    }
                    if(der.val.lista->primer_elem.sgte->valor.tipo!='E'){
                        printf("\nERROR: Los índices deben ser enteros, "
                               "el índice final no lo es, es de tipo %s.",texto_tipo(der.val.lista->primer_elem.sgte->valor.tipo));
                        *error=-1;
                        return rpta;
                    }
                    int indice_ini = (int)*der.val.lista->primer_elem.valor.val.entero,
                        indice_fin = (int)*der.val.lista->primer_elem.sgte->valor.val.entero;
                    
                    if(indice_ini<1){
                        printf("\nERROR: Primer índice menor a 1, No se permiten índices menores a 1\n");
                        *error=-1;
                        return rpta;
                    }
                    if(indice_ini>izq.val.arreglo->num_elems){
                        printf("\nERROR: Primer Indice fuera de rango, el Arreglo contiene solo %d"
                               " elementos y usted está intentando acceder al elemento %d\n",
                               izq.val.arreglo->num_elems,indice_ini);
                        *error=-1;
                        return rpta;
                    }
                    if(indice_ini>=indice_fin){
                        printf("\nERROR: El segundo Indice debe ser mayor que el primero, esta usando %d"
                               " como primer índice y %d como segundo.\n",
                               indice_ini,indice_fin);
                        *error=-1;
                        return rpta;
                    }
                    if(indice_fin>izq.val.arreglo->num_elems){
                        printf("\nERROR: Segundo Indice fuera de rango, el Arreglo contiene solo %d"
                               " elementos y usted está intentando acceder al elemento %d.\n",
                               izq.val.arreglo->num_elems,indice_fin);
                        *error=-1;
                        return rpta;
                    }

                    rpta.tipo='A';
                    rpta.val.arreglo = (struct Arreglo*)malloc(sizeof(struct Arreglo));
                    rpta.val.arreglo->num_elems= indice_fin-indice_ini+1;
                    rpta.val.arreglo->elems = (struct Valor*)malloc(rpta.val.arreglo->num_elems*sizeof(struct Valor));

                    int j=indice_ini;
                    for(;j<=indice_fin;j++)
                        rpta.val.arreglo->elems[j-indice_ini]=copiar_valor(izq.val.arreglo->elems[j-1]);

                }else{
                    printf("\nERROR: Solo se pueden indexar con dos índices: Textos, Arreglos y Listas\n");
                    *error=-1;
                    return rpta;
                }
            }else if(der.tipo=='T'){
                if(izq.tipo!='D'){
                    printf("\nERROR: SOLO SE PUEDE USAR TEXTO COMO INDICE EN EL CASO DE DICCIONARIOS\n");
                    *error=-1;
                    return rpta;
                }
                if(existe_clave(izq.val.diccionario->datos,der.val.texto))
                    rpta = recuperar_elemento_tabla_simbolos(der.val.texto, error,izq.val.diccionario->datos);
                else if(referencia){
                    asignar_elemento_tabla_simbolos(der.val.texto, rpta,izq.val.diccionario->datos);
                    rpta = recuperar_elemento_tabla_simbolos(der.val.texto, error,izq.val.diccionario->datos);
                }else{
                    printf("\nERROR: LA CLAVE %s NO EXISTE EN EL DICCIONARIO.\n",der.val.texto);
                    *error=-1;
                    return rpta;
                }
            }else{
                printf("\nERROR: LOS INDICES SOLO PUEDEN SER ENTEROS O TEXTOS\n");
                *error=-1;
                return rpta;
            }

            limpiar_valor(der);
        }
        else if(strcmp(expresion.operacion,"SR")==0){ //binario LLAMADA A SUBRUTINA
            _Bool izq_es_id = 0;
            struct BloqueFuncion funcion;
            if(expresion.izquierda->tipo=='I'){
                if(creacion_clase(expresion.izquierda->texto,&rpta,error)){
                    return rpta;
                }

                izq_es_id=1;
                if(!es_funcion_predefinida(expresion.izquierda->texto)){
                     if(existe_funcion(TablaSimbolosFunciones,expresion.izquierda->texto))
                         funcion = recuperar_elemento_tabla_simbolos_funciones(expresion.izquierda->texto,error,
                                             TablaSimbolosFunciones);
                     else{
                         izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
                         if(izq.tipo=='F')
                             funcion = *izq.val.funcion;
                         else{
                            *error=-1;
                            printf("\nERROR: ESTA INTENTANDO LLAMAR COMO SUBRUTINA A UN VALOR DEL TIPO: %s\n",texto_tipo(izq.tipo));
                            return rpta;
                         }
                     }
                }              
            }else{
                izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
                if(izq.tipo=='F')
                    funcion = *izq.val.funcion;
                else{
                   *error=-1;
                   printf("\nERROR: ESTA INTENTANDO LLAMAR COMO SUBRUTINA A UN VALOR DEL TIPO: %s\n",texto_tipo(izq.tipo));
                   return rpta;
                }
            }

            _Bool tiene_argumentos=0;//false
            if(expresion.derecha!=NULL)
                tiene_argumentos=1;//true

            if(tiene_argumentos){
                der = resolver_expresion(*expresion.derecha,error,TablaSimbolos,0);COMPROBAR_ERROR_EXPRESION
            }

            if(strcmp(expresion.izquierda->texto,"ESCRIBIR")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA ESCRIBIR SIN ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }

                 escribir_valor(der,0);
                 printf("\n");
                 rpta.tipo='V';
            }else if(strcmp(expresion.izquierda->texto,"ESCRIBIR_TOKEN_CSRF")==0 && izq_es_id){
                 if(tiene_argumentos){
                     printf("\nERROR: SUBRUTINA ESCRIBIR_TOKEN_CSRF CON ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }

                 printf("<input type='hidden' name='CSRFToken'"
                        " value='OWY4NmQwODE4ODRjN2Q2NTlhMmZlYWEwYzU1YWQwMTVhM2JmNGYxYjJiMGI4MjJjZDE1ZDZjMTViMGYwMGEwOA=='>");
                 rpta.tipo='V';
            }else if(strcmp(expresion.izquierda->texto,"TIPO_DE")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA TIPO_DE SIN ARGUMENTO.\n");
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='T';
                 rpta.val.texto=(char*)malloc(strlen(texto_tipo(der.tipo))+1);
                 strcpy(rpta.val.texto,texto_tipo(der.tipo));
            }else if(strcmp(expresion.izquierda->texto,"CLASE_DE")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA CLASE_DE SIN ARGUMENTO.\n");
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='T';

                 if(der.tipo!='C'){
                     rpta.val.texto=(char*)malloc(strlen("NO ES OBJETO")+1);
                     strcpy(rpta.val.texto,"NO ES OBJETO");
                 }else{
                     rpta.val.texto=(char*)malloc(strlen(der.val.clase->nombre)+1);
                     strcpy(rpta.val.texto,der.val.clase->nombre);
                 }
            }else if(strcmp(expresion.izquierda->texto,"SQLITE3_CLOSE")==0 && izq_es_id){
                 if(tiene_argumentos){
                     printf("\nERROR: SUBRUTINA SQLITE3_CLOSE CON ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }

                 sqlite3_close(db);

            }else if(strcmp(expresion.izquierda->texto,"SQLITE3_FINALIZE")==0 && izq_es_id){
                 if(tiene_argumentos){
                     printf("\nERROR: SUBRUTINA SQLITE3_FINALIZE CON ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }

                 sqlite3_finalize(result);

            }else if(strcmp(expresion.izquierda->texto,"SQLITE3_OPEN")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA SQLITE3_OPEN SIN ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }
                 if(der.tipo!='T'){
                     printf("\nERROR: SUBRUTINA SQLITE3_OPEN CON ARGUMENTO QUE NO ES DE TIPO"
                            " TEXTO, USTED LE ESTA PASANDO ARGUMENTO DEL TIPO %s.\n",texto_tipo(der.tipo));
                     *error= -1;
                     return rpta;
                 }

                   // Abro la conexión con la base de datos
                   rc = sqlite3_open(der.val.texto, &db);
                   // Compruebo que no hay error
                   if (rc != SQLITE_OK) {
                     printf("\nERROR: No se puede acceder a la base de datos: %s.\n", sqlite3_errmsg(db));
                     sqlite3_close(db);
                     *error=-1;
                     return rpta;
                   }
            }else if(strcmp(expresion.izquierda->texto,"SQLITE3_PREPARE")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA SQLITE3_PREPARE SIN ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }
                 if(der.tipo!='T'){
                     printf("\nERROR: SUBRUTINA SQLITE3_PREPARE CON ARGUMENTO QUE NO ES DE TIPO"
                            " TEXTO, USTED LE ESTA PASANDO ARGUMENTO DEL TIPO %s.\n",texto_tipo(der.tipo));
                     *error= -1;
                     return rpta;
                 }
                 if(db==NULL){
                     printf("\nERROR: DEBE ABRIR UNA CONEXION ANTES DE PREPARAR UNA SENTENCIA");
                     *error= -1;
                     return rpta;
                 }

                   rc = sqlite3_prepare(db, der.val.texto, -1, &result, NULL);
                   // Compruebo que no hay error
                   if (rc != SQLITE_OK) {
                     printf("Error en la consulta: %s.\n", sqlite3_errmsg(db));
                     sqlite3_close(db);
                     *error=-1;
                     return rpta;
                   }
            }else if(strcmp(expresion.izquierda->texto,"TXT_A_NUM")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA TXT_A_NUM SIN ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }
                 if(der.tipo!='T'){
                     printf("\nERROR: SUBRUTINA TXT_A_NUM CON ARGUMENTO QUE NO ES DE TIPO"
                            " TEXTO, USTED LE ESTA PASANDO ARGUMENTO DEL TIPO %s.\n",texto_tipo(der.tipo));
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='N';
                 rpta.val.numero = (double*)malloc(sizeof(double));
                 sscanf(der.val.texto,"%lf",rpta.val.numero);
            }else if(strcmp(expresion.izquierda->texto,"LOG_SI_DETECTO_SQL_INJECTION")==0 && izq_es_id){
                if(!tiene_argumentos){
                    printf("\nERROR: LA SUBRUTINA LOG_SI_DETECTO_SQL_INJECTION NECESITA DOS ARGUMENTOS: ruta del log Y sql\n");
                    *error= -1;
                    return rpta;
                }else if(der.tipo!='L'){
                    printf("\nERROR: LA SUBRUTINA LOG_SI_DETECTO_SQL_INJECTION NECESITA DOS ARGUMENTOS: ruta del log Y sql\n"
                           "USTED LE ESTA PASANDO SOLO UN ARGUMENTO");
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->num_elems!=2){
                    printf("\nERROR: LA SUBRUTINA LOG_SI_DETECTO_SQL_INJECTION NECESITA DOS ARGUMENTOS: ruta del log Y sql\n"
                     "USTED LE ESTA PASANDO %d ARGUMENTOS",der.val.lista->num_elems);
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->primer_elem.valor.tipo!='T' || der.val.lista->primer_elem.sgte->valor.tipo!='T'){
                    printf("\nERROR: LA SUBRUTINA LOG_SI_DETECTO_SQL_INJECTION NECESITA DOS ARGUMENTOS: ruta del log Y sql\n"
                     "USTED LE ESTA PASANDO PRIMER ARGUMENTO DE TIPO %s Y SEGUNDO DE TIPO %s\n",
                      texto_tipo(der.val.lista->primer_elem.valor.tipo),
                      texto_tipo(der.val.lista->primer_elem.sgte->valor.tipo));
                    *error= -1;
                    return rpta;
                }

                 char *message=NULL;
                 if(detectar_sql_injection(der.val.lista->primer_elem.sgte->valor.val.texto,&message))
                     milog(der.val.lista->primer_elem.valor.val.texto,message);

                 if(message!=NULL)
                     free(message);

            }else if(strcmp(expresion.izquierda->texto,"NUM_A_ENT")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA NUM_A_ENT SIN ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }
                 if(der.tipo!='N'){
                     printf("\nERROR: SUBRUTINA NUM_A_ENT CON ARGUMENTO QUE NO ES DE TIPO"
                            " NUMERO CON DECIMALES, USTED LE ESTA PASANDO ARGUMENTO DEL TIPO %s.\n",texto_tipo(der.tipo));
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='E';
                 rpta.val.entero = (int*)malloc(sizeof(int));
                 *rpta.val.entero = (int)*der.val.numero;
            }else if(strcmp(expresion.izquierda->texto,"SQLITE3_COLUMN_TEXT")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA SQLITE3_COLUMN_TEXT SIN ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }
                 if(der.tipo!='E'){
                     printf("\nERROR: SUBRUTINA SQLITE3_COLUMN_TEXT CON ARGUMENTO QUE NO ES DE TIPO"
                            " ENTERO, USTED LE ESTA PASANDO ARGUMENTO DEL TIPO %s.\n",texto_tipo(der.tipo));
                     *error= -1;
                     return rpta;
                 }
                 if(result==NULL){
                     printf("\nERROR: ANTES DEBE PREPARAR Y EJECUTAR UNA SENTENCIA\n");
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='T';

                 if(sqlite3_column_text(result, *der.val.entero)==NULL){
                     rpta.val.texto=(char*)malloc(1);
                     strcpy(rpta.val.texto,"");
                 }else{
                     rpta.val.texto = (char*)malloc(strlen(sqlite3_column_text(result, *der.val.entero))+1);
                     strcpy(rpta.val.texto,sqlite3_column_text(result, *der.val.entero));
                 }
            }else if(strcmp(expresion.izquierda->texto,"SQLITE3_COLUMN_DOUBLE")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA SQLITE3_COLUMN_DOUBLE SIN ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }
                 if(der.tipo!='E'){
                     printf("\nERROR: SUBRUTINA SQLITE3_COLUMN_DOUBLE CON ARGUMENTO QUE NO ES DE TIPO"
                            " ENTERO, USTED LE ESTA PASANDO ARGUMENTO DEL TIPO %s.\n",texto_tipo(der.tipo));
                     *error= -1;
                     return rpta;
                 }
                 if(result==NULL){
                     printf("\nERROR: ANTES DEBE PREPARAR Y EJECUTAR UNA SENTENCIA\n");
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='N';
                 rpta.val.numero = (double*)malloc(sizeof(double));
                 *rpta.val.numero = sqlite3_column_double(result, *der.val.entero);
            }else if(strcmp(expresion.izquierda->texto,"SQLITE3_COLUMN_INT")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA SQLITE3_COLUMN_INT SIN ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }
                 if(der.tipo!='E'){
                     printf("\nERROR: SUBRUTINA SQLITE3_COLUMN_INT CON ARGUMENTO QUE NO ES DE TIPO"
                            " ENTERO, USTED LE ESTA PASANDO ARGUMENTO DEL TIPO %s.\n",texto_tipo(der.tipo));
                     *error= -1;
                     return rpta;
                 }
                 if(result==NULL){
                     printf("\nERROR: ANTES DEBE PREPARAR Y EJECUTAR UNA SENTENCIA\n");
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='E';
                 rpta.val.entero = (int*)malloc(sizeof(int));
                 *rpta.val.entero = sqlite3_column_int(result, *der.val.entero);
            }else if(strcmp(expresion.izquierda->texto,"ENT_A_NUM")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA ENT_A_NUM SIN ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }
                 if(der.tipo!='E'){
                     printf("\nERROR: SUBRUTINA ENT_A_NUM CON ARGUMENTO QUE NO ES DE TIPO"
                            " ENTERO, USTED LE ESTA PASANDO ARGUMENTO DEL TIPO %s.\n",texto_tipo(der.tipo));
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='N';
                 rpta.val.numero = (double*)malloc(sizeof(double));
                 *rpta.val.numero = *der.val.entero;
            }else if(strcmp(expresion.izquierda->texto,"NUM_A_TXT")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA NUM_A_TXT SIN ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }
                 if(der.tipo!='N'){
                     printf("\nERROR: SUBRUTINA NUM_A_TXT CON ARGUMENTO QUE NO ES DE TIPO"
                            " NUMERO, USTED LE ESTA PASANDO ARGUMENTO DEL TIPO %s.\n",texto_tipo(der.tipo));
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='T';
                 char cadena[100];
                 sprintf(cadena,"%lf",*der.val.numero);
                 rpta.val.texto = (char*)malloc(strlen(cadena)+1);
                 strcpy(rpta.val.texto,cadena);
            }else if(strcmp(expresion.izquierda->texto,"TXT_A_ENT")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA TXT_A_ENT SIN ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }
                 if(der.tipo!='T'){
                     printf("\nERROR: SUBRUTINA TXT_A_ENT CON ARGUMENTO QUE NO ES DE TIPO"
                            " TEXTO, USTED LE ESTA PASANDO ARGUMENTO DEL TIPO %s.\n",texto_tipo(der.tipo));
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='E';
                 rpta.val.entero = (int*)malloc(sizeof(int));
                 sscanf(der.val.texto,"%d",rpta.val.entero);
            }else if(strcmp(expresion.izquierda->texto,"ENT_A_TXT")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA ENT_TXT SIN ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }
                 if(der.tipo!='E'){
                     printf("\nERROR: SUBRUTINA ENT_TXT CON ARGUMENTO QUE NO ES DE TIPO"
                            " ENTERO, USTED LE ESTA PASANDO ARGUMENTO DEL TIPO %s.\n",texto_tipo(der.tipo));
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='T';
                 char cadena[100];
                 sprintf(cadena,"%d",(int)*der.val.entero);
                 rpta.val.texto = (char*)malloc(strlen(cadena)+1);
                 strcpy(rpta.val.texto,cadena);
            }else if(strcmp(expresion.izquierda->texto,"LEER_NUMERO")==0 && izq_es_id){
                 if(tiene_argumentos){
                     printf("\nERROR: SUBRUTINA LEER_NUMERO CON ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='N';
                 rpta.val.numero = (double*)malloc(sizeof(double));

                 char linea_codigo[250];
                 scanf("%249[^\n]",linea_codigo); LIMPIA_ENTRADA_SOBRA
                 linea_codigo[249] = '\0';
                 sscanf(linea_codigo,"%lf",rpta.val.numero);
            }else if(strcmp(expresion.izquierda->texto,"OBTENER_POST")==0 && izq_es_id){
                 if(tiene_argumentos){
                     printf("\nERROR: SUBRUTINA OBTENER_POST CON ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }

                 struct Par pares[MAXPARES];
                 int cant_pares=0;

                 if(obtener_datos_post(pares, &cant_pares)==0){
                     rpta.tipo='D';
                     rpta.val.diccionario=(struct Diccionario*)malloc(sizeof(struct Diccionario));
                     inicializar_tabla(rpta.val.diccionario->datos);

                     //REVISAR CSRF AUTOMATICAMENTE
                     int i=0;
                     _Bool csrf_verificado=0;

                     for(;i<cant_pares;i++)
                         if(strcmp(pares[i].nombre,"CSRFToken")==0 && strcmp(pares[i].valor,
                            "OWY4NmQwODE4ODRjN2Q2NTlhMmZlYWEwYzU1YWQwMTVhM2JmNGYxYjJiMGI4MjJjZDE1ZDZjMTViMGYwMGEwOA==")==0)
                              csrf_verificado=1;

                     if(!csrf_verificado){
                         printf("\nERROR: SUBRUTINA OBTENER_POST NO PUDO VERIFICAR SEGURIDAD FRENTE A CSRF, \n"
                                "DEBE LLAMAR A LA SUBRUTINA ESCRIBIR_TOKEN_CSRF DENTRO DE CADA FORMULARIO\n");
                         *error= -1;
                         return rpta;
                     }
                     ////////////////////////////////777

                     struct Valor val;

                     for(i=0;i<cant_pares;i++){
                         val.tipo='T';
                         val.val.texto=(char*)malloc(strlen(pares[i].valor)+1);
                         strcpy(val.val.texto,pares[i].valor);
                         asignar_elemento_tabla_simbolos(pares[i].nombre, val,rpta.val.diccionario->datos);
                     }
                 }
                 else
                     rpta.tipo='V';
            }else if(strcmp(expresion.izquierda->texto,"OBTENER_GET")==0 && izq_es_id){
                 if(tiene_argumentos){
                     printf("\nERROR: SUBRUTINA OBTENER_GET CON ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }

                 struct Par pares[MAXPARES];
                 int cant_pares=0;

                 if(obtener_datos_get(pares, &cant_pares)==0){
                     rpta.tipo='D';
                     rpta.val.diccionario=(struct Diccionario*)malloc(sizeof(struct Diccionario));
                     inicializar_tabla(rpta.val.diccionario->datos);

                     struct Valor val;

                     int i=0;
                     for(;i<cant_pares;i++){
                         val.tipo='T';
                         val.val.texto=(char*)malloc(strlen(pares[i].valor)+1);
                         strcpy(val.val.texto,pares[i].valor);
                         asignar_elemento_tabla_simbolos(pares[i].nombre, val,rpta.val.diccionario->datos);
                     }
                 }
                 else
                     rpta.tipo='V';
            }else if(strcmp(expresion.izquierda->texto,"SQLITE3_STEP")==0 && izq_es_id){
                 if(tiene_argumentos){
                     printf("\nERROR: SUBRUTINA SQLITE3_STEP CON ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }

                 if(result==NULL){
                     printf("\nERROR: ANTES DEBE PREPARAR UNA SENTENCIA\n");
                     *error= -1;
                     return rpta;
                 }

                rpta.tipo='B';
                rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                *rpta.val.booleano = (sqlite3_step(result) == SQLITE_ROW);
            }else if(strcmp(expresion.izquierda->texto,"LEER_ENTERO")==0 && izq_es_id){
                 if(tiene_argumentos){
                     printf("\nERROR: SUBRUTINA LEER_ENTERO CON ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='E';
                 rpta.val.entero = (int*)malloc(sizeof(int));

                 char linea_codigo[250];
                 scanf("%249[^\n]",linea_codigo); LIMPIA_ENTRADA_SOBRA
                 linea_codigo[249] = '\0';
                 sscanf(linea_codigo,"%d",rpta.val.entero);
            }else if(strcmp(expresion.izquierda->texto,"LEER_TEXTO")==0 && izq_es_id){
                 if(tiene_argumentos){
                     printf("\nERROR: SUBRUTINA LEER_TEXTO CON ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='T';

                 char linea_codigo[250];
                 scanf("%249[^\n]",linea_codigo); LIMPIA_ENTRADA_SOBRA
                 linea_codigo[249] = '\0';
                 rpta.val.texto=(char*)malloc(strlen(linea_codigo)+1);
                 strcpy(rpta.val.texto,linea_codigo);
            }else if(strcmp(expresion.izquierda->texto,"DICCIONARIO")==0 && izq_es_id){
                if(tiene_argumentos){
                    printf("\nERROR: SUBRUTINA DICCIONARIO CON ARGUMENTOS\n");
                    *error= -1;
                    return rpta;
                }

                rpta.tipo='D';
                rpta.val.diccionario=(struct Diccionario*)malloc(sizeof(struct Diccionario));
                inicializar_tabla(rpta.val.diccionario->datos);
            }else if(strcmp(expresion.izquierda->texto,"SQLITE3_BIND_DOUBLE")==0 && izq_es_id){
                if(!tiene_argumentos){
                    printf("\nERROR: LA SUBRUTINA SQLITE3_BIND_DOUBLE NECESITA DOS ARGUMENTOS:"
" TEXTO DEL PARAMETRO Y EL VALOR DEL PARAMETRO (NUMERO CON DECIMALES)\n");
                    *error= -1;
                    return rpta;
                }else if(der.tipo!='L'){
                    printf("\nERROR: LA SUBRUTINA SQLITE3_BIND_DOUBLE NECESITA DOS ARGUMENTOS: TEXTO DEL PARAMETRO Y EL VALOR DEL PARAMETRO (NUMERO CON DECIMALES)\n"
                           "USTED LE ESTA PASANDO SOLO UN ARGUMENTO");
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->num_elems!=2){
                    printf("\nERROR: LA SUBRUTINA SQLITE3_BIND_DOUBLE NECESITA DOS ARGUMENTOS: TEXTO DEL PARAMETRO Y EL VALOR DEL PARAMETRO (NUMERO CON DECIMALES)\n"
                     "USTED LE ESTA PASANDO %d ARGUMENTOS",der.val.lista->num_elems);
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->primer_elem.valor.tipo!='T' || der.val.lista->primer_elem.sgte->valor.tipo!='N'){
                    printf("\nERROR: LA SUBRUTINA SQLITE3_BIND_DOUBLE NECESITA DOS ARGUMENTOS: TEXTO DEL PARAMETRO Y EL VALOR DEL PARAMETRO (NUMERO CON DECIMALES)\n"
                     "USTED LE ESTA PASANDO PRIMER ARGUMENTO DE TIPO %s Y SEGUNDO DE TIPO %s\n",
                      texto_tipo(der.val.lista->primer_elem.valor.tipo),
                      texto_tipo(der.val.lista->primer_elem.sgte->valor.tipo));
                    *error= -1;
                    return rpta;
                }

                const int index = sqlite3_bind_parameter_index( result, der.val.lista->primer_elem.valor.val.texto );
                sqlite3_bind_double( result, index, *der.val.lista->primer_elem.sgte->valor.val.numero); 

            }else if(strcmp(expresion.izquierda->texto,"SQLITE3_BIND_INT")==0 && izq_es_id){
                if(!tiene_argumentos){
                    printf("\nERROR: LA SUBRUTINA SQLITE3_BIND_INT NECESITA DOS ARGUMENTOS: TEXTO DEL PARAMETRO Y EL VALOR DEL PARAMETRO (NUMERO ENTERO)\n");
                    *error= -1;
                    return rpta;
                }else if(der.tipo!='L'){
                    printf("\nERROR: LA SUBRUTINA SQLITE3_BIND_INT NECESITA DOS ARGUMENTOS: TEXTO DEL PARAMETRO Y EL VALOR DEL PARAMETRO (NUMERO ENTERO)\n"
                           "USTED LE ESTA PASANDO SOLO UN ARGUMENTO");
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->num_elems!=2){
                    printf("\nERROR: LA SUBRUTINA SQLITE3_BIND_INT NECESITA DOS ARGUMENTOS: TEXTO DEL PARAMETRO Y EL VALOR DEL PARAMETRO (NUMERO ENTERO)\n"
                     "USTED LE ESTA PASANDO %d ARGUMENTOS",der.val.lista->num_elems);
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->primer_elem.valor.tipo!='T' || der.val.lista->primer_elem.sgte->valor.tipo!='E'){
                    printf("\nERROR: LA SUBRUTINA SQLITE3_BIND_INT NECESITA DOS ARGUMENTOS: TEXTO DEL PARAMETRO Y EL VALOR DEL PARAMETRO (NUMERO ENTERO)\n"
                     "USTED LE ESTA PASANDO PRIMER ARGUMENTO DE TIPO %s Y SEGUNDO DE TIPO %s\n",
                      texto_tipo(der.val.lista->primer_elem.valor.tipo),
                      texto_tipo(der.val.lista->primer_elem.sgte->valor.tipo));
                    *error= -1;
                    return rpta;
                }

                const int index = sqlite3_bind_parameter_index( result, der.val.lista->primer_elem.valor.val.texto );
                sqlite3_bind_int( result, index, *der.val.lista->primer_elem.sgte->valor.val.entero); 

            }else if(strcmp(expresion.izquierda->texto,"SQLITE3_BIND_TEXT")==0 && izq_es_id){
                if(!tiene_argumentos){
                    printf("\nERROR: LA SUBRUTINA SQLITE3_BIND_TEXT NECESITA DOS ARGUMENTOS: TEXTO DEL PARAMETRO Y EL VALOR DEL PARAMETRO (TEXTO)\n");
                    *error= -1;
                    return rpta;
                }else if(der.tipo!='L'){
                    printf("\nERROR: LA SUBRUTINA SQLITE3_BIND_TEXT NECESITA DOS ARGUMENTOS: TEXTO DEL PARAMETRO Y EL VALOR DEL PARAMETRO (TEXTO)\n"
                           "USTED LE ESTA PASANDO SOLO UN ARGUMENTO");
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->num_elems!=2){
                    printf("\nERROR: LA SUBRUTINA SQLITE3_BIND_TEXT NECESITA DOS ARGUMENTOS: TEXTO DEL PARAMETRO Y EL VALOR DEL PARAMETRO (TEXTO)\n"
                     "USTED LE ESTA PASANDO %d ARGUMENTOS",der.val.lista->num_elems);
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->primer_elem.valor.tipo!='T' || der.val.lista->primer_elem.sgte->valor.tipo!='T'){
                    printf("\nERROR: LA SUBRUTINA SQLITE3_BIND_TEXT NECESITA DOS ARGUMENTOS: TEXTO DEL PARAMETRO Y EL VALOR DEL PARAMETRO (TEXTO)\n"
                     "USTED LE ESTA PASANDO PRIMER ARGUMENTO DE TIPO %s Y SEGUNDO DE TIPO %s\n",
                      texto_tipo(der.val.lista->primer_elem.valor.tipo),
                      texto_tipo(der.val.lista->primer_elem.sgte->valor.tipo));
                    *error= -1;
                    return rpta;
                }

                const int index = sqlite3_bind_parameter_index( result, der.val.lista->primer_elem.valor.val.texto );
                sqlite3_bind_text( result, index, der.val.lista->primer_elem.sgte->valor.val.texto,-1,SQLITE_STATIC ); 

            }else if(strcmp(expresion.izquierda->texto,"EXISTE_CLAVE")==0 && izq_es_id){
                if(!tiene_argumentos){
                    printf("\nERROR: LA SUBRUTINA EXISTE_CLAVE NECESITA DOS ARGUMENTOS: DICCIONARIO Y CLAVE\n");
                    *error= -1;
                    return rpta;
                }else if(der.tipo!='L'){
                    printf("\nERROR: LA SUBRUTINA EXISTE_CLAVE NECESITA DOS ARGUMENTOS: DICCIONARIO Y CLAVE\n"
                           "USTED LE ESTA PASANDO SOLO UN ARGUMENTO");
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->num_elems!=2){
                    printf("\nERROR: LA SUBRUTINA EXISTE_CLAVE NECESITA DOS ARGUMENTOS: DICCIONARIO Y CLAVE\n"
                     "USTED LE ESTA PASANDO %d ARGUMENTOS",der.val.lista->num_elems);
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->primer_elem.valor.tipo!='D' || der.val.lista->primer_elem.sgte->valor.tipo!='T'){
                    printf("\nERROR: LA SUBRUTINA EXISTE_CLAVE NECESITA DOS ARGUMENTOS: DICCIONARIO Y CLAVE\n"
                     "USTED LE ESTA PASANDO PRIMER ARGUMENTO DE TIPO %s Y SEGUNDO DE TIPO %s\n",
                      texto_tipo(der.val.lista->primer_elem.valor.tipo),
                      texto_tipo(der.val.lista->primer_elem.sgte->valor.tipo));
                    *error= -1;
                    return rpta;
                }

                rpta.tipo='B';
                rpta.val.booleano=(_Bool*)malloc(sizeof(_Bool));
                *rpta.val.booleano = existe_clave(
                    der.val.lista->primer_elem.valor.val.diccionario->datos,
                    der.val.lista->primer_elem.sgte->valor.val.texto
                );
            }else if(strcmp(expresion.izquierda->texto,"INDICE_EN_LISTA")==0 && izq_es_id){
                if(!tiene_argumentos){
                    printf("\nERROR: LA SUBRUTINA INDICE_EN_LISTA NECESITA DOS ARGUMENTOS: VALOR A BUSCAR Y LISTA\n"
                           "EL VALOR A BUSCAR DEBE SER UN DATO SIMPLE:\n"
                           "VALOR DE VERDAD, NUMERO CON DECIMALES O ENTERO, TEXTO O VACIO\n");
                    *error= -1;
                    return rpta;
                }else if(der.tipo!='L'){
                    printf("\nERROR: LA SUBRUTINA INDICE_EN_LISTA NECESITA DOS ARGUMENTOS: VALOR A BUSCAR Y LISTA\n"
                           "EL VALOR A BUSCAR DEBE SER UN DATO SIMPLE:\n"
                           "VALOR DE VERDAD, NUMEROCON DECIMALES O ENTERO, TEXTO O VACIO\n"
                           "USTED LE ESTA PASANDO SOLO UN ARGUMENTO\n");
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->num_elems!=2){
                    printf("\nERROR: LA SUBRUTINA INDICE_EN_LISTA NECESITA DOS ARGUMENTOS: VALOR A BUSCAR Y LISTA\n"
                           "EL VALOR A BUSCAR DEBE SER UN DATO SIMPLE:\n"
                           "VALOR DE VERDAD, NUMEROCON DECIMALES O ENTERO, TEXTO O VACIO\n");
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->primer_elem.sgte->valor.tipo!='L'){ 
                    printf("\nERROR: LA SUBRUTINA INDICE_EN_LISTA NECESITA DOS ARGUMENTOS: VALOR A BUSCAR Y LISTA\n"
                     "USTED LE ESTA PASANDO PRIMER ARGUMENTO DE TIPO %s Y SEGUNDO DE TIPO %s\n",
                      texto_tipo(der.val.lista->primer_elem.valor.tipo),
                      texto_tipo(der.val.lista->primer_elem.sgte->valor.tipo));
                    *error= -1;
                    return rpta;
                }else if((der.val.lista->primer_elem.valor.tipo=='B') ||
                 (der.val.lista->primer_elem.valor.tipo=='N') ||
                 (der.val.lista->primer_elem.valor.tipo=='E') ||
                 (der.val.lista->primer_elem.valor.tipo=='T') ||
                 (der.val.lista->primer_elem.valor.tipo=='V')){
                    rpta.tipo='E';
                    rpta.val.entero=(int*)malloc(sizeof(int));
                    *rpta.val.entero = -1;//SI NO LO ENCONTRO QUEDA EN -1
                    _Bool encontro=0;
                    struct Valor temp;
                    int i=0;
                    for(;i<der.val.lista->primer_elem.sgte->valor.val.lista->num_elems;i++){
                        temp=recuperar_elemento_lista(&der.val.lista->primer_elem.sgte->valor.val.lista->primer_elem, i+1, error,0);
                        if(temp.tipo!=der.val.lista->primer_elem.valor.tipo)
                            continue;
                        else{
                            switch(der.val.lista->primer_elem.valor.tipo){
                                case 'B':
                                    encontro = *der.val.lista->primer_elem.valor.val.booleano==*temp.val.booleano;
                                    break;
                                case 'N':
                                    encontro = *der.val.lista->primer_elem.valor.val.numero==*temp.val.numero;
                                    break;
                                case 'E':
                                    encontro = *der.val.lista->primer_elem.valor.val.entero==*temp.val.entero;
                                    break;
                                case 'T':
                                    encontro = strcmp(der.val.lista->primer_elem.valor.val.texto,temp.val.texto)==0;
                                    break;
                                case 'V': encontro = 1; break;
                            }
                        }
                        if(encontro){
                            *rpta.val.entero = i+1;
                            break;
                        }
                    }
                    limpiar_valor(temp);
                }else{
                    printf("\nERROR: LA SUBRUTINA INDICE_EN_LISTA NECESITA DOS ARGUMENTOS: VALOR A BUSCAR Y LISTA\n"
                     "USTED LE ESTA PASANDO PRIMER ARGUMENTO DE TIPO %s Y SEGUNDO DE TIPO %s\n"
                           "EL VALOR A BUSCAR DEBE SER UN DATO SIMPLE:\n"
                           "VALOR DE VERDAD, NUMERO, TEXTO O VACIO\n",
                      texto_tipo(der.val.lista->primer_elem.valor.tipo),
                      texto_tipo(der.val.lista->primer_elem.sgte->valor.tipo));
                    *error= -1;
                    return rpta;
                }
            }else if(strcmp(expresion.izquierda->texto,"INDICE_EN_ARREGLO")==0 && izq_es_id){
                if(!tiene_argumentos){
                    printf("\nERROR: LA SUBRUTINA INDICE_EN_ARREGLO NECESITA DOS ARGUMENTOS: VALOR A BUSCAR Y ARREGLO\n"
                           "EL VALOR A BUSCAR DEBE SER UN DATO SIMPLE:\n"
                           "VALOR DE VERDAD, NUMERO, TEXTO O VACIO\n");
                    *error= -1;
                    return rpta;
                }else if(der.tipo!='L'){
                    printf("\nERROR: LA SUBRUTINA INDICE_EN_LISTA NECESITA DOS ARGUMENTOS: VALOR A BUSCAR Y ARREGLO\n"
                           "EL VALOR A BUSCAR DEBE SER UN DATO SIMPLE:\n"
                           "VALOR DE VERDAD, NUMERO, TEXTO O VACIO\n"
                           "USTED LE ESTA PASANDO SOLO UN ARGUMENTO\n");
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->num_elems!=2){
                    printf("\nERROR: LA SUBRUTINA INDICE_EN_LISTA NECESITA DOS ARGUMENTOS: VALOR A BUSCAR Y ARREGLO\n"
                           "EL VALOR A BUSCAR DEBE SER UN DATO SIMPLE:\n"
                           "VALOR DE VERDAD, NUMERO, TEXTO O VACIO\n");
                    *error= -1;
                    return rpta;
                }else if(der.val.lista->primer_elem.sgte->valor.tipo!='A'){ 
                    printf("\nERROR: LA SUBRUTINA INDICE_EN_LISTA NECESITA DOS ARGUMENTOS: VALOR A BUSCAR Y ARREGLO\n"
                     "USTED LE ESTA PASANDO PRIMER ARGUMENTO DE TIPO %s Y SEGUNDO DE TIPO %s\n",
                      texto_tipo(der.val.lista->primer_elem.valor.tipo),
                      texto_tipo(der.val.lista->primer_elem.sgte->valor.tipo));
                    *error= -1;
                    return rpta;
                }else if((der.val.lista->primer_elem.valor.tipo=='B') ||
                 (der.val.lista->primer_elem.valor.tipo=='N') ||
                 (der.val.lista->primer_elem.valor.tipo=='E') ||
                 (der.val.lista->primer_elem.valor.tipo=='T') ||
                 (der.val.lista->primer_elem.valor.tipo=='V')){
                    rpta.tipo='E';
                    rpta.val.entero=(int*)malloc(sizeof(int));
                    *rpta.val.entero = -1;//SI NO LO ENCONTRO QUEDA EN -1
                    _Bool encontro=0;
                    struct Valor temp;
                    int i=0;
                    for(;i<der.val.lista->primer_elem.sgte->valor.val.arreglo->num_elems;i++){
                        temp=der.val.lista->primer_elem.sgte->valor.val.arreglo->elems[i];
                        if(temp.tipo!=der.val.lista->primer_elem.valor.tipo)
                            continue;
                        else{
                            switch(der.val.lista->primer_elem.valor.tipo){
                                case 'B':
                                    encontro = *der.val.lista->primer_elem.valor.val.booleano==*temp.val.booleano;
                                    break;
                                case 'N':
                                    encontro = *der.val.lista->primer_elem.valor.val.numero==*temp.val.numero;
                                    break;
                                case 'E':
                                    encontro = *der.val.lista->primer_elem.valor.val.entero==*temp.val.entero;
                                    break;
                                case 'T':
                                    encontro = strcmp(der.val.lista->primer_elem.valor.val.texto,temp.val.texto)==0;
                                    break;
                                case 'V': encontro = 1; break;
                            }
                        }
                        if(encontro){
                            *rpta.val.entero = i+1;
                            break;
                        }
                    }
                }else{
                    printf("\nERROR: LA SUBRUTINA INDICE_EN_ARREGLO DOS ARGUMENTOS: VALOR A BUSCAR Y ARREGLO\n"
                     "USTED LE ESTA PASANDO PRIMER ARGUMENTO DE TIPO %s Y SEGUNDO DE TIPO %s\n"
                           "EL VALOR A BUSCAR DEBE SER UN DATO SIMPLE:\n"
                           "VALOR DE VERDAD, NUMERO, TEXTO O VACIO\n",
                      texto_tipo(der.val.lista->primer_elem.valor.tipo),
                      texto_tipo(der.val.lista->primer_elem.sgte->valor.tipo));
                    *error= -1;
                    return rpta;
                }
            }else if(strcmp(expresion.izquierda->texto,"CLAVES")==0 && izq_es_id){
                if(!tiene_argumentos){
                    printf("\nERROR: SUBRUTINA CLAVES SIN ARGUMENTO\n");
                    *error= -1;
                    return rpta;
                }
                if(der.tipo!='D'){
                    printf("\nERROR: LA SUBRUTINA CLAVES RECIBE ARGUMENTO DE TIPO"
                           " DICCIONARIO, USTED LE ESTA PASANDO %s\n",texto_tipo(der.tipo));
                    *error= -1;
                    return rpta;
                }

                rpta.tipo='L';
                rpta.val.lista=(struct Lista*)malloc(sizeof(struct Lista));
                rpta.val.lista->num_elems=0;
                rpta.val.lista->primer_elem.sgte=NULL;

                int i=0,cantidad=cantidad_claves(der.val.diccionario->datos);
                struct Nodo nuevo_nodo;
                char ** claves;

                if(cantidad>0)
                    claves=devolver_claves(der.val.diccionario->datos,cantidad);

                for(;i<cantidad;i++){
                    nuevo_nodo.sgte=NULL;
                    nuevo_nodo.valor.tipo='T';
                    nuevo_nodo.valor.val.texto=(char*)malloc(strlen(claves[i])+1);
                    strcpy(nuevo_nodo.valor.val.texto,claves[i]);

                    if(rpta.val.lista->num_elems==0){
                        rpta.val.lista->primer_elem=nuevo_nodo;
                    }else{
                        concatenar_nodos(&rpta.val.lista->primer_elem,nuevo_nodo);
                    }
                    rpta.val.lista->num_elems++;
                }

                if(cantidad>0)
                    free(claves);                 
            }else if(strcmp(expresion.izquierda->texto,"ARREGLO_A_LISTA")==0 && izq_es_id){
                if(!tiene_argumentos){
                    printf("\nERROR: SUBRUTINA ARREGLO_A_LISTA SIN ARGUMENTO\n");
                    *error= -1;
                    return rpta;
                }
                if(der.tipo!='A'){
                    printf("\nERROR: LA SUBRUTINA ARREGLO_A_LISTA RECIBE ARGUMENTO DE TIPO"
                           " ARREGLOS, USTED LE ESTA PASANDO ARGUMENTO DE TIPO %s\n",texto_tipo(der.tipo));
                    *error= -1;
                    return rpta;
                }
                rpta.tipo='L';
                rpta.val.lista=(struct Lista*)malloc(sizeof(struct Lista));
                rpta.val.lista->num_elems=0;
                rpta.val.lista->primer_elem.sgte=NULL;

                struct Nodo nuevo_nodo;
                int i=0;
                for(;i<der.val.arreglo->num_elems;i++){
                    nuevo_nodo.sgte=NULL;
                    nuevo_nodo.valor=copiar_valor(der.val.arreglo->elems[i]);

                    if(rpta.val.lista->num_elems==0){
                        rpta.val.lista->primer_elem=nuevo_nodo;
                    }else{
                        concatenar_nodos(&rpta.val.lista->primer_elem,nuevo_nodo);
                    }
                    rpta.val.lista->num_elems++;
                }
            }else if(strcmp(expresion.izquierda->texto,"LISTA_A_ARREGLO")==0 && izq_es_id){
                if(!tiene_argumentos){
                    printf("\nERROR: SUBRUTINA LISTA_A_ARREGLO SIN ARGUMENTO\n");
                    *error= -1;
                    return rpta;
                }
                if(der.tipo!='L'){
                    printf("\nERROR: LA SUBRUTINA LISTA_A_ARREGLO RECIBE ARGUMENTO DE TIPO"
                           " LISTA, USTED LE ESTA PASANDO ARGUMENTO DE TIPO %s\n",texto_tipo(der.tipo));
                    *error= -1;
                    return rpta;
                }
                if(der.val.lista->num_elems==0){
                    printf("\nERROR: LA SUBRUTINA LISTA_A_ARREGLO DEBE RECIBIR UN ARREGLO CON AL MENOS UN ELEMENTO\n");
                    *error= -1;
                    return rpta;
                }
                rpta.tipo='A';
                rpta.val.arreglo=(struct Arreglo*)malloc(sizeof(struct Arreglo));
                rpta.val.arreglo->num_elems=der.val.lista->num_elems;
                rpta.val.arreglo->elems=(struct Valor*)malloc(sizeof(struct Valor)*rpta.val.arreglo->num_elems);

                int i=0;
                for(;i<rpta.val.arreglo->num_elems;i++)
                    rpta.val.arreglo->elems[i]=copiar_valor(recuperar_elemento_lista(&der.val.lista->primer_elem, i+1, error,0));
            }else if(strcmp(expresion.izquierda->texto,"LISTA")==0){
                 if(tiene_argumentos){
                     printf("\nERROR: SUBRUTINA LISTA CON ARGUMENTOS\n");
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='L';
                 rpta.val.lista=(struct Lista*)malloc(sizeof(struct Lista));
                 rpta.val.lista->num_elems=0;
                 rpta.val.lista->primer_elem.sgte=NULL;
            }else if(strcmp(expresion.izquierda->texto,"LONGITUD")==0 && izq_es_id){
                 int longitud=0;
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA LONGITUD SIN ARGUMENTOS, DEBE TENER UN ARGUMENTO DE TIPO:\n"
                            "LISTA, DICCIONARIO, ARREGLO O TEXTO.\n");
                     *error= -1;
                     return rpta;
                 }else if(der.tipo=='L'){ longitud=der.val.lista->num_elems;
                 }else if(der.tipo=='T'){ longitud=strlen(der.val.texto);
                 }else if(der.tipo=='D'){ longitud=cantidad_claves(der.val.diccionario->datos);
                 }else if(der.tipo=='A'){ longitud=der.val.arreglo->num_elems;
                 }else{
                     printf("\nERROR: SUBRUTINA LONGITUD CON ARGUMENTO EQUIVOCADO, DEBE TENER UN ARGUMENTO DE TIPO:\n"
                            "LISTA, DICCIONARIO, ARREGLO O TEXTO, Y USTED LE ESTA PASANDO UN ARGUMENTO DEL TIPO %s\n",texto_tipo(der.tipo));
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='E';
                 rpta.val.entero=(int*)malloc(sizeof(int));
                 *rpta.val.entero=longitud;
            }else if(strcmp(expresion.izquierda->texto,"ARREGLO")==0 && izq_es_id){
                 if(!tiene_argumentos){
                     printf("\nERROR: SUBRUTINA ARREGLO SIN ARGUMENTOS, DEBE TENER UN ARGUMENTO DE TIPO:\n"
                            "NUMERO, QUE INDIQUE LA CANTIDAD DE ELEMENTOS DEL ARREGLO A CREAR\n");
                     *error= -1;
                     return rpta;
                 }else if(der.tipo!='E'){
                     printf("\nERROR: LA SUBRUTINA ARREGLO DEBE TENER UN ARGUMENTO DE TIPO:\n"
                            "ENTERO, QUE INDIQUE LA CANTIDAD DE ELEMENTOS DEL ARREGLO A CREAR\n");
                     *error= -1;
                     return rpta;
                 }

                 rpta.tipo='A';
                 rpta.val.arreglo=(struct Arreglo*)malloc(sizeof(struct Arreglo));
                 rpta.val.arreglo->num_elems=(int)*der.val.entero;
                 rpta.val.arreglo->elems=(struct Valor*)malloc(sizeof(struct Valor)*rpta.val.arreglo->num_elems);
                 int i=0;
                 for(;i<rpta.val.arreglo->num_elems;i++){
                     rpta.val.arreglo->elems[i].tipo='V';
                     rpta.val.arreglo->elems[i].referencia=NULL;
                 }
            }else{

                //CHEQUEO COINCIDAN CANTIDAD DE PARAMETROS
                if(tiene_argumentos){
                    if(funcion.num_parametros==0){
                        printf("\nERROR: LA SUBRUTINA %s NO RECIBE ARGUMENTOS. USTED LE ESTA PASANDO ARGUMENTOS\n",
                               expresion.izquierda->texto);
                        *error=-1;
                        return rpta;
                    }

                    if(der.tipo!='L'){
                        if(funcion.num_parametros>1){
                            printf("\nERROR: LA SUBRUTINA %s RECIBE %d ARGUMENTOS. USTED LE ESTA PASANDO SOLO UN ARGUMENTO\n",
                                   expresion.izquierda->texto, funcion.num_parametros);
                            *error=-1;
                            return rpta;
                        }
                    }
                    else{
                        if(funcion.num_parametros!=der.val.lista->num_elems){
                            printf("\nERROR: LA SUBRUTINA %s RECIBE %d ARGUMENTOS. USTED LE ESTA PASANDO %d ARGUMENTOS\n",
                                   expresion.izquierda->texto, funcion.num_parametros, der.val.lista->num_elems);
                            *error=-1;
                            return rpta;
                        }
                    }
                }else{
                    if(funcion.num_parametros>0){
                        printf("\nERROR: USTED NO LE ESTA PASANDO ARGUMENTOS A LA SUBRUTINA %s, LA CUAL RECIBE %d ARGUMENTOS.\n",
                           expresion.izquierda->texto,funcion.num_parametros);
                        *error=-1;
                        return rpta;
                    }
                }
                //////////////////////////EJECUTO LA SUBRUTINA
                //CREO TABLA DE SIMBOLOS DE LA SUBRUTINA
                struct ElementoTabla *TablaSimbolosFuncion[TAMANO_TABLA];
                //inicializo
                inicializar_tabla(TablaSimbolosFuncion);

                //PASO PARAMETROS
                if(funcion.num_parametros==1){
                    asignar_elemento_tabla_simbolos(funcion.parametros[0], copiar_valor(der),TablaSimbolosFuncion);
                }else{
                    int i=0;
                    for(;i<funcion.num_parametros;i++)
                        asignar_elemento_tabla_simbolos(funcion.parametros[i], 
                          recuperar_elemento_lista(&der.val.lista->primer_elem, i+1, error,0),
                          TablaSimbolosFuncion
                        );
                }

                //EJECUTO

                if(funcion.clase_pertenece!=NULL){//SI ES METODO INSERTO REFERENCIA AL OBJETO COMO "ESTE"
                    //struct Clase *clase_ref = funcion.objeto_pertenece;
                    struct Valor objeto;
                    objeto.tipo='C';
                    objeto.val.clase=funcion.objeto_pertenece;
                    asignar_elemento_tabla_simbolos("ESTE", 
                                 objeto,
                                 TablaSimbolosFuncion
                    );
                    /*int i=0,cantidad=cantidad_claves(clase_ref->atributos);

                    if(cantidad>0){
                        char **claves = devolver_claves(clase_ref->atributos,cantidad);

                        for(i=0;i<cantidad;i++){
                            asignar_elemento_tabla_simbolos(claves[i], 
                                 copiar_valor(recuperar_elemento_tabla_simbolos(claves[i], error,clase_ref->atributos)),
                                 TablaSimbolosFuncion
                            );
                        }

                        free(claves);
                    }*/
                }

                struct NodoSentencia *nodo;
                nodo = &funcion.nodos;
                while(nodo!=NULL){
                    *error=ejecutar_sentencia(nodo->sentencia,'F',TablaSimbolosFuncion, &rpta,0);
                    if(*error==1){printf("\nERROR: SENTENCIA CONTINUAR FUERA DE BUCLE\n"); *error=-1;}
                    if(*error==2){printf("\nERROR: SENTENCIA ROMPER FUERA DE BUCLE\n"); *error=-1;}
                    if(*error==3){*error=0; break;}
                    nodo = nodo->sgte;
                }

                /*if(funcion.clase_pertenece!=NULL){//SI ES METODO RECUPERO LOS ATRIBUTOS DEL METODO
                    struct Clase *clase_ref = funcion.objeto_pertenece;
                    int i=0,cantidad=cantidad_claves(clase_ref->atributos);

                    if(cantidad>0){
                        char **claves = devolver_claves(clase_ref->atributos,cantidad);

                        for(i=0;i<cantidad;i++){
                            asignar_elemento_tabla_simbolos(claves[i], 
                                 copiar_valor(recuperar_elemento_tabla_simbolos(claves[i], error,TablaSimbolosFuncion)),
                                 clase_ref->atributos
                            );
                        }

                        free(claves);
                    }
                }*/

                limpiar_tabla(TablaSimbolosFuncion);
                ///////////////////////////////////////////////////7////////////////
            }
            if(tiene_argumentos){
                limpiar_valor(der);
            }
        }else if(strcmp(expresion.operacion,".")==0){ //binario PUNTO
            izq = resolver_expresion(*expresion.izquierda,error,TablaSimbolos,referencia);COMPROBAR_ERROR_EXPRESION
            if(izq.tipo!='C'){
                *error=-1;
                printf("\nLO QUE SE ENCUENTRA A LA IZQUIERDA DE UN PUNTO DEBE DE SER UN OBJETO\n");
                return rpta;
            }
            if(expresion.derecha->tipo!='I'){
                *error=-1;
                printf("\nLO QUE SE ENCUENTRA A LA DERECHA DE UN PUNTO DEBE DE SER UN IDENTIFICADOR\n");
                return rpta;
            }

            if(existe_clave(izq.val.clase->atributos,expresion.derecha->texto)){
                if(referencia)
                    rpta = recuperar_elemento_tabla_simbolos(expresion.derecha->texto, error,izq.val.clase->atributos);
                else{
                    rpta = copiar_valor(recuperar_elemento_tabla_simbolos(expresion.derecha->texto, error,izq.val.clase->atributos));}
            }else if(existe_funcion(izq.val.clase->metodos,expresion.derecha->texto)){
                 rpta.tipo='F';
                 rpta.val.funcion = (struct BloqueFuncion*)malloc(sizeof(struct BloqueFuncion));
                 *rpta.val.funcion = recuperar_elemento_tabla_simbolos_funciones(expresion.derecha->texto,error,
                                             izq.val.clase->metodos);
            }else{
                *error=-1;
                printf("\nEL OBJETO DE LA CLASE %s NO TIENE ATRIBUTO NI METODO DE NOMBRE %s\n",izq.val.clase->nombre,expresion.derecha->texto);
                return rpta;
            }

            COMPROBAR_ERROR_EXPRESION
        }
    }

    return rpta;
}


void mostrar_nodos(struct Nodo nodo,int nivel){
    mostrar_valor(nodo.valor,nivel);
    if(nodo.sgte!=NULL)
        mostrar_nodos(*nodo.sgte,nivel);
}

void identar(int nivel){
    int i=0;
    for(;i<nivel;i++)
        printf("    ");
}

void mostrar_elementos(struct ElementoTabla **TablaSimbolos,int nivel){
    int error=0,i=0,cantidad = cantidad_claves(TablaSimbolos);
    if(cantidad==0){
        identar(nivel);
        printf("EL DICCIONARIO AUN NO TIENE ELEMENTOS\n");
        return;
    }

    char ** claves = devolver_claves(TablaSimbolos,cantidad);

    for(;i<cantidad;i++){
        identar(nivel);
        printf("Clave: %s",claves[i]);
        mostrar_valor(recuperar_elemento_tabla_simbolos(claves[i], &error,TablaSimbolos),nivel+1);
    }

    free(claves);
}

void mostrar_valor(struct Valor valor,int nivel){
    printf("\n");
    //printf("TIPO: %s\n",texto_tipo(valor.tipo));

    if(valor.tipo=='L'){
        identar(nivel);
        printf("LOS ELEMENTOS DE LA LISTA SON: \n");
        if(valor.val.lista->num_elems>0)
            mostrar_nodos(valor.val.lista->primer_elem,nivel+1);
        else{
            identar(nivel+1);
            printf("LA LISTA ESTA VACIA.\n");
        }
    }
    else if(valor.tipo=='D'){
        identar(nivel);
        printf("LOS ELEMENTOS DEL DICCIONARIO SON: \n");
        mostrar_elementos(valor.val.diccionario->datos,nivel+1);
    }
    else if(valor.tipo=='A'){
        identar(nivel);
        printf("LOS ELEMENTOS DEL ARREGLO SON: \n");
        int i=0;
        for(;i<valor.val.arreglo->num_elems;i++){
            identar(nivel+1);
            printf("Elemento %d:",i+1);
            mostrar_valor(valor.val.arreglo->elems[i],nivel+2);
        }
    }
    else if(valor.tipo=='T'){
        identar(nivel);
        printf("El texto es: %s\n", valor.val.texto);
    }
    else if(valor.tipo=='N'){
        identar(nivel);
        printf("El numero con decimales es: %lf\n", *valor.val.numero);
    }
    else if(valor.tipo=='E'){
        identar(nivel);
        printf("El numero entero es: %d\n", *valor.val.entero);
    }
    else if(valor.tipo=='B'){
        identar(nivel);
        printf("El valor de verdad es: %s\n", *valor.val.booleano?"VERDADERO":"FALSO");
    }
    else if(valor.tipo=='V'){
        identar(nivel);
        printf("El valor es: VACIO\n");
    }
    else if(valor.tipo=='F'){
        identar(nivel);
        if(valor.val.funcion->clase_pertenece==NULL)
            printf("Subrutina de nombre: %s\n",valor.val.funcion->nombre);
        else
            printf("Metodo de nombre: %s de la Clase: %s\n",valor.val.funcion->nombre, valor.val.funcion->clase_pertenece);
    }
    else if(valor.tipo=='C'){
        identar(nivel);
        printf("OBJETO de la CLASE: %s\n",valor.val.clase->nombre);
    }
}


void escribir_nodos(struct Nodo nodo,int nivel){
    escribir_valor(nodo.valor,nivel);
    if(nodo.sgte!=NULL)
        escribir_nodos(*nodo.sgte,nivel);
}

void escribir_valor(struct Valor valor,int nivel){

    if(valor.tipo=='L'){
        if(nivel==0)
            escribir_nodos(valor.val.lista->primer_elem,nivel+1);
        else
            mostrar_nodos(valor.val.lista->primer_elem,nivel+1);
    }else if(valor.tipo=='D'){
        mostrar_valor(valor,nivel);
    }else if(valor.tipo=='A'){
        mostrar_valor(valor,nivel);
    }else if(valor.tipo=='F'){
        mostrar_valor(valor,nivel);
    }else if(valor.tipo=='C'){
        mostrar_valor(valor,nivel);
    }
    else if(valor.tipo=='T'){
        printf("%s", valor.val.texto);
    }
    else if(valor.tipo=='N'){
        printf("%lf", *valor.val.numero);
    }
    else if(valor.tipo=='E'){
        printf("%d", *valor.val.entero);
    }
    else if(valor.tipo=='B'){
        printf("%s", *valor.val.booleano?"VERDADERO":"FALSO");
    }
    else if(valor.tipo=='V'){
        printf("VACIO");
    }

}

//el ultimo parametro es utilizado por la sentencia DEVOLVER
int ejecutar_sentencia(struct Sentencia sentencia,char tipo_bloque,struct ElementoTabla **TablaSimbolos,struct Valor *rpta,_Bool interactivo){
    int error=0;
    struct Valor resultado,vacio;
    struct Clase clase, *clase_ref;
    struct NodoSentencia *nodo;
    FILE *archivo; 
    switch(sentencia.tipo){
      case 'T': //SENTENCIA IMPORTAR
        archivo = fopen(sentencia.val.sent_importar->arch,"r");

        char linea_codigo[250]; linea_codigo[0] = '\0';
        struct Token tokens[250];

        int linea=0,cant_tokens=0;
        if (archivo == NULL){
            printf("\nERROR: No se pudo abrir el archivo fuente %s.\n",sentencia.val.sent_importar->arch);
            return -1;
        }
 
        _Bool hubo_error=0;
        while (feof(archivo) == 0)
        {
            linea++;
            linea_codigo[0]='\0'; //IMPORTANTE
            fgets(linea_codigo,250,archivo);

            //quito el \n ENTER
            if(linea_codigo[strlen(linea_codigo)-1]=='\n')
                linea_codigo[strlen(linea_codigo)-1]='\0';

            if(analisis_lexico(linea_codigo, tokens, &cant_tokens)!=0){
                printf("\n\nSE ENCONTRARON ERRORES AL ANALIZAR SU CODIGO FUENTE IMPORTADO, CARACTERES INESPERADOS EN LA LINEA: %d\n\n",linea);
                hubo_error=1;
                continue;
            }

            if(cant_tokens>0)
                if(analisis_sintactico(tokens, cant_tokens, sentencia.val.sent_importar->central,linea)!=0){
                    printf("\n\nSE ENCONTRARON ERRORES SINTACTICOS AL ANALIZAR SU CODIGO FUENTE IMPORTADO EN LA LINEA: %d\n\n",linea);
                    hubo_error=1;
                    continue;
                }

            if(cant_tokens>0 && !hubo_error)
                if(ejecutar(*sentencia.val.sent_importar->central,TablaSimbolos,0)!=0){
                    printf("\n\nSE ENCONTRARON ERRORES AL EJECUTAR SU CODIGO FUENTE IMPORTADO EN LA LINEA: %d\n\n",linea);
                    hubo_error=1;
                    break;
                }
        }
 
        fclose(archivo);
        
        if(hubo_error){
            printf("\nERROR AL IMPORTAR FUENTE DE ARCHIVO: %s\n",sentencia.val.sent_importar->arch);
            return -1;
        }

        return 0;
      case ' ':
            printf("\nERROR: BLOQUE VACIO DE CODIGO\n");
            if(sentencia.linea>0)printf("\nERROR EN LA LINEA %d\n",sentencia.linea);
            return -1;
        case 'C':
            return 1;
        case 'R':
            return 2;
        case 'D':
            if(sentencia.val.expre!=NULL && rpta!=NULL)
                *rpta=resolver_expresion(*sentencia.val.expre, &error,TablaSimbolos,0);
                if(error<0){
                    printf("\nERROR AL RESOLVER EXPRESION\n");
                    if(sentencia.linea>0)printf("\nERROR EN LA LINEA %d\n",sentencia.linea);
                }
            return 3;
        case 'F':
            if(sentencia.val.funcion->clase_pertenece==NULL){//SUBRUTINA
                asignar_elemento_tabla_simbolos_funciones(sentencia.val.funcion->nombre, *sentencia.val.funcion,TablaSimbolosFunciones,&error);

                if(error<0){
                    printf("ERROR: YA EXISTE UNA SUBRUTINA DE ESE NOMBRE: %s.\n",sentencia.val.funcion->nombre);
                    if(sentencia.linea>0)printf("\nERROR EN LA LINEA %d\n",sentencia.linea);
                    return -1;
                }
            }
            else{ //METODO
                clase_ref = recuperar_ref_elemento_tabla_simbolos_clases(sentencia.val.funcion->clase_pertenece,
                        &error,TablaSimbolosClases);

                if(error<0)return -1;

                asignar_elemento_tabla_simbolos_funciones(sentencia.val.funcion->nombre, *sentencia.val.funcion,clase_ref->metodos,&error);

                if(error<0){
                    printf("ERROR: YA EXISTE UN METODO DE NOMBRE %s PARA LA CLASE %s.\n",
                     sentencia.val.funcion->nombre,sentencia.val.funcion->clase_pertenece);

                    if(sentencia.linea>0)printf("\nERROR EN LA LINEA %d\n",sentencia.linea);

                    return -1;
                }
            }

            return error;
        case 'L': //sentencia creacion clase
            clase.nombre = (char*)malloc(strlen(sentencia.val.sent_clase->nombre)+1);
            strcpy(clase.nombre,sentencia.val.sent_clase->nombre);
            inicializar_tabla(clase.atributos);
            inicializar_tabla_funciones(clase.metodos);


            struct Clase clase_padre;
            //SI HAY HERENCIA
            if(sentencia.val.sent_clase->num_padres>0){
                int i=0;
                for(i=0;i<sentencia.val.sent_clase->num_padres;i++){
                    clase_padre = recuperar_elemento_tabla_simbolos_clases(sentencia.val.sent_clase->padres[i],
                         &error,TablaSimbolosClases);
                    if(error<0){
                        printf("\nERROR: NO SE PUDO REALIZAR LA HERENCIA DE LA CLASE %s"
                               ", NO EXISTE ESTA CLASE.\n",sentencia.val.sent_clase->padres[i]);
                        return error;
                    }
                    error=copiar_atributos_vacios(clase.atributos,clase_padre.atributos);
                    if(error<0){
                        printf("\nERROR: NO SE PUDO REALIZAR LA HERENCIA DE LA CLASE %s"
                               ", PROBLEMAS AL HEREDAR ATRIBUTOS.\n",sentencia.val.sent_clase->padres[i]);
                        return error;
                    }
                    error=copiar_metodos(clase.metodos,clase_padre.metodos);
                    if(error<0){
                        printf("\nERROR: NO SE PUDO REALIZAR LA HERENCIA DE LA CLASE %s"
                               ", METODOS DUPLICADOS AL HEREDAR.\n",sentencia.val.sent_clase->padres[i]);
                        return error;
                    }
                }
            }

            asignar_elemento_tabla_simbolos_clases(sentencia.val.sent_clase->nombre, clase,
              TablaSimbolosClases,&error);

            if(error<0){
                 printf("ERROR: YA EXISTE UNA CLASE DE NOMBRE %s.\n",sentencia.val.sent_clase->nombre);
            }

            return error;
        case 'A':
            clase_ref = recuperar_ref_elemento_tabla_simbolos_clases(sentencia.val.sent_atributos->clase_pertenece,
                        &error,TablaSimbolosClases);
            if(error<0)return -1;

            vacio.tipo='V';
            vacio.referencia=NULL;

            int i=0;
            for(;i<sentencia.val.sent_atributos->num_atributos;i++)
                asignar_elemento_tabla_simbolos(sentencia.val.sent_atributos->atributos[i], vacio,
                    clase_ref->atributos);

            return error;
        case 'E':
            resultado=resolver_expresion(*sentencia.val.expre, &error,TablaSimbolos,0);

            if(error<0){
                if(sentencia.linea>0)printf("\nERROR EN LA LINEA %d\n",sentencia.linea);
                return error;
            }

            if(tipo_bloque=='C' && interactivo){
                mostrar_valor(resultado,0);
                asignar_elemento_tabla_simbolos("RPTA",copiar_valor(resultado),TablaSimbolos);
            }
            limpiar_valor(resultado);
            return error;
        case 'S':
            resultado=resolver_expresion(sentencia.val.si->expre, &error,TablaSimbolos,0);
            if(error<0){
                if(sentencia.linea>0)printf("\nERROR EN LA LINEA %d\n",sentencia.linea);
                return error;
            }

            if(resultado.tipo!='B'){
                printf("\nERROR: EL RESULTADO DE LA CONDICION DEL SI NO ES DE TIPO VALOR DE VERDAD\n");

                if(sentencia.linea>0)printf("\nERROR EN LA LINEA %d\n",sentencia.linea);

                limpiar_valor(resultado);
                return -1;
            }
            else{
                if(*resultado.val.booleano){ //SI
                    nodo = &sentencia.val.si->nodos;
                    while(nodo!=NULL){
                        error=ejecutar_sentencia(nodo->sentencia,'S',TablaSimbolos,rpta,0);
                        if(error!=0)return error;
                        nodo = nodo->sgte;
                    }
                }
                else{//sino_sis y sino
                    _Bool se_ejecuto_bloque=0;
                    //sino_si
                    struct NodoSinoSi *nodoact=sentencia.val.si->sino_si;
                    while(nodoact!=NULL){
                        resultado=resolver_expresion(nodoact->sino_si.expre, &error,TablaSimbolos,0);
                        if(error<0){
                            if(nodoact->sino_si.linea>0)printf("\nERROR EN LA LINEA %d\n",nodoact->sino_si.linea);
                            return error;
                        }

                        if(resultado.tipo!='B'){
                            printf("\nERROR: EL RESULTADO DE LA CONDICION DEL SINO_SI NO ES DE TIPO VALOR DE VERDAD\n");

                            if(nodoact->sino_si.linea>0)printf("\nERROR EN LA LINEA %d\n",nodoact->sino_si.linea);

                            limpiar_valor(resultado);
                            return -1;
                        }

                        if(*resultado.val.booleano){
                            nodo = &nodoact->sino_si.nodos;
                            while(nodo!=NULL){
                                error=ejecutar_sentencia(nodo->sentencia,'I',TablaSimbolos,rpta,0);
                                if(error!=0)return error;
                                nodo = nodo->sgte;
                            }
                            se_ejecuto_bloque=1;
                        }
                        
                        if(se_ejecuto_bloque)
                            break;
                        nodoact = nodoact->sgte;
                    }
                    //sino
                    if(!se_ejecuto_bloque){
                        if(sentencia.val.si->sino!=NULL){
                            nodo = &sentencia.val.si->sino->nodos;
                            while(nodo!=NULL){
                                error=ejecutar_sentencia(nodo->sentencia,'N',TablaSimbolos,rpta,0);
                                if(error!=0)return error;
                                nodo = nodo->sgte;
                            }
                        }
                    }
                }
            }
            return error;
        case 'M':
            while(1){
                resultado=resolver_expresion(sentencia.val.mientras->expre, &error,TablaSimbolos,0);
                if(error!=0)return error;

                if(resultado.tipo!='B'){
                    printf("\nERROR: EL RESULTADO DE LA CONDICION DEL MIENTRAS NO ES DE TIPO VALOR DE VERDAD\n");

                    if(sentencia.linea>0)printf("\nERROR EN LA LINEA %d\n",sentencia.linea);

                    limpiar_valor(resultado);
                    return -1;
                }
                else{
                    if(*resultado.val.booleano){
                        nodo = &sentencia.val.mientras->nodos;
                        while(nodo!=NULL){
                            error=ejecutar_sentencia(nodo->sentencia,'M',TablaSimbolos,rpta,0);
                            if(error==1)break;
                            if(error==2)break;
                            if(error!=0)return error;
                            nodo = nodo->sgte;
                        }
                        if(error==1){
                            error=0;
                            continue;
                        }else if(error==2){
                            error=0;
                            break;
                        }
                    }
                    else
                        break;
                }
            }
            return error;
    }
    printf("\nERROR INESPERADO, SENTENCIA DE TIPO INDETERMINADO\n");
    return -1;
}

int ejecutar(struct BloqueCentral central, struct ElementoTabla **TablaSimbolos,_Bool interactivo){
    int error=0;
    char tipo_bloque='C';
    struct NodoSentencia *nodo_actual;
    obtener_ultimo_bloque(&central, &tipo_bloque, &nodo_actual);

    //si estoy en el bloque central ejecuto la ultima sentencia ingresada
    if(tipo_bloque=='C'){
        error = ejecutar_sentencia(nodo_actual->sentencia,'C',TablaSimbolos,NULL,interactivo);
        if(error==1)printf("\nERROR: SENTENCIA CONTINUAR FUERA DE BUCLE\n");
        if(error==2)printf("\nERROR: SENTENCIA ROMPER FUERA DE BUCLE\n");
        if(error==3)printf("\nERROR: SENTENCIA DEVOLVER FUERA DE SUBRUTINA\n");
    }

    return error;
}
