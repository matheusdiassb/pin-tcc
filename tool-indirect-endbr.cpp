#include "pin.H"
#include <iostream>
#include <iomanip>
#include <string>

// Rotina de análise: recebe o endereço de destino do desvio/chamada indireta
// e verifica se a primeira instrução no destino é um endbr64 (F3 0F 1E FA).
VOID VerificarDestinoEndbr(ADDRINT targetAddr,
                           ADDRINT origemAddr,
                           const std::string *origemDisasm)
{
    // Buffer para leitura segura dos 4 bytes da instrução de destino
    unsigned char bytes[4] = {0};
    size_t copied = PIN_SafeCopy(bytes, reinterpret_cast<const VOID *>(targetAddr), sizeof(bytes));

    std::cout << "\n[PIN] Desvio/Chamada indireta tomada" << std::endl;
    std::cout << "  Origem: 0x" << std::hex << origemAddr;

    if (origemDisasm)
    {
        std::cout << "  |  Instrucao: " << *origemDisasm;
    }
    std::cout << std::endl;

    if (copied != sizeof(bytes))
    {
        std::cout << "  Nao foi possivel ler 4 bytes do destino em 0x"
                  << std::hex << targetAddr << " (copied=" << std::dec << copied << ")"
                  << std::endl;
        return;
    }

    std::cout << "  Destino: 0x" << std::hex << targetAddr << std::endl;
    std::cout << "  Bytes do destino em HEX: ";
    for (size_t i = 0; i < sizeof(bytes); i++)
    {
        std::cout << std::hex << std::uppercase << std::setw(2)
                  << std::setfill('0') << static_cast<int>(bytes[i]);
    }
    std::cout << std::endl;

    // Verificacao do opcode endbr64: F3 0F 1E FA
    if (bytes[0] == 0xF3 && bytes[1] == 0x0F &&
        bytes[2] == 0x1E && bytes[3] == 0xFA)
    {
        std::cout << "  Sucesso! Instrucao de destino eh endbr64." << std::endl;
    }
    else
    {
        std::cout << "  Instrucao de destino NAO eh endbr64." << std::endl;
    }
}

// Rotina de instrumentacao por instrucao: intercepta todos os saltos/chamadas indiretas
// e insere chamadas para VerificarDestinoEndbr.
VOID InstrumentarInstrucoes(INS ins, VOID *v)
{
    // Filtra apenas desvios/chamadas indiretas (branches e calls indiretos).
    // Usamos somente instruções que:
    //  - são fluxo de controle indireto (destino calculado em tempo de execução)
    //  - e são chamadas ou saltos (não rets, a menos que queira incluí-los depois)
    if (!(INS_IsIndirectControlFlow(ins) &&
          (INS_IsCall(ins) || INS_IsBranch(ins))))
    {
        return;
    }

    // Endereco da instrucao de origem (para log).
    ADDRINT origemAddr = INS_Address(ins);

    // Disassembly/mnemonico da instrucao de origem (para log).
    std::string *origemDisasm = new std::string(INS_Disassemble(ins));

    // Inserimos a chamada no ponto em que o desvio eh realmente tomado.
    INS_InsertCall(ins,
                   IPOINT_TAKEN_BRANCH,
                   (AFUNPTR)VerificarDestinoEndbr,
                   IARG_BRANCH_TARGET_ADDR,          // endereco real de destino em tempo de execucao
                   IARG_ADDRINT, origemAddr,         // endereco da instrucao de origem
                   IARG_PTR, origemDisasm,           // disassembly da instrucao de origem
                   IARG_END);
}

int main(int argc, char *argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        std::cerr << "Erro ao inicializar PIN" << std::endl;
        return 1;
    }

    // Instrumentacao ao nivel de instrucao (INS), sem qualquer dependencia de Veiculo::buzinar.
    INS_AddInstrumentFunction(InstrumentarInstrucoes, 0);

    PIN_StartProgram();

    return 0;
}

