#ifndef HEADER_REGISTER_H
    #define HEADER_REGISTER_H
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <string.h>

    typedef struct headerRegister Header;

    // Estrutura auxiliar para armazenar pares de estacoes
    typedef struct Pares paresEstacoes;
    
    Header *create_header_register();
    void free_header_register(Header **hRegister);

    // Setters 
    void header_set_status(Header *h, char status);
    void header_set_topo(Header *h, int topo);
    void header_set_proxRRN(Header *h, int proxRRN);
    void header_set_nroEstacoes(Header *h, int nroEstacoes);
    void header_set_nroParesEst(Header *h, int nroParesEst);

    // Getters
    int header_get_topo(Header *h);
    int header_get_proxRRN(Header *h);
    int header_get_nroEstacoes(Header *h);
    int header_get_nroParesEst(Header *h);

    // I/O - Escrita campo a campo
    void header_write_to_file(FILE *fp, Header *h);
    void header_read_from_file(FILE *fp, Header *h);

    // Funcoes para verificar o nroEstacoes e nroParEstacoes do header
    char **criar_lista_nomesEstacoes(int nroMaxEstacoes);
    bool tem_repetidos_nomesEstacoes(char **lista_nomesEstacoes, int nroEstacoes, char *nomeEst);
    void adicionar_nomesEstacoes(char **lista_nomesEstacoes, int nroEstacoes, char *nomeEst);
    void aumentar_capacidade_nomesEstacoes(char ***lista_nomesEstacoes, int nroEstacoes, int nroMaxEstacoes);
    void free_lista_nomesEstacoes(char ***lista, int nroEstacoes);

    paresEstacoes *criar_lista_paresEstacoes(int nroMaxPares);
    bool tem_repetido_paresEstacoes(paresEstacoes *lista_paresEstacoes, int nroPares, int codEst, int codProx);
    void adicionar_paresEstacoes(paresEstacoes *lista_paresEstacoes, int nroPares, int codEst, int codProx);
    void aumentar_capacidade_paresEstacoes(paresEstacoes **lista_paresEstacoes, int nroPares, int nroMaxPares);
    void free_lista_paresEstacoes(paresEstacoes **lista);

    void achar_todos_estacoes(FILE *bin, paresEstacoes **lista_paresEstacoes, int *nroParesEncontrados, int *nroMaxPares, char ***lista_nomesEstacoes, int *nroNomesEncontrados, int *nroMaxNomes);

#endif