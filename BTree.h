#ifndef BTREE_H
    #define BTREE_H
    #include <stdio.h>
    #include <stdlib.h>
    #include "BTreeNode.h"
    #include "BTreeHeader.h"

    // Struct com todas as informacoes necessarias na busca do search
    typedef struct NodeSearchResult NodeSearch;

    NodeSearch *btree_search_recursive(FILE *btree, int rrnAtual, int rrnPai, int chave_busca, int *caminho, int profundidade);
    int btree_remove_recursive(FILE *btree, int rrn, int chave_remover);
    
    int alocar_rrn_novo_no(FILE *btree, HeaderBTree *headerB);
    void remover_logicamente_no(FILE *btree, HeaderBTree *headerB, int rrnNo);   
    
    // Busca do no
    NodeSearch *create_btree_nodeSearch(bool found, int rrnFilho, int rrnPai, int indice);
    void free_search_result(NodeSearch **result);
    bool btree_nodeSearch_get_found(NodeSearch *nodeSearch);
    int btree_nodeSearch_get_rrn(NodeSearch *nodeSearch);
    int btree_nodeSearch_get_rrnPai(NodeSearch *nodeSearch);
    int btree_nodeSearch_get_indice(NodeSearch *nodeSearch);
    int btree_nodeSearch_get_profundidade(NodeSearch *nodeSearch);
    int *btree_nodeSearch_get_caminho(NodeSearch *nodeSearch);
    void btree_nodeSearch_set_caminho(NodeSearch *nodeDestino, int *caminho, int size);

    void btree_insert(FILE *btree, HeaderBTree *h, int chave, int offset);

    // Remocao
    NodeSearch *btree_find_successor(FILE *btree, int rrnAtual);

    // Funções de tratamento de underflow
    bool btree_redistribute_right(FILE *btree, int rrnFilho, int rrnPai);
    bool btree_redistribute_left(FILE *btree, int rrnFilho, int rrnPai);
    bool btree_merge_right(FILE *btree, int rrnFilho, int rrnPai);
    bool btree_merge_left(FILE *btree, int rrnFilho, int rrnPai);
#endif