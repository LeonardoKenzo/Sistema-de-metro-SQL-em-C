#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "headerRegister.h"
#include "dataRegister.h"
#include "fornecidas.h"
#include "utils.h"

struct Criterios{
    char nomeCampo[50];
    char valorCampo[100];
};

// Estrutura para armazenar o contexto de busca, como cada funcionalidade vai usar a busca
struct Contextos{
    FILE *bin;
    Header *header;

    // coisas usadas em update
    Criterio *atualizar;
    int p;

    // para pausar a busca apos encontrar um registro
    bool pararBusca;
};

// Procura por registros que atendem aos criterios e executa a funcionalidade passada como parametro
bool search_records(FILE *bin, Criterio *criterios, int m, void(*funcionalidade)(Record *r, int posRecord, Contexto *ctx), Contexto *ctx){
    if(ctx == NULL){
        ctx = criar_contexto(bin, NULL, NULL, 0);
    }

    bool encontrou = false;
   
    fseek(bin, 17, SEEK_SET); // Posiciona o cursor no primeiro registro

    // Percorre os registros do arquivo  
    int RRN = 0, posRecord = 0;
    Record *r;
    char removido;

    while(fread(&removido, sizeof(char), 1, bin) == 1 && ctx->pararBusca == false){
        posRecord = 17 + RRN * 80;
        
        // Se estiver removido, pula para o próximo
        if(removido == '1'){
            RRN++;
            posRecord = 17 + RRN * 80;
            fseek(bin, posRecord, SEEK_SET);
            continue;
        }

        // Lê o registro e verifica se atende aos critérios de busca
        fseek(bin, posRecord, SEEK_SET);
        r = record_read_from_file(bin);
        if(r == NULL){
            RRN++;
            continue;
        }
        if(atende_criterios(r, criterios, m)){
            encontrou = true;
            funcionalidade(r, posRecord, ctx); // Executa a funcionalidade passada
        }

        // Garante que o próximo registro será lido no próximo loop
        RRN++;
        posRecord = 17 + RRN * 80;
        fseek(bin, posRecord, SEEK_SET);
        
        free_record(&r);
    }

    return encontrou;
}

// Verifica se um registro atende aos criterios de busca
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

// Verifica se um registro esta marcado como removido no arquivo
bool esta_removido(FILE *bin, int posRecord){
    fseek(bin, posRecord, SEEK_SET);

    char removido;
    fread(&removido, sizeof(char), 1, bin);

    if(removido == '1')
        return true;

    return false;
}

// Define os valores de nroEstacoes e nroParesEstacoes no header a partir dos registros do arquivo
void set_header_estacoes_unicas(FILE *bin, Header *h){
    int nroMaxPares = header_get_proxRRN(h), nroMaxNomes = header_get_proxRRN(h), nroEstacoes = 0, nroPares = 0;
    char **lista_nomesEstacoes = criar_lista_nomesEstacoes(nroMaxNomes);
    paresEstacoes *lista_paresEstacoes = criar_lista_paresEstacoes(nroMaxPares);

    achar_todos_estacoes(bin, &lista_paresEstacoes, &nroPares, &nroMaxPares, &lista_nomesEstacoes, &nroEstacoes, &nroMaxNomes);
    
    header_set_nroEstacoes(h, nroEstacoes);
    header_set_nroParesEst(h, nroPares);

    free_lista_nomesEstacoes(&lista_nomesEstacoes, nroEstacoes);
    free_lista_paresEstacoes(&lista_paresEstacoes);
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

// Imprime um registro formatado, tratando campos nulos
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

// Imprime um registro formatado a partir de sua posição no arquivo, tratando campos nulos
void printar_record(FILE *bin, int posRecord){
    fseek(bin, posRecord, SEEK_SET);
    
    // Em vez de dar printf campo a campo
    Record *r = record_read_from_file(bin); // Lê e cria o objeto
    
    if (r != NULL) {
        printar_record_object(r); // Usa a lógica centralizada de exibição
        free_record(&r);        
    }
}

// Atualiza os campos de um registro com base nos criterios de atualização
void update_campos(Record *r, Criterio *atualizar, int p){
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

// Verifica se o status do arquivo é instável (status '0' indica instável)
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

// Funcao auxiliar para ler campos fixos dos records do file BIN
int ler_campoFixo(FILE *bin, int posRecord, int posOffset){
    fseek(bin, posRecord + posOffset, SEEK_SET);

    int campoFixo;
    fread(&campoFixo, sizeof(int), 1, bin);

    return campoFixo;
}

// Funcão auxiliar para ler inteiros que podem ser "NULO" da entrada padrão
int input_inteiro_ou_nulo() {
    char buffer[20];
    if (scanf("%s", buffer) != 1) return -1;
    if (strcmp(buffer, "NULO") == 0) return -1;
    return atoi(buffer);
}

// Manipulação do contexto da busca dos registros de dados do arquivo binário
Criterio *input_criterios(int m){
    Criterio *criterios = malloc(sizeof(Criterio) * m);
        
    for(int j = 0; j < m; j++){
        scanf("%s", criterios[j].nomeCampo);
        // scan_quote_string lida com aspas em strings
        ScanQuoteString(criterios[j].valorCampo); 
    }

    return criterios;
}

Contexto *criar_contexto(FILE *bin, Header *header, Criterio *atualizar, int p){
    Contexto *ctx = malloc(sizeof(Contexto));
    if (ctx == NULL){
        return NULL;
    }
    ctx->bin = bin;
    ctx->header = header;
    ctx->atualizar = atualizar;
    ctx->p = p;
    ctx->pararBusca = false;
    return ctx;
}

Contexto *atualizar_contexto(Contexto *ctx, Criterio *atualizar, int p) {
    ctx->atualizar = atualizar;
    ctx->p = p;
    return ctx;
}

Contexto *pausar_busca(Contexto *ctx) {
    ctx->pararBusca = true;
    return ctx;
}

FILE *get_file_from_context(Contexto *ctx) {
    return ctx->bin;
}

Header *get_header_from_context(Contexto *ctx) {
    return ctx->header;
}

Criterio *get_atualizar_from_context(Contexto *ctx) {
    return ctx->atualizar;
}

int get_p_from_context(Contexto *ctx) {
    return ctx->p;
}

