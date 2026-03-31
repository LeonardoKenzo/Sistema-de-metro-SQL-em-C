#ifndef Record_H
    #define Record_H
    #include <stdio.h>
    #include <stdlib.h>

    typedef struct dataRegister Record;

    Record *CreateRecord();
    void FreeRecord(Record *r);

    // setters
    void record_set_removido(Record *r, char removido);
    void record_set_proximo(Record *r, int proximo);

    void record_set_codEstacao(Record *r, int cod);
    void record_set_codLinha(Record *r, int cod);
    void record_set_codProxEstacao(Record *r, int cod);
    void record_set_distProxEstacao(Record *r, int dist);
    void record_set_codLinhaIntegra(Record *r, int cod);
    void record_set_codEstIntegra(Record *r, int cod);

    // Setters para campos de tamanho variável
    void record_set_nomeEstacao(Record *r, char *nome);
    void record_set_nomeLinha(Record *r, char *nome);

    char* GetNomeEstacao(Record *r);
    int GetCodEstacao(Record *r);
    int GetCodProxEstacao(Record *r);

    // Escrita campo a campo com preenchimento de lixo ($)
    void record_write_to_file(FILE *fp, Record *r);
#endif