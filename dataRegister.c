#include "dataRegister.h"

struct dataRegister{
    char removido;
    int proximo;
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;
    char *nomeEstacao;
    int tamNomeLinha;
    char *nomeLinha;
};

Record *create_record() {
    Record *r = (Record*) malloc(sizeof(Record));
    if (r == NULL) {
        return NULL;
    }

    // Inicialização de controle de registros
    r->removido = '0';          // Inicia como não removido
    r->proximo = -1;            // Valor padrão para ausência de próximo 

    // Campos de tamanho fixo: Inicializados com -1 para representar valores nulos
    r->codEstacao = -1;
    r->codLinha = -1;
    r->codProxEstacao = -1;
    r->distProxEstacao = -1;
    r->codLinhaIntegra = -1;
    r->codEstIntegra = -1;

    // Campos de tamanho variável: Inicializados com tamanho zero e ponteiro nulo
    r->tamNomeEstacao = 0;
    r->nomeEstacao = NULL;
    r->tamNomeLinha = 0;
    r->nomeLinha = NULL;

    return r;
}

void record_set_removido(Record *r, char removido) {
    if (r) r->removido = removido; // Define se o registro está ativo ou removido
}

void record_set_proximo(Record *r, int proximo) {
    if (r) r->proximo = proximo; // Define o RRN para a pilha de removidos
}

void record_set_codEstacao(Record *r, int cod) {
    if (r) r->codEstacao = cod; // Campo obrigatório; não aceita nulo no CSV
}

void record_set_codLinha(Record *r, int cod) {
    if (r) r->codLinha = cod; // Código da linha correspondente
}

void record_set_codProxEstacao(Record *r, int cod) {
    if (r) r->codProxEstacao = cod; // Aceita valor nulo representado por -1 
}

void record_set_distProxEstacao(Record *r, int dist) {
    if (r) r->distProxEstacao = dist; // Distância para a próxima estação
}

void record_set_codLinhaIntegra(Record *r, int cod) {
    if (r) r->codLinhaIntegra = cod; // Código da linha de integração
}

void record_set_codEstIntegra(Record *r, int cod) {
    if (r) r->codEstIntegra = cod; // Código da estação de integração
}

// Setters de Campos Variáveis

void record_set_nomeEstacao(Record *r, char *nome) {
    if (!r) return;

    // Libera memória prévia se houver para evitar memory leaks
    if (r->nomeEstacao) {
        free(r->nomeEstacao);
        r->nomeEstacao = NULL;
    }

    // Campos nulos em variáveis devem ter tamanho zero 
    if (nome == NULL || strcmp(nome, "") == 0) {
        r->tamNomeEstacao = 0;
        r->nomeEstacao = NULL;
    } else {
        r->tamNomeEstacao = (int)strlen(nome);
        r->nomeEstacao = (char*) malloc(r->tamNomeEstacao + 1);
        if (r->nomeEstacao) {
            strcpy(r->nomeEstacao, nome);
        }
    }
}

void record_set_nomeLinha(Record *r, char *nome) {
    if (!r) return;

    if (r->nomeLinha) {
        free(r->nomeLinha);
        r->nomeLinha = NULL;
    }

    // Se o valor for nulo, armazena tamanho zero no indicador 
    if (nome == NULL || strcmp(nome, "") == 0) {
        r->tamNomeLinha = 0;
        r->nomeLinha = NULL;
    } else {
        r->tamNomeLinha = (int)strlen(nome);
        r->nomeLinha = (char*) malloc(r->tamNomeLinha + 1);
        if (r->nomeLinha) {
            strcpy(r->nomeLinha, nome);
        }
    }
}

// Getters

int record_get_codEstacao(Record *r) { 
    return r ? r->codEstacao : -1; 
}

int record_get_codLinha(Record *r) { 
    return r ? r->codLinha : -1; 
}

int record_get_codProxEstacao(Record *r) { 
    return r ? r->codProxEstacao : -1; 
}

int record_get_distProxEstacao(Record *r) { 
    return r ? r->distProxEstacao : -1; 
}

int record_get_codLinhaIntegra(Record *r) { 
    return r ? r->codLinhaIntegra : -1; 
}

int record_get_codEstIntegra(Record *r) { 
    return r ? r->codEstIntegra : -1; 
}

char* record_get_nomeEstacao(Record *r) { 
    return r ? r->nomeEstacao : NULL; 
}

char* record_get_nomeLinha(Record *r) { 
    return r ? r->nomeLinha : NULL; 
}

Record* record_read_from_file(FILE *fp){
    Record *r = create_record();
    if(fread(&r->removido, sizeof(char), 1, fp) != 1){
        free(&r);
        return NULL;
    }
    fread(&r->proximo, sizeof(int), 1, fp);
    fread(&r->codEstacao, sizeof(int), 1, fp);
    fread(&r->codLinha, sizeof(int), 1, fp);
    fread(&r->codProxEstacao, sizeof(int), 1, fp);
    fread(&r->distProxEstacao, sizeof(int), 1, fp);
    fread(&r->codLinhaIntegra, sizeof(int), 1, fp);
    fread(&r->codEstIntegra, sizeof(int), 1, fp);

    fread(&r->tamNomeEstacao, sizeof(int), 1, fp);
    if (r->tamNomeEstacao > 0) {
        r->nomeEstacao = malloc(r->tamNomeEstacao + 1);
        fread(r->nomeEstacao, 1, r->tamNomeEstacao, fp);
        r->nomeEstacao[r->tamNomeEstacao] = '\0';
    }

    fread(&r->tamNomeLinha, sizeof(int), 1, fp);
    if (r->tamNomeLinha > 0) {
        r->nomeLinha = malloc(r->tamNomeLinha + 1);
        fread(r->nomeLinha, 1, r->tamNomeLinha, fp);
        r->nomeLinha[r->tamNomeLinha] = '\0';
    }
    // Pular o lixo para chegar ao próximo registro
    int ocupado = 37 + r->tamNomeEstacao + r->tamNomeLinha;
    fseek(fp, 80 - ocupado, SEEK_CUR);

    return r;
}
void record_write_to_file(FILE *fp, Record *r) {
    // Escrita dos campos fixos
    fwrite(&r->removido, sizeof(char), 1, fp);
    fwrite(&r->proximo, sizeof(int), 1, fp);
    fwrite(&r->codEstacao, sizeof(int), 1, fp);
    fwrite(&r->codLinha, sizeof(int), 1, fp);
    fwrite(&r->codProxEstacao, sizeof(int), 1, fp);
    fwrite(&r->distProxEstacao, sizeof(int), 1, fp);
    fwrite(&r->codLinhaIntegra, sizeof(int), 1, fp);
    fwrite(&r->codEstIntegra, sizeof(int), 1, fp);

    // Escrita dos campos variáveis com indicadores de tamanho
    fwrite(&r->tamNomeEstacao, sizeof(int), 1, fp);
    if(r->tamNomeEstacao > 0) fwrite(r->nomeEstacao, 1, r->tamNomeEstacao, fp);

    fwrite(&r->tamNomeLinha, sizeof(int), 1, fp);
    if(r->tamNomeLinha > 0) fwrite(r->nomeLinha, 1, r->tamNomeLinha, fp);

    // Preenchimento com lixo '$' até completar 80 bytes 
    int ocupado = 37 + r->tamNomeEstacao + r->tamNomeLinha;
    char lixo = '$';
    for(int i = ocupado; i < 80; i++) {
        fwrite(&lixo, sizeof(char), 1, fp);
    }
}

void free_record(Record **r) {
    if (*r == NULL) {
        return;
    }

    if ((*r)->nomeEstacao != NULL) {
        free((*r)->nomeEstacao);
    }
    
    if ((*r)->nomeLinha != NULL) {
        free((*r)->nomeLinha);
    }

    free(*r);
    *r = NULL;
}