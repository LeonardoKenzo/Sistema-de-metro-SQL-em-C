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

    void search_btree_table(char *bin_filename, char *btree_filename);
#endif