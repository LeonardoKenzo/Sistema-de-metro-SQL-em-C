#include "headerRegister.h"

struct headerRegister{
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacoes;
};

Header *CreateHeaderRegister(){
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

void header_write_to_file(FILE *fp, Header *h){
    fwrite(&h->status, sizeof(char), 1, fp);
    fwrite(&h->topo, sizeof(int), 1, fp);
    fwrite(&h->proxRRN, sizeof(int), 1, fp);
    fwrite(&h->nroEstacoes, sizeof(int), 1, fp);
    fwrite(&h->nroParesEstacoes, sizeof(int), 1, fp);
}

void FreeHeaderRegister(Header **hRegister){
    free((*hRegister));
    (*hRegister) = NULL;
}