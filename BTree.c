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

// Função de inserção
bool btree_insert_recursive(FILE *btree, HeaderBTree *h, int rrn_atual, int chave_in, int ponteiro_in, 
                            int *chave_pro, int *ponteiro_pro, int *filho_direito_pro) {
    if (rrn_atual == -1) {
        return false; 
    }

    NodeB *node = btree_node_read_from_file_at_offset(btree, 17 + rrn_atual * 53);

    // Evita duplicados de chave primária
    if (btree_node_buscar_chave(node, chave_in) != -1) {
        free_btree_node(&node);
        return false;
    }

    bool result_split = false;

    if (btree_node_get_tipoNo(node) == -1) {
        // É uma folha, inserir direto
        NodeB *novo_no = NULL;
        result_split = btree_node_insert_and_split(node, chave_in, ponteiro_in, -1, 
                                                   chave_pro, ponteiro_pro, &novo_no);
        
        if (result_split) {
            int rrn_novo = alocar_rrn_novo_no(btree, h);
            fseek(btree, 17 + rrn_novo * 53, SEEK_SET);
            btree_node_write_to_file(btree, novo_no);
            free_btree_node(&novo_no);
            *filho_direito_pro = rrn_novo; // Passa o novo RRN para cima
        }
    } else {
        // Não é folha, desce pelo ponteiro filho adequado
        int filho_rrn = btree_node_get_child_rrn(node, chave_in);
        
        int p_chave, p_ptr, p_filho;
        bool promoted = btree_insert_recursive(btree, h, filho_rrn, chave_in, ponteiro_in, &p_chave, &p_ptr, &p_filho);

        if (!promoted) {
            free_btree_node(&node);
            return false;
        }

        // O filho sofreu um split e promoveu uma chave
        NodeB *novo_no = NULL;
        result_split = btree_node_insert_and_split(node, p_chave, p_ptr, p_filho, 
                                                   chave_pro, ponteiro_pro, &novo_no);
        
        if (result_split) {
            int rrn_novo = alocar_rrn_novo_no(btree, h);
            fseek(btree, 17 + rrn_novo * 53, SEEK_SET);
            btree_node_write_to_file(btree, novo_no);
            free_btree_node(&novo_no);
            *filho_direito_pro = rrn_novo;
        }
    }

    // Escreve as modificações do nó atual em disco
    fseek(btree, 17 + rrn_atual * 53, SEEK_SET);
    btree_node_write_to_file(btree, node);
    free_btree_node(&node);

    return result_split;
}

// Wrapper principal
void btree_insert(FILE *btree, HeaderBTree *h, int chave, int offset) {
    int noRaiz = btree_header_get_noRaiz(h);

    if (noRaiz == -1) {
        // Árvore vazia, a primeira inserção cria a folha/raiz inicial
        int rrn_novo = alocar_rrn_novo_no(btree, h);
        NodeB *raiz = create_btree_node(-1); // -1 = folha e raiz
        btree_node_inserir_chave(raiz, chave, offset, -1);
        
        fseek(btree, 17 + rrn_novo * 53, SEEK_SET);
        btree_node_write_to_file(btree, raiz);
        free_btree_node(&raiz);
        
        btree_header_set_noRaiz(h, rrn_novo);
        return;
    }

    int p_chave, p_ptr, p_filho;
    bool promoted = btree_insert_recursive(btree, h, noRaiz, chave, offset, &p_chave, &p_ptr, &p_filho);

    // Se o último retorno para a raiz for true, a raiz da árvore explodiu e é preciso criar um novo topo
    if (promoted) {
        int rrn_nova_raiz = alocar_rrn_novo_no(btree, h);
        NodeB *nova_raiz = create_btree_node(0); // 0 = Raiz exclusiva (não-folha)
        
        btree_node_inserir_chave(nova_raiz, p_chave, p_ptr, p_filho);
        bree_node_set_filho_inicial(nova_raiz, noRaiz); // O ponteiro 0 aponta para a raiz antiga

        fseek(btree, 17 + rrn_nova_raiz * 53, SEEK_SET);
        btree_node_write_to_file(btree, nova_raiz);
        free_btree_node(&nova_raiz);

        btree_header_set_noRaiz(h, rrn_nova_raiz);
    }
}