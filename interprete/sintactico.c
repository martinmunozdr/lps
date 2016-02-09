#include"cabecera.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

struct TempAnalisis{
    char tipo; //E expresion, T token
    struct Token token;
    struct Expresion exp;
};

char* texto_tipo_sintactico(char tipo){ //C central, M mientras, Si sino, N sino, I SinoSI
    switch(tipo){
        case 'O': return "OPERADOR";
        case 'N': return "NUMERO";
        case 'T': return "TEXTO";
        case 'R': return "RESERVADA";
        case 'I': return "IDENTIFICADOR";
    }

    return " ERROR INESPERADO, TIPO DE BLOQUE INDETERMINADO. ";
}

char* texto_bloque(char tipo){ //C central, M mientras, Si sino, N sino, I SinoSI
    switch(tipo){
        case 'C': return "CUERPO PRINCIPAL DEL PROGRAMA";
        case 'M': return "MIENTRAS";
        case 'S': return "SI";
        case 'N': return "SINO";
        case 'I': return "SINO_SI";
        case 'F': return "SUBRUTINA";
    }

    return " ERROR INESPERADO, TIPO DE BLOQUE INDETERMINADO. ";
}

//devuelve el ultimo bloque y su ultimo nodo ademas del tipo de bloque
void* obtener_ultimo_bloque(struct BloqueCentral *central,char *tipo_bloque, struct NodoSentencia **nodo){
    *tipo_bloque = 'C';
    void* bloque=(void*) central;

    struct NodoSentencia *nodo_actual=&central->nodos;

    //COMPRUEBO SI SE TRATA DE LA 1RA SENTENCIA DEL BLOQUE
    if(nodo_actual->sentencia.tipo==' '){
        *nodo = nodo_actual;
        return bloque;
    }

    while(nodo_actual->sgte!=NULL)nodo_actual=nodo_actual->sgte;

    while(1){
        if(nodo_actual->sentencia.tipo=='E' ||//sentencia expresion
           nodo_actual->sentencia.tipo=='C' ||//sentencia CONTINUAR
           nodo_actual->sentencia.tipo=='D' ||//sentencia DEVOLVER
           nodo_actual->sentencia.tipo=='R'||//sentencia CONTINUAR
           nodo_actual->sentencia.tipo=='A' ||//sentencia ATRIBUTOS
           nodo_actual->sentencia.tipo=='T' ||//sentencia IMPORTAR
           nodo_actual->sentencia.tipo=='L'){ //sentencia CLASE
            break;
        }else if(nodo_actual->sentencia.tipo=='M'){
            if(nodo_actual->sentencia.val.mientras->cerrado){
                break;
            }
            *tipo_bloque = 'M';
            bloque=(void*)nodo_actual->sentencia.val.mientras;
            nodo_actual =  &nodo_actual->sentencia.val.mientras->nodos;

            //COMPRUEBO SI SE TRATA DE LA 1RA SENTENCIA DEL BLOQUE
            if(nodo_actual->sentencia.tipo==' ')break;

            while(nodo_actual->sgte!=NULL)nodo_actual=nodo_actual->sgte;
        }else if(nodo_actual->sentencia.tipo=='F'){
            if(nodo_actual->sentencia.val.funcion->cerrado){
                break;
            }
            *tipo_bloque = 'F';
            bloque=(void*)nodo_actual->sentencia.val.funcion;
            nodo_actual =  &nodo_actual->sentencia.val.funcion->nodos;

            //COMPRUEBO SI SE TRATA DE LA 1RA SENTENCIA DEL BLOQUE
            if(nodo_actual->sentencia.tipo==' ')break;

            while(nodo_actual->sgte!=NULL)nodo_actual=nodo_actual->sgte;
        }else if(nodo_actual->sentencia.tipo=='S'){
            if(nodo_actual->sentencia.val.si->cerrado){
                break;
            }
            //en el caso del sino tambien devuelvo el bloque si, pero el ultimo nodo del bloque sino y el tipo bloque 'N'
            //en el caso del sino_si tambien devuelvo el bloque si, pero el ultimo nodo del bloque sino_si ultimo y el tipo bloque 'I'
            bloque=(void*)nodo_actual->sentencia.val.si;

            if(nodo_actual->sentencia.val.si->sino!=NULL){
                *tipo_bloque = 'N';
                nodo_actual =  &nodo_actual->sentencia.val.si->sino->nodos;
            }else if(nodo_actual->sentencia.val.si->sino_si!=NULL){
                *tipo_bloque = 'I';
                struct NodoSinoSi *bloque_sino_si_actual = nodo_actual->sentencia.val.si->sino_si;
                while(bloque_sino_si_actual->sgte!=NULL)bloque_sino_si_actual=bloque_sino_si_actual->sgte;
                nodo_actual = &bloque_sino_si_actual->sino_si.nodos;
            }else{
                *tipo_bloque = 'S';
                nodo_actual =  &nodo_actual->sentencia.val.si->nodos;
            }

            //COMPRUEBO SI SE TRATA DE LA 1RA SENTENCIA DEL BLOQUE
            if(nodo_actual->sentencia.tipo==' ')break;
            while(nodo_actual->sgte!=NULL)nodo_actual=nodo_actual->sgte;
        }
    }

    *nodo = nodo_actual;
    return bloque;
}

int revisar_sintaxis(struct Token *tokens, int cant_tokens){
    char tipo_ant;
    char op_ant[11];
    int i=0, corchetes=0, parentesis=0;

    for(;i<cant_tokens;i++){
        if(i==0){
            if(tokens[i].tipo=='O' && (strcmp(tokens[i].contenido,")")==0 || strcmp(tokens[i].contenido,"]")==0)){
                printf("NO SE PUEDE EMPEZAR UNA EXPRESION CON UN %s DE CIERRE\n",
                      (strcmp(tokens[i].contenido,")")==0)?"PARENTESIS":"CORCHETE");
                return -1;
            }
            if(tokens[i].tipo=='O' && (
                          (strcmp(tokens[i].contenido,"*")==0)|| //SOLO SON BINARIOS
                          (strcmp(tokens[i].contenido,"/")==0)||
                          (strcmp(tokens[i].contenido,"^")==0)||
                          (strcmp(tokens[i].contenido,"DIV_ENTERA")==0)||
                          (strcmp(tokens[i].contenido,"RESTO")==0)||
                          (strcmp(tokens[i].contenido,"Y")==0)||
                          (strcmp(tokens[i].contenido,"O")==0)||
                          (strcmp(tokens[i].contenido,">=")==0)||
                          (strcmp(tokens[i].contenido,"<=")==0)||
                          (strcmp(tokens[i].contenido,"==")==0)||
                          (strcmp(tokens[i].contenido,"<")==0)||
                          (strcmp(tokens[i].contenido,">")==0)||
                          (strcmp(tokens[i].contenido,"!=")==0)||
                          (strcmp(tokens[i].contenido,",")==0)||
                          (strcmp(tokens[i].contenido,";")==0)||
                          (strcmp(tokens[i].contenido,"=")==0)||
                          (strcmp(tokens[i].contenido,".")==0)
            )){
                printf("\nNO SE PUEDE EMPEZAR UNA EXPRESION CON UN OPERADOR BINARIO\n");
                return -1;
            }
        }
        else{
            if(tipo_ant=='O'){
                if(strcmp(op_ant,".")==0){
                    if(tokens[i].tipo!='I'){
                        printf("\nDESPUES DE UN PUNTO SOLO SE PUEDE PONER UN"
                               " IDENTIFICADOR, ERROR EN EL CARACTER %d\n", tokens[i].caracter+1);
                        return -1;
                    }
                }
                else if(
                      (strcmp(op_ant,"(")!=0)&&
                      (strcmp(op_ant,"[")!=0)&&
                      (strcmp(op_ant,")")!=0)&&
                      (strcmp(op_ant,"]")!=0)
                  ) //unario o binario
                {
                    if(tokens[i].tipo=='O' && (
                          (strcmp(tokens[i].contenido,")")==0)||
                          (strcmp(tokens[i].contenido,"]")==0)||

                          (strcmp(tokens[i].contenido,"*")==0)||
                          (strcmp(tokens[i].contenido,"/")==0)||
                          (strcmp(tokens[i].contenido,"^")==0)||
                          (strcmp(tokens[i].contenido,"DIV_ENTERA")==0)||
                          (strcmp(tokens[i].contenido,"RESTO")==0)||
                          (strcmp(tokens[i].contenido,"Y")==0)||
                          (strcmp(tokens[i].contenido,"O")==0)||
                          (strcmp(tokens[i].contenido,">=")==0)||
                          (strcmp(tokens[i].contenido,"<=")==0)||
                          (strcmp(tokens[i].contenido,"==")==0)||
                          (strcmp(tokens[i].contenido,"<")==0)||
                          (strcmp(tokens[i].contenido,">")==0)||
                          (strcmp(tokens[i].contenido,"!=")==0)||
                          (strcmp(tokens[i].contenido,",")==0)||
                          (strcmp(tokens[i].contenido,";")==0)||
                          (strcmp(tokens[i].contenido,"=")==0)||
                          (strcmp(tokens[i].contenido,".")==0)
                      )){
                          printf("\nERROR, sintaxis equivocada, en el caracter: %d\n"
                                 "NO PUEDE PONER UN OPERADOR DESPUES DE OTRO, AL MENOS\n"
                                 "QUE SE TRATE DE UN OPERADOR UNARIO\n",tokens[i].caracter+1);
                          return -1;
                      }
                }
                else if(
                      (strcmp(op_ant,")")==0)||
                      (strcmp(op_ant,"]")==0)
                ){
                    if(! (tokens[i].tipo=='O' && (

                          (strcmp(tokens[i].contenido,"(")==0)||
                          (strcmp(tokens[i].contenido,"[")==0)||

                          (strcmp(tokens[i].contenido,")")==0)||
                          (strcmp(tokens[i].contenido,"]")==0)||

                          (strcmp(tokens[i].contenido,"+")==0)|| //SON BINARIOS Y
                          (strcmp(tokens[i].contenido,"-")==0)|| //UNARIOS

                          (strcmp(tokens[i].contenido,"*")==0)|| //SOLO SON BINARIOS
                          (strcmp(tokens[i].contenido,"/")==0)||
                          (strcmp(tokens[i].contenido,"^")==0)||
                          (strcmp(tokens[i].contenido,"DIV_ENTERA")==0)||
                          (strcmp(tokens[i].contenido,"RESTO")==0)||
                          (strcmp(tokens[i].contenido,"Y")==0)||
                          (strcmp(tokens[i].contenido,"O")==0)||
                          (strcmp(tokens[i].contenido,">=")==0)||
                          (strcmp(tokens[i].contenido,"<=")==0)||
                          (strcmp(tokens[i].contenido,"==")==0)||
                          (strcmp(tokens[i].contenido,"<")==0)||
                          (strcmp(tokens[i].contenido,">")==0)||
                          (strcmp(tokens[i].contenido,"!=")==0)||
                          (strcmp(tokens[i].contenido,",")==0)||
                          (strcmp(tokens[i].contenido,";")==0)||
                          (strcmp(tokens[i].contenido,"=")==0)//||
                          //(strcmp(tokens[i].contenido,".")==0)
                      ))){
                          printf("\nERROR, sintaxis equivocada, en el caracter: %d\n"
                                 "DESPUES DE CERRAR PARENTESIS O CORCHETES SOLO PUEDEN PONERSE: \n"
                                 "PARENTESIS o CORCHETES, ABRIR O CERRAR:\n(\n)\n[\n]\n"
                                 "O UN OPERADOR BINARIO QUE NO SEA PUNTO\n",tokens[i].caracter+1);
                          return -1;
                    }
                }
                else if(
                      (strcmp(op_ant,"(")==0)||
                      (strcmp(op_ant,"[")==0)
                ){

                    if(tokens[i].tipo=='O' && (
                                  (strcmp(tokens[i].contenido,"*")==0)|| //SOLO SON BINARIOS
                                  (strcmp(tokens[i].contenido,"/")==0)||
                                  (strcmp(tokens[i].contenido,"^")==0)||
                                  (strcmp(tokens[i].contenido,"DIV_ENTERA")==0)||
                                  (strcmp(tokens[i].contenido,"RESTO")==0)||
                                  (strcmp(tokens[i].contenido,"Y")==0)||
                                  (strcmp(tokens[i].contenido,"O")==0)||
                                  (strcmp(tokens[i].contenido,">=")==0)||
                                  (strcmp(tokens[i].contenido,"<=")==0)||
                                  (strcmp(tokens[i].contenido,"==")==0)||
                                  (strcmp(tokens[i].contenido,"<")==0)||
                                  (strcmp(tokens[i].contenido,">")==0)||
                                  (strcmp(tokens[i].contenido,"!=")==0)||
                                  (strcmp(tokens[i].contenido,",")==0)||
                                  (strcmp(tokens[i].contenido,";")==0)||
                                  (strcmp(tokens[i].contenido,"=")==0)||
                                  (strcmp(tokens[i].contenido,".")==0)
                    )){
                        printf("\nNO SE PUEDE PONER UN OPERADOR BINARIO DESPUES DE ABRIR UN %s\n"
                               "EN EL CARACTER %d\n",
                              (strcmp(op_ant,"(")==0)?"PARENTESIS":"CORCHETE",tokens[i].caracter+1);
                        return -1;
                    }
                    if(strcmp(op_ant,"(")==0 && strcmp(tokens[i].contenido,"]")==0){
                        printf("\nNO SE PUEDE CERRAR UN PARENTESIS CON UN CORCHETE\n"
                               "EN EL CARACTER %d\n",tokens[i].caracter+1);
                        return -1;
                    }
                    if(strcmp(op_ant,"[")==0 && strcmp(tokens[i].contenido,")")==0){
                        printf("\nNO SE PUEDE CERRAR UN CORCHETE CON UN PARENTESIS\n"
                               "EN EL CARACTER %d\n",tokens[i].caracter+1);
                        return -1;
                    }
                }
            }
            else if(tipo_ant=='I'){
                if(! (tokens[i].tipo=='O' && (

                          (strcmp(tokens[i].contenido,"(")==0)||
                          (strcmp(tokens[i].contenido,"[")==0)||

                          (strcmp(tokens[i].contenido,")")==0)||
                          (strcmp(tokens[i].contenido,"]")==0)||

                          (strcmp(tokens[i].contenido,"+")==0)|| //SON BINARIOS Y
                          (strcmp(tokens[i].contenido,"-")==0)|| //UNARIOS

                          (strcmp(tokens[i].contenido,"*")==0)|| //SOLO SON BINARIOS
                          (strcmp(tokens[i].contenido,"/")==0)||
                          (strcmp(tokens[i].contenido,"^")==0)||
                          (strcmp(tokens[i].contenido,"DIV_ENTERA")==0)||
                          (strcmp(tokens[i].contenido,"RESTO")==0)||
                          (strcmp(tokens[i].contenido,"Y")==0)||
                          (strcmp(tokens[i].contenido,"O")==0)||
                          (strcmp(tokens[i].contenido,">=")==0)||
                          (strcmp(tokens[i].contenido,"<=")==0)||
                          (strcmp(tokens[i].contenido,"==")==0)||
                          (strcmp(tokens[i].contenido,"<")==0)||
                          (strcmp(tokens[i].contenido,">")==0)||
                          (strcmp(tokens[i].contenido,"!=")==0)||
                          (strcmp(tokens[i].contenido,",")==0)||
                          (strcmp(tokens[i].contenido,";")==0)||
                          (strcmp(tokens[i].contenido,"=")==0)||
                          (strcmp(tokens[i].contenido,".")==0)
                      ))){
                          printf("\nERROR, sintaxis equivocada, en el caracter: %d\n"
                                 "DESPUES DE UN IDENTIFICADOR SOLO PUEDEN PONERSE: \n"
                                 "PARENTESIS o CORCHETES, ABRIR O CERRAR:\n(\n)\n[\n]\n"
                                 "O UN OPERADOR BINARIO\n",tokens[i].caracter+1);
                          return -1;
                }
            }
            else if(tipo_ant=='N'||tipo_ant=='T'){
                if(! (tokens[i].tipo=='O' && (
                          (strcmp(tokens[i].contenido,")")==0)||
                          (strcmp(tokens[i].contenido,"]")==0)||

                          (strcmp(tokens[i].contenido,"+")==0)|| //SON BINARIOS Y
                          (strcmp(tokens[i].contenido,"-")==0)|| //UNARIOS

                          (strcmp(tokens[i].contenido,"*")==0)|| //SOLO SON BINARIOS
                          (strcmp(tokens[i].contenido,"/")==0)||
                          (strcmp(tokens[i].contenido,"^")==0)||
                          (strcmp(tokens[i].contenido,"DIV_ENTERA")==0)||
                          (strcmp(tokens[i].contenido,"RESTO")==0)||
                          (strcmp(tokens[i].contenido,"Y")==0)||
                          (strcmp(tokens[i].contenido,"O")==0)||
                          (strcmp(tokens[i].contenido,">=")==0)||
                          (strcmp(tokens[i].contenido,"<=")==0)||
                          (strcmp(tokens[i].contenido,"==")==0)||
                          (strcmp(tokens[i].contenido,"<")==0)||
                          (strcmp(tokens[i].contenido,">")==0)||
                          (strcmp(tokens[i].contenido,"!=")==0)||
                          (strcmp(tokens[i].contenido,",")==0)||
                          (strcmp(tokens[i].contenido,";")==0)||
                          (strcmp(tokens[i].contenido,"=")==0)//||
                          //(strcmp(tokens[i].contenido,".")==0)
                      ))){
                          printf("\nERROR, sintaxis equivocada, en el caracter: %d\n"
                                 "DESPUES DE UN %s SOLO PUEDEN PONERSE: \n"
                                 "PARENTESIS o CORCHETES, SOLO CERRANDO:\n)\n]\n"
                                 "O UN OPERADOR BINARIO QUE NO SEA EL PUNTO\n",
                                  tokens[i].caracter+1,(tipo_ant=='N')?"NUMERO":"TEXTO");
                          return -1;
                }
            }

        }

        //CONTROLO LOS PARENTESIS Y CORCHETES
        if(tokens[i].tipo=='O' && strcmp(tokens[i].contenido,"(")==0)
            parentesis++;
        if(tokens[i].tipo=='O' && strcmp(tokens[i].contenido,"[")==0)
            corchetes++;

        if(tokens[i].tipo=='O' && strcmp(tokens[i].contenido,")")==0)
            parentesis--;
        if(tokens[i].tipo=='O' && strcmp(tokens[i].contenido,"]")==0)
            corchetes--;

        if(parentesis<0){
            printf("ERROR, sintaxis equivocada en el caracter: %d\n"
                   "PARENTESIS DE CIERRE NO ESPERADO\n",tokens[i].caracter+1);

            return -1;
        }

        if(corchetes<0){
            printf("ERROR, sintaxis equivocada en el caracter: %d\n"
                   "CORCHETE DE CIERRE NO ESPERADO\n",tokens[i].caracter+1);

            return -1;
        }
        /////////////////////////////////////

        if(tokens[i].tipo=='O')
            strcpy(op_ant,tokens[i].contenido);

        tipo_ant = tokens[i].tipo;
    }

    if(tokens[i-1].tipo=='O' && strcmp(tokens[i-1].contenido,")")!=0 && strcmp(tokens[i-1].contenido,"]")!=0){
        printf("ERROR, sintaxis equivocada.\n"
               "LA EXPRESION TERMINA EN UN OPERADOR, EN EL CARACTER %d\n",tokens[i-1].caracter+1);

        return -1;
    }

    if(parentesis!=0){
        printf("ERROR, sintaxis equivocada.\n"
               "HAY PARENTESIS SIN CERRAR\n");

        return -1;
    }

    if(corchetes!=0){
        printf("ERROR, sintaxis equivocada.\n"
               "HAY CORCHETES SIN CERRAR\n");

        return -1;
    }

    //REVISO QUE SOLO HAYAN PARENTESIS SIN CONTENIDO, DONDE PUEDE TRATARSE DE LLAMADA A FUNCION
    //DESPUES DE OTROS PARENTESIS O CORCHETES, O DESPUES DE UN ID
    //TAMBIEN REVISO QUE NO HAYA CORCHETES SIN CONTENIDO
    i=0;
    for(;i<cant_tokens-1;i++){
        if(i==0){
            if(tokens[i].tipo=='O'
               && (strcmp(tokens[i].contenido,"(")==0 || strcmp(tokens[i].contenido,"[")==0)
               && tokens[i+1].tipo=='O'
               && (strcmp(tokens[i+1].contenido,")")==0 || strcmp(tokens[i+1].contenido,"]")==0)){
                printf("NO SE PUEDEN PONER PARENTESIS NI CORCHETES VACIOS\n"
                       "AL MENOS QUE SE TRATE DE LLAMADA A FUNCION (EN EL CASO DE PARENTESIS)\n"
                       "ERROR EN EL CARACTER %d\n",tokens[i].caracter+1);
                return -1;
            }
        }
        else{
            if(tokens[i].tipo=='O'
               && strcmp(tokens[i].contenido,"[")==0 
               && tokens[i+1].tipo=='O'
               && strcmp(tokens[i+1].contenido,"]")==0 ){
                printf("NO SE PUEDEN PONER CORCHETES VACIOS\n"
                       "ERROR EN EL CARACTER %d\n",tokens[i].caracter+1);
                return -1;
            }
            if(tokens[i].tipo=='O'
               && strcmp(tokens[i].contenido,"(")==0 
               && tokens[i+1].tipo=='O'
               && strcmp(tokens[i+1].contenido,")")==0 
               && !(
                  tokens[i-1].tipo=='I' ||
                  (tokens[i-1].tipo=='O' && 
                    (strcmp(tokens[i-1].contenido,")")==0 ||strcmp(tokens[i-1].contenido,"]")==0))
               )){
                printf("NO SE PUEDEN PONER PARENTESIS VACIOS\n"
                       "AL MENOS QUE SE TRATE DE LLAMADA A FUNCION\n"
                       "ERROR EN EL CARACTER %d\n",tokens[i].caracter+1);
                return -1;
            }
        }
    }

    return 0;
}

void construir_expresion(struct Token *tokens, int cant_tokens, struct Expresion *expresion){
    struct TempAnalisis *temps = (struct TempAnalisis *)malloc(cant_tokens*sizeof(struct TempAnalisis));
    int contador_op_agrupa=0; //para resolver recursivamente expresiones dentro de operadores que agrupan ('(' y '[')

    struct Token *tokens_agrupa=NULL;

    int i=0;
    for(;i<cant_tokens;i++){
        temps[i].tipo = 'T';
        temps[i].token = tokens[i];
    }

    struct Expresion exp,exp_agrupa;
    char op[11];

    int cant_temps=cant_tokens,j,n=0,t=0,k=0;

    int cantidades_prioridad[11]={3, //parentesis, corchete y punto
                            3, //signo menos, signo mas, Y NEGACION
                            1, //exponente
                            4, //multiplicacion, división, resto y div_entera
                            2, //suma y resta
                            4, //menor, menor igual, mayor, mayor igual
                            2, //== y !=
                            1, //Y
                            1, //O
                            1, //=
                            2  //,
                            };

    char* ops_prioridad[11][4]={
        {"(","[",".",""},
        {"-","+","NEGACION",""},
        {"^","","",""},
        {"*","/","RESTO","DIV_ENTERA"},
        {"+","-","",""},
        {"<",">","<=",">="},
        {"==","!=","",""},
        {"Y","","",""},
        {"O","","",""},
        {"=","","",""},
        {",",";","",""}
    };

    i=0;
    j=0;
    k=0;
    n=0;
    t=0;
  for(t=0;t<11;t++)
    //t 1 y 8 se evaluan de derecha a izquierda
    for((t==1||t==2||t==9)?(i=cant_temps-1):(i=0);(t==1||t==2||t==9)?i>=0:i<cant_temps;(t==1||t==2||t==9)?i--:i++){
        for(n=0;n<cantidades_prioridad[t];n++){
            strcpy(op,ops_prioridad[t][n]);
            //AGRUPADOR

            if(temps[i].tipo=='T' && temps[i].token.tipo=='O' && strcmp(temps[i].token.contenido,op)==0
               && (strcmp(op,"(")==0||strcmp(op,"[")==0)){
                contador_op_agrupa=0;
                for(j=i+1;j<cant_temps;j++){
                    if(temps[j].tipo=='T' && temps[j].token.tipo=='O'
                       && strcmp(temps[j].token.contenido,op)==0){
                        contador_op_agrupa++;
                    }
                    if(temps[j].tipo=='T' && temps[j].token.tipo=='O'
                       && strcmp(temps[j].token.contenido,(strcmp(op,"(")==0)?")":"]")==0){
                        contador_op_agrupa--;
                    }
                    if(contador_op_agrupa<0)
                        break;
                }

                //COPIO LOS TOKENS NECESARIOS PARA RESOLVER RECURSIVAMENTE
                tokens_agrupa=malloc((j-(i+1))*sizeof(struct Token));
                for(k=0;k<(j-(i+1));k++)
                    tokens_agrupa[k]=temps[k+i+1].token; //DEBIDO A LAS PRIORIDADES SIEMPRE VAN A SER TIPO TOKEN

                //RESUELVO RECURSIVAMENTE
                construir_expresion(tokens_agrupa, j-(i+1), &exp_agrupa);

                //libero memoria
                free(tokens_agrupa);

                //SE TRATA DE LLAMADA A FUNCION O INDEXACION
                if(i>0 && (temps[i-1].tipo=='E' || (temps[i-1].tipo=='T'&&temps[i-1].token.tipo=='I' ))){
                    exp.tipo='O';
                    strcpy(exp.operacion,(strcmp(op,"(")==0)?"SR":"IN");

                    exp.izquierda = malloc(sizeof(struct Expresion));

                    if(temps[i-1].tipo=='E'){
                        *exp.izquierda = temps[i-1].exp;
                    }
                    else{
                        exp.izquierda->tipo = 'I';
                        strcpy(exp.izquierda->texto,temps[i-1].token.contenido);
                    }

                    if(j-(i+1) == 0)
                        exp.derecha=NULL;
                    else{
                        exp.derecha = malloc(sizeof(struct Expresion));
                        *exp.derecha = exp_agrupa;
                    }
                }
                else{//NO SE TRATA DE LLAMADA A FUNCION O INDEXACION
                    exp=exp_agrupa;
                }

                //cambio elemento actual
                temps[i].tipo='E';
                temps[i].exp = exp;

                //SE TRATA DE LLAMADA A FUNCION O INDEXACION
                if(i>0 && (temps[i-1].tipo=='E' || (temps[i-1].tipo=='T'&&temps[i-1].token.tipo=='I' ))){
                    //redusco los elementos a analizar
                    temps[i-1]=temps[i];
                    cant_temps -= (j-i +1);
                    //j tiene el indice del parentesis o corchete de cierre
                    for(k=i;k<cant_temps;k++)temps[k]=temps[k+(j-i +1)];

                    i--;//el for debe evaluar el mismo i
                }//NO SE TRATA DE LLAMADA A FUNCION O INDEXACION
                else{
                    //redusco los elementos a analizar
                    cant_temps -= (j-i);
                    //j tiene el indice del parentesis o corchete de cierre
                    for(k=i+1;k<cant_temps;k++)temps[k]=temps[k+(j-i)];
                }

            }
            //UNARIOS
            else if(temps[i].tipo=='T' && temps[i].token.tipo=='O' && strcmp(temps[i].token.contenido,op)==0
                    && (i==0 || 
                           (temps[i-1].tipo=='T' && temps[i-1].token.tipo=='O')
                       )){
                exp.tipo='O';
                strcpy(exp.operacion,op);

                exp.izquierda = NULL;

                exp.derecha = malloc(sizeof(struct Expresion));

                if(temps[i+1].tipo=='T'){//es un token
                    exp.derecha->tipo=temps[i+1].token.tipo;
                    strcpy(exp.derecha->texto,temps[i+1].token.contenido);
                }else{//es una expresion
                    *exp.derecha=temps[i+1].exp;
                }

                //cambio elemento actual
                temps[i].tipo='E';
                temps[i].exp = exp;

                //reduzco los elementos a analizar
                cant_temps--;
                for(j=i+1;j<cant_temps;j++)temps[j]=temps[j+1];
            }
            //BINARIOS
            else if(temps[i].tipo=='T' && temps[i].token.tipo=='O' && strcmp(temps[i].token.contenido,op)==0 
                && t!=1 // && t!=1 resuelve el problema de los operadores que son binarios y unarios
                ){
                exp.tipo='O'; //es una operacion
                strcpy(exp.operacion,op);

                exp.izquierda = malloc(sizeof(struct Expresion));

                if(temps[i-1].tipo=='T'){//es un token
                    exp.izquierda->tipo=temps[i-1].token.tipo;
                    strcpy(exp.izquierda->texto,temps[i-1].token.contenido);
                }else{//es una expresion
                    *exp.izquierda=temps[i-1].exp;
                }


                exp.derecha = malloc(sizeof(struct Expresion));

                if(temps[i+1].tipo=='T'){//es un token
                    exp.derecha->tipo=temps[i+1].token.tipo;
                    strcpy(exp.derecha->texto,temps[i+1].token.contenido);
                }else{//es una expresion
                    *exp.derecha=temps[i+1].exp;
                }

                //cambio elemento actual
                temps[i].tipo='E';
                temps[i].exp = exp;

                //redusco los elementos a analizar
                temps[i-1]=temps[i];
                cant_temps -= 2;
                for(j=i;j<cant_temps;j++)temps[j]=temps[j+2];

                i--;//el for debe evaluar el mismo i, 
                    //este artificio es válido también para cuando se evalua
                    //de derecha a izquierda (en ese caso debe retroceder
                    // dos en lugar de una posición para la siguiente
                    //iteración, esto también se consigue con i--)
            }
        }
    }

    if(cant_tokens>0){//ME CUIDO DEL CASO DE PARENTESIS VACIOS
        if(temps[0].tipo=='E')
            *expresion=temps[0].exp;
        else{
            expresion->tipo=temps[0].token.tipo;
            strcpy(expresion->texto,temps[0].token.contenido);
        }
    }

    free(temps);
}

int analisis_sintactico(struct Token *tokens, int cant_tokens, struct BloqueCentral *central,int linea){
    char tipo_bloque_actual='C'; //C central, M mientras, Si sino, N sino, I SinoSI
    struct Expresion *expresion=NULL;
    struct NodoSentencia *nodo_actual;
    void *bloque_actual = obtener_ultimo_bloque(central,&tipo_bloque_actual,&nodo_actual);
    void *nuevo_bloque=NULL;
    struct Sentencia sent;

    if(tokens[0].tipo=='R'){
        if(strcmp(tokens[0].contenido,"IMPORTAR")==0){
            if(tipo_bloque_actual!='C'){
                printf("ERROR: SOLO SE PUEDEN IMPORTAR FUENTES EN EL BLOQUE CENTRAL.\n");
                return -1;
            }

            if(cant_tokens<2){
                printf("ERROR: DEBE INCLUIR EL NOMBRE DEL ARCHIVO A IMPORTAR.\n");
                return -1;
            }

            if(cant_tokens>2){
                printf("ERROR: SOLO DEBE INCLUIR EL NOMBRE DEL ARCHIVO A IMPORTAR.\n");
                return -1;
            }

            if(tokens[1].tipo!='T'){
                printf("ERROR: EL NOMBRE DEL ARCHIVO A IMPORTAR DEBE ESTAR ENTRE COMILLAS.\n");
                return -1;
            }

            struct SentenciaImportar sent_importar;
            strcpy(sent_importar.arch,tokens[1].contenido);
            sent_importar.central = central;
            sent.tipo='T'; //CARACTER QUE INDICA SENTENCIA IMPORTAR
            sent.linea=linea;
            sent.val.sent_importar=(struct SentenciaImportar*)malloc(sizeof(struct SentenciaImportar));
            *sent.val.sent_importar = sent_importar;

            //agrego sentencia
            if(nodo_actual->sentencia.tipo==' ') //caso 1ra sentencia bloque
                nodo_actual->sentencia=sent;
            else{
                nodo_actual->sgte = (struct NodoSentencia*)malloc(sizeof(struct NodoSentencia));
                nodo_actual->sgte->sentencia = sent;
                nodo_actual->sgte->sgte = NULL;
            }

            return 0;
        }else if(strcmp(tokens[0].contenido,"CLASE")==0){
            if(tipo_bloque_actual!='C'){
                printf("ERROR: SOLO SE PUEDEN CREAR CLASES EN EL BLOQUE CENTRAL.\n");
                return -1;
            }

            if(cant_tokens<2){
                printf("ERROR: LA CLASE DEBE TENER NOMBRE.\n");
                return -1;
            }

            int i=1;
            for(;i<cant_tokens;i++)
                if(tokens[i].tipo!='I'){
                    printf("ERROR: EL NOMBRE DE LA CLASE Y DE LAS CLASES DE LAS QUE HEREDA DEBEN SER IDENTIFICADORES.\n");
                    printf("EL TOKEN: %s ES DE TIPO %s\n",tokens[i].contenido,texto_tipo_sintactico(tokens[i].tipo));
                    return -1;
                }

            struct SentenciaClase sentclase;
            strcpy(sentclase.nombre,tokens[1].contenido);
            sentclase.num_padres=cant_tokens-2;
            sentclase.padres=NULL;

            if(sentclase.num_padres>0){
                sentclase.padres = (char**)malloc(sentclase.num_padres*sizeof(char*));
                for(i=0;i<sentclase.num_padres;i++){
                    sentclase.padres[i] = (char*)malloc(strlen(tokens[i+2].contenido)+1);
                    strcpy(sentclase.padres[i],tokens[i+2].contenido);
                }
            }

            sent.tipo='L';
            sent.linea=linea;
            sent.val.sent_clase=(struct SentenciaClase*)malloc(sizeof(struct SentenciaClase));
            *sent.val.sent_clase = sentclase;

            //agrego sentencia
            if(nodo_actual->sentencia.tipo==' ') //caso 1ra sentencia bloque
                nodo_actual->sentencia=sent;
            else{
                nodo_actual->sgte = (struct NodoSentencia*)malloc(sizeof(struct NodoSentencia));
                nodo_actual->sgte->sentencia = sent;
                nodo_actual->sgte->sgte = NULL;
            }

            return 0;
        }else if(strcmp(tokens[0].contenido,"ATRIBUTOS")==0){
            if(tipo_bloque_actual!='C'){
                printf("ERROR: SOLO SE PUEDEN CREAR ATRIBUTOS EN EL BLOQUE CENTRAL.\n");
                return -1;
            }

            if(cant_tokens<3){
                printf("ERROR: DEBE INDICAR LA CLASE Y LUEGO LOS ATRIBUTOS A CREAR (POR LO MENOS 1).\n");
                return -1;
            }

            int i=1;
            for(;i<cant_tokens;i++)
                if(tokens[i].tipo!='I'){
                    printf("ERROR: EL NOMBRE DE LA CLASE Y LOS ATRIBUTOS A CREAR DEBEN SER IDENTIFICADORES.\n");
                    printf("EL TOKEN: %s ES DE TIPO %s\n",tokens[i].contenido,texto_tipo_sintactico(tokens[i].tipo));
                    return -1;
                }

            struct SentenciaAtributos sentatributos;
            sentatributos.clase_pertenece=(char*)malloc(strlen(tokens[1].contenido)+1);
            strcpy(sentatributos.clase_pertenece,tokens[1].contenido);
            sentatributos.num_atributos=cant_tokens-2;
            sentatributos.atributos = (char**)malloc(sizeof(char*)*(cant_tokens-2));

            for(i=0;i<cant_tokens-2;i++){
                sentatributos.atributos[i] = (char*)malloc(strlen(tokens[i+2].contenido)+1);
                strcpy(sentatributos.atributos[i],tokens[i+2].contenido);
            }

            sent.tipo='A';
            sent.linea=linea;
            sent.val.sent_atributos=(struct SentenciaAtributos*)malloc(sizeof(struct SentenciaAtributos));
            *sent.val.sent_atributos = sentatributos;

            //agrego sentencia
            if(nodo_actual->sentencia.tipo==' ') //caso 1ra sentencia bloque
                nodo_actual->sentencia=sent;
            else{
                nodo_actual->sgte = (struct NodoSentencia*)malloc(sizeof(struct NodoSentencia));
                nodo_actual->sgte->sentencia = sent;
                nodo_actual->sgte->sgte = NULL;
            }

            return 0;
        }else if(strcmp(tokens[0].contenido,"METODO")==0){
            if(tipo_bloque_actual!='C'){
                printf("ERROR: SOLO SE PUEDEN CREAR METODOS EN EL BLOQUE CENTRAL.\n");
                return -1;
            }

            if(cant_tokens<3){
                printf("ERROR: EL METODO DEBE INDICAR LA CLASE A LA QUE PERTENECE Y LUEGO DEBE DE TENER UN NOMBRE.\n");
                return -1;
            }

            int i=1;
            for(;i<cant_tokens;i++)
                if(tokens[i].tipo!='I'){
                    printf("ERROR: EL NOMBRE DE LA CLASE, EL NOMBRE DEL METODO"
                           " Y LOS PARAMETROS DEBEN DE SER IDENTIFICADORES.\n");
                    printf("EL TOKEN: %s ES DE TIPO %s\n",tokens[i].contenido,texto_tipo_sintactico(tokens[i].tipo));
                    return -1;
                }

            struct BloqueFuncion funcion;
            funcion.cerrado=0; //false
            funcion.num_parametros = cant_tokens-3;
            funcion.nodos.sentencia.tipo=' ';
            funcion.nodos.sgte=NULL;
            strcpy(funcion.nombre,tokens[2].contenido);
            funcion.clase_pertenece = (char*)malloc(strlen(tokens[1].contenido)+1);
            strcpy(funcion.clase_pertenece,tokens[1].contenido);

            if( funcion.num_parametros>0){
                funcion.parametros=(char**)malloc(sizeof(char*)*funcion.num_parametros);
                for(i=0;i<funcion.num_parametros;i++){
                    funcion.parametros[i]=(char*)malloc(strlen(tokens[i+3].contenido)+1);
                    strcpy(funcion.parametros[i],tokens[i+3].contenido);
                }
            }else{
                funcion.parametros=NULL;
            }

            nuevo_bloque = malloc(sizeof(struct BloqueFuncion));

            //inicializo valores
            *((struct BloqueFuncion*)nuevo_bloque) = funcion;

            //creo sentencia
            sent.tipo='F';
            sent.linea=linea;
            sent.val.funcion=(struct BloqueFuncion*)nuevo_bloque;

            //agrego sentencia
            if(nodo_actual->sentencia.tipo==' ') //caso 1ra sentencia bloque
                nodo_actual->sentencia=sent;
            else{
                nodo_actual->sgte = (struct NodoSentencia*)malloc(sizeof(struct NodoSentencia));
                nodo_actual->sgte->sentencia = sent;
                nodo_actual->sgte->sgte = NULL;
            }

            return 0;
        }else if(strcmp(tokens[0].contenido,"SUBRUTINA")==0){
            if(tipo_bloque_actual!='C'){
                printf("ERROR: SOLO SE PUEDEN CREAR SUBRUTINAS EN EL BLOQUE CENTRAL.\n");
                return -1;
            }

            if(cant_tokens<2){
                printf("ERROR: LA SUBRUTINA DEBE DE TENER UN NOMBRE.\n");
                return -1;
            }

            int i=1;
            for(;i<cant_tokens;i++)
                if(tokens[i].tipo!='I'){
                    printf("ERROR: EL NOMBRE DE LA SUBRUTINA Y SUS PARAMETROS DEBEN DE SER IDENTIFICADORES.\n");
                    printf("EL TOKEN: %s ES DE TIPO %s\n",tokens[i].contenido,texto_tipo_sintactico(tokens[i].tipo));
                    return -1;
                }

            struct BloqueFuncion funcion;
            funcion.cerrado=0; //false
            funcion.num_parametros = cant_tokens-2;
            funcion.nodos.sentencia.tipo=' ';
            funcion.nodos.sgte=NULL;
            strcpy(funcion.nombre,tokens[1].contenido);
            funcion.clase_pertenece=NULL;

            if( funcion.num_parametros>0){
                funcion.parametros=(char**)malloc(sizeof(char*)*funcion.num_parametros);
                for(i=0;i<funcion.num_parametros;i++){
                    funcion.parametros[i]=(char*)malloc(strlen(tokens[i+2].contenido)+1);
                    strcpy(funcion.parametros[i],tokens[i+2].contenido);
                }
            }else{
                funcion.parametros=NULL;
            }

            nuevo_bloque = malloc(sizeof(struct BloqueFuncion));

            //inicializo valores
            *((struct BloqueFuncion*)nuevo_bloque) = funcion;

            //creo sentencia
            sent.tipo='F';
            sent.linea=linea;
            sent.val.funcion=(struct BloqueFuncion*)nuevo_bloque;

            //agrego sentencia
            if(nodo_actual->sentencia.tipo==' ') //caso 1ra sentencia bloque
                nodo_actual->sentencia=sent;
            else{
                nodo_actual->sgte = (struct NodoSentencia*)malloc(sizeof(struct NodoSentencia));
                nodo_actual->sgte->sentencia = sent;
                nodo_actual->sgte->sgte = NULL;
            }

            return 0;

            /*asignar_elemento_tabla_simbolos_funciones(tokens[1].contenido, funcion,TablaSimbolosFunciones,&error);

            if(error<0){
                limpiar_funcion(funcion);
                printf("ERROR: YA EXISTE UNA SUBRUTINA DE ESE NOMBRE.\n");
                return -1;
            }*/
        }

        if(cant_tokens>1){
            //LA MAYORIA DE PALABRAS RESERVADAS SON SEGUIDAS POR NADA O POR UNA EXPRESION PERO NO LAS SUBRUTINAS
            if(revisar_sintaxis(&tokens[1],cant_tokens-1)!=0)
                return -1;

            expresion = (struct Expresion*)malloc(sizeof(struct Expresion));
            construir_expresion(&tokens[1],cant_tokens-1,expresion);
            

            if(strcmp(tokens[0].contenido,"MIENTRAS")==0){
                nuevo_bloque = malloc(sizeof(struct BloqueMientras));
                ((struct BloqueMientras*)nuevo_bloque)->expre=*expresion;

                //inicializo valores
                ((struct BloqueMientras*)nuevo_bloque)->cerrado=0;//false
                ((struct BloqueMientras*)nuevo_bloque)->nodos.sentencia.tipo=' ';//' ' sentencia inicial del bloque
                ((struct BloqueMientras*)nuevo_bloque)->nodos.sgte=NULL;

                //creo sentencia
                sent.tipo='M';
                sent.linea=linea;
                sent.val.mientras=(struct BloqueMientras*)nuevo_bloque;

                //agrego sentencia
                if(nodo_actual->sentencia.tipo==' ') //caso 1ra sentencia bloque
                    nodo_actual->sentencia=sent;
                else{
                    nodo_actual->sgte = (struct NodoSentencia*)malloc(sizeof(struct NodoSentencia));
                    nodo_actual->sgte->sentencia = sent;
                    nodo_actual->sgte->sgte = NULL;
                }
            }
            else if(strcmp(tokens[0].contenido,"DEVOLVER")==0){
                //creo sentencia
                sent.tipo='D';
                sent.linea=linea;
                sent.val.expre=expresion;

                //agrego sentencia
                if(nodo_actual->sentencia.tipo==' ') //caso 1ra sentencia bloque
                    nodo_actual->sentencia=sent;
                else{
                    nodo_actual->sgte = (struct NodoSentencia*)malloc(sizeof(struct NodoSentencia));
                    nodo_actual->sgte->sentencia = sent;
                    nodo_actual->sgte->sgte = NULL;
                }
            }
            else if(strcmp(tokens[0].contenido,"SI")==0){
                nuevo_bloque = malloc(sizeof(struct BloqueSi));
                ((struct BloqueSi*)nuevo_bloque)->expre=*expresion;

                //inicializo valores
                ((struct BloqueSi*)nuevo_bloque)->cerrado=0;//false
                ((struct BloqueSi*)nuevo_bloque)->nodos.sentencia.tipo=' ';//' ' sentencia inicial del bloque
                ((struct BloqueSi*)nuevo_bloque)->nodos.sgte=NULL;
                ((struct BloqueSi*)nuevo_bloque)->sino=NULL;
                ((struct BloqueSi*)nuevo_bloque)->sino_si=NULL;

                //creo sentencia
                sent.tipo='S';
                sent.linea=linea;
                sent.val.si=(struct BloqueSi*)nuevo_bloque;

                //agrego sentencia
                if(nodo_actual->sentencia.tipo==' ') //caso 1ra sentencia bloque
                    nodo_actual->sentencia=sent;
                else{
                    nodo_actual->sgte = (struct NodoSentencia*)malloc(sizeof(struct NodoSentencia));
                    nodo_actual->sgte->sentencia = sent;
                    nodo_actual->sgte->sgte = NULL;
                }
            }
            else if(strcmp(tokens[0].contenido,"SINO_SI")==0){
                if(tipo_bloque_actual!='S' && tipo_bloque_actual!='I'){
                    printf("\nERROR: SINO_SI inesperado, se encuentra dentro de bloque: %s\n",
                           texto_bloque(tipo_bloque_actual));
                    return -1;
                }
                struct BloqueSinoSi nuevo_bloque_sinosi;
                nuevo_bloque_sinosi.expre=*expresion;

                //inicializo valores
                nuevo_bloque_sinosi.nodos.sentencia.tipo=' ';//' ' sentencia inicial del bloque
                nuevo_bloque_sinosi.nodos.sgte=NULL;
                nuevo_bloque_sinosi.linea=linea;

                //inserto el bloque
                struct NodoSinoSi **bloque_sinosi_actual = &((struct BloqueSi*)bloque_actual)->sino_si;
                while(*bloque_sinosi_actual!=NULL)bloque_sinosi_actual=&(*bloque_sinosi_actual)->sgte;

                *bloque_sinosi_actual = (struct NodoSinoSi*)malloc(sizeof(struct NodoSinoSi));
                (*bloque_sinosi_actual)->sino_si = nuevo_bloque_sinosi;
                (*bloque_sinosi_actual)->sgte=NULL;
            }
            else if(strcmp(tokens[0].contenido,"SINO")==0){
                    printf("\nERROR: LA PALABRA RESERVADA SINO DEBE IR SOLA\n");
                    return -1;
            }
            else if(strcmp(tokens[0].contenido,"FIN_SI")==0){
                    printf("\nERROR: LA PALABRA RESERVADA FIN_SI DEBE IR SOLA\n");
                    return -1;
            }
            else if(strcmp(tokens[0].contenido,"FIN_MIENTRAS")==0){
                    printf("\nERROR: LA PALABRA RESERVADA FIN_MIENTRAS DEBE IR SOLA\n");
                    return -1;
            }
            else if(strcmp(tokens[0].contenido,"CONTINUAR")==0){
                    printf("\nERROR: LA PALABRA RESERVADA CONTINUAR DEBE IR SOLA\n");
                    return -1;
            }
            else if(strcmp(tokens[0].contenido,"ROMPER")==0){
                    printf("\nERROR: LA PALABRA RESERVADA ROMPER DEBE IR SOLA\n");
                    return -1;
            }
            else if(strcmp(tokens[0].contenido,"FIN_SUBRUTINA")==0){
                    printf("\nERROR: LA PALABRA RESERVADA FIN_SUBRUTINA DEBE IR SOLA\n");
                    return -1;
            }
            else if(strcmp(tokens[0].contenido,"FIN_METODO")==0){
                    printf("\nERROR: LA PALABRA RESERVADA FIN_METODO DEBE IR SOLA\n");
                    return -1;
            }
        }
        else{
            if(strcmp(tokens[0].contenido,"FIN_MIENTRAS")==0){
                if(tipo_bloque_actual!='M'){
                    printf("\nERROR: FIN_MIENTRAS inesperado, se encuentra dentro de bloque: %s\n",
                           texto_bloque(tipo_bloque_actual));
                    return -1;
                }
                ((struct BloqueMientras*)bloque_actual)->cerrado=1;//true
            }
            else if(strcmp(tokens[0].contenido,"FIN_SI")==0){
                if(tipo_bloque_actual!='S' && tipo_bloque_actual!='N' && tipo_bloque_actual!='I'){
                    printf("\nERROR: FIN_SI inesperado, se encuentra dentro de bloque: %s\n",
                           texto_bloque(tipo_bloque_actual));
                    return -1;
                }
                ((struct BloqueSi*)bloque_actual)->cerrado=1;//true
            }
            else if(strcmp(tokens[0].contenido,"FIN_SUBRUTINA")==0){
                if(tipo_bloque_actual!='F'){
                    printf("\nERROR: FIN_SUBRUTINA inesperado, se encuentra dentro de bloque: %s\n",
                           texto_bloque(tipo_bloque_actual));
                    return -1;
                }
                if(((struct BloqueFuncion*)bloque_actual)->clase_pertenece!=NULL){
                    printf("\nERROR: FIN_SUBRUTINA DEBE USARSE EN CONJUNTO CON SUBRUTINA,"
                           " CON METODO DEBE USARSE FIN_METODO\n");
                    return -1;
                }
                ((struct BloqueFuncion*)bloque_actual)->cerrado=1;//true
            }
            else if(strcmp(tokens[0].contenido,"FIN_METODO")==0){
                if(tipo_bloque_actual!='F'){
                    printf("\nERROR: FIN_METODO inesperado, se encuentra dentro de bloque: %s\n",
                           texto_bloque(tipo_bloque_actual));
                    return -1;
                }
                if(((struct BloqueFuncion*)bloque_actual)->clase_pertenece==NULL){
                    printf("\nERROR: FIN_METODO DEBE USARSE EN CONJUNTO CON METODO,"
                           " CON SUBRUTINA DEBE USARSE FIN_SUBRUTINA\n");
                    return -1;
                }
                ((struct BloqueFuncion*)bloque_actual)->cerrado=1;//true
            }
            else if(strcmp(tokens[0].contenido,"SINO")==0){
                if(tipo_bloque_actual!='S' && tipo_bloque_actual!='I'){
                    printf("\nERROR: SINO inesperado, se encuentra dentro de bloque: %s\n",
                           texto_bloque(tipo_bloque_actual));
                    return -1;
                }
                ((struct BloqueSi*)bloque_actual)->sino=(struct BloqueSino*)malloc(sizeof(struct BloqueSino));
                ((struct BloqueSi*)bloque_actual)->sino->nodos.sgte=NULL;
                ((struct BloqueSi*)bloque_actual)->sino->nodos.sentencia.tipo=' ';
            }
            else if(strcmp(tokens[0].contenido,"CONTINUAR")==0){
                //creo sentencia
                sent.tipo='C';
                sent.linea=linea;

                //agrego sentencia
                if(nodo_actual->sentencia.tipo==' ') //caso 1ra sentencia bloque
                    nodo_actual->sentencia=sent;
                else{
                    nodo_actual->sgte = (struct NodoSentencia*)malloc(sizeof(struct NodoSentencia));
                    nodo_actual->sgte->sentencia = sent;
                    nodo_actual->sgte->sgte = NULL;
                }
            }
            else if(strcmp(tokens[0].contenido,"ROMPER")==0){
                //creo sentencia
                sent.tipo='R';
                sent.linea=linea;

                //agrego sentencia
                if(nodo_actual->sentencia.tipo==' ') //caso 1ra sentencia bloque
                    nodo_actual->sentencia=sent;
                else{
                    nodo_actual->sgte = (struct NodoSentencia*)malloc(sizeof(struct NodoSentencia));
                    nodo_actual->sgte->sentencia = sent;
                    nodo_actual->sgte->sgte = NULL;
                }
            }
            else if(strcmp(tokens[0].contenido,"DEVOLVER")==0){
                //creo sentencia
                sent.tipo='D';
                sent.linea=linea;
                sent.val.expre=NULL;

                //agrego sentencia
                if(nodo_actual->sentencia.tipo==' ') //caso 1ra sentencia bloque
                    nodo_actual->sentencia=sent;
                else{
                    nodo_actual->sgte = (struct NodoSentencia*)malloc(sizeof(struct NodoSentencia));
                    nodo_actual->sgte->sentencia = sent;
                    nodo_actual->sgte->sgte = NULL;
                }
            }
            else if(strcmp(tokens[0].contenido,"MIENTRAS")==0){
                    printf("\nERROR: MIENTRAS SIN CONDICION\n");
                    return -1;
            }
            else if(strcmp(tokens[0].contenido,"SI")==0){
                    printf("\nERROR: SI SIN CONDICION\n");
                    return -1;
            }
            else if(strcmp(tokens[0].contenido,"SINO_SI")==0){
                    printf("\nERROR: SINO_SI SIN CONDICION\n");
                    return -1;
            }
        }
    }
    else{
        if(revisar_sintaxis(tokens,cant_tokens)!=0)
            return -1;

        expresion = (struct Expresion*)malloc(sizeof(struct Expresion));
        construir_expresion(tokens,cant_tokens,expresion);

        //creo sentencia
        sent.tipo='E';
        sent.linea=linea;
        sent.val.expre=expresion;

        //agrego sentencia
        if(nodo_actual->sentencia.tipo==' ') //caso 1ra sentencia bloque
            nodo_actual->sentencia=sent;
        else{
            nodo_actual->sgte = (struct NodoSentencia*)malloc(sizeof(struct NodoSentencia));
            nodo_actual->sgte->sentencia = sent;
            nodo_actual->sgte->sgte = NULL;
        }
    }

    return 0;    
}
