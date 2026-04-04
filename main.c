#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headerRegister.h"
#include "dataRegister.h"
#include "functionalities.h"
#include "fornecidas.h"


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
    }
    return 0;
}