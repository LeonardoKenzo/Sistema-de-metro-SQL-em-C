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

// Funcoes para as funcionalidades 2, 4 e 6
void print_register(Record *r, int posRecord, Contexto *ctx);
void remove_register(Record *r, int posRecord, Contexto *ctx);
void update_register(Record *r, int posRecord, Contexto *ctx);

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
    FILE *bin = fopen(bin_filename, "r");
    FILE *btree = fopen(btree_filename, "wb");

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
            // Caso codEstação seja criterio de busca, usa-se busca na arvore
            int caminho[8];
            NodeSearch *nodeFound = btree_search_recursive(btree, noRaiz, -1, chave_busca, caminho, 0);
            int rrn = btree_nodeSearch_get_rrn(nodeFound);

            if (rrn != -1)
            {
                int offset_dados = 17 + rrn * 53;

                fseek(bin, offset_dados, SEEK_SET);
                Record *r = record_read_from_file(bin);
                // a Arvore devolve o registro, precisa ver se nao esta removido logicamente
                if (r != NULL)
                {
                    if (record_get_removido(r) == '0' && atende_criterios(r, criterios, m))
                    {
                        print_register(r, offset_dados, NULL); // Imprime formatado
                        encontrou = true;
                    }
                    free_record(&r);
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
void insert_btree(char *bin_filename, char *btree_filename)
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
        return;
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
        return;
    }
    btree_header_set_status(hBTree, '0');
    btree_header_write_to_file(btree, hBTree);

    int n;
    if (scanf(" %d", &n) != 1)
        return;

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
}

// Funcionalidade 10 ------------------------------------------------------
void remove_btree(char *bin_filename, char *btree_filename)
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
        return;
    }

    HeaderBTree *headerB = create_btree_header();
    btree_header_read_from_file(btree, headerB);
    btree_header_set_status(headerB, '0');
    btree_header_write_to_file(btree, headerB);

    Header *header = create_header_register();
    header_read_from_file(bin, header);

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

        // Caso codEstação seja criterio de busca, usa-se busca na arvore
        if (chave_busca != -1)
        {

            int caminho[8];
            NodeSearch *result = btree_search_recursive(btree, noRaiz, -1, chave_busca, caminho, 0);

            // Verifica se achou a chave
            bool foundNode = btree_nodeSearch_get_found(result);
            if (foundNode == false)
            {
                free_search_result(&result);
                free(criterios);
                continue;
            }

            int rrnEncontrado = btree_nodeSearch_get_rrn(result);
            int indice = btree_nodeSearch_get_indice(result);
            int profundidade = btree_nodeSearch_get_profundidade(result);

            NodeB *nodeRemove = btree_node_read_from_file_at_offset(btree, 17 + rrnEncontrado * 53);

            // Remocao do arquivo binario
            int posOffsetRemover = btree_node_get_ponteiro_chave(nodeRemove, btree_nodeSearch_get_indice(result));
            Record *r = record_read_from_file_at_offset(bin, posOffsetRemover);
            remove_register(r, posOffsetRemover, ctx);
            free_record(&r);

            // Se nao eh no folha
            int rrnParaEscrever = rrnEncontrado;
            int indiceParaRemover = indice;

            if (btree_node_get_tipoNo(nodeRemove) != -1)
            {
                // Encontra a sucessora imediata
                NodeSearch *sucessora = btree_find_successor(btree, rrnEncontrado, indice);

                int rrnSucessora = btree_nodeSearch_get_rrn(sucessora);
                int indiceSucessora = btree_nodeSearch_get_indice(sucessora);

                NodeB *nodeSucessora = btree_node_read_from_file_at_offset(btree, 17 + rrnSucessora * 53);

                // Copia chave e ponteiro_chave da sucessora para a posicao da chave removida
                btree_node_set_parChave(nodeRemove, indice, btree_node_get_chave(nodeSucessora, indiceSucessora), btree_node_get_ponteiro_chave(nodeSucessora, indiceSucessora));

                // Grava o no interno modificado
                fseek(btree, 17 + rrnEncontrado * 53, SEEK_SET);
                btree_node_write_to_file(btree, nodeRemove);
                free_btree_node(&nodeRemove);

                // A remocao fisica acontece no no folha da sucessora
                profundidade = btree_nodeSearch_get_profundidade(sucessora);
                btree_nodeSearch_set_caminho(result, btree_nodeSearch_get_caminho(sucessora), profundidade * sizeof(int));
                nodeRemove = nodeSucessora;
                rrnParaEscrever = rrnSucessora;
                indiceParaRemover = indiceSucessora;

                free_search_result(&sucessora);
            }
            // Remove a chave
            btree_node_remover_chave(nodeRemove, btree_nodeSearch_get_indice(result));
            fseek(btree, 17 + btree_nodeSearch_get_rrn(result) * 53, SEEK_SET);
            btree_node_write_to_file(btree, nodeRemove);

            int nivelAtual = profundidade - 1;

            // Verifica e trata underflow
            while (btree_node_has_underflow(nodeRemove) && nivelAtual > 0)
            {
                int *caminho = btree_nodeSearch_get_caminho(result);
                int rrnAtual = caminho[nivelAtual];
                int rrnPai = caminho[nivelAtual - 1];

                free_btree_node(&nodeRemove);

                // Ordem: redistribuicao direita, esquerda, merge esquerda, merge direita
                if (btree_redistribute_right(btree, rrnAtual, rrnPai) ||
                    btree_redistribute_left(btree, rrnAtual, rrnPai) ||
                    btree_merge_left(btree, rrnAtual, rrnPai) ||
                    btree_merge_right(btree, rrnAtual, rrnPai))
                {

                    // Relê o pai para verificar se ele entrou em underflow
                    nodeRemove = btree_node_read_from_file_at_offset(btree, 17 + rrnPai * 53);
                    nivelAtual--;
                }
                else
                {
                    break;
                }
            }
            NodeB *raiz = btree_node_read_from_file_at_offset(btree, 17 + noRaiz * 53);
            if (btree_node_get_nroChaves(raiz) == 0)
            {
                int novoRrnRaiz = btree_node_get_ponteiro(raiz, 0);
                remover_logicamente_no(btree, headerB, noRaiz);
                btree_header_set_noRaiz(headerB, novoRrnRaiz);
                btree_header_write_to_file(btree, headerB);
                noRaiz = novoRrnRaiz;
            }
            free_btree_node(&raiz);
            free_btree_node(&nodeRemove);
            free_search_result(&result);
            free(criterios);
            continue;
        }
        /*
        Caso não seja, usa busca como na funcionalidade 3 (sem a arvore B)
        else{
            encontrou = search_records(bin, criterios, m, remove_register, ctx);
        }
        */

        free(criterios);
    }

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

// Resolve underflow em rrnFilho cujo pai e rrnPai e retorna o rrn que ficou apos a operacao
int btree_fix_underflow(FILE *btree, int rrnFilho, int rrnPai)
{
    if (btree_redistribute_right(btree, rrnFilho, rrnPai))
        return -1; // sem underflow a propagar

    if (btree_redistribute_left(btree, rrnFilho, rrnPai))
        return -1;

    // 3. Concatenacao a esquerda e o resultado fica no irmao esquerdo, destroi rrnFilho
    if (btree_merge_left(btree, rrnFilho, rrnPai))
        return rrnPai; // pai perdeu uma chave verificar underflow

    // 4. Concatenacao a direita e o resultado fica em rrnFilho, destroi irmao direito
    btree_merge_right(btree, rrnFilho, rrnPai);
    return rrnPai; // pai perdeu uma chave verificar underflow
}