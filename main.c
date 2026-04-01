#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headerRegister.h"
#include "dataRegister.h"
#include "functionalities.h"


int main(void){
    
    int funcionalidade;
    scanf(" %d", &funcionalidade);

    switch (funcionalidade)
    {
    case 1:{   
        // funcionalidade 1 (CREATE TABLE)
        char arq_csv[100], arq_bin[100];
        scanf(" %s %s", arq_csv, arq_bin);
        create_table(arq_csv, arq_bin);
        BinarioNaTela(arq_bin);
        break;
    }
    case 2:{    
        // funcionalidade 2
        break;
    }
    case 3:{
        // funcionalidade 3
        break;
    }
    case 4:{
        // funcionalidade 4
        break;
    }
    case 5:{
        // funcionalidade 5
        break;
    }
    case 6:{
        // funcionalidade 6
        break;
    }
    }
    return 0;
}