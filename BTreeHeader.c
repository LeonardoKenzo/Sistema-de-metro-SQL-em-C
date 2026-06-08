#include "BTreeHeader.h"

struct BTreeHeader{
    char status;
    int noRaiz;
    int topo;
    int proxRRN;
    int nroNos;
};

HeaderBTree *create_btree_header(){
    HeaderBTree *header = (HeaderBTree *)calloc(1, sizeof(HeaderBTree));
    if(header == NULL){
        printf("Erro: alocar memória (HeaderBTree)!\n");
        return NULL;
    }

    header->status = '1';
    header->noRaiz = -1;
    header->topo = -1;
    header->proxRRN = 0;
    header->nroNos = 0;

    return header;
}

char btree_header_get_status(HeaderBTree *header){
    if(header) return header->status;
    printf("Erro: header nulo (HeaderBTree)!\n");
    return '0';
}

int btree_header_get_noRaiz(HeaderBTree *header){
    if(header) return header->noRaiz;
    printf("Erro: header nulo (HeaderBTree)!\n");
    return -1;
}

int btree_header_get_topo(HeaderBTree *header){
    if(header) return header->topo;
    printf("Erro: header nulo (HeaderBTree)!\n");
    return -1;
}

int btree_header_get_proxRRN(HeaderBTree *header){
    if(header) return header->proxRRN;
    printf("Erro: header nulo (HeaderBTree)!\n");
    return -1;
}

int btree_header_get_nroNos(HeaderBTree *header){
    if(header) return header->nroNos;
    printf("Erro: header nulo (HeaderBTree)!\n");
    return -1;
}

void btree_header_set_status(HeaderBTree *header, char status){
    if(header) header->status = status;
    else printf("Erro: header nulo (HeaderBTree)!\n");
}

void btree_header_set_noRaiz(HeaderBTree *header, int raiz){
    if(header) header->noRaiz = raiz;
    else printf("Erro: header nulo (HeaderBTree)!\n");
}

void btree_header_set_topo(HeaderBTree *header, int topo){
    if(header) header->topo = topo;
    else printf("Erro: header nulo (HeaderBTree)!\n");
}

void btree_header_set_proxRRN(HeaderBTree *header, int proxRRN){
    if(header) header->proxRRN = proxRRN;
    else printf("Erro: header nulo (HeaderBTree)!\n");
}

void btree_header_set_nroNos(HeaderBTree *header, int nroNos){
    if(header) header->nroNos = nroNos;
    else printf("Erro: header nulo (HeaderBTree)!\n");
}

void btree_header_write_to_file(FILE *fp, HeaderBTree *h){
    fseek(fp, 0, SEEK_SET);
    fwrite(&h->status, sizeof(char), 1, fp);
    fwrite(&h->noRaiz, sizeof(int), 1, fp);
    fwrite(&h->topo, sizeof(int), 1, fp);
    fwrite(&h->proxRRN, sizeof(int), 1, fp);
    fwrite(&h->nroNos, sizeof(int), 1, fp);
}

void btree_header_read_from_file(FILE *fp, HeaderBTree *h){
    fseek(fp, 0, SEEK_SET);
    fread(&h->status, sizeof(char), 1, fp);
    fread(&h->noRaiz, sizeof(int), 1, fp);
    fread(&h->topo, sizeof(int), 1, fp);
    fread(&h->proxRRN, sizeof(int), 1, fp);
    fread(&h->nroNos, sizeof(int), 1, fp);
}

void free_btree_header(HeaderBTree **header){
    if(header == NULL || *header == NULL)
        return;

    free(*header);
    *header = NULL;
}