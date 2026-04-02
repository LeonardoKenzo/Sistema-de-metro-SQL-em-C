#include "headerRegister.h"

struct headerRegister{
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacoes;
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