#include "functionalities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headerRegister.h"
#include "dataRegister.h"

// Função auxiliar para ler campos do CSV tratando nulos
char* get_field(char **line) {
    char *start = *line;
    char *end = strchr(start, ',');
    if (end) {
        *end = '\0';
        *line = end + 1;
    } else {
        // Último campo da linha (que termina em \n ou \r)
        char *newline = strpbrk(start, "\n\r");
        if (newline) *newline = '\0';
        *line = start + strlen(start);
    }
    return start;
}

void create_table(char *csv_filename, char *bin_filename){
    FILE *csv = fopen(csv_filename, "r");
    FILE *bin = fopen(bin_filename, "wb");

    if(!bin || !csv){
        printf("Falha ao abrir os arquivos!\n");
        return;
    }

    // Criar e escrever cabeçalho inicial (status '0')
    Header *h = CreateHeaderRegister();
    header_write_to_file(bin, h);

    char line[256];
    fgets(line, sizeof(line), csv); // Pular a primeira linha (cabeçalho CSV)

    int rrn_count = 0;
    while(fgets(line, sizeof(line), csv)){
        char *ptr = line;
        Record *r = CreateRecord();

        int codEst = atoi(get_field(&ptr));
        char *nomeEst = get_field(&ptr);
        int codLinha = atoi(get_field(&ptr));
        char *nomeLinha = get_field(&ptr);

        char *proxEstStr = get_field(&ptr);
        int codProx = (strlen(proxEstStr) > 0) ? atoi(proxEstStr) : -1;

        char *distStr = get_field(&ptr);
        int dist = (strlen(distStr) > 0) ? atoi(distStr) : -1;

        char *linIntegraStr = get_field(&ptr);
        int linIntegra = (strlen(linIntegraStr) > 0) ? atoi(linIntegraStr) : -1;

        char *estIntegraStr = get_field(&ptr);
        int estIntegra = (strlen(estIntegraStr) > 0) ? atoi(estIntegraStr) : -1;

        // Setar valores no Record
        record_set_codEstacao(r, codEst);
        record_set_nomeEstacao(r, nomeEst);
        record_set_codLinha(r, codLinha);
        record_set_nomeLinha(r, nomeLinha);
        record_set_codProxEstacao(r, codProx);
        record_set_distProxEstacao(r, dist);
        record_set_codLinhaIntegra(r, linIntegra);
        record_set_codEstIntegra(r, estIntegra);

        record_write_to_file(bin, r);
        FreeRecord(r);
        rrn_count++;
    }

    header_set_status(h, '1');
    header_set_proxRRN(h, rrn_count);

    fseek(bin, 0, SEEK_SET);
    header_write_to_file(bin, h);

    FreeHeaderRegister(&h);
    fclose(csv);
    fclose(bin);
}
