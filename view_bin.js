// 1. O Gatekeeper: Aceita qualquer arquivo .bin para a extensão abrir
registerFileType((fileExt, filePath) => {
    return fileExt === 'bin';
});

// 2. O Roteador: Recebe o 'filePath' nativamente e decide o que renderizar
registerParser((filePath) => {
    
    // Cores padrão para o tema escuro
    setDefaults({
        "dark-colors": {
            "collapse": "var(--vscode-textLink-foreground)",
            "offset": "var(--vscode-descriptionForeground)",
            "size": "var(--vscode-descriptionForeground)",
            "name": "var(--vscode-foreground)",
            "value": "var(--vscode-editorInfo-foreground)",
            "description": "var(--vscode-descriptionForeground)",
            "row-header": "var(--vscode-sideBar-background)",
            "row-odd": "var(--vscode-editor-background)",
            "row-even": "var(--vscode-sideBar-background)"
        }
    });

    let nome = filePath.toLowerCase();

    // ROTEAMENTO: Se o nome contiver "indice" ou "arv", desenha a Árvore B
    if (nome.includes('btree') || nome.includes('arv')) {
        
        // ==========================================
        // PARSER DA ÁRVORE-B (53 bytes)
        // ==========================================
        read(1); addRow('status', getStringValue(), '0 inconsistente, 1 consistente');
        read(4); addRow('noRaiz', getSignedNumberValue(), 'RRN do nó raíz');
        read(4); addRow('topo', getSignedNumberValue(), 'topo da pilha de removidos');    
        read(4); addRow('proxRRN', getSignedNumberValue(), '');
        read(4); let nroNos = getSignedNumberValue(); addRow('nroNos', nroNos, '');

        for (let i = 0; i < nroNos; i++) {
            read(53);
            addRow(`Nó da Árvore (RRN ${i})`, '');
            addDetails(() => {
                read(1); addRow('removido', getStringValue(), '');
                read(4); addRow('proximo', getSignedNumberValue(), '');
                read(4); addRow('tipoNo', getSignedNumberValue(), '-1=folha, 0=raíz, 1=intermediário');
                read(4); addRow('nroChaves', getSignedNumberValue(), '');
                
                read(4); addRow('C1', getSignedNumberValue(), 'Chave 1');
                read(4); addRow('Pr1', getSignedNumberValue(), 'RRN da Chave 1');
                read(4); addRow('C2', getSignedNumberValue(), 'Chave 2');
                read(4); addRow('Pr2', getSignedNumberValue(), 'RRN da Chave 2');
                read(4); addRow('C3', getSignedNumberValue(), 'Chave 3');
                read(4); addRow('Pr3', getSignedNumberValue(), 'RRN da Chave 3');
                
                read(4); addRow('P1', getSignedNumberValue(), 'Ponteiro < C1');
                read(4); addRow('P2', getSignedNumberValue(), 'Ponteiro < C2');
                read(4); addRow('P3', getSignedNumberValue(), 'Ponteiro < C3');
                read(4); addRow('P4', getSignedNumberValue(), 'Ponteiro > C3');
            });
        }
        
    } else {
        // ==========================================
        // PARSER DO ARQUIVO DE DADOS (80 bytes)
        // (Será executado para estacoes.bin, etc.)
        // ==========================================
        read(1); addRow('status', getStringValue(), '0 inconsistente, 1 consistente');
        read(4); addRow('topo', getSignedNumberValue(), 'RRN do topo da pilha');
        read(4); let proxRRN = getSignedNumberValue(); addRow('proxRRN', proxRRN, '');
        read(4); addRow('nroEstacoes', getSignedNumberValue(), '');
        read(4); addRow('nroParesEstacao', getSignedNumberValue(), '');

        for (let i = 0; i < proxRRN; i++) {
            read(80);
            addRow(`Registro de Dados (RRN ${i})`, '');
            addDetails(() => {
                read(1); addRow('removido', getStringValue(), '');
                read(4); addRow('proximo', getSignedNumberValue(), '');
                read(4); addRow('codEstacao', getSignedNumberValue(), '');
                read(4); addRow('codLinha', getSignedNumberValue(), '');
                read(4); addRow('codProxEstacao', getSignedNumberValue(), '');
                read(4); addRow('distProxEstacao', getSignedNumberValue(), '');
                read(4); addRow('codLinhaIntegra', getSignedNumberValue(), '');
                read(4); addRow('codEstIntegra', getSignedNumberValue(), '');

                read(4); let tamNomeEstacao = getSignedNumberValue(); addRow('tamNomeEstacao', tamNomeEstacao, '');
                if (tamNomeEstacao > 0) { read(tamNomeEstacao); addRow('nomeEstacao', getStringValue(), ''); }

                read(4); let tamNomeLinha = getSignedNumberValue(); addRow('tamNomeLinha', tamNomeLinha, '');
                if (tamNomeLinha > 0) { read(tamNomeLinha); addRow('nomeLinha', getStringValue(), ''); }

                let bytesLidos = 37 + tamNomeEstacao + tamNomeLinha;
                let lixo = 80 - bytesLidos;
                if (lixo > 0) { read(lixo); addRow('Lixo ($)', getStringValue(), `${lixo} bytes finais`); }
            });
        }
    }
});