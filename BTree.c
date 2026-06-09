#include "BTree.h"
#include "utils.h"
#include "dataRegister.h"
#include "headerRegister.h"

bool create_btree(char *bin_filename, char *btree_filename){
    FILE *bin = fopen(bin_filename, "r");
    FILE *btree = fopen(btree_filename, "wb");
    
    if(!bin || !btree || status_esta_instavel(bin)){
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    HeaderBTree *headerB = create_btree_header();
    btree_header_write_to_file(btree, headerB);

    Record *record;
    NodeB *no;
    char removido;
    int RRN = 0, posRecord = 0, RRNNo = -1;
    btree_header_set_noRaiz(headerB, RRNNo);

    // Percorre todo o arquivo binario para criar o indice de arvore B
    fseek(bin, 17, SEEK_SET);
    while(fread(&removido, sizeof(char), 1, bin) == 1){

        // Ignora os registros removidos
        if(removido == '1'){
            RRN++;
            posRecord = 17 + RRN * 80;
            fseek(bin, posRecord, SEEK_SET);
            continue;
        }   

        // Pega a chave primária e o posOffset do registro encontrado
        posRecord = 17 + RRN * 80;
        fseek(bin, posRecord, SEEK_SET);
        record = record_read_from_file(bin);
        int chave = record_get_codEstacao(record);

        // Criação da arvore b -------------------------------------
        
        // Se não tiver nenhum no da arvore, cria o primeiro no raiz
        if(RRNNo == -1){
            RRNNo++;
            no = create_btree_node(-1);
            btree_header_set_noRaiz(headerB, RRNNo);
        }
        else {
            // Pega o no raiz pelo RRN do noRaiz
            int posOffsetNo = 17 + 53 * btree_header_get_noRaiz(headerB);
            no = btree_node_read_from_file_at_offset(btree, posOffsetNo);
        }

        // Realiza uma busca (talvez binária) para verificar se já existe essa chave e garantir que estejamos em um nó raiz
        busca_binaria(no, chave);

        RRN++;

        free_btree_node(&no);
        free_record(&record);
    }

    
    btree_header_set_status(headerB, '1');
    btree_header_write_to_file(btree, headerB);
    free_btree_header(headerB);

    fclose(bin);
    fclose(btree);
}

int busca_binaria(NodeB *no, int chave){
    // Verifica se achou o registro
    int posOffsetChave = btree_node_buscar_chave(no, chave);
    
    // Se achou retorna
    if(posOffsetChave != -1){
        return posOffsetChave; 
    }

    // Se não, verifica se é folha
    if(btree_node_get_tipoNo(no) != -1){
        int meio = btree_node_get_nroChaves(no) / 2;
        printf("%d", meio);
    }
    
}
