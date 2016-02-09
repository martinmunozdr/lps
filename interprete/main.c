#include"cabecera.h"
#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include<stdlib.h>

/*void mostrar_expresion(struct Expresion exp){
    printf("\nTIPO: %c\n",exp.tipo);

    if(exp.tipo=='O'){
        printf("\n\nOPERACION %s\n",exp.operacion);
        printf("\nEXP IZQUIERDA:\n");
        if(exp.izquierda!=NULL)
            mostrar_expresion(*exp.izquierda);
        printf("\nEXP DERECHA:\n");
        if(exp.derecha!=NULL)
            mostrar_expresion(*exp.derecha);
    }
    else
        printf("El contenido es: %s\n", exp.texto);
}*/

//struct ElementoTabla *TablaSimbolos[TAMANO_TABLA];

struct ElementoTablaFunciones *TablaSimbolosFunciones[TAMANO_TABLA];
struct ElementoTablaClases *TablaSimbolosClases[TAMANO_TABLA];

int main(int argc, char *argv[]){
    //DECLARACIONES LEXICO
    char linea_codigo[250]; linea_codigo[0] = '\0';
    struct Token tokens[250];
    int cant_tokens;

    //DECLARACIONES SINTATICO
    struct Valor resultado;
    int error=0;

    //DECLARACIONES EJECUTOR
    struct ElementoTabla *TablaSimbolos[TAMANO_TABLA];
    struct BloqueCentral central;
    //inicializo
    inicializar_tabla(TablaSimbolos);
    central.nodos.sentencia.tipo=' ';
    central.nodos.sgte=NULL;
    inicializar_tabla_funciones(TablaSimbolosFunciones);

    //MODO NORMAL
    if(argc>1){
        FILE *archivo; 
        archivo = fopen(argv[1],"r");
        int linea=0;
        if (archivo == NULL){
            printf("\nERROR: No se pudo abrir el archivo fuente.\n");
            exit(1);
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
                printf("\n\nSE ENCONTRARON ERRORES AL ANALIZAR SU CODIGO FUENTE, CARACTERES INESPERADOS EN LA LINEA: %d\n\n",linea);
                hubo_error=1;
                continue;
            }

            if(cant_tokens>0)
                if(analisis_sintactico(tokens, cant_tokens, &central,linea)!=0){
                    printf("\n\nSE ENCONTRARON ERRORES SINTACTICOS AL ANALIZAR SU CODIGO FUENTE EN LA LINEA: %d\n\n",linea);
                    hubo_error=1;
                    continue;
                }

            if(cant_tokens>0 && !hubo_error)
                if(ejecutar(central,TablaSimbolos,0)!=0){
                    printf("\n\nSE ENCONTRARON ERRORES AL EJECUTAR SU CODIGO FUENTE EN LA LINEA: %d\n\n",linea);
                    break;
                }
        }
 
        fclose(archivo);
    }else{    //MODO INTERACTIVO
        while(1){
            printf("Ingrese su codigo: ");
            linea_codigo[0] = '\0';        //LIMPIO ENTRADA
            scanf("%249[^\n]",linea_codigo); LIMPIA_ENTRADA_SOBRA
            linea_codigo[249] = '\0';

            if(strcmp(linea_codigo,"SALIR")==0)
                break;

            if(analisis_lexico(linea_codigo, tokens, &cant_tokens)!=0){
                printf("\n\nSE ENCONTRARON ERRORES AL ANALIZAR SU CODIGO FUENTE, CARACTERES INESPERADOS\n\n");
                continue;
            }

            if(cant_tokens>0)
                if(analisis_sintactico(tokens, cant_tokens, &central,-1)!=0){
                    printf("\n\nSE ENCONTRARON ERRORES SINTACTICOS AL ANALIZAR SU CODIGO FUENTE\n\n");
                    continue;
                }

            if(cant_tokens>0)
                if(ejecutar(central,TablaSimbolos,1)!=0){
                    printf("\n\nSE ENCONTRARON ERRORES AL EJECUTAR SU CODIGO FUENTE\n\n");
                    continue;
                }
        }
    }

    limpiar_tabla(TablaSimbolos);
    limpiar_sentencias(central.nodos);
    limpiar_tabla_funciones(TablaSimbolosFunciones);
    limpiar_tabla_clases(TablaSimbolosClases);

    return 0;
}
