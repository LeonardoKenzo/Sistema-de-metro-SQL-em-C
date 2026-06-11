#ifndef UTILS_H
    #define UTILS_H
    #include "headerRegister.h"
    #include "dataRegister.h"
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdbool.h>

    // Structs para buscar registro de dados
    typedef struct Criterios Criterio;
    typedef struct Contextos Contexto;

    // Funções para printar valores
    void printar_record(FILE *bin, int posRecord);
    void printar_record_object(Record *r);
    void print_str_or_nulo(char *str);
    void print_int_or_nulo(int valor);
    
    // 
    void set_header_estacoes_unicas(FILE *bin, Header *h); 
    void update_campos(Record *r, Criterio *atualizar, int p);
    bool atende_criterios(Record *r, Criterio *criterios, int m);
    
    // Verificação dos registros
    bool esta_removido(FILE *bin, int posRecord);
    bool status_esta_instavel(FILE *bin);

    // Função de busca dos registros do arquivo binário
    bool search_records(FILE *bin, Criterio *criterios, int m, void(*funcionalidade)(Record *r, int posRecord, Contexto *ctx), Contexto *ctx);
    
    // Leitura de campos e inputs
    char* get_field(char **line);
    char *ler_campoVariavel(FILE *bin, int posRecord, int posOffset);
    int input_inteiro_ou_nulo();
    int ler_campoFixo(FILE *bin, int posRecord, int posOffset);
    Criterio *input_criterios(int m);
    
    // Manipular contexto de busca para determinar o que fazer com o registro encontrado após encontrá-lo
    Contexto *criar_contexto(FILE *bin, Header *header, Criterio *atualizar, int p);
    Contexto *atualizar_contexto(Contexto *ctx, Criterio *atualizar, int p);
    Contexto *pausar_busca(Contexto *ctx);

    // Verifica se 'codEstacao' foi fornecido como criterio de busca
    int get_chave_busca_criterio(Criterio *criterios, int m);
    
    // getters do contexto
    FILE *get_file_from_context(Contexto *ctx);
    Criterio *get_atualizar_from_context(Contexto *ctx);
    Header *get_header_from_context(Contexto *ctx);
    int get_p_from_context(Contexto *ctx);

#endif