#include <iostream>
#include <string>


class Veiculo {

	protected:
		std::string marca;

	public:
		Veiculo(std::string m) : marca(m){}

		void buzinar () {
			std::cout << "Beep beep!" << std::endl;
		}
};

class Carro : public Veiculo {
	private:
		std::string modelo;

	public:
		Carro(std::string m, std::string mod) : Veiculo(m), modelo(mod){}

		void mostrarDetalhes () {
			std::cout << "Marca: " << marca <<  " | Modelo: " << modelo << std::endl;
		}
};

int main() {
	Carro meuCarro("Ford", "Mustang");

	meuCarro.buzinar();

	return 0;
}
