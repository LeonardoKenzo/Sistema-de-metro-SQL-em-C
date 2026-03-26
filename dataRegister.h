#ifndef DATA_REGISTER_H
    #define DATA_REGISTER_H
    #include <stdio.h>
    #include <stdlib.h>

    typedef struct dataRegister DATA_REGISTER;

    DATA_REGISTER *CreateDataRegister(int codEstacao, int codLinha, int codProxEstacao, int distProxEstacao, int codEstIntegra, int codLinhaIntegra, int tamNomeEstacao, int tamNomeLinha);
    int GetNomeEstacao(DATA_REGISTER *dRegister);
    int GetCodEstacao(DATA_REGISTER *dRegister);
    int GetCodProxEstacao(DATA_REGISTER *dRegister);
    void FreeDataRegister(DATA_REGISTER **dRegister);

#endif