#include "headerRegister.h"

struct headerRegister{
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacoes;
};

HEADER_REGISTER *CreateHeaderRegister(){
    HEADER_REGISTER *hRegister = (HEADER_REGISTER *)calloc(1, sizeof(HEADER_REGISTER));
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

void FreeHeaderRegister(HEADER_REGISTER **hRegister){
    free((*hRegister));
    (*hRegister) = NULL;
}