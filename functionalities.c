#include "functionalities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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

// Funcao auxiliar para ler os records do file BIN
char *ler_nomeEstacao(FILE *bin, int posRecord){
    fseek(bin, posRecord, SEEK_SET);
    
    char removido;
    fread(&removido, sizeof(char), 1, bin);

    if(removido == '1'){
        return NULL;
    }

    fseek(bin, posRecord + 30, SEEK_SET);

    int tamNome;
    fread(&tamNome, sizeof(int), 1, bin);

    char *nome = (char *)malloc((tamNome + 1)* sizeof(char));
    fread(nome, sizeof(char), tamNome, bin);
    nome[tamNome] = '\0';

    return nome;
}

// Funcoes auxiliares
char **criar_lista_nomesEstacoes(int nroMaxEstacoes){
    char **lista_nomesEstacoes = (char **)calloc(nroMaxEstacoes, sizeof(char *));
    if(lista_nomesEstacoes == NULL){
        printf("Erro: alocação de memória lista de estações!\n");
        return NULL; 
    }
    return lista_nomesEstacoes;
}

void free_lista_nomesEstacoes(char ***lista, int nroEstacoes){
    for(int i = 0; i < nroEstacoes; i++){
        free((*lista)[i]);
    }
    free((*lista));
    *lista = NULL;
}

typedef struct PARES_ESTACOES
{
    int codEstacao;
    int proxCodEstacao;
}paresEstacoes;
    
paresEstacoes *criar_lista_paresEstacoes(int nroMaxPares){
    paresEstacoes *lista_pares = (paresEstacoes *)calloc(nroMaxPares, sizeof(paresEstacoes));
    if(lista_pares == NULL){
        printf("Erro: alocação de memória lista de pares de estações!\n");
        return NULL;
    }
    for(int i = 0; i < nroMaxPares; i++){
        lista_pares[i].codEstacao = -1;
        lista_pares[i].proxCodEstacao = -1;
    }
    return lista_pares;
}

void free_lista_paresEstacoes(paresEstacoes **lista){
    free((*lista));
    *lista = NULL;
}


void create_table(char *csv_filename, char *bin_filename){
    FILE *csv = fopen(csv_filename, "r");
    FILE *bin = fopen(bin_filename, "wb");

    if(!bin || !csv){
        printf("Falha ao abrir os arquivos!\n");
        return;
    }

    // Criar e escrever cabeçalho inicial (status '0')
    Header *h = create_header_register();
    header_write_to_file(bin, h);
    
    // Lista para verificar nomes unicos e pares unicos de estacoes
    int nroMaxEstacoes = 25, nroMaxPares = 25;
    char **lista_nomesEstacoes = criar_lista_nomesEstacoes(nroMaxEstacoes);
    paresEstacoes *lista_paresEstacoes = criar_lista_paresEstacoes(nroMaxPares);
    
    char line[256];
    fgets(line, sizeof(line), csv); // Pular a primeira linha (cabeçalho CSV)
    
    int rrn_count = 0, nroEstacoes = 0, nroPares = 0;
    while(fgets(line, sizeof(line), csv)){
        char *ptr = line;
        Record *r = create_record();
        
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
        free_record(&r);
        
        // Aumenta a capacidade da lista caso necessario
        if(nroEstacoes >= nroMaxEstacoes){
            nroMaxEstacoes *= 2;
            lista_nomesEstacoes = realloc(lista_nomesEstacoes, nroMaxEstacoes * sizeof(char *));
            for(int i = nroEstacoes; i < nroMaxEstacoes; i++)
                lista_nomesEstacoes[i] = NULL;
        }
        if(nroPares >= nroMaxPares){
            nroMaxPares *= 2;
            lista_paresEstacoes = realloc(lista_paresEstacoes, nroMaxPares * sizeof(paresEstacoes));
        }
        
        // Verificar quantidade de nomes e pares de estacoes diferentes
        int repetidoNome = 0, repetidoPar = 0;
        
        // Percorre a lista comparando se possui nomes repetidos
        int tamanho = strlen(nomeEst);
        for(int i = 0; i < nroEstacoes; i++){
            if(lista_nomesEstacoes[i] != NULL)
                repetidoNome = (strcmp(lista_nomesEstacoes[i], nomeEst) == 0) ? repetidoNome + 1 : repetidoNome;
        }
        if(repetidoNome <= 0){
            lista_nomesEstacoes[nroEstacoes] = (char *)calloc(tamanho + 1, sizeof(char));
            strcpy(lista_nomesEstacoes[nroEstacoes], nomeEst);
            nroEstacoes++;
        }

        // Mesma logica da lista_nomeEstacoes
        for(int i = 0; i < nroPares; i++){
            repetidoPar = (lista_paresEstacoes[i].codEstacao == codEst && lista_paresEstacoes[i].proxCodEstacao == codProx) ? repetidoPar + 1 : repetidoPar;
        }
        if(repetidoPar <= 0){
            lista_paresEstacoes[nroPares].codEstacao = codEst;
            lista_paresEstacoes[nroPares].proxCodEstacao = codProx;
            nroPares++;
        }
        
        rrn_count++;
    }

    header_set_status(h, '1');
    header_set_proxRRN(h, rrn_count);
    header_set_nroEstacoes(h, nroEstacoes);
    header_set_nroParesEst(h, nroPares);

    fseek(bin, 0, SEEK_SET);
    header_write_to_file(bin, h);  

    free_lista_nomesEstacoes(&lista_nomesEstacoes, nroEstacoes);
    free_lista_paresEstacoes(&lista_paresEstacoes);
    free_header_register(&h);
    fclose(csv);
    fclose(bin);
}

void BinarioNaTela(char *arquivo) {
    FILE *fs;
    if (arquivo == NULL || !(fs = fopen(arquivo, "rb"))) {
        fprintf(stderr,
                "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): "
                "não foi possível abrir o arquivo que me passou para leitura. "
                "Ele existe e você tá passando o nome certo? Você lembrou de "
                "fechar ele com fclose depois de usar?\n");
        return;
    }

    fseek(fs, 0, SEEK_END);
    size_t fl = ftell(fs);

    fseek(fs, 0, SEEK_SET);
    unsigned char *mb = (unsigned char *)malloc(fl);
    fread(mb, 1, fl, fs);

    unsigned long cs = 0;
    for (unsigned long i = 0; i < fl; i++) {
        cs += (unsigned long)mb[i];
    }

    printf("%lf\n", (cs / (double)100));

    free(mb);
    fclose(fs);
}