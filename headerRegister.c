#include "headerRegister.h"
#include "dataRegister.h"

struct headerRegister{
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacoes;
};

struct Pares{
    int codEstacao;
    int proxCodEstacao;
};

Header *create_header_register(){
    Header *hRegister = (Header *)calloc(1, sizeof(Header));
    if(!hRegister){
        printf("Erro: alocar memória (Header Register)!\n");
        return NULL;
    }

    hRegister->status = '0';
    hRegister->topo = -1;
    hRegister->proxRRN = 0;
    hRegister->nroEstacoes = 0;
    hRegister->nroParesEstacoes = 0;

    return hRegister;
}

void header_set_status(Header *h, char status){
    h->status = status;
}

void header_set_topo(Header *h, int topo){
    h->topo = topo;
}

void header_set_proxRRN(Header *h, int proxRRN){
    h->proxRRN = proxRRN;
}

void header_set_nroEstacoes(Header *h, int nroEstacoes){
    h->nroEstacoes = nroEstacoes;
}

void header_set_nroParesEst(Header *h, int nroParesEst){
    h->nroParesEstacoes = nroParesEst;
}

int header_get_topo(Header *h){
    return h->topo;
}

int header_get_proxRRN(Header *h) {
    return h->proxRRN;
}

int header_get_nroEstacoes(Header *h) {
    return h->nroEstacoes;
}

int header_get_nroParesEst(Header *h) {
    return h->nroParesEstacoes;
}

void header_write_to_file(FILE *fp, Header *h){
    fseek(fp, 0, SEEK_SET);
    fwrite(&h->status, sizeof(char), 1, fp);
    fwrite(&h->topo, sizeof(int), 1, fp);
    fwrite(&h->proxRRN, sizeof(int), 1, fp);
    fwrite(&h->nroEstacoes, sizeof(int), 1, fp);
    fwrite(&h->nroParesEstacoes, sizeof(int), 1, fp);
}

void header_read_from_file(FILE *fp, Header *h){
    fseek(fp, 0, SEEK_SET);
    fread(&h->status, sizeof(char), 1, fp);
    fread(&h->topo, sizeof(int), 1, fp);
    fread(&h->proxRRN, sizeof(int), 1, fp);
    fread(&h->nroEstacoes, sizeof(int), 1, fp);
    fread(&h->nroParesEstacoes, sizeof(int), 1, fp);
}

void free_header_register(Header **hRegister){
    free((*hRegister));
    (*hRegister) = NULL;
}

char **criar_lista_nomesEstacoes(int nroMaxEstacoes){
    char **lista_nomesEstacoes = (char **)calloc(nroMaxEstacoes, sizeof(char *));
    if(lista_nomesEstacoes == NULL){
        printf("Erro: alocação de memória lista de estações!\n");
        return NULL; 
    }
    return lista_nomesEstacoes;
}

bool tem_repetidos_nomesEstacoes(char **lista_nomesEstacoes, int nroEstacoes, char *nomeEst){
    
    // Percorre a lista comparando se possui nomes repetidos
    for(int i = 0; i < nroEstacoes; i++){
        if(lista_nomesEstacoes[i] != NULL)
        if((strcmp(lista_nomesEstacoes[i], nomeEst) == 0))
            return true;
    }
    return false;
}

void adicionar_nomesEstacoes(char **lista_nomesEstacoes, int nroEstacoes, char *nomeEst){

    int tamanho = strlen(nomeEst);
    lista_nomesEstacoes[nroEstacoes] = (char *)calloc(tamanho + 1, sizeof(char));
    strcpy(lista_nomesEstacoes[nroEstacoes], nomeEst);
}

void aumentar_capacidade_nomesEstacoes(char ***lista_nomesEstacoes, int nroEstacoes, int nroMaxEstacoes){

    char **lista_temporaria = realloc(*lista_nomesEstacoes, (nroMaxEstacoes) * sizeof(char *));
    for (int i = nroEstacoes; i < nroMaxEstacoes; i++) {
        lista_temporaria[i] = NULL;
    }

    *lista_nomesEstacoes = lista_temporaria;
}

void free_lista_nomesEstacoes(char ***lista, int nroEstacoes){
    for(int i = 0; i < nroEstacoes; i++){
        free((*lista)[i]);
    }
    free((*lista));
    *lista = NULL;
}
    
paresEstacoes *criar_lista_paresEstacoes(int nroMaxPares){
    paresEstacoes *lista_pares = (paresEstacoes *)calloc(nroMaxPares, sizeof(paresEstacoes));
    if(lista_pares == NULL){
        printf("Erro: alocação de memória lista de pares de estações!\n");
        return NULL;
    }
    for(int i = 0; i < nroMaxPares; i++){
        lista_pares[i].codEstacao = -1;
        lista_pares[i].proxCodEstacao = -1;
    }
    return lista_pares;
}

bool tem_repetido_paresEstacoes(paresEstacoes *lista_paresEstacoes, int nroPares, int codEst, int codProx){

    // Percorre a lista comparando se possui pares repetidos
    for(int i = 0; i < nroPares; i++){
        if(codProx != -1)
            if((lista_paresEstacoes[i].codEstacao == codEst && lista_paresEstacoes[i].proxCodEstacao == codProx) ||
                (lista_paresEstacoes[i].codEstacao == codProx && lista_paresEstacoes[i].proxCodEstacao == codEst))
                return true;
    }
    return false;
}

void adicionar_paresEstacoes(paresEstacoes *lista_paresEstacoes, int nroPares, int codEst, int codProx){
    if(codProx != -1){
        lista_paresEstacoes[nroPares].codEstacao = codEst;
        lista_paresEstacoes[nroPares].proxCodEstacao = codProx;
    }
}

void aumentar_capacidade_paresEstacoes(paresEstacoes **lista_paresEstacoes, int nroPares, int nroMaxPares){
    paresEstacoes *temp = realloc(*lista_paresEstacoes, nroMaxPares * sizeof(paresEstacoes));
    for (int i = nroPares; i < nroMaxPares; i++) {
        temp[i].codEstacao = -1;
        temp[i].proxCodEstacao = -1;
    }

    *lista_paresEstacoes = temp;
}

void free_lista_paresEstacoes(paresEstacoes **lista){
    free((*lista));
    *lista = NULL;
}

void achar_todos_estacoes(FILE *bin, paresEstacoes **lista_paresEstacoes, int *nroParesEncontrados, int *nroMaxPares, char ***lista_nomesEstacoes, int *nroNomesEncontrados, int *nroMaxNomes){
    fseek(bin, 17, SEEK_SET);

    Record *r;
    while((r = record_read_from_file(bin)) != NULL){

         if(record_get_removido(r) == '1'){
            free_record(&r);
            continue;
        }

        int codEst, codProx;
        char *nome = record_get_nomeEstacao(r);
        codEst = record_get_codEstacao(r);
        codProx = record_get_codProxEstacao(r);

        if(*nroNomesEncontrados >= *nroMaxNomes){
            *nroMaxNomes *= 2;
            aumentar_capacidade_nomesEstacoes(lista_nomesEstacoes, *nroNomesEncontrados, *nroMaxNomes);
        }
        if(*nroParesEncontrados >= *nroMaxPares){
            *nroMaxPares *= 2;
            aumentar_capacidade_paresEstacoes(lista_paresEstacoes, *nroParesEncontrados, *nroMaxPares);
        }

        if(tem_repetidos_nomesEstacoes(*lista_nomesEstacoes, *nroNomesEncontrados, nome) == false){
            adicionar_nomesEstacoes(*lista_nomesEstacoes, *nroNomesEncontrados, nome);
            (*nroNomesEncontrados) += 1;
        }
        if(tem_repetido_paresEstacoes(*lista_paresEstacoes, *nroParesEncontrados, codEst, codProx) == false && codProx != -1){
            adicionar_paresEstacoes(*lista_paresEstacoes, *nroParesEncontrados, codEst, codProx);
            (*nroParesEncontrados) += 1;
        }
        
        free_record(&r);
    }
}