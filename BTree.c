#include "BTree.h"
#include "utils.h"
#include "dataRegister.h"
#include "headerRegister.h"

struct NodeSearchResult{
    bool found;
    int rrn;
    int rrnPai;
    int indice;
};

NodeSearch *btree_search_recursive(FILE *btree, int rrnAtual, int rrnPai, int chave_busca){
    if(rrnAtual == -1){
        NodeSearch *result = create_btree_nodeSearch(false, -1, rrnPai, -1);
        return result;
    }
    
    int byteOffset = 17 + rrnAtual * 53;
    NodeB *node = btree_node_read_from_file_at_offset(btree, byteOffset);
    if (node == NULL){
        NodeSearch *result = create_btree_nodeSearch(false, -1, rrnPai, -1);
        return result;
    }    

    // Verifica se a chave está neste nó
    int indice = btree_node_buscar_chave(node, chave_busca);
    if (indice != -1) {
        free_btree_node(&node);
        NodeSearch *result = create_btree_nodeSearch(true, rrnAtual, rrnPai, indice);
        return result;
    }
    
    // Se for folha e não achou, a chave não existe na árvore
    if (btree_node_get_tipoNo(node) == -1) {
        free_btree_node(&node);
        NodeSearch *result = create_btree_nodeSearch(false, -1, rrnPai, -1);
        return result;
    }

    // Caso não seja folha, encontra o filho correto e continua a busca
    int filho_rrn = btree_node_get_child_rrn(node, chave_busca);
    free_btree_node(&node);

    return btree_search_recursive(btree, filho_rrn, rrnAtual, chave_busca);
}

int alocar_rrn_novo_no(FILE *btree, HeaderBTree *headerB){
    int topo = btree_header_get_topo(headerB);
    if(topo == -1){
        btree_header_set_nroNos(headerB, btree_header_get_nroNos(headerB) + 1);
        return btree_header_get_proxRRN(headerB);
    }

    int posOffset = 17 + topo * 53;

    NodeB* node = btree_node_read_from_file_at_offset(btree, posOffset);
    int novoTopo = btree_node_get_proximo(node);

    btree_header_set_nroNos(headerB, btree_header_get_nroNos(headerB) + 1);
    btree_header_set_topo(headerB, novoTopo);
    
    free_btree_node(&node);

    return topo;
}

void remover_logicamente_no(FILE *btree, HeaderBTree *headerB, int rrnNo){
    int antigoTopo = btree_header_get_topo(headerB);
    int posOffset = 17 + rrnNo * 53;

    NodeB *node = btree_node_read_from_file_at_offset(btree, posOffset);
    btree_node_set_removido(node, '1');
    btree_node_set_proximo(node, antigoTopo);
    btree_header_set_nroNos(headerB, btree_header_get_nroNos(headerB) - 1);

    fseek(btree, posOffset, SEEK_SET);
    btree_node_write_to_file(btree, node);
    
    btree_header_set_topo(headerB, rrnNo);

    free_btree_node(&node);
}

NodeSearch *create_btree_nodeSearch(bool found, int rrnfilho, int rrnPai, int indice){
    NodeSearch *nodeSearch = (NodeSearch *)calloc(1, sizeof(NodeSearch));
    if(nodeSearch == NULL)
        return NULL;

    nodeSearch->found = found;
    nodeSearch->rrn = rrnfilho;
    nodeSearch->rrnPai = rrnPai;
    nodeSearch->indice = indice;
    
    return nodeSearch;
}

bool btree_nodeSearch_get_found(NodeSearch *nodeSearch){
    if(nodeSearch) return nodeSearch->found;
    return false;
}

int btree_nodeSearch_get_rrn(NodeSearch *nodeSearch){
    if(nodeSearch) return nodeSearch->rrn;
    return -1;
}

int btree_nodeSearch_get_indice(NodeSearch *nodeSearch){
    if(nodeSearch) return nodeSearch->indice;
    return -1;
}

void free_search_result(NodeSearch **result) {
    if (result && *result) {
        free(*result);
        *result = NULL;
    }
}