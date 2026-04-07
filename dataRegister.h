#ifndef Record_H
    #define Record_H
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    typedef struct dataRegister Record;

    Record *create_record();
    void free_record(Record **r);

    // Setters
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

    // Getters
    char record_get_removido(Record *r);
    int record_get_proximo(Record *r);
    int record_get_codEstacao(Record *r);
    int record_get_codLinha(Record *r);
    int record_get_codProxEstacao(Record *r);
    int record_get_distProxEstacao(Record *r);
    int record_get_codLinhaIntegra(Record *r);
    int record_get_codEstIntegra(Record *r);

    char* record_get_nomeEstacao(Record *r);
    char* record_get_nomeLinha(Record *r);

    Record* record_read_from_file(FILE *fp);
    Record* record_read_from_file_at_offset(FILE *fp, int offset);

    // Escrita campo a campo com preenchimento de lixo ($)
    void record_write_to_file(FILE *fp, Record *r);
    
#endif