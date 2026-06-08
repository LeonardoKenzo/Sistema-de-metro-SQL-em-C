#ifndef BTREE_H
    #define BTREE_H
    #include <stdio.h>
    #include <stdlib.h>
    #include "BTreeNode.h"
    #include "BTreeHeader.h"

    // Funções principais
    bool create_btree(char *bin_filename, char *btree_filename);
    void search_btree(char *bin_filename, char *btree_filename);
    void insert_btree(char *bin_filename, char *btree_filename);
    void remove_btree(char *bin_filename, char *btree_filename);
    
#endif