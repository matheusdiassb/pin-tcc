#include "pin.H"
#include <iostream>
#include <string>

VOID VerificarInstrucao(const std::string* nomeRotina, const std::string* mnemonico) {
	std::cout << "\n Interceptado: " << *nomeRotina << std::endl;
	std::cout << "Instrucao seguinte: " << *mnemonico << std::endl;

	if (*mnemonico == "endbr64") {
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
