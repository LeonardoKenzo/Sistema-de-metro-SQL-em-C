#ifndef FUNCTIONALITIES_H
    #define FUNCTIONALITIES_H
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "functionalities.h"
    #include "headerRegister.h"
    #include "dataRegister.h"

    // Função auxiliar para ler campos do CSV tratando nulos
    char* get_field(char **line);

    void create_table(char *csv_filename,char *bin_filename);
#endif