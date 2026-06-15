#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functionalities.h"
#include "BTree.h"
#include "fornecidas.h"


// Leonardo Kenzo Tanaka - 16882154
// Pedro Teidi de Sa Yamacita - 16897290

int main(void){
    
    int funcionalidade;
    scanf(" %d", &funcionalidade);

    switch (funcionalidade)
    {
    case 1:{   
        // funcionalidade 1
        char arq_csv[100], arq_bin[100];
        scanf(" %s %s", arq_csv, arq_bin);
        if(create_table(arq_csv, arq_bin))
            BinarioNaTela(arq_bin);
        break;
    }
    case 2:{    
        // funcionalidade 2
        char arq_bin[100];
        scanf(" %s", arq_bin);
        print_table(arq_bin);
        break;
    }
    case 3:{
        // funcionalidade 3
        char arq_bin[100];
        scanf(" %s", arq_bin);
        search_table(arq_bin);
        break;
    }
    case 4:{
        // funcionalidade 4
        char arq_bin[100];
        scanf(" %s", arq_bin);
        remove_record_table(arq_bin);
        BinarioNaTela(arq_bin);
        break;
    }
    case 5:{
        // funcionalidade 5
        char arq_bin[100];
        scanf("%s", arq_bin);
        insert_record_table(arq_bin);
        BinarioNaTela(arq_bin); 
        break;
    }
    case 6:{
        // funcionalidade 6
        char arq_bin[100];
        scanf("%s", arq_bin);
        update_table(arq_bin);
        BinarioNaTela(arq_bin); 
        break;
    }
    case 7:{
        // funcionalidade 7
        char arq_bin[100], arq_btree[100];
        scanf("%s %s", arq_bin, arq_btree);
        create_btree(arq_bin, arq_btree);
        BinarioNaTela(arq_btree);
        break;
    }
    case 8:{
        // funcionalidade 8
        char arq_bin[100], arq_btree[100];
        scanf("%s %s", arq_bin, arq_btree);
        search_btree(arq_bin, arq_btree);
        break;
    }
    case 9:{
        // funcionalidade 9
        char arq_bin[100], arq_btree[100];
        scanf("%s %s", arq_bin, arq_btree);
        if(insert_btree(arq_bin, arq_btree)){
            BinarioNaTela(arq_bin);
            BinarioNaTela(arq_btree);
        }
        break;
    }
    case 10:{
        // funcionalidade 10
        char arq_bin[100], arq_btree[100];
        scanf("%s %s", arq_bin, arq_btree);
        remove_btree(arq_bin, arq_btree);
        BinarioNaTela(arq_bin);
        BinarioNaTela(arq_btree);
        break;
    }
    }   
    return 0;
}