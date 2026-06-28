#ifndef FUNCTIONALITIES_H
    #define FUNCTIONALITIES_H
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdbool.h>
    #include <ctype.h>
    #include "headerRegister.h"
    #include "dataRegister.h"
    #include "BTree.h"      
    #include "BTreeHeader.h"

    bool create_table(char *csv_filename,char *bin_filename);
    void print_table(char *bin_filename);
    void search_table(char *bin_filename);
    void remove_record_table(char *bin_filename);
    void  insert_record_table(char *bin_filename);
    void update_table(char *bin_filename);

    bool create_btree(char *bin_filename, char *btree_filename);    
    void search_btree(char *bin_filename, char *btree_filename);
    bool insert_btree(char *bin_filename, char *btree_filename);
    bool remove_btree(char *bin_filename, char *btree_filename);

    bool create_order_by(char *bin_filename, char *campoOrdenacao, char *order_filename);
    void join_order_by(char *bin_filename1, char *joinCampo1, char *bin_filename2, char *joinCampo2);

#endif