registerFileType((fileExt, filePath, fileData) => {
	if (fileExt == 'bin') return true;
	return false;
});

registerParser(() => {


	addStandardHeader();

	//Cabeçalho
	read(1);
	addRow('status', getStringValue(), '0 para inconsistente, 1 para consistente');
	read(4);
	addRow('topo', getSignedNumberValue(), '-1 para nenhum removido');
	read(4);
	let proxRRN = getSignedNumberValue();
	addRow('proxRRN', proxRRN, '');
	read(4);
	addRow('nroEstacoes', getSignedNumberValue(), '');
	read(4);
	addRow('nroParesEstacao', getSignedNumberValue(), '');

	//Dados
	let tamNomeEstacao, tamNomeLinha;
	for (let i = 0; i < proxRRN; i++) {
		read(80);
		addRow(`RRN ${i}`, '');
		addDetails(() => {
			read(1);
			addRow('removido', getStringValue(), '0 para não removido, 1 para removido');
			read(4);
			addRow('proximo', getSignedNumberValue(), '-1 para não removido');
			read(4);
			addRow('codEstacao', getSignedNumberValue(), '');
			read(4);
			addRow('codLinha', getSignedNumberValue(), '');
			read(4);
			addRow('codProxEstacao', getSignedNumberValue(), '');
			read(4);
			addRow('distProxEstacao', getSignedNumberValue(), '');
			read(4);
			addRow('codLinhaIntegra', getSignedNumberValue(), '');
			read(4);
			addRow('codEstIntegra', getSignedNumberValue(), '');

			read(4);
			tamNomeEstacao = getSignedNumberValue();
			addRow('tamNomeEstacao', tamNomeEstacao, '');
			read(tamNomeEstacao);
			addRow('nomeEstacao', getStringValue(), '');

			read(4);
			tamNomeLinha = getSignedNumberValue();
			addRow('tamNomeLinha', tamNomeLinha, '');
			read(tamNomeLinha);
			addRow('nomeLinha', getStringValue(), '');

			read(43 - tamNomeEstacao - tamNomeLinha);
			addRow('lixo', getStringValue(), '');
		})
	}
});