#include "dataRegister.h"

struct dataRegister{
    char removido;
    int proximo;
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;
    char *nomeEstacao;
    int tamNomeLinha;
    char *nomeLinha;
};

DATA_REGISTER *CreateDataRegister(int codEstacao, int codLinha, int codProxEstacao, int distProxEstacao, int codEstIntegra, int codLinhaIntegra, 
    int tamNomeEstacao, int tamNomeLinha){
    DATA_REGISTER *dRegister = (DATA_REGISTER *)calloc(1, sizeof(DATA_REGISTER));
    if(!dRegister){
        printf("Erro: alocar memória (Data Register)!\n");
        return NULL;
    }

    // Inicializa todos os campos
    dRegister->removido = '0';
    dRegister->proximo = -1;
    dRegister->codEstacao = codEstacao;
    dRegister->codLinha = codLinha;
    dRegister->codProxEstacao = codProxEstacao;
    dRegister->distProxEstacao = distProxEstacao;
    dRegister->codEstIntegra = codEstIntegra;
    dRegister->codLinhaIntegra = codLinhaIntegra;
    dRegister->tamNomeEstacao = tamNomeEstacao;
    dRegister->tamNomeLinha = tamNomeLinha;

    return dRegister;
}

int GetNomeEstacao(DATA_REGISTER *dRegister){
    return dRegister->nomeEstacao;
}

int GetCodEstacao(DATA_REGISTER *dRegister){
    return dRegister->codEstacao;
}

int GetCodProxEstacao(DATA_REGISTER *dRegister){
    return dRegister->codProxEstacao;
}

void FreeDataRegister(DATA_REGISTER **dRegister){
    free((*dRegister));
    (*dRegister) = NULL;
}