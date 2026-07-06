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
        if(create_btree(arq_bin, arq_btree)){
            BinarioNaTela(arq_btree);
        }
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
        if(remove_btree(arq_bin, arq_btree)){
            BinarioNaTela(arq_bin);
            BinarioNaTela(arq_btree);
        }
        break;
    }
    case 11: {
        // funcionalidade 11
        char arq_bin1[100], campo1[50], arq_bin2[100], campo2[50];
        scanf("%s %s %s %s", arq_bin1, campo1, arq_bin2, campo2);
        nested_loop_join(arq_bin1, campo1, arq_bin2, campo2);
        break;
    }
    case 12: {
        // funcionalidade 12
        char arq_bin1[100], campo1[50], arq_bin2[100], campo2[50], arq_btree[100];
        scanf("%s %s %s %s %s", arq_bin1, campo1, arq_bin2, campo2, arq_btree);
        single_loop_join(arq_bin1, campo1, arq_bin2, campo2, arq_btree);
        break;
    }
    case 13:{
        // funcionalidade 13
        char arq_bin[100], campoOrdenacao[50], arq_order[100];
        scanf("%s ", arq_bin);
        ScanQuoteString(campoOrdenacao);
        scanf("%s ", arq_order);
        if(create_order_by(arq_bin, campoOrdenacao, arq_order)){
            BinarioNaTela(arq_order);
        }
    }
    case 14:{
        // funcionalidade 14
        char arq_bin1[100], joinCampo1[50], arq_bin2[100], joinCampo2[50];
        scanf("%s ", arq_bin1);
        ScanQuoteString(joinCampo1);
        scanf("%s ", arq_bin2);
        ScanQuoteString(joinCampo2);
        join_order_by(arq_bin1, joinCampo1, arq_bin2, joinCampo2);
    }
    }   
    return 0;
}