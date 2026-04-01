#include "functionalities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "headerRegister.h"
#include "dataRegister.h"

typedef struct PARES_ESTACOES
{
    int codEstacao;
    int proxCodEstacao;
}paresEstacoes;

char* get_field(char **line);

char **criar_lista_nomesEstacoes(int nroMaxEstacoes);
void free_lista_nomesEstacoes(char ***lista, int nroEstacoes);
paresEstacoes *criar_lista_paresEstacoes(int nroMaxPares);
void free_lista_paresEstacoes(paresEstacoes **lista);

int ler_campoFixo(FILE *bin, int posRecord, int posOffset);
char *ler_campoVariavel(FILE *bin, int posRecord, int posOffset);
void printar_record(FILE *bin, int posRecord);
bool status_esta_instavel(FILE *bin);
bool esta_removido(FILE *bin, int posRecord);

// Funcionalidade 1 ------------------------------------------
void create_table(char *csv_filename, char *bin_filename){
    FILE *csv = fopen(csv_filename, "r");
    FILE *bin = fopen(bin_filename, "wb");

    if(!bin || !csv){
        printf("Falha no processamento do arquivo.\n");
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
        
        // Aumenta a capacidade das listas caso necessario
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

// Funcionalidade 2 -------------------------------------------
void print_table(char *bin_filename){
    FILE *bin = fopen(bin_filename, "rb");
    if(!bin){
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    if(status_esta_instavel(bin)){
        printf("Falha no processamento do arquivo.\n");
        fclose(bin);
        return;
    }
    
    // Vai para a posicao do primeiro registro record
    int RRN = 0, removidos = 0;

    while(1){
        int posRecord = 17 + RRN * 80;
        fseek(bin, posRecord, SEEK_SET);
        
        // Se nao for possivel ler mais, indica que o arquivo acabou
        char removido;
        if(fread(&removido, sizeof(char), 1, bin) != 1)
            break;

        if(removido == '1'){
            RRN++;
            removidos++;
            continue;
        }
    
        printar_record(bin, posRecord);
        RRN++;
    }

    // Caso todos os registros tenham sido removidos 
    if(removidos == RRN)
        printf("Registro inexistente.\n");

    fclose(bin);
}

// Funcionalidade 3 ------------------------------------



/*
 * Você não precisa entender o código dessa função.
 *
 * Use essa função para comparação no run.codes.
 * Lembre-se de ter fechado (fclose) o arquivo anteriormente.
 *
 * Ela vai abrir de novo para leitura e depois fechar
 * (você não vai perder pontos por isso se usar ela).
 */
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

bool status_esta_instavel(FILE *bin){
    char status;
    fread(&status, sizeof(char), 1, bin);

    if(status == '0'){
        return true;
    }

    return false;
}

// Funcao auxiliar para ler os campos variaveis dos records do file BIN 
// (lembre-se de dar free no campo depois e usar o posOffset do tamanho do campo)
char *ler_campoVariavel(FILE *bin, int posRecord, int posOffset){
    fseek(bin, posRecord + posOffset, SEEK_SET);

    int tamNome;
    fread(&tamNome, sizeof(int), 1, bin);
    
    if(tamNome > 0){
        char *nome = (char *)malloc((tamNome + 1)* sizeof(char));
        fread(nome, sizeof(char), tamNome, bin);
        nome[tamNome] = '\0';

        return nome;
    }

    return NULL;
}

int ler_campoFixo(FILE *bin, int posRecord, int posOffset){
    fseek(bin, posRecord + posOffset, SEEK_SET);

    int campoFixo;
    fread(&campoFixo, sizeof(int), 1, bin);

    return campoFixo;
}

bool esta_removido(FILE *bin, int posRecord){
    fseek(bin, posRecord, SEEK_SET);

    char removido;
    fread(&removido, sizeof(char), 1, bin);

    if(removido == '1')
        return true;

    return false;
}

void print_auxiliar_record(int codigo){
    if(codigo == -1)
        printf("NULO ");
    else
        printf("%d ", codigo);
}

void printar_record(FILE *bin, int posRecord){
    if(esta_removido(bin, posRecord))
        return;
    
    int posCodEst = 5, posNomeEst = 29, posCodLinha = 9, posCodProxEst = 13, posDistEst = 17, posCodLinhaInt = 21, posCodEstInt = 25;  
    
    int codEst = ler_campoFixo(bin, posRecord, posCodEst);
    char *nomeEst = ler_campoVariavel(bin, posRecord, posNomeEst);
    
    printf("%d %s ", codEst, nomeEst);

    // Calcular o posNomeLinha (sempre vai ter nomeEst)
    int tamanhoEst = strlen(nomeEst);
    int posNomeLinha = posNomeEst + tamanhoEst + 4;
    
    int codLinha = ler_campoFixo(bin, posRecord, posCodLinha);
    char *nomeLinha = ler_campoVariavel(bin, posRecord, posNomeLinha);

    print_auxiliar_record(codLinha);
    if(nomeLinha)
        printf("%s ", nomeLinha);
    else
        printf("NULO ");

    int codProxEst = ler_campoFixo(bin, posRecord, posCodProxEst);
    int distExt = ler_campoFixo(bin, posRecord, posDistEst);
    int codLinhaInt = ler_campoFixo(bin, posRecord, posCodLinhaInt);
    int codEstInt = ler_campoFixo(bin, posRecord, posCodEstInt);

    print_auxiliar_record(codProxEst);
    print_auxiliar_record(distExt);
    print_auxiliar_record(codLinhaInt);
    print_auxiliar_record(codEstInt);
    printf("\b\n");

    free(nomeEst);
    free(nomeLinha);
}

// Funcoes auxiliares para verificar nroEstacoes e paresEstacoes
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