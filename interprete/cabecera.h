//#define ENTER 10 //OJO en windows es 13
#define LIMPIA_ENTRADA_SOBRA while(getchar()!=((int)'\n'));//USAR después de cada scanf
#define TAMANO_TABLA 100

//STRUCTS ANALISIS LEXICO
struct Token{
    char contenido[250];
    char tipo; //O operador; N numero; T texto; R palabra reservada; I identificador
    int linea;
    int caracter;
};

//STRUCTS ANALISIS SINTACTICO
struct Expresion{
     char tipo;     //operacion O, numero N, texto T, identificador I
     char operacion[11]; //operador, SUBRUTINA SR, INDEXACION IN
     char texto[250];   //en caso sea texto o numero, tambien se guarda aqui nombre identificador si lo es
     struct Expresion *izquierda;       
     struct Expresion *derecha;
};

//Lista enlazada de sentencias, usada en bloques
struct BloqueSi;
struct BloqueMientras;

struct SentenciaClase{
    char nombre[250];
    int num_padres;
    char **padres;
};

struct SentenciaImportar{
    char arch[250];
    struct BloqueCentral *central;
};

struct SentenciaAtributos{
    char *clase_pertenece;
    int num_atributos;
    char **atributos;
};

struct Sentencia{
    char tipo; //S bloque Si, M bloque Mientras, E expresion,  C continuar, R romper, D devolver, F subrutina y metodo(declaracion)
               //L clase,  A atributos, T importar
    union{
        struct BloqueSi *si;
        struct BloqueMientras *mientras;
        struct BloqueFuncion *funcion;
        struct Expresion *expre;
        struct SentenciaClase *sent_clase;
        struct SentenciaAtributos *sent_atributos;
        struct SentenciaImportar *sent_importar;
    }val;

    int linea;
};

struct NodoSentencia{
    struct Sentencia sentencia;
    struct NodoSentencia* sgte;
};

struct BloqueSinoSi{
    struct Expresion expre;
    struct NodoSentencia nodos;
    int linea;
};


struct NodoSinoSi{
    struct BloqueSinoSi sino_si;
    struct NodoSinoSi *sgte;
};

struct BloqueSino{
    struct NodoSentencia nodos;
};

struct BloqueFuncion{
    char *clase_pertenece;
    struct Clase *objeto_pertenece;
    char nombre[250];
    char **parametros;
    int num_parametros;
    struct NodoSentencia nodos;
    _Bool cerrado;
};

struct BloqueSi{
    struct Expresion expre;
    struct NodoSentencia nodos;
    struct NodoSinoSi *sino_si;
    struct BloqueSino *sino;
    _Bool cerrado;
};

struct BloqueMientras{
    struct Expresion expre;
    struct NodoSentencia nodos;
    _Bool cerrado;
};

struct BloqueCentral{
    struct NodoSentencia nodos;
};

//STRUCTS EJECUTOR
struct Lista;
struct Diccionario;
struct Arreglo;
struct ElementoTabla;
struct ElementoTablaFunciones;

struct Valor{ //Valor, RESULTADO DE UNA EXPRESION, O VALOR DE UNA VARIABLE O AMBAS
     char tipo;     //numero N, texto T, 
                    //lista (conjunto valores) L
                    //booleano B, Vacio V,
                    //diccionario (pares clave valor) D
                    //arreglo A, C clase, F funcion o metodo
                    //entero E

     union{
     int *entero; //en caso sea entero
     double *numero;  //en caso sea numero
     char *texto;   //en caso sea texto
     struct Lista *lista; //en caso sea lista
     struct Diccionario *diccionario; //en caso sea diccionario
     struct Arreglo *arreglo;
     _Bool *booleano; //en caso sea valor de verdad
     struct Clase *clase; //en caso sea clase
     struct BloqueFuncion *funcion; //en caso sea funcion
     }val;

    struct Valor *referencia; //usado para alterar elementos listas
};

struct Clase{ //ALMACENA ATRIBUTOS Y METODOS DE UNA CLASE DEFINIDA POR LE USUARIO
    char *nombre;
    struct ElementoTabla *atributos[TAMANO_TABLA];
    struct ElementoTablaFunciones *metodos[TAMANO_TABLA];
};

struct Nodo{//Nodo de una lista
    struct Valor valor;
    struct Nodo *sgte;
};

struct Lista{//Lista utilizada para almacenar listas
    int num_elems;
    struct Nodo primer_elem;
};

struct Diccionario{ //Utilizado para almacenar pares claves: valor
    struct ElementoTabla *datos[TAMANO_TABLA];
};

struct Arreglo{
    int num_elems;
    struct Valor *elems;
};
//tabla de simbolos
struct ElementoTabla{
    char *nombre;
    struct Valor valor;
    struct ElementoTabla *sgte; //NULL si no hay colisión de hash
};

//tabla de simbolos funciones
struct ElementoTablaFunciones{
    char *nombre;
    struct BloqueFuncion funcion;
    struct ElementoTablaFunciones *sgte; //NULL si no hay colisión de hash
};

//tabla de simbolos Clases
struct ElementoTablaClases{
    char *nombre;
    struct Clase clase;
    struct ElementoTablaClases *sgte; //NULL si no hay colisión de hash
};

//FUNCIONES DE ANALISIS LEXICO
int es_texto(char *linea, struct Token *token, int *contador);
int es_numero(char *linea, struct Token *token, int *contador);
int es_operador(char *linea, struct Token *token, int *contador);
int es_id_o_reservada(char *linea, struct Token *token, int *contador);
int analisis_lexico(char *linea, struct Token *tokens, int *cant_tokens);

//FUNCIONES DE ANALISIS SINTACTICO
int revisar_sintaxis(struct Token *tokens, int cant_tokens);
int analisis_sintactico(struct Token *tokens, int cant_tokens, struct BloqueCentral *central,int linea);
void construir_expresion(struct Token *tokens, int cant_tokens, struct Expresion *expresion);
void* obtener_ultimo_bloque(struct BloqueCentral *central,char *tipo_bloque, struct NodoSentencia **nodo);

//FUNCIONES DE EJECUTOR
void mostrar_valor(struct Valor valor,int nivel);
int mi_hash(char *cadena);
//TABLA SIMBOLOS VARIABLES
void inicializar_tabla(struct ElementoTabla **TablaSimbolos);
void limpiar_elemento_tabla(struct ElementoTabla **elemento);
void limpiar_tabla(struct ElementoTabla **TablaSimbolos);
void llenar_elemento(char *nombre, struct Valor valor,struct ElementoTabla **elemento);
void cambiar_valor_elemento(struct Valor valor,struct ElementoTabla **elemento);
void asignar_elemento(char *nombre, struct Valor valor,struct ElementoTabla **elemento);
void asignar_elemento_tabla_simbolos(char *nombre, struct Valor valor,struct ElementoTabla **TablaSimbolos);
struct Valor recuperar_elemento(char *nombre, struct ElementoTabla *elemento,int *error);
struct Valor recuperar_elemento_tabla_simbolos(char *nombre, int *error,struct ElementoTabla **TablaSimbolos);
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//TABLA SIMBOLOS FUNCIONES
void inicializar_tabla_funciones(struct ElementoTablaFunciones **TablaSimbolosFunciones);
void limpiar_elemento_tabla_funciones(struct ElementoTablaFunciones **elemento);
void limpiar_tabla_funciones(struct ElementoTablaFunciones **TablaSimbolosFunciones);
void llenar_elemento_funciones(char *nombre, struct BloqueFuncion funcion,struct ElementoTablaFunciones **elemento);
void asignar_elemento_funciones(char *nombre, struct BloqueFuncion funcion,struct ElementoTablaFunciones **elemento,int *error);
void asignar_elemento_tabla_simbolos_funciones(char *nombre, struct BloqueFuncion funcion,
            struct ElementoTablaFunciones **TablaSimbolos,int *error);
struct BloqueFuncion recuperar_elemento_funciones(char *nombre, struct ElementoTablaFunciones *elemento,int *error);
struct BloqueFuncion recuperar_elemento_tabla_simbolos_funciones(char *nombre, int *error,struct ElementoTablaFunciones **TablaSimbolos);
_Bool existe_funcion(struct ElementoTablaFunciones **TablaSimbolos,char* clave);
int cantidad_funciones(struct ElementoTablaFunciones **TablaSimbolos);
char** devolver_nombres_funciones(struct ElementoTablaFunciones **TablaSimbolos,int cantidad);
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//TABLA SIMBOLOS CLASES
void inicializar_tabla_clase(struct ElementoTablaClases **TablaSimbolos);
void limpiar_clase(struct Clase clase);
void limpiar_elemento_tabla_clases(struct ElementoTablaClases **elemento);
void limpiar_tabla_clases(struct ElementoTablaClases **TablaSimbolos);
void llenar_elemento_clases(char *nombre, struct Clase clase,struct ElementoTablaClases **elemento);
void asignar_elemento_clases(char *nombre, struct Clase clase,struct ElementoTablaClases **elemento,int *error);
void asignar_elemento_tabla_simbolos_clases(char *nombre, struct Clase clase,
        struct ElementoTablaClases **TablaSimbolos,int *error);
struct Clase recuperar_elemento_clases(char *nombre, struct ElementoTablaClases *elemento,int *error);
struct Clase recuperar_elemento_tabla_simbolos_clases(char *nombre, int *error,struct ElementoTablaClases **TablaSimbolos);
_Bool existe_clase(struct ElementoTablaClases **TablaSimbolos,char* clave);
struct Clase* recuperar_ref_elemento_clases(char *nombre, struct ElementoTablaClases *elemento,int *error);
struct Clase* recuperar_ref_elemento_tabla_simbolos_clases(char *nombre, int *error,struct ElementoTablaClases **TablaSimbolos);
struct Clase copiar_clase(struct Clase original);
void llenar_referencia_objeto(struct ElementoTablaFunciones **TablaSimbolos,struct Clase *clase);
int copiar_atributos_vacios(struct ElementoTabla **TablaSimbolos,struct ElementoTabla **TablaSimbolosOrig);
int copiar_metodos(struct ElementoTablaFunciones **TablaSimbolos,struct ElementoTablaFunciones **TablaSimbolosOrig);
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void limpiar_nodo(struct Nodo *nodo);
void limpiar_lista(struct Lista *lista);
void limpiar_valor(struct Valor valor);
struct Valor resolver_expresion(struct Expresion expresion, int *error,struct ElementoTabla **TablaSimbolos,_Bool referencia);
struct Valor copiar_valor(struct Valor valor);
char* texto_tipo(char tipo);
int ejecutar(struct BloqueCentral central,struct ElementoTabla **TablaSimbolos,_Bool interactivo);
void limpiar_sentencias(struct NodoSentencia nodos);
void escribir_valor(struct Valor valor,int nivel);
int ejecutar_sentencia(struct Sentencia sentencia,char tipo_bloque,struct ElementoTabla **TablaSimbolos,struct Valor *rpta,_Bool interactivo);
////DICCIONARIO
int cantidad_claves(struct ElementoTabla **TablaSimbolos);
_Bool valor_predefinido(char* nombre,struct Valor *rpta);
///////////////////
char** devolver_claves(struct ElementoTabla **TablaSimbolos,int cantidad);



//STRUCT CGI
struct Par{
    char *nombre;
    char *valor;
};

#define MAXPARES 1000

//FUNCIONES CGI
inline int ishex(int x); 
int decode(const char *s, char *dec);
int obtener_par(char *cadena,struct Par *pares,int *cant_pares,int *contador);
int obtener_datos_post(struct Par *pares, int *cant_pares);
int obtener_datos_get(struct Par *pares, int *cant_pares);
