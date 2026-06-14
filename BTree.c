#include "BTree.h"
#include "utils.h"
#include "dataRegister.h"
#include "headerRegister.h"

struct NodeSearchResult{
    bool found;
    int rrn;
    int rrnPai;
    int indice;

    int caminho[8];
    int profundidade
};

NodeSearch *btree_search_recursive(FILE *btree, int rrnAtual, int rrnPai, int chave_busca, int *caminho, int profundidade){
    if(rrnAtual == -1){
        NodeSearch *result = create_btree_nodeSearch(false, -1, rrnPai, -1);
        memcpy(result->caminho, caminho, profundidade * sizeof(int));
        result->profundidade = profundidade;
        return result;
    }

    caminho[profundidade] = rrnAtual;
    
    int byteOffset = 17 + rrnAtual * 53;
    NodeB *node = btree_node_read_from_file_at_offset(btree, byteOffset);
    if (node == NULL){
        NodeSearch *result = create_btree_nodeSearch(false, -1, rrnPai, -1);
        result->profundidade = profundidade;
        return result;
    }    

    // Verifica se a chave está neste nó
    int indice = btree_node_buscar_chave(node, chave_busca);
    if (indice != -1) {
        free_btree_node(&node);
        NodeSearch *result = create_btree_nodeSearch(true, rrnAtual, rrnPai, indice);
        memcpy(result->caminho, caminho, (profundidade + 1) * sizeof(int));
        result->profundidade = profundidade + 1;
        return result;
    }
    
    // Se for folha e não achou, a chave não existe na árvore
    if (btree_node_get_tipoNo(node) == -1) {
        free_btree_node(&node);
        NodeSearch *result = create_btree_nodeSearch(false, -1, rrnPai, -1);
        memcpy(result->caminho, caminho, (profundidade + 1) * sizeof(int));
        result->profundidade = profundidade + 1;
        return result;
    }

    // Caso não seja folha, encontra o filho correto e continua a busca
    int filho_rrn = btree_node_get_child_rrn(node, chave_busca);
    free_btree_node(&node);

    return btree_search_recursive(btree, filho_rrn, rrnAtual, chave_busca, caminho, profundidade + 1);
}

int alocar_rrn_novo_no(FILE *btree, HeaderBTree *headerB){
    int topo = btree_header_get_topo(headerB);
    if(topo == -1){
        btree_header_set_nroNos(headerB, btree_header_get_nroNos(headerB) + 1);
        int proxRRN = btree_header_get_proxRRN(headerB);
        btree_header_set_proxRRN(headerB, proxRRN + 1);
        return proxRRN;
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

int btree_nodeSearch_get_rrnPai(NodeSearch *nodeSearch){
    if(nodeSearch) return nodeSearch->rrnPai;
    return -1;
}

int btree_nodeSearch_get_indice(NodeSearch *nodeSearch){
    if(nodeSearch) return nodeSearch->indice;
    return -1;
}

int btree_nodeSearch_get_profundidade(NodeSearch *nodeSearch){
    if(nodeSearch) return nodeSearch->profundidade;
    return -1;
}

int *btree_nodeSearch_get_caminho(NodeSearch *nodeSearch){
    if(nodeSearch) return nodeSearch->caminho;
    return -1;
}

void btree_nodeSearch_set_caminho(NodeSearch *nodeDestino, int *caminho, int size){
    if(nodeDestino) memcpy(nodeDestino->caminho, caminho, size);
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
        //btree_header_set_proxRRN(h, 1); | alocar_rrn_novo_no() já cuida de incrementar o proxRRN
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

NodeSearch *btree_find_successor(FILE *btree, int rrnAtual){
    int rrnPai = -1;

    while(rrnAtual != -1){

        int byteOffset = 17 + rrnAtual * 53;
        NodeB *node = btree_node_read_from_file_at_offset(btree, byteOffset);

        if(node == NULL)
            return create_btree_nodeSearch(false, -1, -1, -1);

        // Chegou em uma folha
        if(btree_node_get_tipoNo(node) == -1){
            free_btree_node(&node);
            return create_btree_nodeSearch(true, rrnAtual, rrnPai, 0);
        }

        // Continua pelo filho mais à esquerda
        rrnPai = rrnAtual;
        rrnAtual = btree_node_get_ponteiro(node, 0);

        free_btree_node(&node);
    }

    return create_btree_nodeSearch(false, -1, -1, -1);
}

bool btree_redistribute_right(FILE *btree, int rrnFilho, int rrnPai){
    int offsetPai = 17 + rrnPai * 53, offsetFilhoUnderflow = 17 + rrnFilho * 53;

    NodeB* nodePai = btree_node_read_from_file_at_offset(btree, offsetPai);
    NodeB *nodeUnderflow = btree_node_read_from_file_at_offset(btree, offsetFilhoUnderflow);
    
    int indiceFilhoDireito = -1, indiceChavePai;
    for(int i = 0; i <= btree_node_get_nroChaves(nodePai); i++){
        if(btree_node_get_ponteiro(nodePai, i) == rrnFilho){
            indiceFilhoDireito = i + 1;
            break;
        }
    }
    if(indiceFilhoDireito > btree_node_get_nroChaves(nodePai) || indiceFilhoDireito == -1){
        free_btree_node(&nodePai);
        free_btree_node(&nodeUnderflow);
        return false;
    }

    int offsetFilhoDireito = 17 + btree_node_get_ponteiro(nodePai, indiceFilhoDireito) * 53;
    NodeB *nodeDireito = btree_node_read_from_file_at_offset(btree, offsetFilhoDireito);
    indiceChavePai = indiceFilhoDireito - 1;

    // Verificacao de underflow
    int minChaves = (btree_ordem() - 1) / 2;
    if(btree_node_get_nroChaves(nodeDireito) <= minChaves){
        free_btree_node(&nodePai);
        free_btree_node(&nodeUnderflow);
        free_btree_node(&nodeDireito);
        return false;
    }
    
    // Insere a chave vinda do pai no no que estava em underflow
    if(btree_node_get_tipoNo(nodeUnderflow) == -1)
        btree_node_inserir_chave(nodeUnderflow, btree_node_get_chave(nodePai, indiceChavePai), btree_node_get_ponteiro_chave(nodePai, indiceChavePai), -1);
    else
        btree_node_inserir_chave(nodeUnderflow, btree_node_get_chave(nodePai, indiceChavePai), btree_node_get_ponteiro_chave(nodePai, indiceChavePai), btree_node_get_ponteiro(nodeDireito, 0));
    
    // Promove a primeira chave do no direito para o pai
    btree_node_set_parChave(nodePai, indiceChavePai, btree_node_get_chave(nodeDireito, 0), btree_node_get_ponteiro_chave(nodeDireito, 0));
    
    // Corrige os ponteiros
    int ponteiroSucessor = btree_node_get_ponteiro(nodeDireito, 1);
    btree_node_remover_chave(nodeDireito, 0);
    btree_node_set_ponteiro(nodeDireito, 0, ponteiroSucessor);

    fseek(btree, offsetPai, SEEK_SET);
    btree_node_write_to_file(btree, nodePai);

    fseek(btree, offsetFilhoUnderflow, SEEK_SET);
    btree_node_write_to_file(btree, nodeUnderflow);

    fseek(btree, offsetFilhoDireito, SEEK_SET);
    btree_node_write_to_file(btree, nodeDireito);

    free_btree_node(&nodePai);
    free_btree_node(&nodeUnderflow);
    free_btree_node(&nodeDireito);

    return true;
}

bool btree_redistribute_left(FILE *btree, int rrnFilho, int rrnPai){
    int offsetPai = 17 + rrnPai * 53, offsetFilhoUnderflow = 17 + rrnFilho * 53;

    NodeB* nodePai = btree_node_read_from_file_at_offset(btree, offsetPai);
    NodeB *nodeUnderflow = btree_node_read_from_file_at_offset(btree, offsetFilhoUnderflow);
    
    int indiceFilhoEsquerdo = -1, indiceChavePai;
    
    for(int i = 0; i <= btree_node_get_nroChaves(nodePai); i++){
        if(btree_node_get_ponteiro(nodePai, i) == rrnFilho){
            indiceFilhoEsquerdo = i - 1;
            break;
        }
    }
    if(indiceFilhoEsquerdo < 0){
        free_btree_node(&nodePai);
        free_btree_node(&nodeUnderflow);
        return false;
    }

    int offsetFilhoEsquerdo = 17 + btree_node_get_ponteiro(nodePai, indiceFilhoEsquerdo) * 53;
    NodeB *nodeEsquerdo = btree_node_read_from_file_at_offset(btree, offsetFilhoEsquerdo);
    
    indiceChavePai = indiceFilhoEsquerdo;

    // Verificacao de Underflow
    int minChaves = (btree_ordem() - 1) / 2;
    if(btree_node_get_nroChaves(nodeEsquerdo) <= minChaves){
        free_btree_node(&nodePai);
        free_btree_node(&nodeUnderflow);
        free_btree_node(&nodeEsquerdo);
        return false;
    }
    
    int ultIndiceEsquerdo = btree_node_get_nroChaves(nodeEsquerdo) - 1;

    // Se for no folha, apenas inserimos a chave do pai ordenadamente
    if(btree_node_get_tipoNo(nodeUnderflow) == -1) {
        btree_node_inserir_chave(nodeUnderflow, btree_node_get_chave(nodePai, indiceChavePai), btree_node_get_ponteiro_chave(nodePai, indiceChavePai), -1);
    } 
    // Se for no interno, precisamos herdar o ultimo ponteiro de filhos do no esquerdo
    else {
        int ponteiroEsquerdo = btree_node_get_ponteiro(nodeEsquerdo, ultIndiceEsquerdo + 1);
        int P0antigo = btree_node_get_ponteiro(nodeUnderflow, 0);
        
        btree_node_inserir_chave(nodeUnderflow, btree_node_get_chave(nodePai, indiceChavePai), btree_node_get_ponteiro_chave(nodePai, indiceChavePai), P0antigo);
        btree_node_set_ponteiro(nodeUnderflow, 0, ponteiroEsquerdo);
    }
    
    // Promove a ultima chave do no esquerdo para o pai
    btree_node_set_parChave(nodePai, indiceChavePai, btree_node_get_chave(nodeEsquerdo, ultIndiceEsquerdo), btree_node_get_ponteiro_chave(nodeEsquerdo, ultIndiceEsquerdo));
    btree_node_remover_chave(nodeEsquerdo, ultIndiceEsquerdo);

    fseek(btree, offsetPai, SEEK_SET);
    btree_node_write_to_file(btree, nodePai);

    fseek(btree, offsetFilhoUnderflow, SEEK_SET);
    btree_node_write_to_file(btree, nodeUnderflow);

    fseek(btree, offsetFilhoEsquerdo, SEEK_SET);
    btree_node_write_to_file(btree, nodeEsquerdo);

    free_btree_node(&nodePai);
    free_btree_node(&nodeUnderflow);
    free_btree_node(&nodeEsquerdo);

    return true;
}

bool btree_merge_right(FILE *btree, int rrnFilho, int rrnPai) {
    int offsetPai = 17 + rrnPai * 53, offsetFilhoUnderflow = 17 + rrnFilho * 53;

    NodeB* nodePai = btree_node_read_from_file_at_offset(btree, offsetPai);
    NodeB *nodeUnderflow = btree_node_read_from_file_at_offset(btree, offsetFilhoUnderflow);
    
    int indiceFilhoDireito = -1, indiceChavePai;
    for(int i = 0; i <= btree_node_get_nroChaves(nodePai); i++) {
        if(btree_node_get_ponteiro(nodePai, i) == rrnFilho) {
            indiceFilhoDireito = i + 1;
            break;
        }
    }
    
    // Se não houver irmao a direita nao da para fazer merge a direita
    if(indiceFilhoDireito > btree_node_get_nroChaves(nodePai)) {
        free_btree_node(&nodePai);
        free_btree_node(&nodeUnderflow);
        return false;
    }

    int rrnDireito = btree_node_get_ponteiro(nodePai, indiceFilhoDireito);
    int offsetFilhoDireito = 17 + rrnDireito * 53;
    NodeB *nodeDireito = btree_node_read_from_file_at_offset(btree, offsetFilhoDireito);
    
    indiceChavePai = indiceFilhoDireito - 1;

    // Desce a chave do pai para o nodeUnderflow
    btree_node_inserir_chave(nodeUnderflow, btree_node_get_chave(nodePai, indiceChavePai), btree_node_get_ponteiro_chave(nodePai, indiceChavePai), btree_node_get_ponteiro(nodeDireito, 0));

    // Copia todas as chaves e ponteiros do nodeDireito para o nodeUnderflow
    int nChavesDireito = btree_node_get_nroChaves(nodeDireito);
    for(int i = 0; i < nChavesDireito; i++) {
        btree_node_inserir_chave(nodeUnderflow, btree_node_get_chave(nodeDireito, i), btree_node_get_ponteiro_chave(nodeDireito, i), btree_node_get_ponteiro(nodeDireito, i + 1));
    }

    btree_node_remover_chave(nodePai, indiceChavePai);

    fseek(btree, offsetPai, SEEK_SET);
    btree_node_write_to_file(btree, nodePai);

    fseek(btree, offsetFilhoUnderflow, SEEK_SET);
    btree_node_write_to_file(btree, nodeUnderflow);

    // Remove o nodedireito
    HeaderBTree *headerB = create_btree_header();
    btree_header_read_from_file(btree, headerB);
    remover_logicamente_no(btree, headerB, rrnDireito);

    free_btree_node(&nodePai);
    free_btree_node(&nodeUnderflow);
    free_btree_node(&nodeDireito);

    return true;
}

bool btree_merge_left(FILE *btree, int rrnFilho, int rrnPai) {
    int offsetPai = 17 + rrnPai * 53, offsetFilhoUnderflow = 17 + rrnFilho * 53;

    NodeB* nodePai = btree_node_read_from_file_at_offset(btree, offsetPai);
    NodeB *nodeUnderflow = btree_node_read_from_file_at_offset(btree, offsetFilhoUnderflow);
    
    int indiceFilhoEsquerdo = -1, indiceChavePai;
    for(int i = 0; i <= btree_node_get_nroChaves(nodePai); i++) {
        if(btree_node_get_ponteiro(nodePai, i) == rrnFilho) {
            indiceFilhoEsquerdo = i - 1;
            break;
        }
    }
    
    // Se o no em underflow for o mais a esquerda nao ha irmao esquerdo para merge
    if(indiceFilhoEsquerdo < 0) {
        free_btree_node(&nodePai);
        free_btree_node(&nodeUnderflow);
        return false;
    }

    int rrnEsquerdo = btree_node_get_ponteiro(nodePai, indiceFilhoEsquerdo);
    int offsetFilhoEsquerdo = 17 + rrnEsquerdo * 53;
    NodeB *nodeEsquerdo = btree_node_read_from_file_at_offset(btree, offsetFilhoEsquerdo);
    
    indiceChavePai = indiceFilhoEsquerdo;

    // Desce a chave do pai para o nodeEsquerdo
    btree_node_inserir_chave(nodeEsquerdo, btree_node_get_chave(nodePai, indiceChavePai), btree_node_get_ponteiro_chave(nodePai, indiceChavePai), btree_node_get_ponteiro(nodeUnderflow, 0));

    // Copia todos os ponteiros e chaves do nodeUnderflow para o nodeEsquerdo
    int nChavesUnderflow = btree_node_get_nroChaves(nodeUnderflow);
    for(int i = 0; i < nChavesUnderflow; i++) {
        btree_node_inserir_chave(nodeEsquerdo, btree_node_get_chave(nodeUnderflow, i), btree_node_get_ponteiro_chave(nodeUnderflow, i), btree_node_get_ponteiro(nodeUnderflow, i + 1));
    }

    btree_node_remover_chave(nodePai, indiceChavePai);

    fseek(btree, offsetPai, SEEK_SET);
    btree_node_write_to_file(btree, nodePai);

    fseek(btree, offsetFilhoEsquerdo, SEEK_SET);
    btree_node_write_to_file(btree, nodeEsquerdo);

    // Remove o nodeUnderflow
    HeaderBTree *headerB = create_btree_header();
    btree_header_read_from_file(btree, headerB);
    remover_logicamente_no(btree, headerB, rrnFilho);

    free_btree_node(&nodePai);
    free_btree_node(&nodeUnderflow);
    free_btree_node(&nodeEsquerdo);

    return true;
}