#include "BTree.h"
#include "utils.h"
#include "dataRegister.h"
#include "headerRegister.h"

bool create_btree(char *bin_filename, char *btree_filename){
    FILE *bin = fopen(bin_filename, "r");
    FILE *btree = fopen(btree_filename, "wb");
    
    if(!bin || !btree || status_esta_instavel(bin)){
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    HeaderBTree *headerB = create_btree_header();
    btree_header_write_to_file(btree, headerB);

    Record *record;
    NodeB *no = create_btree_node(-1);
    char removido;
    int RRN = 0, posRecord = 0;
    while(fread(&removido, sizeof(char), 1, bin) == 1){
        if(removido == '1'){
            RRN++;
            posRecord = 17 + RRN * 80;
            fseek(bin, posRecord, SEEK_SET);
            continue;
        }   

        posRecord = 17 + RRN * 80;
        fseek(bin, posRecord, SEEK_SET);
        record = record_read_from_file(bin);
        // Fazer a criação da arvore b

        free_btree_node(&no);
        free_record(&record);
    }


    btree_header_set_status(headerB, '1');
    btree_header_write_to_file(btree, headerB);

    free_btree_header(headerB);
    fclose(bin);
    fclose(btree);
}