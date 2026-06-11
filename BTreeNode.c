#include "BTreeNode.h"
#define ORDEM 4
#define MAX_CHAVES (ORDEM - 1)

typedef struct ParNo{
    int chave;
    int ponteiro;
} ParNo;

struct BTreeNode{
    char removido;
    int proximo;
    int tipoNo;
    int nroChaves;
    ParNo chaves[MAX_CHAVES];
    int ponteiros[ORDEM];
};

// O tipoNo é -1 para folha, 0 para raiz e 1 para nó intermediário
NodeB *create_btree_node(int tipoNo){
    NodeB *node = (NodeB *)calloc(1, sizeof(NodeB));
    if(node == NULL)
        return NULL;

    node->removido = '0';
    node->proximo = -1;
    node->tipoNo = tipoNo;
    node->nroChaves = 0;

    node->chaves[0] = (ParNo){-1, -1};
    node->chaves[1] = (ParNo){-1, -1};
    node->chaves[2] = (ParNo){-1, -1};

    for(int i = 0; i < ORDEM; i++){
        node->ponteiros[i] = -1;
    }

    return node;
}

char btree_node_get_removido(NodeB *node){
    if(node) return node->removido;
    return '0';
}

int btree_node_get_proximo(NodeB *node){
    if(node) return node->proximo;
    return -1;
}

int btree_node_get_tipoNo(NodeB *node){
    if(node) return node->tipoNo;
    return -1;
}

int btree_node_get_nroChaves(NodeB *node){
    if(node) return node->nroChaves;
    return -1;
}

void btree_node_set_removido(NodeB *node, char removido){
    if(node) node->removido = removido;
}

void btree_node_set_proximo(NodeB *node, int proximo){
    if(node) node->proximo = proximo;
}

void btree_node_set_tipoNo(NodeB *node, int tipoNo){
    if(node) node->tipoNo = tipoNo;
}

void btree_node_set_nroChaves(NodeB *node, int nroChaves){
    if(node) node->nroChaves = nroChaves;
}

void bree_node_set_filho_inicial(NodeB *node, int filho){
    if(node) node->ponteiros[0] = filho;
}

int btree_node_buscar_chave(NodeB *node, int chave){
    if(node == NULL)
        return -1;

    for(int i = 0; i < node->nroChaves; i++){
        if(node->chaves[i].chave == chave){
            return i; // Retorna o indice
        }
    }
    // Se nao achou a chave
    return -1;
}

bool btree_node_inserir_chave(NodeB *node, int chave, int ponteiro, int filho){
    if(node == NULL)
        return false;

    // Verifica se a chave já existe no nó
    if(btree_node_buscar_chave(node, chave) != -1){
        return false;
    }
    
    // Insere a chave e o ponteiro ordenadamente shiftando as chaves maiores para a direita
    if(node->nroChaves < MAX_CHAVES){
        int i = node->nroChaves - 1;
        while(i >= 0 && node->chaves[i].chave > chave){
            node->chaves[i + 1] = node->chaves[i];
            node->ponteiros[i + 2] = node->ponteiros[i + 1];
            i--;
        }
        node->chaves[i + 1].chave = chave;
        node->chaves[i + 1].ponteiro = ponteiro;
        node->ponteiros[i + 2] = filho;
    } 
    else {
        return false;
    }

    node->nroChaves++;
    return true;
}

bool btree_node_remover_chave(NodeB *node, int indice){
    if(node == NULL)
        return false;

    if(node->nroChaves == 0 || indice < 0 || indice >= node->nroChaves){
        return false;
    }

    // remove shiftando as chaves maiores para a esquerda
    for(int j = indice; j < node->nroChaves - 1; j++){
        node->chaves[j] = node->chaves[j + 1];
        node->ponteiros[j + 1] = node->ponteiros[j + 2];
    }
    node->nroChaves--;
    node->chaves[node->nroChaves] = (ParNo){-1, -1};
    node->ponteiros[node->nroChaves + 1] = -1;
    return true;
}

void btree_node_write_to_file(FILE *fp, NodeB *node){
    if(fp == NULL || node == NULL)
        return;

    fwrite(&node->removido, sizeof(char), 1, fp);
    fwrite(&node->proximo, sizeof(int), 1, fp);
    fwrite(&node->tipoNo, sizeof(int), 1, fp);
    fwrite(&node->nroChaves, sizeof(int), 1, fp);

    for(int i = 0; i < MAX_CHAVES; i++){
        fwrite(&node->chaves[i].chave, sizeof(int), 1, fp);
        fwrite(&node->chaves[i].ponteiro, sizeof(int), 1, fp);
    }

    for(int i = 0; i < ORDEM; i++){
        fwrite(&node->ponteiros[i], sizeof(int), 1, fp);
    }
}

NodeB *btree_node_read_from_file_at_offset(FILE *fp, int byteOffset){
    if(fp == NULL){
        return NULL;
    }

    NodeB *node = (NodeB *)calloc(1, sizeof(NodeB));
    if(node == NULL){
        return NULL;
    }

    fseek(fp, byteOffset, SEEK_SET);

    fread(&node->removido, sizeof(char), 1, fp);
    fread(&node->proximo, sizeof(int), 1, fp);
    fread(&node->tipoNo, sizeof(int), 1, fp);
    fread(&node->nroChaves, sizeof(int), 1, fp);

    for(int i = 0; i < MAX_CHAVES; i++){
        fread(&node->chaves[i].chave, sizeof(int), 1, fp);
        fread(&node->chaves[i].ponteiro, sizeof(int), 1, fp);
    }

    for(int i = 0; i < ORDEM; i++){
        fread(&node->ponteiros[i], sizeof(int), 1, fp);
    }

    return node;
}

void free_btree_node(NodeB **node){
    if(node == NULL || *node == NULL)
        return;

    free(*node);
    *node = NULL;
}

int btree_node_get_child_rrn(NodeB *node, int chave) {
    if(node == NULL) return -1;
    int i;
    for (i = 0; i < node->nroChaves; i++) {
        if (chave < node->chaves[i].chave) {
            break;
        }
    }
    return node->ponteiros[i];
}