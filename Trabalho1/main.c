#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0

int indices_inconsistentes = FALSE;//variavel global para alterar a flag dos arquivos de indice caso
//tenha alguma modificacao nos indices

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

//estrutura de dados para o indice primario
typedef struct Lista_p{
    char chave_primaria[6];
    int RRN;//numero de registros que antecedem cada registro
    struct Lista_p *prox;
}Lista_p;

//estrutura de dados para o indice secundario
typedef struct Lista_s{
    char titulo[50];
    char chave_primaria[6];
    struct Lista_s *prox;
}Lista_s;

// Protótipos das funções
void criar_lista(Lista_p **h_p, Lista_s **h_s);

FILE *criar_arquivo_dados(Lista_p **h_p, Lista_s **h_s);

void criar_arquivo_primario(FILE *d, Lista_p **h);

void criar_arquivo_secundario(FILE *d, Lista_s **h);

int calcular_rrn(Lista_p **h);

void inserir_ordenado_primario(Lista_p **h, char *chave, int RRN);

void inserir_ordenado_secundario(Lista_s **h, char *chave, char *titulo);

void inserir_filme_usuario(FILE *d, Lista_p **h_p, Lista_s **h_s);

void remover_filme_usuario(char *chave, int byte_offset, Lista_p **h_p, Lista_s **h_s);

int remover_primario(Lista_p **h, char *chave);

int remover_secundario(Lista_s **h, char *chave);

int buscar_indice_chave(Lista_p *h, char* chave);

int busca_indice_titulo(FILE *d, Lista_s *h_s, Lista_p *h_p, char* titulo);

void buscar_filme(FILE *d, int byte_offset);

void formatar(char *titulo_port, char *titulo_ori, char *diretor, char *ano, char *pais, int nota);

void modificar_nota(int byte_offset, int nota);

void listar_filmes(FILE *d);

void finalizar_execucao(FILE *d, Lista_p *h_p, Lista_s *h_s);

void reconstruir_indices_primario(Lista_p **h, FILE *d);

void reconstruir_indices_secundario(Lista_s **h, FILE *d);

void desalocar_memoria(Lista_p *h_p, Lista_s *h_s);

//funcao principal
int main(){
    //inicializa com um ponteiro NULL para o primeiro elemento das listas
    Lista_p *h_p = NULL;
    Lista_s *h_s = NULL;
    //cria o arquivo de dados, caso nao exista, se nao, abre para leitura e escrita e retorna o ponteiro
    //para o arquivo
    FILE *d = criar_arquivo_dados(&h_p,&h_s);
    int opcao;
    int opcao2;
    int RRN, byte_offset, nota;
    char chave[6], titulo[50];

    do{
        printf("\n1.Insercao de filme\n2.Remocao de referencia\n3.Busca de filme\n4.Modificar a nota de um filme\n5.Listar todos os filmes no catalogo\n6.Finalizar a execucao.\n");
        printf("Selecione uma opcao: ");
        scanf("%d", &opcao);
        switch(opcao){
            case 1: inserir_filme_usuario(d,&h_p,&h_s); break;
            case 2: printf("Digite a chave: ");
            scanf(" %[^\n]s", chave);
            RRN = buscar_indice_chave(h_p,chave);
            if(RRN != -1){
                byte_offset = RRN * 192;
                remover_filme_usuario(chave,byte_offset,&h_p,&h_s);
            }
            else{
                printf("Chave informada não existe.\n");
            }
            break;
            case 3: printf("\n1.Pela chave\n2.Pelo titulo do filme\n");
            printf("Selecione um meio: ");
            scanf("%d", &opcao2);
            if(opcao2 == 1){
                //busca pela chave
                printf("Digite a chave: ");
                scanf(" %[^\n]s", chave);
                RRN = buscar_indice_chave(h_p,chave);
                if(RRN != -1){
                    byte_offset = 192 * RRN;
                    buscar_filme(d, byte_offset);
                }
                else{
                    printf("Chave informada não existe.\n");
                }
            }
            else if(opcao2 == 2){
                //busca pelo titulo
                printf("Digite o titulo: ");
                scanf(" %[^\n]s", titulo);
                if(busca_indice_titulo(d,h_s,h_p,titulo) == -1){
                    printf("Nenhum filme foi encontrado com o título");
                }
            }
            else{
                printf("Invalido.");
            } break;
            case 4: printf("Digite a chave do filme: ");
            scanf(" %[^\n]s", chave);
            do{
                printf("Digite a nova nota (entre 0 a 9): ");
                scanf("%d", &nota);
            }while(nota < 0 || nota > 9);
            RRN = buscar_indice_chave(h_p,chave);
            if(RRN != -1){
                byte_offset = 192 * RRN;
                //funcao para modificar nota
                modificar_nota(byte_offset,nota);
            }
            else{
                printf("Chave informada não existe.\n");
            }
            break;
            case 5: listar_filmes(d); break;
            case 6: finalizar_execucao(d,h_p,h_s); break;
            default: printf("\nSelecione uma opcao valida.\n"); break;
        }
    }while(opcao != 6);

    return 0;
}

//funcao para criar lista de indices (nao usada)
void criar_lista(Lista_p **h_p, Lista_s **h_s){
    *h_p = malloc(sizeof(Lista_p));
    *h_s = malloc(sizeof(Lista_s));
    (*h_p)->RRN = 0;
    (*h_p)->prox = NULL;
    (*h_s)->prox = NULL;
}

//funcao para criar arquivos de dados, se existir, abre para leitura e escrita
FILE *criar_arquivo_dados(Lista_p **h_p, Lista_s **h_s){
    FILE *d;
    //Caso exista o arquivo de dados, entra na condicao
    if((d = fopen("movies.dat", "r")) != NULL){
        d = fopen("movies.dat", "a+");
        //chama as funcoes para carregar os indices dos filmes na memoria
        criar_arquivo_primario(d,h_p);
        criar_arquivo_secundario(d,h_s);
    }
    else{
        d = fopen("movies.dat", "a+");
    }
  return d;
}

//funcao para carregar os indices do arquivo de indice primario na memoria RAM
void criar_arquivo_primario(FILE *d, Lista_p **h){
    FILE *p = fopen("primary.idx", "r");
    int flag, RRN;
    char chave[6];

    //entra na conficao se o arquivo existir
    if(p != NULL){
            p = fopen("primary.idx", "a+");

            //le a flag do arquivo
            fscanf(p, "%d\n", &flag);

            //entra na condicao se a flag for 0 e o arquivo estiver consistente
            if(flag == 0){
                //enquanto o arquivo tiver os campos le a chave e o RRN e insere o registro na lista
                while(fscanf(p, "%[^#]#%d\n", chave, &RRN) == 2){
                inserir_ordenado_primario(h, chave, RRN);
                }
            
            }
            //entra na condicao se a flag for 1
            else if(flag == 1){
                //refaz os indices primarios na RAM e muda a flag para arquivo consistente
                reconstruir_indices_primario(h,d);
                indices_inconsistentes = FALSE;
            }
    }
    //se o arquivo nao existir cria os indices na RAM
    else{
        reconstruir_indices_primario(h,d);
    }
      fclose(p);
}

//funcao para carregar os indices do arquivo de indice secundario na memoria RAM
void criar_arquivo_secundario(FILE *d, Lista_s **h){
    FILE *s = fopen("ititle.idx", "r");
    int flag;
    char chave[6], titulo[100];

    //entra na condicao se o arquivo existir
    if(s != NULL){
            s = fopen("ititle.idx", "a+");
            //le a flag do arquivo
            fscanf(s, "%d\n", &flag);
            //entra na condicao se a flag for 0 e o arquivo estiver consistente
            if(flag == 0){
                //enquanto o arquivo tiver os campos le a chave e o titulo e insere o registro na lista
                while(fscanf(s, "%[^#]#%[^\n]\n", titulo, chave) == 2){
                    inserir_ordenado_secundario(h,chave,titulo);
                }
            }
            else{
                //refaz os indices secundarios na RAM e muda a flag para arquivo consistente
                reconstruir_indices_secundario(h,d);
                indices_inconsistentes = FALSE;
            }
    }
    //se o arquivo nao existir cria os indices na RAM
    else{
        reconstruir_indices_secundario(h,d);
    }
      fclose(s);
  //caso o arquivo de dados esteja criado e o arquivo de indice nao, sera necessario ler o arquivo de dados e criar os indices na memoria RAM
}

//funcao para reconstruir ou criar os indices primarios a partir do arquivo de dados
void reconstruir_indices_primario(Lista_p **h, FILE *d){
    char chave[6];
    int i = 0;

    //posiciona o ponteiro para o comeco do arquivo
    fseek(d, 0, SEEK_SET);
    //enquanto tiver linhas no arquivo recupera 5 bytes
    while(fgets(chave, sizeof(chave), d)){
        chave[strlen(chave)] = '\0';//adiciono o fim da string

        //condicao para checar se o registro foi removido, caso contrario, insere na lista
        if(chave[0] != '|' && chave[1] != '*'){
            inserir_ordenado_primario(h,chave,i);
        }

        i++;

        fseek(d, i * 192, SEEK_SET);//posiciona o ponteiro para o proximo registro
    }
}

//funcao para reconstruir ou criar os indices secundarios na RAM
void reconstruir_indices_secundario(Lista_s **h, FILE *d){
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

//funcao para contar a quantidade de indices primarios procedentes
int calcular_rrn(Lista_p **h){
    Lista_p *aux = *h;
    int rrn = 0;
    while(aux!=NULL){
        rrn++;
        aux = aux->prox;
    }
    return rrn;
}

//funcao para inserir na lista de indice primario ordenando pela chave
void inserir_ordenado_primario(Lista_p **h, char *chave, int RRN){
    Lista_p *atual = malloc(sizeof(Lista_p)), *aux = *h, *antes = NULL;

    strcpy(atual->chave_primaria,chave);
    atual->RRN = RRN;
    atual-> prox = NULL;

    if(*h == NULL){//se a lista estiver vazia se torna o primeiro elemento
        atual->prox = NULL;
        *h = atual;
        return;
    }

    //se o indice vier antes do primeiro elemento, ele se torna o primeiro elemento
    if(strcmp(atual->chave_primaria,(*h)->chave_primaria) < 0){
        atual->prox = (*h);
        *h = atual;
        return;
    }
    //enquanto nao chegar ao final da lista e o indice vier depois dos elementos da lista
    while(aux!=NULL && strcmp(atual->chave_primaria,aux->chave_primaria) > 0){
        antes = aux;
        aux = aux->prox;
    }
    antes->prox = atual;
    atual->prox = aux;
    return;
}

//funcao para inserir indice secundarios na lista ordenados pelo titulo
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

//funcao para o usuario inserir filme
void inserir_filme_usuario(FILE *d, Lista_p **h_p, Lista_s **h_s){
    registro filme;
    int i, RRN;
    int posicao_sobrenome;
    int bytes_campos = 17; //campos fixos e delimitadores
    char codigo[6];
    char primeiro_nome[30];

    printf("\nDigite o titulo do filme em portugues: ");
    scanf(" %[^\n]s", filme.titulo_port);
    printf("Digite o titulo original: ");
    scanf(" %[^\n]s", filme.titulo_original);
    printf("Digite o diretor: ");
    scanf(" %[^\n]s", filme.diretor);
    printf("Digite o ano de lancamento: ");
    scanf(" %[^\n]s", filme.ano);
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

    //atualiza os índices
    RRN = calcular_rrn(h_p);
    inserir_ordenado_primario(h_p,filme.codigo, RRN);
    inserir_ordenado_secundario(h_s,filme.codigo,filme.titulo_port);
    //fez uma modificacao dos indices na ram, deve alterar a flag de mudanca
    if(indices_inconsistentes == FALSE){
        indices_inconsistentes = TRUE;
    }
}

//funcao para o usuario remover o filme
void remover_filme_usuario(char *chave, int byte_offset, Lista_p **h_p, Lista_s **h_s){
    FILE *d = fopen("movies.dat", "r+");

    if(d != NULL){
        fseek(d, byte_offset, SEEK_SET);
        fprintf(d, "|*");
        fclose(d);

        //remover dos arquivos de indice
        if (remover_primario(h_p,chave) && remover_secundario(h_s,chave)){
            printf("Filme removido");
        }
        if(indices_inconsistentes == FALSE){
            indices_inconsistentes = TRUE;
        }
    }
    else{
        printf("Erro ao abrir arquivo");
    }
}

int remover_primario(Lista_p **h, char *chave){
    if(*h == NULL){
        return 0;
    }

    Lista_p *aux = *h;
    if(strcmp((*h)->chave_primaria,chave) == 0){
        *h = (*h)->prox;
        free(aux);
        return 1;
    }

    Lista_p *antes;
    while(aux != NULL && strcmp(aux->chave_primaria,chave) != 0){
        antes = aux;
        aux = aux->prox;
    }

    if(aux != NULL){
        antes->prox = aux->prox;
        free(aux);
        return 1;
    }
    return 0;
}

int remover_secundario(Lista_s **h, char *chave){
    if(*h == NULL){
        return 0;
    }

    Lista_s *aux = *h;
    if(strcmp((*h)->chave_primaria,chave) == 0){
        *h = (*h)->prox;
        free(aux);
        return 1;
    }

    Lista_s *antes;
    while(aux != NULL && strcmp(aux->chave_primaria,chave) != 0){
        antes = aux;
        aux = aux->prox;
    }

    if(aux != NULL){
        antes->prox = aux->prox;
        free(aux);
        return 1;
    }
    return 0;
}

//funcao para buscar indice primario pela chave e retornar o RRN
int buscar_indice_chave(Lista_p *h, char* chave){
    Lista_p *aux = h;

    while(aux != NULL && strcmp(aux->chave_primaria,chave) != 0){
        aux = aux->prox;
    }

    if(aux != NULL){
        return aux->RRN;
    }
    else{
        return -1;
    }
}

//funcao para buscar indice secundario pelo titulo e retornar a chave primaria
int busca_indice_titulo(FILE *d, Lista_s *h_s, Lista_p *h_p, char* titulo){
    Lista_s *aux = h_s;
    int RRN, flag = 1;

    while(aux != NULL){
        if(strcmp(aux->titulo,titulo) == 0){
            flag = 0;
            RRN = buscar_indice_chave(h_p,aux->chave_primaria);
            buscar_filme(d, RRN*192);
        }
        aux = aux->prox;
    }

    if(flag == 1){
        return -1;
    }
    return 0;
}

//funcao para buscar filme pela sua posicao no arquivo
void buscar_filme(FILE *d, int byte_offset){
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

//funcao para modificar a nota de um filme
void modificar_nota(int byte_offset, int nota){
    char chave[6], titulo_port[50], titulo_ori[50], diretor[50], pais[22], ano[5];
    int nota_antiga;
    FILE *d = fopen("movies.dat", "r+");

    if(d != NULL){
        fseek(d, byte_offset, SEEK_SET);
        fscanf(d, "%5[^@]@%49[^@]@%49[^@]@%49[^@]@%4[^@]@%21[^@]@%d@", chave, titulo_port, titulo_ori, diretor, ano, pais, &nota_antiga);
        fseek(d, byte_offset, SEEK_SET);
        fprintf(d, "%s@%s@%s@%s@%s@%s@%d@", chave, titulo_port, titulo_ori, diretor, ano, pais, nota);

        fflush(d);
        fclose(d);
    }
    else{
        printf("Erro ao abrir o arquivo");
    }
}

//funcao para imprimir todos os filmes do arquivo
void listar_filmes(FILE *d){
    char chave[6], titulo_port[50], titulo_ori[50], diretor[50], pais[22], ano[5];
    int nota, i = 0;
    fseek(d, 0, SEEK_SET);

    while(fscanf(d, "%5[^@]@%49[^@]@%49[^@]@%49[^@]@%4[^@]@%21[^@]@%d", chave, titulo_port, titulo_ori, diretor, ano, pais, &nota) == 7){
        if(chave[0] != '|' && chave[1] != '*'){
            formatar(titulo_port, titulo_ori, diretor, ano,pais, nota);
        }
        i++;
        fseek(d, i * 192, SEEK_SET);
    }
}

//funcao para finalizar a execucao, gravar as listas de indices nos arquivos e desalocar a memoria
void finalizar_execucao(FILE *d, Lista_p *h_p, Lista_s *h_s){
    Lista_p *aux = h_p;
    Lista_s *aux2 = h_s;
    FILE *p;
    FILE *s;
    //abrir os arquivos de indice e atualizar os indices
    p = fopen("primary.idx","w");
    s = fopen("ititle.idx","w");

    if(aux != NULL){
        if(indices_inconsistentes == TRUE){
            fprintf(p, "%d\n", 1);
        }
        else{
            fprintf(p, "%d\n", 0);
        }
    }
    while(aux != NULL){
        fprintf(p,"%s#%d\n", aux->chave_primaria, aux->RRN);
        aux = aux->prox;
    }

    if(aux2 != NULL){
        if(indices_inconsistentes == TRUE){
            fprintf(s, "%d\n", 1);
        }
        else{
            fprintf(s, "%d\n", 0);
        }
    }
    while(aux2 != NULL){
        fprintf(s,"%s#%s\n", aux2->titulo, aux2->chave_primaria);
        aux2 = aux2->prox;
    }

    desalocar_memoria(h_p,h_s);

    //fechar os arquivos
    fclose(d);
    fclose(p);
    fclose(s);
}

void desalocar_memoria(Lista_p *h_p, Lista_s *h_s){
    Lista_p *proximo;
    Lista_s *proximo2;

    while(h_p != NULL){
        proximo = h_p->prox;
        free(h_p);
        h_p = proximo;
    }

    while(h_s != NULL){
        proximo2 = h_s->prox;
        free(h_s);
        h_s = proximo2;
    }
}