#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

//index set
#define ordem 4
#define MAX 3 //ordem-1
#define MIN 1 //(ordem/2)-1
//sequence set
//blocos com mesmo tamanho da ordem


//registro para os filmes. Tamanho fixo de 192 bytes
typedef struct{
    char codigo[6];
    char ano[5];
    int nota;
    char titulo_port[50];
    char titulo_original[50];
    char diretor[50];
    char pais[22];
}registro;

//estrutura de uma página da Árvore B+
typedef struct{
    int RNN; //RNN do nó/página no arquivo de índice primário em Árvore B+
    char eFolha; //Verifica se o nó é folha
    int numChaves; //número de chaves na página
    char chaves[ordem][6]; //Vetor com os valores das chaves
    int dataRRN[ordem]; //RRNs do arquivo de dados associados a cada chave nos nós folhas
    int filhos[ordem+1]; //Ponteiros para os nós filhos
    int pai; //Ponteiro para o nó pai
    int prox_no; //referência para a próxima página na lista de nós folhas
}pagina;

typedef struct{
    pagina Pagina;
    struct arvoreBplus *prox_no;
}arvoreBplus;

//estrutura de dados para o indice secundario
typedef struct Lista_s{
    char titulo[50];
    char chave_primaria[6];
    struct Lista_s *prox;
}Lista_s;

arvoreBplus *criar_arquivo_primario(arvoreBplus **cs);

arvoreBplus criar_no(int RNN);

arvoreBplus *encontrar_pagina(int RNN, arvoreBplus ** cs, FILE *t);

void criar_arquivo_secundario(FILE *d, Lista_s **h);

void reconstruir_indice_secundario(FILE *d, Lista_s **h);

void inserir_ordenado_secundario(Lista_s **h, char *chave, char *titulo);

void insercao_filme_usuario(FILE *d, arvoreBplus **raiz, arvoreBplus **cs, Lista_s **h);

arvoreBplus *buscar_na_arvore(char *chave, arvoreBplus **raiz, arvoreBplus **cs, FILE *t);

int buscar_chave(arvoreBplus *no, char *chave);

void insercao_pagina(FILE *t, arvoreBplus **raiz, arvoreBplus **cs, char *chave, int RRN_data);

int calcular_RNN_pagina(FILE *t);

void inserir_ordenado_conjunto_sequencias(arvoreBplus **conjSequencias, arvoreBplus *folha);

int calcular_RRN(Lista_s **h);

void insercao_na_folha(FILE *t, arvoreBplus *no, char *chave, int RRN_data);

void insercao_no_pai(FILE *t, arvoreBplus **raiz, arvoreBplus **cs, arvoreBplus *no, arvoreBplus *novo_no, char *chave);

void atualiza_arquivo_arvore(FILE *t, arvoreBplus *no);

void insere_arquivo_arvore(FILE *t, arvoreBplus *no);

void buscar_filme_usuario(FILE *d, arvoreBplus **raiz, arvoreBplus **cs, Lista_s *h);

void buscar_filme_arquivo(FILE *d, int byte_offset);

void formatar(char *titulo_port, char *titulo_ori, char *diretor, char *ano, char *pais, int nota);

int busca_indice_titulo(FILE *d, FILE *t, Lista_s *h, arvoreBplus **raiz, arvoreBplus **cs, char* titulo);

void modificar_nota_filme(arvoreBplus **raiz, arvoreBplus **cs);

void conjunto_de_sequencias(FILE *t, arvoreBplus **cs);

void listar_filmes(FILE *d, arvoreBplus *raiz, arvoreBplus **cs);

void listar_filmes_intervalo(FILE *d, arvoreBplus *raiz, arvoreBplus **cs, char *chave);

void encerrar_execucao(FILE *d, arvoreBplus *raiz, arvoreBplus *cs, Lista_s *h);

int main(){
    arvoreBplus *raiz = NULL;
    Lista_s *h = NULL;
    arvoreBplus *conjSequencias = NULL;

    FILE *d;

    int opcao;
    char chave[6];

    if((d = fopen("movies.dat", "r")) != NULL){
        fclose(d);
        d = fopen("movies.dat", "a+");
    }
    else{
        d = fopen("movies.dat", "a+");
    }
    criar_arquivo_secundario(d,&h);
    raiz = criar_arquivo_primario(&conjSequencias);

    do{
        printf("\n1.Insercao de filme\n2.Busca de filme\n3.Modificar a nota de um filme\n4.Listar todos os filmes no catalogo\n5.Listar um intervalo de filmes\n6.Finalizar a execucao.\n");
        printf("Selecione uma opcao: ");
        scanf("%d", &opcao);

        switch(opcao){
            case 1: insercao_filme_usuario(d, &raiz, &conjSequencias, &h);
            break;
            case 2: buscar_filme_usuario(d, &raiz, &conjSequencias, h);
            break;
            case 3: modificar_nota_filme(&raiz,&conjSequencias);
            break;
            case 4: listar_filmes(d,raiz,&conjSequencias);
            break;
            case 5: printf("Digite a primeira chave do intervalo: ");
            scanf(" %[^\n]s", chave);
            listar_filmes_intervalo(d,raiz,&conjSequencias,chave);
            break;
            case 6: encerrar_execucao(d,raiz,conjSequencias,h);
            break;
        }
    }while(opcao != 6);

    return 0;
}

arvoreBplus *criar_arquivo_primario(arvoreBplus **cs){
    arvoreBplus* raiz = malloc(sizeof(arvoreBplus));
    FILE *t;
    int RNN_raiz, i;

    if((t = fopen("ibtree.idx", "r")) != NULL){
        fscanf(t, "%d\n", &RNN_raiz);

        raiz = encontrar_pagina(RNN_raiz, cs, t);

    }
    else{
        t = fopen("ibtree.idx", "w+");
        fprintf(t, "1\n");
        fprintf(t, "1|T|0|");
        for(i=0;i<MAX;i++){
          if(i == MAX - 1){
            fprintf(t, "NULL|");
          }
          else{
            fprintf(t, "NULL,");
          }
        }
        for(i=0;i<MAX;i++){
          if(i == MAX - 1){
            fprintf(t, "-1|");
          }
          else{
            fprintf(t, "-1,");
          }
        }
        for(i=0;i<MAX+1;i++){
          if(i == MAX){
            fprintf(t, "-1|");
          }
          else{
            fprintf(t, "-1,");
          }
        }
      fprintf(t, "-1|-1|\n");
      *raiz = criar_no(1);
      inserir_ordenado_conjunto_sequencias(cs,raiz);
    }
  
    fclose(t);
    return raiz;
}

arvoreBplus criar_no(int RNN){
    arvoreBplus no;
    int i;

    no.Pagina.RNN = RNN;
    no.Pagina.eFolha = 'T';
    no.Pagina.numChaves = 0;
    for(i=0;i<MAX;i++){
        strcpy(no.Pagina.chaves[i], "NULL");
    }
    for(i=0;i<MAX;i++){
        no.Pagina.dataRRN[i] = -1;
    }
    for(i=0;i<ordem;i++){
        no.Pagina.filhos[i] = -1;
    }
    no.Pagina.pai = -1;
    no.Pagina.prox_no = -1;
    no.prox_no = NULL;
    return no;
}

arvoreBplus *encontrar_pagina(int RNN, arvoreBplus **cs, FILE *t){
    pagina Pagina;
    arvoreBplus *no = malloc(sizeof(arvoreBplus));
    int i;
    char estrutura[200];

    fseek(t, 0, SEEK_SET);

    for(i=0;i<RNN;i++){
        fgets(estrutura, 200, t);
    }

    fscanf(t, "%d|", &Pagina.RNN);
    if(Pagina.RNN == RNN){
        fscanf(t, "%c|%d|", &Pagina.eFolha, &Pagina.numChaves);
        //leitura das chaves 
        for(i=0;i<MAX;i++){
            if(i != MAX-1){
                fscanf(t, "%[^,],", Pagina.chaves[i]);
            }
            else{
                fscanf(t, "%[^|]|", Pagina.chaves[i]);
            }
        }

        for(i=0;i<MAX;i++){
            if(i != MAX-1){
                fscanf(t, "%d,", &Pagina.dataRRN[i]);
            }
            else{
                fscanf(t, "%d|", &Pagina.dataRRN[i]);
            }
        }

        for(i=0;i<ordem;i++){
            if(i != ordem-1){
                fscanf(t, "%d,", &Pagina.filhos[i]);
            }
            else{
                fscanf(t, "%d|", &Pagina.filhos[i]);
            }
        }
        fscanf(t, "%d|%d|", &Pagina.pai, &Pagina.prox_no);
    }

    //criar o no para adicionar na árvore B+ na memória
    no->Pagina = Pagina;
    no->prox_no = NULL;
    return no;
}

void criar_arquivo_secundario(FILE *d, Lista_s **h){
    Lista_s *aux;
    FILE *s;
    //se existir arquivo de índice secundário, refazê-lo
    if((s = fopen("ititle.idx", "r")) != NULL){
        fclose(s);
        s = fopen("ititle.idx", "w");
    }
    else{
        s = fopen("ititle.idx", "w");
    }

    reconstruir_indice_secundario(d, h);

    aux = *h;
    while(aux != NULL){
        fprintf(s,"%s#%s\n", aux->titulo, aux->chave_primaria);
        aux = aux->prox;
    }
    fflush(s);
    fclose(s);
}

void reconstruir_indice_secundario(FILE *d, Lista_s **h){
    char chave[6];
    char titulo[50];
    int i = 0;

    fseek(d, 0, SEEK_SET);//posiciona o ponteiro no inicio do arquivo
    while(fscanf(d, "%5[^@]@%49[^@]", chave, titulo) == 2){

        if(chave[0] != '|' && chave[1] != '*'){//condicao para verificar se o registro foi removido
            inserir_ordenado_secundario(h,chave,titulo);
        }
        i++;
        fseek(d, i * 192, SEEK_SET);//posiciona o ponteiro para o proximo registro
    }
}

void inserir_ordenado_secundario(Lista_s **h, char *chave, char *titulo){
    Lista_s *atual = malloc(sizeof(Lista_s)), *aux = *h, *antes = NULL;

    strcpy(atual->chave_primaria,chave);
    strcpy(atual->titulo,titulo);
    atual->prox = NULL;

    if(*h == NULL){//se lista estiver vazia, se torna o primeiro elemento
        atual->prox = NULL;
        *h = atual;
        return;
    }
    //se indice vier antes do primeiro elemento, se torna o primeiro da lista
    if(strcmp(atual->titulo,(*h)->titulo) < 0){
        atual->prox = (*h);
        *h = atual;
        return;
    }
    //enquanto nao chegar no final e indice vier depois dos elementos da lista
    while(aux!=NULL && strcmp(atual->titulo,aux->titulo) > 0){
        antes = aux;
        aux = aux->prox;
    }
    antes->prox = atual;
    atual->prox = aux;
    return;
}

void insercao_filme_usuario(FILE *d, arvoreBplus **raiz, arvoreBplus **cs, Lista_s **h){
    registro filme;
    int i, RRN;
    int posicao_sobrenome;
    int bytes_campos = 17; //campos fixos e delimitadores
    char codigo[6];
    char primeiro_nome[30];
    arvoreBplus *no;
    FILE *t = fopen("ibtree.idx", "r+");

    printf("\nDigite o titulo do filme em portugues: ");
    scanf(" %[^\n]s", filme.titulo_port);
    printf("Digite o titulo original: ");
    scanf(" %[^\n]s", filme.titulo_original);
    printf("Digite o diretor: ");
    scanf(" %[^\n]s", filme.diretor);
    printf("Digite o ano de lancamento: ");
    scanf(" %[^\n]s", filme.ano);
    getchar();
    printf("Digite o pais: ");
    scanf(" %[^\n]s", filme.pais);
    printf("Digite a nota: ");
    scanf("%d", &filme.nota);

    //chave primaria
    for(i=0;i<strlen(filme.diretor);i++){
        if(filme.diretor[i] == ' '){
            posicao_sobrenome = i + 1;
        }
    }

    //funcao que recebe de parametro um numero maximo de caracteres a serem copiados
    strncpy(codigo, &filme.diretor[posicao_sobrenome], 3);
    codigo[0] = toupper(codigo[0]);
    codigo[1] = toupper(codigo[1]);
    codigo[2] = toupper(codigo[2]);
    codigo[3] = filme.ano[2];
    codigo[4] = filme.ano[3];
    codigo[5] = '\0';

    strcpy(filme.codigo,codigo);

    //verificar, atraves de uma busca na Arvore B+, se a já chave primaria existe
    no = buscar_na_arvore(filme.codigo, raiz, cs, t);

    if(buscar_chave(no, codigo) == -1){
        //separar nome do sobrenome
        for(i=0;i<strlen(filme.diretor);i++){
            if(filme.diretor[i] == ' '){
                posicao_sobrenome = i + 1;
                break;
            }
            primeiro_nome[i] = filme.diretor[i];
        }
        primeiro_nome[i] = '\0';

        //inserir no arquivo
        fprintf(d, "%s@", filme.codigo);
        fprintf(d, "%s@", filme.titulo_port);
        if((strcmp(filme.titulo_original,filme.titulo_port)) == 0){
            strcpy(filme.titulo_original, "Idem");
        }
        fprintf(d, "%s@", filme.titulo_original);
        fprintf(d, "%s, %s@", &filme.diretor[posicao_sobrenome], primeiro_nome);
        fprintf(d, "%s@", filme.ano);
        fprintf(d, "%s@", filme.pais);
        fprintf(d, "%d@", filme.nota);

        //calculo dos bytes
        bytes_campos += strlen(filme.titulo_port) + strlen(filme.titulo_original) + strlen(filme.pais) + strlen(filme.diretor) + 1;
        for(i=0;i<(192 - bytes_campos);i++){
            fprintf(d, "#"); //preenche registro
        }

        //atualizar os índices
        RRN = calcular_RRN(h);
        inserir_ordenado_secundario(h,filme.codigo,filme.titulo_port);
        insercao_pagina(t, raiz, cs, filme.codigo, RRN);
    }
    else{
        printf("\nFilme nao pode ser inserido por conta de duplicacao de chave.\n");
    }

    fflush(d);
    fclose(t);
}

//função para buscar um nó na árvore B+ na memória
arvoreBplus *buscar_na_arvore(char *chave, arvoreBplus **raiz, arvoreBplus **cs, FILE *t){
    arvoreBplus *no = *raiz;
    char chaves_no[MAX][6];
    int i, RNN_filho;

    while(no != NULL && no->Pagina.eFolha == 'F'){
        for(i=0;i<no->Pagina.numChaves;i++){
            strcpy(chaves_no[i], no->Pagina.chaves[i]);
        }

        for(i=0;i<no->Pagina.numChaves;i++){
            if(strcmp(chave,chaves_no[i]) < 0){//se a chave é menor que a chave atual do nó
                //muda para o filho a esquerda e sai do loop
                RNN_filho = no->Pagina.filhos[i];
                no = encontrar_pagina(RNN_filho, cs, t);
                break;
            }
            else if(strcmp(chave,chaves_no[i]) == 0 || i+1 == no->Pagina.numChaves){//se a chave é igual a chave atual do nó
                //muda para o filho a direita e sai do loop
                //alcançou a última chave no nó e não entrou nas outras condições
                RNN_filho = no->Pagina.filhos[i+1];
                no = encontrar_pagina(RNN_filho, cs, t);
                break;
            }
        }
    }
    //se for folha adicionar na lista do Conjunto de sequências
    inserir_ordenado_conjunto_sequencias(cs,no);
    
    return no;
}

int buscar_chave(arvoreBplus *no, char *chave){
    int i;

    for(i=0;i<no->Pagina.numChaves;i++){
        if(strcmp(chave,no->Pagina.chaves[i]) == 0){
            return no->Pagina.dataRRN[i];
        }
    }

    return -1;
}

void insercao_pagina(FILE *t, arvoreBplus **raiz, arvoreBplus **cs, char *chave, int RRN_data){
    int RNN, mediana, i, j = 0;
    arvoreBplus *no = buscar_na_arvore(chave, raiz, cs, t);//Encontre o nó folha adequado para inserção
    arvoreBplus *novo_no = malloc(sizeof(arvoreBplus));
    insercao_na_folha(t, no, chave, RRN_data);

    if(no->Pagina.numChaves == ordem){// se o nó estiver cheio é necessário dividí-lo (split)
        RNN = calcular_RNN_pagina(t);
        *novo_no = criar_no(RNN);
        novo_no->Pagina.pai = no->Pagina.pai;

        mediana = (int)(ceil((double)no->Pagina.numChaves / 2)) - 1;
        for(i=mediana+1;i<no->Pagina.numChaves;i++){
            strcpy(novo_no->Pagina.chaves[j], no->Pagina.chaves[i]);
            strcpy(no->Pagina.chaves[i], "NULL");
            novo_no->Pagina.dataRRN[j] = no->Pagina.dataRRN[i];
            no->Pagina.dataRRN[i] = -1;
            j++;
        }
        novo_no->Pagina.numChaves = j;
        no->Pagina.numChaves = mediana + 1;
        novo_no->Pagina.prox_no = no->Pagina.prox_no;
        no->Pagina.prox_no = novo_no->Pagina.RNN;

        insercao_no_pai(t, raiz, cs, no, novo_no, novo_no->Pagina.chaves[0]);
        inserir_ordenado_conjunto_sequencias(cs,novo_no);
    }
    else{
        atualiza_arquivo_arvore(t,no);
    }
    
}

int calcular_RNN_pagina(FILE *t){
    int RNN, num;

    fseek(t, 0, SEEK_SET);
    do{
        num = fscanf(t, "%d%*[^\n]", &RNN);
    }while(num == 1);

    RNN++;
    return RNN;
}

void insercao_na_folha(FILE *t, arvoreBplus *no, char *chave, int RRN_data){
    int i, j;
    char chaves_no[MAX][6];

    if(no->Pagina.numChaves != 0){
        for(i=0;i<no->Pagina.numChaves;i++){
            strcpy(chaves_no[i], no->Pagina.chaves[i]);
        }
        for(i=0;i<no->Pagina.numChaves;i++){
            if(strcmp(chave,chaves_no[i]) == 0){
                //chave já existe
                break;
            }
            else if(strcmp(chave,chaves_no[i]) < 0){
                for(j=no->Pagina.numChaves-1;j>=i;j--){
                    strcpy(no->Pagina.chaves[j+1], no->Pagina.chaves[j]);
                    no->Pagina.dataRRN[j+1] = no->Pagina.dataRRN[j];
                }
                strcpy(no->Pagina.chaves[i], chave);
                no->Pagina.dataRRN[i] = RRN_data;
                no->Pagina.numChaves++;
                break;
            }
            else if(i+1 == no->Pagina.numChaves){
                strcpy(no->Pagina.chaves[i+1], chave);
                no->Pagina.dataRRN[i+1] = RRN_data;
                no->Pagina.numChaves++;
                break;
            }
        }
    }
    else{
        strcpy(no->Pagina.chaves[0], chave);
        no->Pagina.numChaves++;
        no->Pagina.dataRRN[0] = RRN_data;
    }
}

void insercao_no_pai(FILE *t, arvoreBplus **raiz, arvoreBplus **cs, arvoreBplus *no, arvoreBplus *novo_no, char *chave){
    arvoreBplus *nova_raiz = malloc(sizeof(arvoreBplus)), *no_interno = malloc(sizeof(arvoreBplus));
    arvoreBplus *no_pai, *no_filho;
    int RNN, i = 0, numFilhos = 0, j, mediana;
    char chave_promovida[6];

    if(no->Pagina.RNN == (*raiz)->Pagina.RNN){
        RNN = (novo_no->Pagina.RNN) + 1;
        *nova_raiz = criar_no(RNN); //criar um novo nó para se tornar raiz
        strcpy(nova_raiz->Pagina.chaves[0], chave);
        nova_raiz->Pagina.filhos[0] = no->Pagina.RNN;
        nova_raiz->Pagina.filhos[1] = novo_no->Pagina.RNN;
        nova_raiz->Pagina.eFolha = 'F';
        nova_raiz->Pagina.numChaves = 1;

        (*raiz) = nova_raiz;
        no->Pagina.pai = (*raiz)->Pagina.RNN;
        novo_no->Pagina.pai = (*raiz)->Pagina.RNN;

        atualiza_arquivo_arvore(t,no);
        insere_arquivo_arvore(t,novo_no);
        insere_arquivo_arvore(t,(*raiz));

        return;
    }
    //nó não é raiz. Pegamos o nó pai do nó atual
    no_pai = encontrar_pagina(no->Pagina.pai, cs, t);

    while(no_pai->Pagina.filhos[i] != -1){
        numFilhos++;
        i++;
    }

    for(i=0;i<numFilhos;i++){
        if(no_pai->Pagina.filhos[i] == no->Pagina.RNN){
            for(j=no_pai->Pagina.numChaves-1;j>=i;j--){
                    strcpy(no_pai->Pagina.chaves[j+1], no_pai->Pagina.chaves[j]);
                }
            strcpy(no_pai->Pagina.chaves[i], chave);//Insere a chave promovida na posição correta dos valores do nó pai
            no_pai->Pagina.numChaves++;

            for(j=no_pai->Pagina.numChaves-1;j>=i+1;j--){
                    no_pai->Pagina.filhos[j+1] = no_pai->Pagina.filhos[j];
                }
            no_pai->Pagina.filhos[i+1] = novo_no->Pagina.RNN;//Insere o RNN do novo nó na posição correta dos filhos do nó pai
            numFilhos++;
            break;
        }
    }

    if(no_pai->Pagina.numChaves == ordem){// excedeu a capacidade máxima após a inserção
        RNN = (novo_no->Pagina.RNN) + 1;
        *no_interno = criar_no(RNN);
        no_interno->Pagina.eFolha = 'F';
        no_interno->Pagina.pai = no_pai->Pagina.pai;

        j = 0;
        mediana = (int)(ceil((double)no_pai->Pagina.numChaves / 2)) - 1;
        strcpy(chave_promovida, no_pai->Pagina.chaves[mediana]);
        strcpy(no_pai->Pagina.chaves[mediana], "NULL");
        for(i=mediana+1;i<no_pai->Pagina.numChaves;i++){
            strcpy(no_interno->Pagina.chaves[j], no_pai->Pagina.chaves[i]);
            strcpy(no_pai->Pagina.chaves[i], "NULL");
            j++;
        }
        no_pai->Pagina.numChaves = mediana;
        no_interno->Pagina.numChaves = j;

        j = 0;
        for(i=mediana+1;i<numFilhos;i++){
            no_interno->Pagina.filhos[j] = no_pai->Pagina.filhos[i];
            if(i != numFilhos - 1){
                no_pai->Pagina.filhos[i] = -1;
            }
            j++;
        }
        atualiza_arquivo_arvore(t,no);
        //atualiza_arquivo_arvore(t,no_pai);
        insere_arquivo_arvore(t,novo_no);
        //insere_arquivo_arvore(t,no_interno);
        i = 0;
        while(no_pai->Pagina.filhos[i] != -1){
            no_filho = encontrar_pagina(no_pai->Pagina.filhos[i], cs, t);
            no_filho->Pagina.pai = no_pai->Pagina.RNN;
            atualiza_arquivo_arvore(t, no_filho);
            i++;
        }
        i = 0;
        while(no_interno->Pagina.filhos[i] != -1){
            no_filho = encontrar_pagina(no_interno->Pagina.filhos[i], cs, t);
            no_filho->Pagina.pai = no_interno->Pagina.RNN;
            atualiza_arquivo_arvore(t, no_filho);
            i++;
        }
        insercao_no_pai(t, raiz, cs, no_pai, no_interno, chave_promovida);
    }
    else{
        atualiza_arquivo_arvore(t,no);
        atualiza_arquivo_arvore(t,no_pai);
        insere_arquivo_arvore(t,novo_no);
    }
}

void atualiza_arquivo_arvore(FILE *t, arvoreBplus *no){
    int i, RNN = no->Pagina.RNN;
    char estrutura[200];

    fseek(t, 0, SEEK_SET);

    if(no->Pagina.pai == -1){
      fprintf(t, "%d\n", RNN);
    }

    fseek(t, 0, SEEK_SET);

    for(i=0;i<RNN;i++){
        fgets(estrutura, 200, t);
    }

    fseek(t, 0, SEEK_CUR);

    fprintf(t, "%d|%c|%d|", no->Pagina.RNN, no->Pagina.eFolha, no->Pagina.numChaves);
    for(i=0;i<MAX;i++){
        if(i != MAX - 1){
            fprintf(t, "%s,", no->Pagina.chaves[i]);
        }
        else{
            fprintf(t, "%s|", no->Pagina.chaves[i]);
        }
    }
    for(i=0;i<MAX;i++){
        if(i != MAX - 1){
            fprintf(t, "%d,", no->Pagina.dataRRN[i]);
        }
        else{
            fprintf(t, "%d|", no->Pagina.dataRRN[i]);
        }
    }
    for(i=0;i<ordem;i++){
        if(i != ordem - 1){
            fprintf(t, "%d,", no->Pagina.filhos[i]);
        }
        else{
            fprintf(t, "%d|", no->Pagina.filhos[i]);
        }
    }
    fprintf(t, "%d|%d|", no->Pagina.pai, no->Pagina.prox_no);

    //fflush(t);
}

void insere_arquivo_arvore(FILE *t, arvoreBplus *no){
    atualiza_arquivo_arvore(t,no);
    fseek(t,0,SEEK_END);
    fprintf(t,"\n");
}

void inserir_ordenado_conjunto_sequencias(arvoreBplus **conjSequencias, arvoreBplus *folha){
    arvoreBplus *atual = NULL, *aux = (*conjSequencias), *antes = NULL;

    atual = folha;

    if(*conjSequencias == NULL){//se lista estiver vazia, se torna o primeiro elemento
        atual->prox_no = NULL;
        *conjSequencias = atual;
        return;
    }
    // Verifica se a chave já existe na lista, se existir, não insere duplicado
   
    //se a primeira chave vier antes da primeira chave do primeiro elemento, se torna o primeiro da lista
    if(strcmp(atual->Pagina.chaves[0], (*conjSequencias)->Pagina.chaves[0]) < 0){
        atual->prox_no = (*conjSequencias);
        *conjSequencias = atual;
        return;
    }
    //enquanto nao chegar no final e a primeira chave vier depois dos elementos da lista
    while(aux!=NULL && strcmp(atual->Pagina.chaves[0],aux->Pagina.chaves[0]) >= 0){
        if(strcmp(atual->Pagina.chaves[0],aux->Pagina.chaves[0]) == 0){
            return;
        }
        antes = aux;
        aux = aux->prox_no;
    }
    antes->prox_no = atual;
    atual->prox_no = aux;
    return;
}

int calcular_RRN(Lista_s **h){
    Lista_s *aux = *h;
    int rrn = 0;
    while(aux!=NULL){
        rrn++;
        aux = aux->prox;
    }
    return rrn;
}

void buscar_filme_usuario(FILE *d, arvoreBplus **raiz, arvoreBplus **cs, Lista_s *h){
    int opcao, RRN, byte_offset;
    char chave[6], titulo[50];
    arvoreBplus *no;

    FILE *t = fopen("ibtree.idx", "r");

    printf("\n1.Pela chave\n2.Pelo titulo do filme\n");
    printf("Selecione um meio: ");
    scanf("%d", &opcao);
    if(opcao == 1){
        printf("Digite a chave: ");
        scanf(" %[^\n]s", chave);
        no = buscar_na_arvore(chave, raiz, cs, t);
        RRN = buscar_chave(no, chave);
        if(RRN != -1){
            byte_offset = 192 * RRN;
            buscar_filme_arquivo(d, byte_offset);
        }
        else{
            printf("Chave informada nao existe.\n");
        }
    }
    else if(opcao == 2){
        //busca pelo titulo
        printf("Digite o titulo: ");
        scanf(" %[^\n]s", titulo);
        if(busca_indice_titulo(d, t, h, raiz, cs, titulo) == -1){
            printf("Nenhum filme foi encontrado com o titulo\n");
        }
    }
    else{
        printf("Opcao invalida.\n");
    }

    fclose(t);
}

void buscar_filme_arquivo(FILE *d, int byte_offset){
    char titulo_port[50], titulo_ori[50], diretor[50], pais[22], ano[5];
    int nota;
    fseek(d, byte_offset + 6, SEEK_SET);

    fscanf(d, "%49[^@]@%49[^@]@%49[^@]@%4[^@]@%21[^@]@%d", titulo_port, titulo_ori, diretor, ano, pais, &nota);

    //formatar os dados
    formatar(titulo_port, titulo_ori, diretor, ano,pais, nota);
}

void formatar(char *titulo_port, char *titulo_ori, char *diretor, char *ano, char *pais, int nota){
    char nome[15], sobrenome[35];
    int i, posicao;
    printf("\n-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
    printf("Titulo: %s\n", titulo_port);
    if(strcmp(titulo_ori,"Idem") != 0){
        printf("Titulo Original: %s\n", titulo_ori);
    }
    //formatar nome do diretor
    for(i=0;i<strlen(diretor);i++){
        if(diretor[i] == ','){
            posicao = i;
            break;
        }
        sobrenome[i] = diretor[i];
    }
    sobrenome[i] = '\0';
    strcpy(nome, &diretor[posicao + 2]);
    
    printf("Diretor: %s %s\n", nome, sobrenome);
    printf("Ano de lancamento: %s\n", ano);
    printf("Pais: %s\n", pais);
    printf("Nota: %d\n", nota);
    printf("\n-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");
}

int busca_indice_titulo(FILE *d, FILE *t, Lista_s *h, arvoreBplus **raiz, arvoreBplus **cs, char* titulo){
    Lista_s *aux = h;
    int RRN, flag = 1;
    arvoreBplus *no;

    while(aux != NULL){
        if(strcmp(aux->titulo,titulo) == 0){
            no = buscar_na_arvore(aux->chave_primaria, raiz, cs, t);
            RRN = buscar_chave(no, aux->chave_primaria);
            if(RRN != -1){
                flag = 0;
                buscar_filme_arquivo(d, RRN*192);
            }
        }
        aux = aux->prox;
    }

    if(flag == 1){
        return -1;
    }
    return 0;
}

void modificar_nota_filme(arvoreBplus **raiz, arvoreBplus **cs){
    char chave[6], titulo_port[50], titulo_ori[50], diretor[50], pais[22], ano[5];
    int nota_antiga;
    int RRN, byte_offset, nota;
    arvoreBplus *no;
    FILE *t = fopen("ibtree.idx", "r");
    FILE *d = fopen("movies.dat", "r+");

    printf("Digite a chave do filme: ");
    scanf(" %[^\n]s", chave);
    no = buscar_na_arvore(chave, raiz, cs, t);
    RRN = buscar_chave(no, chave);
    if(RRN != -1){
        do{
            printf("Digite a nova nota (entre 0 a 9): ");
            scanf("%d", &nota);
        }while(nota < 0 || nota > 9);

        if(d != NULL){
            byte_offset = 192 * RRN;
            fseek(d, byte_offset, SEEK_SET);
            fscanf(d, "%5[^@]@%49[^@]@%49[^@]@%49[^@]@%4[^@]@%21[^@]@%d@", chave, titulo_port, titulo_ori, diretor, ano, pais, &nota_antiga);
            fseek(d, byte_offset, SEEK_SET);
            fprintf(d, "%s@%s@%s@%s@%s@%s@%d@", chave, titulo_port, titulo_ori, diretor, ano, pais, nota);

            printf("Nota foi corrigida.\n");
            fflush(d);
            fclose(d);
        }
        else{
            printf("Erro ao abrir o arquivo.\n");
        }
    }
    else{
        printf("Chave informada nao existe.\n");
    }
    fclose(t);
}

void conjunto_de_sequencias(FILE *t, arvoreBplus **cs){
    arvoreBplus *aux = (*cs), *no, *proximo_no, *anterior;
    int RNN;

    if(aux == NULL || aux->Pagina.RNN != 1){
            no = encontrar_pagina(1, cs, t);
            inserir_ordenado_conjunto_sequencias(cs,no);
        }
        aux = (*cs);
        while(aux != NULL){
            proximo_no = aux->prox_no;
            while(proximo_no && aux->Pagina.prox_no != proximo_no->Pagina.RNN){
                RNN = proximo_no->Pagina.RNN;
                proximo_no = encontrar_pagina(RNN, cs, t);
                inserir_ordenado_conjunto_sequencias(cs,proximo_no);

                proximo_no = proximo_no->prox_no;
            }
            anterior = aux;
            aux = proximo_no;
        }
        
        while(anterior && anterior->Pagina.prox_no != -1){
            RNN = anterior->Pagina.prox_no;
            anterior = encontrar_pagina(RNN, cs, t);
            inserir_ordenado_conjunto_sequencias(cs,anterior);
        }
}

void listar_filmes(FILE *d, arvoreBplus *raiz, arvoreBplus **cs){
    arvoreBplus *aux = (*cs);
    int i, RRN;
    FILE *t = fopen("ibtree.idx", "r");
    if(raiz->Pagina.numChaves != 0){
        conjunto_de_sequencias(t,cs);
        aux = (*cs); 
        while(aux != NULL){
            for(i=0;i<aux->Pagina.numChaves;i++){
                RRN = aux->Pagina.dataRRN[i];
                buscar_filme_arquivo(d, RRN*192);
            }
            aux = aux->prox_no;
        }
    }
    else{
        printf("\nNao ha cadastro de nenhum filme.");
    }

    fclose(t);
}

void listar_filmes_intervalo(FILE *d, arvoreBplus *raiz, arvoreBplus **cs, char *chave){
    FILE *t = fopen("ibtree.idx", "r");
    arvoreBplus *no = buscar_na_arvore(chave,&raiz,cs,t), *aux = (*cs);
    int i, RRN;

    if(raiz->Pagina.numChaves != 0){
        conjunto_de_sequencias(t,cs);
        aux = (*cs); 
        while(aux != NULL){
            for(i=0;i<aux->Pagina.numChaves;i++){
               if(strcmp(aux->Pagina.chaves[i],chave) >= 0){
                    RRN = aux->Pagina.dataRRN[i];
                    buscar_filme_arquivo(d, RRN*192);
                }
            }
            aux = aux->prox_no;
        }
    }
    else{
        printf("\nNao ha cadastro de nenhum filme.");
    }

    fclose(t);
}

void encerrar_execucao(FILE *d, arvoreBplus *raiz, arvoreBplus *cs, Lista_s *h){
    FILE *s = fopen("ititle.idx", "a+");
    Lista_s *aux = h;

    while(aux != NULL){
        fprintf(s,"%s#%s\n", aux->titulo, aux->chave_primaria);
        aux = aux->prox;
    }

    fflush(s);
    fclose(s);
    free(raiz);
    free(cs);
    free(h);
    fclose(d);
}