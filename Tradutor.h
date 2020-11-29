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
    void translate_data();
    void translate_text();
    vector<string> opcode_to_ia32(string opcode, vector<string> operands);
    // void link_2();
    void if_equ_handler();
    void create_overflow();
    void create_leerchar();
    void create_escreverchar();
    void create_leerstring();
    void create_escreverstring();
    string asm_path, preprocessed_path, ia32_path, line;
    vector<string> directive_list,
    data_list,
    new_data_list,
    bss_list,
    text_list,
    new_text_list;
    vector<pair<string, int>> symbol_table, opcode_list;
};


#endif