#include "pin.H"
#include <iostream>
#include <string>

VOID VerificarInstrucao(
		const std::string* nomeRotina,
	       	const std::string* mnemonico,
		ADDRINT endereco,
		UINT32 tamanho) {
	std::cout << "\n Interceptado: " << *nomeRotina << std::endl;
	std::cout << "Instrucao seguinte: " << *mnemonico << std::endl;
	
	unsigned char* bytes = (unsigned char*) endereco;

	std::cout << "Instrucao em HEX: " << std::endl;
	for (UINT32 i = 0 ; i < tamanho; i++) {
		std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)bytes[i];	
	}

	std::cout << std::endl;

	if (tamanho == 4 && bytes[0] == 0xF3 && bytes[1] == 0x0F && bytes[2] == 0x1E && bytes[3] == 0xFA) {
		std::cout << "Sucesso! Instrucao encontrada endbr64" << std::endl;
	} else {
		std::cout << "Instrucao esperada: endbr64, porem encontrou: " << *mnemonico << std::endl;
	}
}

VOID InstrumentarRotinas(RTN rtn, VOID* v) {
	std::string nome = PIN_UndecorateSymbolName(RTN_Name(rtn), UNDECORATION_COMPLETE);

	if (nome.find("Veiculo::buzinar()") != std::string::npos) {
		RTN_Open(rtn);
		INS primeiraIns = RTN_InsHead(rtn);

		if (INS_Valid(primeiraIns)) {
			std::string* mnemonico = new std::string(INS_Mnemonic(primeiraIns));
			std::string* nomeRotina = new std::string(nome);

			RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)VerificarInstrucao,
					IARG_PTR, nomeRotina,
					IARG_PTR, mnemonico,
					IARG_ADDRINT, INS_Address(primeiraIns),
					IARG_UINT32, INS_Size(primeiraIns),
					IARG_END);
		}
		RTN_Close(rtn);
	}
}

int main (int argc, char* argv[]){

	PIN_InitSymbols();

	if (PIN_Init(argc, argv)) {
		std::cerr << "Erro ao inicializar PIN" << std::endl;
		return 1;
	}

	RTN_AddInstrumentFunction(InstrumentarRotinas, 0);

	PIN_StartProgram();

	return 0;
}
