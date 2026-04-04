#include "functionalities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "headerRegister.h"
#include "dataRegister.h"
#include "fornecidas.h"

// Estrutura auxiliar para armazenar os criterios de busca
typedef struct {
    char nomeCampo[50];
    char valorCampo[100];
} Criterio;

char* get_field(char **line);

// Funcoes para ler um registro de um arquivo.bin
int ler_campoFixo(FILE *bin, int posRecord, int posOffset);
char *ler_campoVariavel(FILE *bin, int posRecord, int posOffset);
void printar_record_object(Record *r);
void printar_record(FILE *bin, int posRecord);
bool status_esta_instavel(FILE *bin);
bool esta_removido(FILE *bin, int posRecord);

// Funcoes para buscar um registro a partir de criterios
Criterio *input_criterios(int m);
int *search_offset_records(FILE *bin, Criterio *criterios, int m, int *quantRecordEncontrados);
Record **read_records_at_offset(FILE *bin, int *posOffsetRecords, int tamanhoPosOffset);
void free_search_records(Record ***lista_records, int quantRecordEncontrados);
void update_register(Record *r, Criterio *atualizar, int p);
bool atende_criterios(Record *r, Criterio *criterios, int m);

// Função auxiliar para ler inteiros que podem ser "NULO" da entrada padrão
int input_inteiro_ou_nulo();

// Funcionalidade 1 ------------------------------------------
bool create_table(char *csv_filename, char *bin_filename){
    FILE *csv = fopen(csv_filename, "r");
    FILE *bin = fopen(bin_filename, "wb");

    if(!bin || !csv){
        printf("Falha no processamento do arquivo.\n");
        return false;
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
            aumentar_capacidade_nomesEstacoes(&lista_nomesEstacoes, nroEstacoes, nroMaxEstacoes);
        }

        if(nroPares >= nroMaxPares){
            nroMaxPares *= 2;
            aumentar_capacidade_paresEstacoes(&lista_paresEstacoes, nroPares, nroMaxPares);
        }

        if(tem_repetidos_nomesEstacoes(lista_nomesEstacoes, nroEstacoes, nomeEst) == false){
            adicionar_nomesEstacoes(lista_nomesEstacoes, nroEstacoes, nomeEst);
            nroEstacoes++;
        }

        if(tem_repetido_paresEstacoes(lista_paresEstacoes, nroPares, codEst, codProx) == false && codProx != -1){
            adicionar_paresEstacoes(lista_paresEstacoes, nroPares, codEst, codProx);
            nroPares++;
        }
        
        rrn_count++;
    }
    
    header_set_status(h, '1');
    header_set_proxRRN(h, rrn_count);
    header_set_nroEstacoes(h, nroEstacoes);
    header_set_nroParesEst(h, nroPares);

    header_write_to_file(bin, h);  

    free_lista_nomesEstacoes(&lista_nomesEstacoes, nroEstacoes);
    free_lista_paresEstacoes(&lista_paresEstacoes);
    free_header_register(&h);
    fclose(csv);
    fclose(bin);
    return true;
}

// Funcionalidade 2 -------------------------------------------
void print_table(char *bin_filename){
    FILE *bin = fopen(bin_filename, "rb");
    if(!bin || status_esta_instavel(bin)){
        printf("Falha no processamento do arquivo.\n");
        if(bin)
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
void search_table(char *bin_filename){
    FILE *bin = fopen(bin_filename, "rb");
    if(!bin || status_esta_instavel(bin)){
        printf("Falha no processamento do arquivo.\n");
        if (bin) fclose(bin);
        return;
    }

    int n;
    scanf(" %d", &n);
    for(int i = 0; i < n; i++){
        int m, quantRegistroEncontrado;
        scanf(" %d", &m);
        Criterio *criterios = input_criterios(m);

        // Todos os registros encontrados na busca
        int *lista_posOffset_records = search_offset_records(bin, criterios, m, &quantRegistroEncontrado); // Filtra por criterio
        Record **lista_records = read_records_at_offset(bin, lista_posOffset_records, quantRegistroEncontrado); // Carrega os registros
        for(int j = 0; j < quantRegistroEncontrado; j++){
            printar_record_object(lista_records[j]);
        }
        if(quantRegistroEncontrado == 0){
            printf("Registro inexistente.\n");
        }

        printf("\n");

        free(criterios);
        free(lista_posOffset_records);
        free_search_records(&lista_records, quantRegistroEncontrado);
    }

    fclose(bin);
}

// Funcionalidade 4 -----------------------------------
void remove_record_table(char *bin_filename){
    FILE *bin = fopen(bin_filename, "r+b");
    if(!bin || status_esta_instavel(bin)){
        printf("Falha no processamento do arquivo.\n");
        if (bin) fclose(bin);
        return;
    }
    // Marcar o header como instavel
    Header *header = create_header_register();
    header_read_from_file(bin, header);
    header_set_status(header, '0');
    header_write_to_file(bin, header);

    // Listas para atualizar os numeros de estacoes e pares do header
    int nroMaxPares = 10, nroMaxNomes = 10;
    char **lista_nomesEstacoes = criar_lista_nomesEstacoes(nroMaxNomes);
    paresEstacoes *lista_paresEstacoes = criar_lista_paresEstacoes(nroMaxPares);

    int n;
    scanf(" %d", &n);
    for(int i = 0; i < n; i++){
        int m, quantRegistroEncontrado, posRecord, topoAntigo, proximoRRN;
        scanf(" %d", &m);
        Criterio *criterios = input_criterios(m);

        // Todos os registros encontrados na busca
        int *lista_posOffset_records = search_offset_records(bin, criterios, m, &quantRegistroEncontrado); // Filtra por criterio
        Record **lista_records = read_records_at_offset(bin, lista_posOffset_records, quantRegistroEncontrado); // Carrega os registros
        for(int j = 0; j < quantRegistroEncontrado; j++){

            Record *record = lista_records[j];
            posRecord = lista_posOffset_records[j];

            // Remocao logica e atualizacao do header
            topoAntigo = header_get_topo(header);
            proximoRRN = (topoAntigo == -1) ? -1 : (topoAntigo - 17) / 80;
            
            record_set_removido(record, '1');
            record_set_proximo(record, proximoRRN);
            
            fseek(bin, posRecord, SEEK_SET);
            record_write_to_file(bin, record);

            header_set_topo(header, posRecord);
            header_write_to_file(bin, header);
        }
        
        free(criterios);
        free(lista_posOffset_records);
        free_search_records(&lista_records, quantRegistroEncontrado);
    }

    // Verifica a quantidade de nomes diferentes de estacoes e de pares de estacoes
    int nroNomesEncontrados = 0, nroParesEncontrados = 0;
    achar_todos_nomesEstacoes(bin, &lista_nomesEstacoes, &nroNomesEncontrados, &nroMaxNomes);
    achar_todos_paresEstacoes(bin, &lista_paresEstacoes, &nroParesEncontrados, &nroMaxPares);
    
    header_set_nroEstacoes(header, nroNomesEncontrados);
    header_set_nroParesEst(header, nroParesEncontrados);
    
    // Marcar o header como estavel
    header_set_status(header, '1');
    header_write_to_file(bin, header);
    
    free_lista_nomesEstacoes(&lista_nomesEstacoes, nroNomesEncontrados);
    free_lista_paresEstacoes(&lista_paresEstacoes);
    free_header_register(&header);
    fclose(bin);
}

// Funcionalidade 5 -----------------------------------

void insert_record_table(char *bin_filename){
    FILE *bin = fopen(bin_filename, "r+b");
    if(!bin || status_esta_instavel(bin)){
        printf("Falha no processamento do arquivo.\n"); 
        if (bin) fclose(bin);
        return;
    }

    Header *h = create_header_register();
    header_read_from_file(bin, h);
    header_set_status(h, '0');
    header_write_to_file(bin, h);

    // Listas para atualizar os numeros de estacoes e pares do header
    int nroMaxPares = 10, nroMaxNomes = 10;
    char **lista_nomesEstacoes = criar_lista_nomesEstacoes(nroMaxNomes);
    paresEstacoes *lista_paresEstacoes = criar_lista_paresEstacoes(nroMaxPares);

    int n;
    scanf(" %d", &n);

    for(int i = 0; i < n; i++){
        Record *r = create_record();
        char buffer_str[100];

        int codEstacao;
        scanf(" %d", &codEstacao);
        record_set_codEstacao(r,codEstacao);

        // nomeEstacao (não aceita nulo)
        ScanQuoteString(buffer_str);
        record_set_nomeEstacao(r, buffer_str);

        //codLinha
        record_set_codLinha(r, input_inteiro_ou_nulo());

        //nomeLinha
        ScanQuoteString(buffer_str);
        record_set_nomeLinha(r, buffer_str);

        // Campos que são inteiros ou nulos
        record_set_codProxEstacao(r, input_inteiro_ou_nulo());
        record_set_distProxEstacao(r, input_inteiro_ou_nulo());
        record_set_codLinhaIntegra(r, input_inteiro_ou_nulo());
        record_set_codEstIntegra(r, input_inteiro_ou_nulo());

        int topo = header_get_topo(h);

        if (topo == -1) {
            // Caso 1: Pilha vazia, insere no fim do arquivo
            int proxRRN = header_get_proxRRN(h);
            fseek(bin, 17 + (proxRRN * 80), SEEK_SET);
            record_write_to_file(bin, r);
            header_set_proxRRN(h, proxRRN + 1);
        }
        else {
            // Caso 2: Reaproveitamento de espaço (Pilha)
            // Vai até o registro removido indicado pelo topo
            fseek(bin, topo, SEEK_SET);
            
            // Lê o RRN do próximo da pilha antes de sobrescrever
            char removido_flag;
            int prox_na_pilha;
            fread(&removido_flag, sizeof(char), 1, bin);
            fread(&prox_na_pilha, sizeof(int), 1, bin);
        
            // Volta para a posição do registro e escreve o novo dado
            fseek(bin, (topo), SEEK_SET);
            record_write_to_file(bin, r);

            // Atualiza o topo do cabeçalho com o próximo da lista encadeada
            int novo_topo_offset = (prox_na_pilha == -1) ? -1 : 17 + (prox_na_pilha * 80);
            header_set_topo(h, novo_topo_offset);
        }
        free_record(&r);
    }

    // Verifica a quantidade de nomes diferentes de estacoes e de pares de estacoes
    int nroNomesEncontrados = 0, nroParesEncontrados = 0;
    achar_todos_nomesEstacoes(bin, &lista_nomesEstacoes, &nroNomesEncontrados, &nroMaxNomes);
    achar_todos_paresEstacoes(bin, &lista_paresEstacoes, &nroParesEncontrados, &nroMaxPares);
    
    header_set_nroEstacoes(h, nroNomesEncontrados);
    header_set_nroParesEst(h, nroParesEncontrados);

    header_set_status(h, '1'); 
    header_write_to_file(bin, h);
    
    free_lista_nomesEstacoes(&lista_nomesEstacoes, nroNomesEncontrados);
    free_lista_paresEstacoes(&lista_paresEstacoes);
    free_header_register(&h);
    fclose(bin);
}

// Funcionalidade 6 -------------------------------------------------------

void update_table(char *bin_filename){
    FILE *bin = fopen(bin_filename, "r+b");
    if(!bin || status_esta_instavel(bin)){
        printf("Falha no processamento do arquivo.\n");
        if (bin) fclose(bin);
        return;
    }
    // Marcar o header como instavel
    Header *header = create_header_register();
    header_read_from_file(bin, header);
    header_set_status(header, '0');
    header_write_to_file(bin, header);

    int n;
    scanf(" %d", &n);
    for(int i = 0; i < n; i++){
        int m, p, quantRegistroEncontrado, posRecord, topoAntigo, proximoRRN;
        scanf(" %d", &m);
        Criterio *criterios = input_criterios(m);

        scanf(" %d", &p);
        Criterio *atualizar = input_criterios(p);

        // Todos os registros encontrados na busca
        int *lista_posOffset_records = search_offset_records(bin, criterios, m, &quantRegistroEncontrado); // Filtra por criterio
        Record **lista_records = read_records_at_offset(bin, lista_posOffset_records, quantRegistroEncontrado); // Carrega os registros
        for(int j = 0; j < quantRegistroEncontrado; j++){
            update_register(lista_records[j], atualizar, p);
            
            fseek(bin, lista_posOffset_records[j], SEEK_SET);
            record_write_to_file(bin, lista_records[j]);
        }

        free(criterios);
        free(atualizar);
        free(lista_posOffset_records);
        free_search_records(&lista_records, quantRegistroEncontrado);
    }

    // Marcar o header como estavel
    header_set_status(header, '1');
    header_write_to_file(bin, header);

    free_header_register(&header);
    fclose(bin);
}



// Funcoes auxiliares -----------------------------------------------------

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

// Auxiliar para imprimir inteiros tratando o valor -1 como NULO
void print_int_or_nulo(int valor) {
    if (valor == -1) {
        printf("NULO");
    } else {
        printf("%d", valor);
    }
}

// Auxiliar para imprimir strings tratando o ponteiro NULL como NULO
void print_str_or_nulo(char *str) {
    if (str == NULL || strlen(str) == 0) {
        printf("NULO");
    } else {
        printf("%s", str);
    }
}

void printar_record_object(Record *r) {
    if (r == NULL) return;

    // codEstacao (Nunca é nulo conforme especificação)
    printf("%d ", record_get_codEstacao(r));

    // nomeEstacao (Nunca é nulo conforme especificação)
    printf("%s ", record_get_nomeEstacao(r));

    // codLinha
    print_int_or_nulo(record_get_codLinha(r));
    printf(" ");

    // nomeLinha
    print_str_or_nulo(record_get_nomeLinha(r));
    printf(" ");

    // codProxEstacao
    print_int_or_nulo(record_get_codProxEstacao(r));
    printf(" ");

    // distProxEstacao
    print_int_or_nulo(record_get_distProxEstacao(r));
    printf(" ");

    // codLinhaIntegra
    print_int_or_nulo(record_get_codLinhaIntegra(r));
    printf(" ");

    // codEstIntegra
    print_int_or_nulo(record_get_codEstIntegra(r));

    // Quebra de linha final para o registro
    printf("\n");
}

void printar_record(FILE *bin, int posRecord){
    fseek(bin, posRecord, SEEK_SET);
    
    // Em vez de dar printf campo a campo
    Record *r = record_read_from_file(bin); // Lê e cria o objeto
    
    if (r != NULL) {
        printar_record_object(r); // Usa a lógica centralizada de exibição
        free_record(&r);        
    }
}

Record **read_records_at_offset(FILE *bin, int *posOffsetRecords, int tamanhoPosOffset){
    Record **lista_records = (Record **)calloc(tamanhoPosOffset, sizeof(Record *));
    if(!lista_records){
        printf("Erro: alocação de memória busca de registros.\n");
        return NULL;
    }

    Record *r;
    for(int i = 0; i < tamanhoPosOffset; i++){
        int posRecord = posOffsetRecords[i];
        fseek(bin, posRecord, SEEK_SET);

        r = record_read_from_file(bin);
        if(r == NULL){
            printf("Erro: alocação de memória ao criar registro de dados!.\n");
            return NULL;
        }

        lista_records[i] = r;
    }

    return lista_records;
}

void free_search_records(Record ***lista_records, int quantRecordEncontrados){
    if((*lista_records) == NULL)
        return;
    for(int i = 0; i < quantRecordEncontrados; i++){
        free_record(&(*lista_records)[i]);
    }
    free((*lista_records));
    *lista_records = NULL;
}   

int *search_offset_records(FILE *bin, Criterio *criterios, int m, int *quantRecordEncontrados){
    int quantRecords = 0, quantRecordsMax = 10;
    int *lista_posOffset_records = (int *)calloc(quantRecordsMax, sizeof(int));
    if(!lista_posOffset_records){
        printf("Erro: alocação de memória busca de registros.\n");
        return NULL;
    }

    fseek(bin, 17, SEEK_SET); // Posiciona o cursor no primeiro registro

    Record *r;

    while((r = record_read_from_file(bin)) != NULL){
        int posRecord = ftell(bin) - 80;

        if (record_get_removido(r) == '1'){
            free_record(&r);
            continue;
        }

        if(atende_criterios(r, criterios, m)){
            if(quantRecords >= quantRecordsMax){
                quantRecordsMax *= 2;
                lista_posOffset_records = realloc(lista_posOffset_records, quantRecordsMax * sizeof(int));
            }
            lista_posOffset_records[quantRecords++] = posRecord;
        }
        
        free_record(&r);
    }

    *quantRecordEncontrados = quantRecords;
    return lista_posOffset_records;
}

Criterio *input_criterios(int m){
    Criterio *criterios = malloc(sizeof(Criterio) * m);
        
    for(int j = 0; j < m; j++){
        scanf("%s", criterios[j].nomeCampo);
        // scan_quote_string lida com aspas em strings
        ScanQuoteString(criterios[j].valorCampo); 
    }

    return criterios;
}

bool atende_criterios(Record *r, Criterio *criterios, int m){
    for(int i = 0; i < m; i++){
        if (strncmp(criterios[i].nomeCampo, "codEstacao", 10) == 0) {
            if (record_get_codEstacao(r) != atoi(criterios[i].valorCampo))
                return false;
        } 
        else if (strncmp(criterios[i].nomeCampo, "codLinhaIntegra", 15) == 0) {
            int val = (strcmp(criterios[i].valorCampo, "") == 0) ? -1 : atoi(criterios[i].valorCampo);
            if (record_get_codLinhaIntegra(r) != val) 
                return false;
        } 
        else if (strncmp(criterios[i].nomeCampo, "codLinha", 8) == 0) {
            if (record_get_codLinha(r) != atoi(criterios[i].valorCampo)) 
                return false;
        } 
        else if (strncmp(criterios[i].nomeCampo, "codProxEstacao", 14) == 0) {
            int val = (strcmp(criterios[i].valorCampo, "") == 0) ? -1 : atoi(criterios[i].valorCampo);
            if (record_get_codProxEstacao(r) != val) 
                return false;
        } 
        else if (strncmp(criterios[i].nomeCampo, "distProxEstacao", 15) == 0) {
            int val = (strcmp(criterios[i].valorCampo, "") == 0) ? -1 : atoi(criterios[i].valorCampo);
            if (record_get_distProxEstacao(r) != val) 
                return false;
        } 
        else if (strncmp(criterios[i].nomeCampo, "codEstIntegra", 13) == 0) {
            int val = (strcmp(criterios[i].valorCampo, "") == 0) ? -1 : atoi(criterios[i].valorCampo);
            if (record_get_codEstIntegra(r) != val) 
                return false;
        } 
        else if (strncmp(criterios[i].nomeCampo, "nomeEstacao", 11) == 0) {
            char *nome = record_get_nomeEstacao(r);
            if (nome == NULL || strcmp(nome, criterios[i].valorCampo) != 0) 
                return false;
        } 
        else if (strncmp(criterios[i].nomeCampo, "nomeLinha", 9) == 0) {
            char *nome = record_get_nomeLinha(r);
            if (nome == NULL || strcmp(nome, criterios[i].valorCampo) != 0)     
                return false;
        }
    }
    return true;
}

void update_register(Record *r, Criterio *atualizar, int p){
    for(int i = 0; i < p; i++){
        if (strncmp(atualizar[i].nomeCampo, "codEstacao", 10) == 0) {
            record_set_codEstacao(r, atoi(atualizar[i].valorCampo));
        } 
        else if (strncmp(atualizar[i].nomeCampo, "codLinhaIntegra", 15) == 0) {
            int val = (strcmp(atualizar[i].valorCampo, "") == 0) ? -1 : atoi(atualizar[i].valorCampo);
            record_set_codLinhaIntegra(r, val);
        } 
        else if (strncmp(atualizar[i].nomeCampo, "codLinha", 8) == 0) {
            record_set_codLinha(r, atoi(atualizar[i].valorCampo));
        } 
        else if (strncmp(atualizar[i].nomeCampo, "codProxEstacao", 14) == 0) {
            int val = (strcmp(atualizar[i].valorCampo, "") == 0) ? -1 : atoi(atualizar[i].valorCampo);
            record_set_codProxEstacao(r, val);
        } 
        else if (strncmp(atualizar[i].nomeCampo, "distProxEstacao", 15) == 0) {
            int val = (strcmp(atualizar[i].valorCampo, "") == 0) ? -1 : atoi(atualizar[i].valorCampo);
            record_set_distProxEstacao(r, val);
        } 
        else if (strncmp(atualizar[i].nomeCampo, "codEstIntegra", 13) == 0) {
            int val = (strcmp(atualizar[i].valorCampo, "") == 0) ? -1 : atoi(atualizar[i].valorCampo);
            record_set_codEstIntegra(r, val);
        } 
        else if (strncmp(atualizar[i].nomeCampo, "nomeEstacao", 11) == 0) {
            record_set_nomeEstacao(r, atualizar[i].valorCampo);
        } 
        else if (strncmp(atualizar[i].nomeCampo, "nomeLinha", 9) == 0) {
            record_set_nomeLinha(r, atualizar[i].valorCampo);
        }
    }
}

int input_inteiro_ou_nulo() {
    char buffer[20];
    if (scanf("%s", buffer) != 1) return -1;
    if (strcmp(buffer, "NULO") == 0) return -1;
    return atoi(buffer);
}