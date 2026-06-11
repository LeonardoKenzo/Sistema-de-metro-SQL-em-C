#ifndef NODEB_H
    #define NODEB_H
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>

    typedef struct BTreeNode NodeB;

    NodeB *create_btree_node(int tipoNo);
    void free_btree_node(NodeB **node);

    // funcoes
    int btree_node_buscar_chave(NodeB *node, int chave);
    bool btree_node_inserir_chave(NodeB *node, int chave, int ponteiro, int filho);
    bool btree_node_remover_chave(NodeB *node, int indice);
    
    // getters
    char btree_node_get_removido(NodeB *node);
    int btree_node_get_proximo(NodeB *node);
    int btree_node_get_tipoNo(NodeB *node);
    int btree_node_get_nroChaves(NodeB *node);

    // setters
    void btree_node_set_removido(NodeB *node, char removido);
    void btree_node_set_proximo(NodeB *node, int proximo);
    void btree_node_set_tipoNo(NodeB *node, int tipoNo);
    void btree_node_set_nroChaves(NodeB *node, int nroChaves);
    void bree_node_set_filho_inicial(NodeB *node, int filho);

    // I/O - Escrita campo a campo
    void btree_node_write_to_file(FILE *fp, NodeB *node);
    NodeB *btree_node_read_from_file_at_offset(FILE *fp, int byteOffset);
    
#endif