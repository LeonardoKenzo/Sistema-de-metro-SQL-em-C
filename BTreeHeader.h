#ifndef BTREEHEADER_H
    #define BTREEHEADER_H
    #include <stdio.h>
    #include <stdlib.h>
    #include "BTreeNode.h"
    #include "headerRegister.h"

    typedef struct BTreeHeader HeaderBTree;

    HeaderBTree *create_btree_header();
    void free_btree_header(HeaderBTree **header);

    // getters
    char btree_header_get_status(HeaderBTree *header);
    int btree_header_get_noRaiz(HeaderBTree *header);
    int btree_header_get_topo(HeaderBTree *header);
    int btree_header_get_proxRRN(HeaderBTree *header);
    int btree_header_get_nroNos(HeaderBTree *header);
    int btree_node_get_child_rrn(NodeB *node, int chave);
    
    // setters
    void btree_header_set_status(HeaderBTree *header, char status);
    void btree_header_set_noRaiz(HeaderBTree *header, int raiz);
    void btree_header_set_topo(HeaderBTree *header, int topo);
    void btree_header_set_proxRRN(HeaderBTree *header, int proxRRN);
    void btree_header_set_nroNos(HeaderBTree *header, int nroNos);

    // I/O - Escrita campo a campo
    void btree_header_write_to_file(FILE *fp, HeaderBTree *header);
    void btree_header_read_from_file(FILE *fp, HeaderBTree *header);

#endif