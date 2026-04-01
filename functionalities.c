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

// Estrutura auxiliar para armazenar os critérios de busca
typedef struct {
    char nomeCampo[50];
    char valorCampo[100];
} Criterio;

char* get_field(char **line);

char **criar_lista_nomesEstacoes(int nroMaxEstacoes);
void free_lista_nomesEstacoes(char ***lista, int nroEstacoes);
paresEstacoes *criar_lista_paresEstacoes(int nroMaxPares);
void free_lista_paresEstacoes(paresEstacoes **lista);

int ler_campoFixo(FILE *bin, int posRecord, int posOffset);
char *ler_campoVariavel(FILE *bin, int posRecord, int posOffset);
void printar_record_object(Record *r);
void printar_record(FILE *bin, int posRecord);
bool status_esta_instavel(FILE *bin);
bool esta_removido(FILE *bin, int posRecord);

// Função para verificar se um registro atende aos critérios
bool atende_criterios(Record *r, Criterio *criterios, int m);
void ScanQuoteString(char *str);

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
        int m;
        scanf(" %d", &m);
        Criterio *criterios = malloc(sizeof(Criterio) * m);
        
        for(int j = 0; j < m; j++){
            scanf("%s", criterios[j].nomeCampo);
            // scan_quote_string lida com aspas em strings
            ScanQuoteString(criterios[j].valorCampo); 
        }

        printf("Busca %d\n", i+1);
        fseek(bin, 17, SEEK_SET);

        bool encontrado = false;
        Record *r;

        while((r = record_read_from_file(bin)) != NULL){
            if (record_get_removido(r) == "1"){
                free_record(&r);
                continue;
            }

            if(atende_criterios(r, criterios, m)){
                printar_record_object(r);
                encontrado=true;
            }
            free_record(&r);
        }
        if(!encontrado){
            printf("Registro inexistente.\n");
        }
        free(criterios);
    }
    fclose(bin);
}


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

    // 4. nomeLinha
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


bool atende_criterios(Record *r, Criterio *criterios, int m){
    for(int i = 0; i < m; i++){
        if (strncmp(criterios[i].nomeCampo, "codEstacao", 10) == 0) {
            if (record_get_codEstacao(r) != atoi(criterios[i].valorCampo)) 
                return false;
        } 
        else if (strncmp(criterios[i].nomeCampo, "codLinha", 8) == 0) {
            if (record_get_codLinha(r) != atoi(criterios[i].valorCampo)) 
                return false;
        } 
        else if (strncmp(criterios[i].nomeCampo, "codProxEstacao", 14) == 0) {
            int val = (strcmp(criterios[i].valorCampo, "NULO") == 0) ? -1 : atoi(criterios[i].valorCampo);
            if (record_get_codProxEstacao(r) != val) 
                return false;
        } 
        else if (strncmp(criterios[i].nomeCampo, "distProxEstacao", 15) == 0) {
            int val = (strcmp(criterios[i].valorCampo, "NULO") == 0) ? -1 : atoi(criterios[i].valorCampo);
            if (record_get_distProxEstacao(r) != val) 
                return false;
        } 
        else if (strncmp(criterios[i].nomeCampo, "codLinhaIntegra", 15) == 0) {
            int val = (strcmp(criterios[i].valorCampo, "NULO") == 0) ? -1 : atoi(criterios[i].valorCampo);
            if (record_get_codLinhaIntegra(r) != val) 
                return false;
        } 
        else if (strncmp(criterios[i].nomeCampo, "codEstIntegra", 13) == 0) {
            int val = (strcmp(criterios[i].valorCampo, "NULO") == 0) ? -1 : atoi(criterios[i].valorCampo);
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

void ScanQuoteString(char *str) {
    char R;

    while ((R = getchar()) != EOF && isspace(R))
        ; // ignorar espaços, \r, \n...

    if (R == 'N' || R == 'n') { // campo NULO
        getchar();
        getchar();
        getchar();       // ignorar o "ULO" de NULO.
        strcpy(str, ""); // copia string vazia
    } else if (R == '\"') {
        if (scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
            strcpy(str, "");
        }
        getchar();         // ignorar aspas fechando
    } else if (R != EOF) { // vc tá tentando ler uma string que não tá entre
                           // aspas! Fazer leitura normal %s então, pois deve
                           // ser algum inteiro ou algo assim...
        str[0] = R;
        scanf("%s", &str[1]);
    } else { // EOF
        strcpy(str, "");
    }
}