#include "headerRegister.h"
#include "dataRegister.h"
#include "fornecidas.h"
#include "functionalities.h"
#include "utils.h"
#include "BTree.h"
#include "BTreeHeader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Funcoes para as funcionalidades 2, 4, 6, 10 e 13
void print_register(Record *r, int posRecord, Contexto *ctx);
void remove_register(Record *r, int posRecord, Contexto *ctx);
void update_register(Record *r, int posRecord, Contexto *ctx);
void remove_register_with_btree(Record *r, int posRecord, Contexto *ctx);
Record *comparar_record_codEstacao(Record *a, Record *b);
Record *comparar_record_codProxEstacao(Record *a, Record *b);
void print_join(Record *r1, Record *r2);

// Funcionalidade 1 ------------------------------------------
bool create_table(char *csv_filename, char *bin_filename)
{
    FILE *csv = fopen(csv_filename, "r");
    FILE *bin = fopen(bin_filename, "wb");

    if (!bin || !csv)
    {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    // Criar e escrever cabeçalho inicial (status '0')
    Header *h = create_header_register();
    header_write_to_file(bin, h);

    // Variaveis para controle de estacoes unicas e pares de estacoes
    int nroMaxPares = 20, nroMaxNomes = 20, nroEstacoes = 0, nroPares = 0;
    char **lista_nomesEstacoes = criar_lista_nomesEstacoes(nroMaxNomes);
    paresEstacoes *lista_paresEstacoes = criar_lista_paresEstacoes(nroMaxPares);

    char line[256];
    fgets(line, sizeof(line), csv); // Pular a primeira linha (cabeçalho CSV)

    int rrn_count = 0;
    while (fgets(line, sizeof(line), csv))
    {
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

        // Gerenciar listas de estações únicas e pares de estações para o header
        char *nome = record_get_nomeEstacao(r);

        if (nroEstacoes >= nroMaxNomes)
        {
            nroMaxNomes *= 2;
            aumentar_capacidade_nomesEstacoes(&lista_nomesEstacoes, nroEstacoes, nroMaxNomes);
        }
        if (nroPares >= nroMaxPares)
        {
            nroMaxPares *= 2;
            aumentar_capacidade_paresEstacoes(&lista_paresEstacoes, nroPares, nroMaxPares);
        }

        if (tem_repetidos_nomesEstacoes(lista_nomesEstacoes, nroEstacoes, nome) == false)
        {
            adicionar_nomesEstacoes(lista_nomesEstacoes, nroEstacoes, nome);
            (nroEstacoes) += 1;
        }
        if (tem_repetido_paresEstacoes(lista_paresEstacoes, nroPares, codEst, codProx) == false && codProx != -1)
        {
            adicionar_paresEstacoes(lista_paresEstacoes, nroPares, codEst, codProx);
            (nroPares) += 1;
        }

        free_record(&r);
        rrn_count++;
    }
    header_set_proxRRN(h, rrn_count);

    // Atualiza o header com os valores corretos
    header_set_nroEstacoes(h, nroEstacoes);
    header_set_nroParesEst(h, nroPares);
    header_set_status(h, '1');
    header_write_to_file(bin, h);

    // Libera memória e fecha arquivos
    free_header_register(&h);
    free_lista_paresEstacoes(&lista_paresEstacoes);
    free_lista_nomesEstacoes(&lista_nomesEstacoes, nroEstacoes);
    fclose(csv);
    fclose(bin);
    return true;
}

// Funcionalidade 2 -------------------------------------------
void print_table(char *bin_filename)
{
    FILE *bin = fopen(bin_filename, "rb");
    if (!bin || status_esta_instavel(bin))
    {
        printf("Falha no processamento do arquivo.\n");
        if (bin)
            fclose(bin);
        return;
    }

    // Vai para a posicao do primeiro registro record
    fseek(bin, 17, SEEK_SET);

    bool encontrou = false;
    Record *r;
    char removido;
    int RRN = 0, posRecord = 0;
    while (fread(&removido, sizeof(char), 1, bin) == 1)
    {
        if (removido == '1')
        {
            RRN++;
            posRecord = 17 + RRN * 80;
            fseek(bin, posRecord, SEEK_SET);
            continue;
        }

        posRecord = 17 + RRN * 80;
        fseek(bin, posRecord, SEEK_SET);

        r = record_read_from_file(bin);
        if (r == NULL)
        {
            RRN++;
            continue;
        }

        printar_record_object(r);
        encontrou = true;

        RRN++;
        posRecord = 17 + RRN * 80;
        fseek(bin, posRecord, SEEK_SET);

        free_record(&r);
    }

    // Caso todos os registros tenham sido removidos
    if (encontrou == false)
        printf("Registro inexistente.\n");

    fclose(bin);
}

// Funcionalidade 3 ------------------------------------
void search_table(char *bin_filename)
{
    FILE *bin = fopen(bin_filename, "rb");
    if (!bin || status_esta_instavel(bin))
    {
        printf("Falha no processamento do arquivo.\n");
        if (bin)
            fclose(bin);
        return;
    }

    int n;
    scanf(" %d", &n);
    for (int i = 0; i < n; i++)
    {
        int m;
        scanf(" %d", &m);
        Criterio *criterios = input_criterios(m);

        // Filtra por criterio e imprime os encontrados
        bool encontrou = search_records(bin, criterios, m, print_register, NULL);

        if (encontrou == false)
        {
            printf("Registro inexistente.\n");
        }
        printf("\n");

        free(criterios);
    }

    fclose(bin);
}

// Funcionalidade 4 -----------------------------------
void remove_record_table(char *bin_filename)
{
    FILE *bin = fopen(bin_filename, "r+b");
    if (!bin || status_esta_instavel(bin))
    {
        printf("Falha no processamento do arquivo.\n");
        if (bin)
            fclose(bin);
        return;
    }
    // Marcar o header como instavel
    Header *header = create_header_register();
    header_read_from_file(bin, header);
    header_set_status(header, '0');
    header_write_to_file(bin, header);

    // Cria o contexto da remocao
    Contexto *ctx = criar_contexto(bin, header, NULL, 0);

    int n;
    scanf(" %d", &n);
    for (int i = 0; i < n; i++)
    {
        int m;
        scanf(" %d", &m);
        Criterio *criterios = input_criterios(m);

        // Filtra por criterio e remove os encontrados
        search_records(bin, criterios, m, remove_register, ctx);

        free(criterios);
    }

    // Marcar o header como estavel
    set_header_estacoes_unicas(bin, header);
    header_set_status(header, '1');
    header_write_to_file(bin, header);

    free_header_register(&header);
    free(ctx);
    fclose(bin);
}

// Funcionalidade 5 -----------------------------------

void insert_record_table(char *bin_filename)
{
    FILE *bin = fopen(bin_filename, "r+b");
    if (!bin || status_esta_instavel(bin))
    {
        printf("Falha no processamento do arquivo.\n");
        if (bin)
            fclose(bin);
        return;
    }

    Header *h = create_header_register();
    header_read_from_file(bin, h);
    header_set_status(h, '0');
    header_write_to_file(bin, h);

    int n;
    scanf(" %d", &n);

    for (int i = 0; i < n; i++)
    {
        Record *r = create_record();
        char buffer_str[100];

        int codEstacao;
        scanf(" %d", &codEstacao);
        record_set_codEstacao(r, codEstacao);

        // nomeEstacao (não aceita nulo)
        ScanQuoteString(buffer_str);
        record_set_nomeEstacao(r, buffer_str);

        // codLinha
        record_set_codLinha(r, input_inteiro_ou_nulo());

        // nomeLinha
        ScanQuoteString(buffer_str);
        record_set_nomeLinha(r, buffer_str);

        // Campos que são inteiros ou nulos
        record_set_codProxEstacao(r, input_inteiro_ou_nulo());
        record_set_distProxEstacao(r, input_inteiro_ou_nulo());
        record_set_codLinhaIntegra(r, input_inteiro_ou_nulo());
        record_set_codEstIntegra(r, input_inteiro_ou_nulo());

        int topo = header_get_topo(h);

        if (topo == -1)
        {
            // Caso 1: Pilha vazia, insere no fim do arquivo
            int proxRRN = header_get_proxRRN(h);
            fseek(bin, 17 + (proxRRN * 80), SEEK_SET);
            record_write_to_file(bin, r);
            header_set_proxRRN(h, proxRRN + 1);
        }
        else
        {
            // Caso 2: Reaproveitamento de espaço (Pilha)
            // Vai até o registro removido indicado pelo topo
            topo = 17 + topo * 80;
            fseek(bin, topo, SEEK_SET);

            // Lê o RRN do próximo da pilha antes de sobrescrever
            Record *recordRemovido = record_read_from_file(bin);
            int prox_na_pilha = record_get_proximo(recordRemovido);

            // Volta para a posição do registro e escreve o novo dado
            fseek(bin, topo, SEEK_SET);
            record_write_to_file(bin, r);

            // Atualiza o topo do cabeçalho com o próximo da lista encadeada
            int novo_topo_offset = (prox_na_pilha == -1) ? -1 : prox_na_pilha; // Não é só colocar prox_na_pilha?
            header_set_topo(h, novo_topo_offset);

            free_record(&recordRemovido);
        }
        free_record(&r);
    }

    set_header_estacoes_unicas(bin, h);
    header_set_status(h, '1');
    header_write_to_file(bin, h);

    free_header_register(&h);
    fclose(bin);
}

// Funcionalidade 6 -------------------------------------------------------

void update_table(char *bin_filename)
{
    FILE *bin = fopen(bin_filename, "r+b");
    if (!bin || status_esta_instavel(bin))
    {
        printf("Falha no processamento do arquivo.\n");
        if (bin)
            fclose(bin);
        return;
    }

    // Marcar o header como instavel
    Header *header = create_header_register();
    header_read_from_file(bin, header);
    header_set_status(header, '0');
    header_write_to_file(bin, header);

    Contexto *ctx = criar_contexto(bin, header, NULL, 0);

    int n;
    scanf(" %d", &n);
    for (int i = 0; i < n; i++)
    {
        int m, p, posRecord;
        scanf(" %d", &m);
        Criterio *criterios = input_criterios(m);

        scanf(" %d", &p);
        Criterio *atualizar = input_criterios(p);
        atualizar_contexto(ctx, atualizar, p);

        // Filtra por criterio, atualiza os encontrados e escreve de volta no arquivo
        search_records(bin, criterios, m, update_register, ctx);

        free(atualizar);
        free(criterios);
    }

    // Marcar o header como estavel
    header_set_status(header, '1');
    header_write_to_file(bin, header);

    free_header_register(&header);
    free(ctx);
    fclose(bin);
}

// Funcionalidade 7 --------------------------------------------------

bool create_btree(char *bin_filename, char *btree_filename)
{
    FILE *bin = fopen(bin_filename, "rb");
    FILE *btree = fopen(btree_filename, "w+b");

    if (!bin || !btree || status_esta_instavel(bin))
    {
        printf("Falha no processamento do arquivo.\n");
        return false;
    }

    HeaderBTree *headerB = create_btree_header();
    btree_header_write_to_file(btree, headerB);

    Record *record;
    char removido;
    int RRN = 0, posRecord = 0;
    btree_header_set_noRaiz(headerB, -1);

    // Percorre todo o arquivo binario para criar o indice de arvore B
    fseek(bin, 17, SEEK_SET);
    while (fread(&removido, sizeof(char), 1, bin) == 1)
    {

        // Ignora os registros removidos
        if (removido == '1')
        {
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
        btree_insert(btree, headerB, chave, posRecord);

        RRN++;
        posRecord = 17 + RRN * 80;
        fseek(bin, posRecord, SEEK_SET);

        free_record(&record);
    }

    btree_header_set_status(headerB, '1');
    btree_header_write_to_file(btree, headerB);
    free_btree_header(&headerB);

    fclose(bin);
    fclose(btree);

    return true;
}

// Funcionalidade 8 --------------------------------------------------

void search_btree(char *bin_filename, char *btree_filename)
{
    FILE *bin = fopen(bin_filename, "rb");
    FILE *btree = fopen(btree_filename, "rb");

    HeaderBTree *headerBtree = NULL;
    bool flag_estavel = true;

    // Verificações
    if (!bin || !btree)
    {
        flag_estavel = false;
    }
    else
    {
        if (status_esta_instavel(bin))
            flag_estavel = false;

        headerBtree = create_btree_header();
        btree_header_read_from_file(btree, headerBtree);
        if (btree_header_get_status(headerBtree) == '0')
            flag_estavel = false;
    }

    if (!flag_estavel)
    {
        printf("Falha no processamento do arquivo.\n");
        if (bin)
            fclose(bin);
        if (btree)
            fclose(btree);
        if (headerBtree)
            free_btree_header(&headerBtree);
        return;
    }

    int noRaiz = btree_header_get_noRaiz(headerBtree);

    int n;
    if (scanf(" %d", &n) != 1)
        return;

    for (int i = 0; i < n; i++)
    {
        int m;
        scanf(" %d", &m);
        Criterio *criterios = input_criterios(m);

        // Verificar qual mecanismo de busca usar
        int chave_busca = get_chave_busca_criterio(criterios, m);

        bool encontrou = false;

        if (chave_busca != -1)
        {
            // Busca com a Árvore-B
            int caminho[8];
            NodeSearch *nodeFound = btree_search_recursive(btree, noRaiz, -1, chave_busca, caminho, 0);
            
            // Garantir que a chave realmente foi encontrada na árvore
            if (btree_nodeSearch_get_found(nodeFound))
            {
                int rrn = btree_nodeSearch_get_rrn(nodeFound);
                int indice = btree_nodeSearch_get_indice(nodeFound);

                // Lê o nó da Árvore-B onde a busca parou
                int offset_no_btree = 17 + rrn * 53;
                NodeB *node = btree_node_read_from_file_at_offset(btree, offset_no_btree);
                
                // Extrai o ponteiro para o arquivo de dados
                int offset_dados = btree_node_get_ponteiro_chave(node, indice);
                free_btree_node(&node); // Libera o nó da memória

                // Busca o registro no arquivo .bin
                if (offset_dados != -1) {
                    fseek(bin, offset_dados, SEEK_SET);
                    Record *r = record_read_from_file(bin);
                    
                    if (r != NULL)
                    {
                        // A Árvore devolve o registro, precisa ver se não está logicamente removido
                        // e se atende aos possíveis outros critérios de busca
                        if (record_get_removido(r) == '0' && atende_criterios(r, criterios, m))
                        {
                            print_register(r, offset_dados, NULL); 
                            encontrou = true;
                        }
                        free_record(&r);
                    }
                }
            }
            free_search_result(&nodeFound);
        }
        else
        {
            encontrou = search_records(bin, criterios, m, print_register, NULL);
        }
        if (!encontrou)
        {
            printf("Registro inexistente.\n");
        }
        printf("\n");

        free(criterios);
    }
    free_btree_header(&headerBtree);
    fclose(bin);
    fclose(btree);
}

// Funcionalidade 9
bool insert_btree(char *bin_filename, char *btree_filename)
{
    FILE *bin = fopen(bin_filename, "r+b");
    FILE *btree = fopen(btree_filename, "r+b");

    if (!bin || !btree || status_esta_instavel(bin))
    {
        printf("Falha no processamento do arquivo.\n");
        if (bin)
            fclose(bin);
        if (btree)
            fclose(btree);
        return false;
    }

    // Validações e marcação de instabilidade
    Header *h = create_header_register();
    header_read_from_file(bin, h);
    header_set_status(h, '0');
    header_write_to_file(bin, h);

    HeaderBTree *hBTree = create_btree_header();
    btree_header_read_from_file(btree, hBTree);
    if (btree_header_get_status(hBTree) == '0')
    {
        printf("Falha no processamento do arquivo.\n");
        free_header_register(&h);
        free_btree_header(&hBTree);
        fclose(bin);
        fclose(btree);
        return false;
    }
    btree_header_set_status(hBTree, '0');
    btree_header_write_to_file(btree, hBTree);

    int n;
    scanf(" %d", &n);

    for (int i = 0; i < n; i++)
    {
        Record *r = create_record();
        char buffer_str[100];

        int codEstacao;
        scanf(" %d", &codEstacao);
        record_set_codEstacao(r, codEstacao);

        ScanQuoteString(buffer_str);
        record_set_nomeEstacao(r, buffer_str);

        record_set_codLinha(r, input_inteiro_ou_nulo());

        ScanQuoteString(buffer_str);
        record_set_nomeLinha(r, buffer_str);

        record_set_codProxEstacao(r, input_inteiro_ou_nulo());
        record_set_distProxEstacao(r, input_inteiro_ou_nulo());
        record_set_codLinhaIntegra(r, input_inteiro_ou_nulo());
        record_set_codEstIntegra(r, input_inteiro_ou_nulo());

        // Checa na árvore-B se a chave já existe
        int caminho[8];
        int noRaizAtual = btree_header_get_noRaiz(hBTree);
        NodeSearch *search = btree_search_recursive(btree, noRaizAtual, -1, codEstacao, caminho, 0);
        if (btree_nodeSearch_get_found(search)) {
            // A chave já existe -> Ignora a inserção completamente
            free_search_result(&search);
            free_record(&r);
            continue; 
        }
        free_search_result(&search);

        int topo = header_get_topo(h);
        int offset_inserido; // Guarda o local do novo registro

        if (topo == -1)
        {
            int proxRRN = header_get_proxRRN(h);
            offset_inserido = 17 + (proxRRN * 80);
            fseek(bin, offset_inserido, SEEK_SET);
            record_write_to_file(bin, r);
            header_set_proxRRN(h, proxRRN + 1);
        }
        else
        {
            offset_inserido = 17 + topo * 80;
            fseek(bin, offset_inserido, SEEK_SET);

            Record *recordRemovido = record_read_from_file(bin);
            int prox_na_pilha = record_get_proximo(recordRemovido);

            fseek(bin, offset_inserido, SEEK_SET);
            record_write_to_file(bin, r);

            int novo_topo_offset = (prox_na_pilha == -1) ? -1 : prox_na_pilha;
            header_set_topo(h, novo_topo_offset);

            free_record(&recordRemovido);
        }
        free_record(&r);

        // Inserir a chave e o offset 
        btree_insert(btree, hBTree, codEstacao, offset_inserido);
    }

    // Fechar tudo
    set_header_estacoes_unicas(bin, h);
    header_set_status(h, '1');
    header_write_to_file(bin, h);

    btree_header_set_status(hBTree, '1');
    btree_header_write_to_file(btree, hBTree);

    free_header_register(&h);
    free_btree_header(&hBTree);
    fclose(bin);
    fclose(btree);
    return true;
}
// Funcionalidade 10 ------------------------------------------------------
bool remove_btree(char *bin_filename, char *btree_filename)
{
    FILE *bin = fopen(bin_filename, "r+b");
    FILE *btree = fopen(btree_filename, "r+b");

    if (!bin || !btree || status_esta_instavel(bin) || status_esta_instavel(btree))
    {
        printf("Falha no processamento do arquivo.\n");
        if (bin)
            fclose(bin);
        if (btree)
            fclose(btree);
        return false;
    }

    HeaderBTree *headerB = create_btree_header();
    btree_header_read_from_file(btree, headerB);
    btree_header_set_status(headerB, '0');
    btree_header_write_to_file(btree, headerB);

    Header *header = create_header_register();
    header_read_from_file(bin, header);
    header_set_status(header, '0');
    header_write_to_file(bin, header);

    // Contexto de remocao para o arquivo de registro
    Contexto *ctx = criar_contexto(bin, header, NULL, 0);

    // Busca
    int noRaiz = btree_header_get_noRaiz(headerB);

    int n;
    scanf(" %d", &n);
    for (int i = 0; i < n; i++)
    {
        int m;
        scanf(" %d", &m);
        Criterio *criterios = input_criterios(m);

        // Verificar qual mecanismo de busca usar
        int chave_busca = get_chave_busca_criterio(criterios, m);

        bool encontrou = false;

        // Caso codEstacao seja criterio de busca, usa-se busca na arvore
        if (chave_busca != -1) {
            int caminho[8];
            NodeSearch *result = btree_search_recursive(btree, noRaiz, -1, chave_busca, caminho, 0);

            // Se nao achou resultado retorne
            if (!btree_nodeSearch_get_found(result)) {
                free_search_result(&result);
                free(criterios);
                continue;
            }

            // Faz a remocao do arquivo de registros
            NodeB *node = btree_node_read_from_file_at_offset(btree, 17 + btree_nodeSearch_get_rrn(result) * 53);
            int posOffset = btree_node_get_ponteiro_chave(node, btree_nodeSearch_get_indice(result));
            Record *r = record_read_from_file_at_offset(bin, posOffset);
            remove_register(r, posOffset, ctx);
            free_btree_node(&node);
            free_record(&r);

           // Realiza a remocao na arvore
            btree_remove_key(btree, headerB, &noRaiz, result, caminho);
            free_search_result(&result);
        }
        // Caso não seja, usa busca como na funcionalidade 3 (sem a arvore B)
        else{
            int caminho[8];
            remove_btree_contexto(ctx, btree, headerB, &noRaiz);
            encontrou = search_records(bin, criterios, m, remove_register_with_btree, ctx);
        }

        free(criterios);
    }

    // Fechar tudo
    set_header_estacoes_unicas(bin, header);
    header_set_status(header, '1');
    header_write_to_file(bin, header);

    btree_header_set_status(headerB, '1');
    btree_header_write_to_file(btree, headerB);

    free_header_register(&header);
    free_btree_header(&headerB);
    free(ctx);
    fclose(bin);
    fclose(btree);

    return true;
}

// Funcionalidade 11
void nested_loop_join(char *bin_filename1, char *campo1, char *bin_filename2, char *campo2) {
    FILE *bin1 = fopen(bin_filename1, "rb");
    FILE *bin2 = fopen(bin_filename2, "rb");

    if (!bin1 || !bin2 || status_esta_instavel(bin1) || status_esta_instavel(bin2)) {
        printf("Falha no processamento do arquivo.\n");
        if (bin1) fclose(bin1);
        if (bin2) fclose(bin2);
        return;
    }

    bool encontrou_algum = false;
    
    // Loop para percorre o arquivo 1
    fseek(bin1, 17, SEEK_SET);
    char removido1;
    int rrn1 = 0;

    while (fread(&removido1, sizeof(char), 1, bin1) == 1) {
        if (removido1 == '1') {
            rrn1++;
            fseek(bin1, 17 + rrn1 * 80, SEEK_SET);
            continue;
        }

        fseek(bin1, 17 + rrn1 * 80, SEEK_SET);
        Record *r1 = record_read_from_file(bin1);
        if (r1 == NULL) {
            rrn1++;
            continue;
        }

        int codProx = record_get_codProxEstacao(r1);

        // Loop para percorrer o arquivo 2
        if (codProx != -1) {
            fseek(bin2, 17, SEEK_SET);
            char removido2;
            int rrn2 = 0;
            
            while (fread(&removido2, sizeof(char), 1, bin2) == 1) {
                if (removido2 == '1') {
                    rrn2++;
                    fseek(bin2, 17 + rrn2 * 80, SEEK_SET);
                    continue;
                }

                fseek(bin2, 17 + rrn2 * 80, SEEK_SET);
                Record *r2 = record_read_from_file(bin2);
                if (r2 == NULL) {
                    rrn2++;
                    continue;
                }

                // Condição de Junção
                if (record_get_codEstacao(r2) == codProx) {
                    print_join(r1, r2);
                    encontrou_algum = true;
                }

                free_record(&r2);
                rrn2++;
                fseek(bin2, 17 + rrn2 * 80, SEEK_SET);
            }
        }

        free_record(&r1);
        rrn1++;
        fseek(bin1, 17 + rrn1 * 80, SEEK_SET);
    }

    if (!encontrou_algum) {
        printf("Registro inexistente.\n");
    }

    fclose(bin1);
    fclose(bin2);
}

// Funcionalidade 12
void single_loop_join(char *bin_filename1, char *campo1, char *bin_filename2, char *campo2, char *btree_filename) {
    FILE *bin1 = fopen(bin_filename1, "rb");
    FILE *bin2 = fopen(bin_filename2, "rb");
    FILE *btree = fopen(btree_filename, "rb");

    HeaderBTree *headerB = NULL;
    bool estavel = true;

    // Checagens
    if (!bin1 || !bin2 || !btree || status_esta_instavel(bin1) || status_esta_instavel(bin2)) {
        estavel = false;
    } else {
        headerB = create_btree_header();
        btree_header_read_from_file(btree, headerB);
        if (btree_header_get_status(headerB) == '0') estavel = false;
    }

    if (!estavel) {
        printf("Falha no processamento do arquivo.\n");
        if (bin1) fclose(bin1);
        if (bin2) fclose(bin2);
        if (btree) fclose(btree);
        if (headerB) free_btree_header(&headerB);
        return;
    }

    int noRaiz = btree_header_get_noRaiz(headerB);
    bool encontrou_algum = false;

    // Loop para percorrer o arquivo 1
    fseek(bin1, 17, SEEK_SET);
    char removido1;
    int rrn1 = 0;

    while (fread(&removido1, sizeof(char), 1, bin1) == 1) {
        if (removido1 == '1') {
            rrn1++;
            fseek(bin1, 17 + rrn1 * 80, SEEK_SET);
            continue;
        }

        fseek(bin1, 17 + rrn1 * 80, SEEK_SET);
        Record *r1 = record_read_from_file(bin1);
        if (r1 == NULL) {
            rrn1++;
            continue;
        }

        int codProx = record_get_codProxEstacao(r1);

        // Busca o codProxEstacao correspondente na Árvore-B (condição de junção)
        if (codProx != -1) {
            int caminho[8];
            NodeSearch *result = btree_search_recursive(btree, noRaiz, -1, codProx, caminho, 0);

            if (btree_nodeSearch_get_found(result)) {
                int rrn = btree_nodeSearch_get_rrn(result);
                int indice = btree_nodeSearch_get_indice(result);

                NodeB *node = btree_node_read_from_file_at_offset(btree, 17 + rrn * 53);
                int offset2 = btree_node_get_ponteiro_chave(node, indice);
                free_btree_node(&node);

                if (offset2 != -1) {
                    fseek(bin2, offset2, SEEK_SET);
                    Record *r2 = record_read_from_file(bin2);
                    
                    if (r2 != NULL && record_get_removido(r2) == '0') {
                        print_join(r1, r2);
                        encontrou_algum = true;
                    }
                    free_record(&r2);
                }
            }
            free_search_result(&result);
        }

        free_record(&r1);
        rrn1++;
        fseek(bin1, 17 + rrn1 * 80, SEEK_SET);
    }

    if (!encontrou_algum) {
        printf("Registro inexistente.\n");
    }

    fclose(bin1);
    fclose(bin2);
    fclose(btree);
    free_btree_header(&headerB);
}

// Funcionalidade 13 ----------------------------------------------------
bool create_order_by(char *bin_filename, char *campoOrdenacao, char *order_filename){
    FILE *bin = fopen(bin_filename, "rb");
    FILE *order = fopen(order_filename, "w+b");

    if(!bin || !order || status_esta_instavel(bin)){
        printf("Falha no processamento do arquivo.\n");
        if(bin) fclose(bin);
        if(order) fclose(order);
        return false;
    }

    Header *header = create_header_register();
    header_read_from_file(bin, header);
    int quant_register = header_get_nroEstacoes(header);

    Record *record;
    Record **record_list = (Record **)malloc(quant_register * sizeof(Record *));
    if(!record_list){
        free_header_register(&header);
        fclose(bin);
        fclose(order);
        return false;
    }
    char removido;
    int RRN = 0, posRecord = 0, quant_record = 0;

    // Percorre todo o arquivo binario para criar o indice de arvore B
    fseek(bin, 17, SEEK_SET);
    while (fread(&removido, sizeof(char), 1, bin) == 1)
    {
        // Ignora os registros removidos
        if (removido == '1')
        {
            RRN++;
            posRecord = 17 + RRN * 80;
            fseek(bin, posRecord, SEEK_SET);
            continue;
        }

        // Adiciona o registro na lista em memoria RAM
        posRecord = 17 + RRN * 80;
        fseek(bin, posRecord, SEEK_SET);
        record = record_read_from_file(bin);
        record_list[quant_record] = record;
        quant_record++;

        if(quant_record > quant_register){
            record_list = realloc(record_list, (quant_register * 2) * sizeof(Record *));
            quant_register *= 2;
        }

        RRN++;
        posRecord = 17 + RRN * 80;
        fseek(bin, posRecord, SEEK_SET);
    }

    // Ordena o vetor
    if (strncmp(campoOrdenacao, "codEstacao", 10) == 0) {
        //qsort(record_list, quant_record, sizeof(Record *) ,comparar_record_codEstacao);
    } 
    else if (strncmp(campoOrdenacao, "codProxEstacao", 14) == 0) {
        //qsort(record_list, quant_record, sizeof(Record *) ,comparar_record_codProxEstacao);
    } 

    // Escreve no novo arquivo binario
    header_write_to_file(order, header);
    for(int i = 0; i < quant_record; i++){
        record_write_to_file(order, record_list[i]);
        free_record(&(record_list[i]));
    }
    free(record_list);

    free_header_register(&header);

    fclose(bin);
    fclose(order);

    return true;
}

// Funcionalidade 14 ----------------------------------------
void join_order_by(char *bin_filename1, char *joinCampo1, char *bin_filename2, char *joinCampo2){
    FILE *bin1 = fopen(bin_filename1, "rb");
    FILE *bin2 = fopen(bin_filename2, "rb");

    if(!bin1 || !bin2 || status_esta_instavel(bin1) || status_esta_instavel(bin2)){
        printf("Falha no processamento do arquivo.\n");
        if(bin1) fclose(bin1);
        if(bin2) fclose(bin2);
        return;
    }



    fclose(bin1);
    fclose(bin2);
}

// Funcoes auxiliares -----------------------------------------------------

void print_register(Record *r, int posRecord, Contexto *ctx)
{
    printar_record_object(r);
}

void remove_register(Record *r, int posRecord, Contexto *ctx)
{
    int topoAntigo = header_get_topo(get_header_from_context(ctx));
    int proximoRRN = topoAntigo;
    int novoTopo = (posRecord - 17) / 80;

    record_set_removido(r, '1');
    record_set_proximo(r, proximoRRN);

    fseek(get_file_from_context(ctx), posRecord, SEEK_SET);
    record_write_to_file(get_file_from_context(ctx), r);

    header_set_topo(get_header_from_context(ctx), novoTopo);
}

void update_register(Record *r, int posRecord, Contexto *ctx)
{
    update_campos(r, get_atualizar_from_context(ctx), get_p_from_context(ctx));

    fseek(get_file_from_context(ctx), posRecord, SEEK_SET);
    record_write_to_file(get_file_from_context(ctx), r);
    pausar_busca(ctx);
}

void remove_register_with_btree(Record *r, int posRecord, Contexto *ctx){
    remove_register(r, posRecord, ctx);

    // Busca e remove da arvore B pela chave do registro
    int chave = record_get_codEstacao(r);
    FILE *btree = get_btree_from_context(ctx);
    int *noRaiz = get_noRaiz_from_context(ctx);
    int caminho[8];
    NodeSearch *result = btree_search_recursive(btree, *noRaiz, -1, chave, caminho, 0);

    if (!btree_nodeSearch_get_found(result)) {
        free_search_result(&result);
        return;
    }

    btree_remove_key(btree, get_headerB_from_context(ctx), noRaiz, result, caminho);

    free_search_result(&result);
}

Record *comparar_record_codEstacao(Record *a, Record *b){
    if (record_get_codEstacao(a) == 0 && record_get_codEstacao(b) == 0) return a;
    if (record_get_codEstacao(a) == 0) return b;
    if (record_get_codEstacao(b) == 0) return a;
    return (record_get_codEstacao(a) <= record_get_codEstacao(b)) ? a : b;
}

Record *comparar_record_codProxEstacao(Record *a, Record *b){
    if (record_get_codProxEstacao(a) == 0 && record_get_codProxEstacao(b) == 0) return a;
    if (record_get_codProxEstacao(a) == 0) return b;
    if (record_get_codProxEstacao(b) == 0) return a;
    return (record_get_codProxEstacao(a) <= record_get_codProxEstacao(b)) ? a : b;
}

// Funcao auxiliar para imprimir o resultado da junção
void print_join(Record *r1, Record *r2) {
    // codEstacao (de r1)
    printf("%d ", record_get_codEstacao(r1));

    // nomeEstacao (de r1)
    char *nome1 = record_get_nomeEstacao(r1);
    if (nome1 == NULL || strlen(nome1) == 0) printf("NULO ");
    else printf("%s ", nome1);

    // nomeLinha (de r1)
    char *linha1 = record_get_nomeLinha(r1);
    if (linha1 == NULL || strlen(linha1) == 0) printf("NULO ");
    else printf("%s ", linha1);

    // codProxEstacao (de r1)
    int codProx = record_get_codProxEstacao(r1);
    if (codProx == -1) printf("NULO ");
    else printf("%d ", codProx);

    // nomeProxEstacao (campo recuperado do r2 por meio da junção)
    char *nome2 = record_get_nomeEstacao(r2);
    if (nome2 == NULL || strlen(nome2) == 0) printf("NULO\n");
    else printf("%s\n", nome2);
}