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

#ifndef LIGADOR_BIB
#define LIGADOR_BIB

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class Tradutor {
public:
    Tradutor(string asm_path_to_file);
    ~Tradutor();
    void inicializar_processo();

private:
    void preprocess();
    void link_1();
    void link_2();
    void if_equ_handler();
    string asm_path, preprocessed_path, mounted_path, line;
    vector<string> directive_list;
    vector<pair<string, int>> symbol_table, opcode_list;
};


#endif