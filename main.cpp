/****************************************
 * Trabalho 1 - Software Basico         *
 *                                      *
 * Ian Nery Bandeira                    *
 * 170144739                            *
 *                                      *
 * Versao do compilador:                *
 * g++ (Ubuntu 9.3.0-10ubuntu2) 9.3.0   *
 *                                      *
 ****************************************/

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Tradutor.h"

using namespace std;

int main(int argc, char* argv[]) {
    string file = argv[1];
    // passa para o construtor do montador o nome do arquivo .asm
    Tradutor* tradutor = new Tradutor(file); 
    // passa para a inicializacao do montador o comando "-p" ou "-o" 
    tradutor->inicializar_processo();
	return 0;
}