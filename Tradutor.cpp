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
#include <bits/stdc++.h>
#include "Tradutor.h"
using namespace std;

Tradutor::Tradutor(string asm_path_to_file){
    this->asm_path = asm_path_to_file;
    this->preprocessed_path = this->asm_path.substr(0, this->asm_path.find(".")) + ".pre"; // utiliza o asm_path para criar um path do arquivo .pre
    this->mounted_path = this->asm_path.substr(0, this->asm_path.find(".")) + ".obj"; // utiliza o asm_path para criar um path do arquivo .obj
}

void Tradutor::inicializar_processo(){
    this->directive_list.push_back("SPACE");
    this->directive_list.push_back("CONST");
    // opcode de valor 0 necessário para o indice da lista bater com o codigo do opcode
    this->opcode_list.push_back(make_pair("NEVERCALLED", 0));
    this->opcode_list.push_back(make_pair("ADD", 2));
    this->opcode_list.push_back(make_pair("SUB", 2));
    this->opcode_list.push_back(make_pair("MULT", 2));
    this->opcode_list.push_back(make_pair("DIV", 2));
    this->opcode_list.push_back(make_pair("JMP", 2));
    this->opcode_list.push_back(make_pair("JMPN", 2));
    this->opcode_list.push_back(make_pair("JMPP", 2));
    this->opcode_list.push_back(make_pair("JMPZ", 2));
    this->opcode_list.push_back(make_pair("COPY", 3));
    this->opcode_list.push_back(make_pair("LOAD", 2));
    this->opcode_list.push_back(make_pair("STORE", 2));
    this->opcode_list.push_back(make_pair("INPUT", 2));
    this->opcode_list.push_back(make_pair("OUTPUT", 2));
    this->opcode_list.push_back(make_pair("STOP", 1));
    this->opcode_list.push_back(make_pair("C_INPUT", 2));
    this->opcode_list.push_back(make_pair("C_OUTPUT", 2));
    this->opcode_list.push_back(make_pair("NEVERCALLED", 0));
    this->opcode_list.push_back(make_pair("NEVERCALLED", 0));
    this->opcode_list.push_back(make_pair("S_INPUT", 2));
    this->opcode_list.push_back(make_pair("S_OUTPUT", 2));
    this->preprocess();
    this->link_1();
    this->link_2();
}

/* > preprocess()
 * Abre o arquivo .asm, cria o arquivo .pre, retira os comentarios, substitui os char's por maiusculas,
 * troca tabs por espacos, remove linhas vazias e chama os handlers de macro e if/equ.
 */
void Tradutor::preprocess(){
    ifstream asm_file;
    ofstream preprocessed_file;

    asm_file.open(this->asm_path);
    preprocessed_file.open(this->preprocessed_path);
    if (!asm_file.is_open()){
        cerr << "Erro na abertura do arquivo .asm";
    }
    else if (!preprocessed_file.is_open()){
        cerr << "Erro na abertura do arquivo .pre";
    }
    else{
        while (asm_file.good()){
            getline(asm_file, this->line); // pega linha do arquivo .asm
            this->line = this->line.substr(0, this->line.find(";")); // retira os comentarios
            transform(
                this->line.begin(),
                this->line.end(),
                this->line.begin(),
                ::toupper); // substitui todos os char por maiusculas
            replace(this->line.begin(), this->line.end(), '\t', ' '); // retira tabs
            if(this->line[this->line.length() - 1] == ' '){ // retira espacos no final de comandos
                this->line = this->line.substr(0, this->line.length() -1);
            }
            if (!this->line.empty() || this->line == " "){ // coloca no arquivo pre-processado caso a linha nao seja vazia
                preprocessed_file << this->line + "\n";
            }
        }
        asm_file.close();
        preprocessed_file.close();
        if_equ_handler();
    }
}

/* > if_equ_handler()
 * trata as chamadas das diretivas de EQU's e IF's presentes no codigo, e as remove do arquivo .pre
 */
void Tradutor::if_equ_handler(){
    vector<string> command_list;
    vector< pair <string,string> > equ_statements;
    string  equ_label = "",
            equ_value = "",
            if_value  = "";
    ifstream preprocessed_file_in;
    ofstream preprocessed_file_out;

    preprocessed_file_in.open(this->preprocessed_path);
    if (preprocessed_file_in.is_open()){
        while (preprocessed_file_in.good()){
            getline(preprocessed_file_in, this->line);
            command_list.push_back(this->line); // passa as linhas do arquivo .pre para uma lista de comandos
        }
    }
    preprocessed_file_in.close();
    //IDENTIFICACAO DO EQU
    //Inicializa a lista de pares {string,string} para lidar com as declaracoes de EQU e seus valores
    for(size_t i = 0; i < command_list.size(); i++){
        if(command_list.at(i).find("EQU") != std::string::npos){ // se acha a declaracao do "EQU"
            // guarda a label do EQU, para depois ser feita a substituicao onde ela for chamada
            equ_label = command_list.at(i).substr(
                0,
                command_list.at(i).find(":")
            );
            // guarda o valor do EQU, para ser feita a substituicao da label pelo valor onde for chamada
            equ_value = command_list.at(i).substr(
                command_list.at(i).find("EQU") + 4,
                command_list.at(i).length()
            );
            // coloca na lista de pares a label com o seu respectivo valor
            equ_statements.push_back(make_pair(equ_label, equ_value));
            // retira da lista de comandos a declaracao do EQU
            command_list.erase(command_list.begin() + i);
            i--;
        }
    }
    // SUBSTITUICAO DAS LABELS PELO VALOR DO EQU
    for(size_t i = 0; i < command_list.size(); i++){
        for(size_t j = 0; j < equ_statements.size(); j++){
            // se acha alguma label presente na lista de pares em alguma linha da lista de comandos
            if(command_list.at(i).find(equ_statements.at(j).first) != std::string::npos){
                // substitui a label presente pelo seu respectivo valor
                command_list.at(i).replace(
                    command_list.at(i).find(equ_statements.at(j).first),
                    equ_statements.at(j).first.length(),
                    equ_statements.at(j).second
                );
            }
        }
    }
    // CHECA SE VAI SER COMPUTADO O IF OU NAO 
    for(size_t i = 0; i < command_list.size(); i++){
        if(command_list.at(i).find("IF") != std::string::npos){ // se achar a diretiva IF
            // recebe o valor do IF
            if_value = command_list.at(i).substr(
                command_list.at(i).find("IF") + 3,
                command_list.at(i).length()
            );
            // caso o valor do IF for diferente de 1 ou variacoes
            if(if_value != "1" 
            && if_value != "1 " 
            && if_value != " 1"
            && if_value != " 1 "){
                // deleta o IF junto com a linha abaixo dele
                command_list.erase(
                    command_list.begin() + i,
                    command_list.begin() + i+2 
                );
            }
            // caso contrario
            else{
                // deleta apenas a linha do IF
                command_list.erase(command_list.begin() + i);
            }
            i--;
        }
    }
    remove(this->preprocessed_path.c_str());
    preprocessed_file_out.open(this->preprocessed_path);
    // reescreve o arquivo .pre com as diretivas de IF e EQU devidamente tratadas
    for (size_t i = 0; i < command_list.size(); i++){
        preprocessed_file_out << command_list.at(i) << "\n";
    }
    preprocessed_file_out.close();
}


void Tradutor::link_1(){
    int superscription_error_flag = 0,
        position_counter = 0;
    pair <string,int> aux_pair;
    vector<string> command_list, command_line;
    string  symbol_label = "";
    ifstream preprocessed_file_in;
    preprocessed_file_in.open(this->preprocessed_path);
    if (preprocessed_file_in.is_open()){
        while (preprocessed_file_in.good()){
            getline(preprocessed_file_in, this->line);
            command_list.push_back(this->line); // coloca as linhas do arquivo em uma lista de strings
        }
    }
    preprocessed_file_in.close();
    for(size_t i = 0; i < command_list.size(); i++){
        superscription_error_flag = 0;
        // subrotina para separar todas as substrings de uma linha da lista de comandos
        istringstream iss(command_list.at(i));
        command_line.clear();
        for(string s; iss >> s;){
            command_line.push_back(s);
        }
        // fim da subrotina
        for(auto& s: command_line){ // para cada substring, ou "token", da linha de comando
            if(s.find(":") != std::string::npos){ // se achar um :, quer dizer que existe uma declaracao de label
                // retira o : da substring e coloca numa string separada
                symbol_label = s.substr(
                    0,
                    s.length() - 1
                );
                // caso essa label ja exista na tabela de simbolos, sobe uma flag de erro para nao sobrescreve-la
                for(size_t j = 0; j < this->symbol_table.size(); j++){
                    if(this->symbol_table.at(j).first == symbol_label){
                        superscription_error_flag = 1;
                    }
                }
                // caso nao tenha dado o erro de sobrescricao da label, coloca a label na tabela de simbolos,
                // junto com a sua posicao de memoria do programa
                if(!superscription_error_flag){
                    aux_pair = make_pair(symbol_label, position_counter);
                    this->symbol_table.push_back(aux_pair);
                }
            }
            // se a substring for um opcode, soma no contador de posicao de memoria do programa o "valor de memoria" desse opcode
            for(size_t j = 0; j < this->opcode_list.size(); j++){
                if(this->opcode_list.at(j).first == s){
                    position_counter += this->opcode_list.at(j).second;
                }
            }
            // se a substring for uma diretiva, ou seja, "SPACE" ou "CONST", soma um ao contador de posicao.
            for(size_t j = 0; j < this->directive_list.size(); j++){
                if(this->directive_list.at(j) == s){
                    position_counter += 1;
                }
            }
        }
    }
    // for(size_t i = 0; i < this->symbol_table.size(); i++){
    //     cout << this->symbol_table.at(i).first << '\t' << this->symbol_table.at(i).second << endl;
    // }
}

void Tradutor::link_2(){
    int flag_const          = 0,
        flag_space          = 0,
        flag_section_text   = 0,
        flag_section_data   = 0,
        flag_label          = 0,
        flag_label_continue = 0,
        label_loc = 0;
    vector<int> object_list;
    vector<string> command_list, command_line;
    string  symbol_label = "";
    ifstream preprocessed_file;
    ofstream mounted_file;
    preprocessed_file.open(this->preprocessed_path);
    if (preprocessed_file.is_open()){
        while (preprocessed_file.good()){
            getline(preprocessed_file, this->line);
            command_list.push_back(this->line); // passa as linhas do arquivo .pre para uma lista de comandos
        }
    }
    preprocessed_file.close();
    // para cada linha da lista de comandos
    for(size_t i = 0; i < command_list.size(); i++){
        // se a linha mostrar que esta na secao de texto, sobe a flag que esta nela ate ir para a secao de dados
        if(command_list.at(i) == "SECTION TEXT"){
            flag_section_text = 1;
            flag_section_data = 0;
        }
        // se a linha mostrar que esta na secao de dados, sobe a flag que esta nela
        else if(command_list.at(i) == "SECTION DATA"){
            flag_section_text = 0;
            flag_section_data = 1;
        }
        // subrotina para eliminar a virgula entre os argumentos do opcode COPY
        if(command_list.at(i).find("COPY") != std::string::npos){
            command_list.at(i).erase(
                remove(
                    command_list.at(i).begin(), 
                    command_list.at(i).end(), 
                    ','), 
                command_list.at(i).end()
            );
        }
        // fim da subrotina
        // subrotina para separar todas as substrings de uma linha da lista de comandos
        istringstream iss(command_list.at(i));
        command_line.clear();
        for(string s; iss >> s;){
            command_line.push_back(s);
        }
        // fim da subrotina
        flag_space = 0;
        for(auto& s: command_line){ // para cada substring da linha de comando
            // se estiver dentro da secao de texto
            if(flag_section_text){
                if(flag_label_continue){
                    object_list.push_back(label_loc + stoi(s));
                    label_loc = 0;
                    flag_label = 0;
                    flag_label_continue = 0;
                }
                if(flag_label){
                    if(s != "+"){                        
                        object_list.push_back(label_loc);
                        flag_label = 0;
                        label_loc = 0;
                    }
                    else{
                        flag_label_continue = 1;
                    }
                }
                for(size_t j = 1; j < this->opcode_list.size(); j++){
                    // se a substring for um opcode, coloca na lista de objetos o codigo do opcode
                    if(s == this->opcode_list.at(j).first){
                        int converter = static_cast<int>(j);
                        object_list.push_back(converter);
                    }
                }
                for(size_t j = 0; j < this->symbol_table.size(); j++){
                    // se a substring for uma label, varre a tabela de simbolos ate encontrar, e entao
                    // coloca na lista de objetos a posicao de memoria gravada na tabela
                    if(s == this->symbol_table.at(j).first){
                        flag_label = 1;
                        label_loc = this->symbol_table.at(j).second;
                    }
                }
            }
            else if(flag_section_data){
                // se a substring anterior tivesse sido CONST
                // coloca na lista de objetos o valor inteiro do valor desse const
                if(flag_const){
                    object_list.push_back(stoi(s));
                    flag_const = 0;
                }
                else if(flag_space){
                    for(int i = 0; i < stoi(s) - 1; i++){
                        object_list.push_back(0);
                    }
                    flag_space = 0;
                }
                // se a substring for a diretiva SPACE,
                // sobe uma flag para armazenar a proxima substring na lista de objetos
                else if(s == this->directive_list.at(0)){
                    object_list.push_back(0);
                    flag_space = 1;
                }
                // se a substring for a diretiva CONST,
                // sobe a flag para armazenar a proxima substring na lista de objetos
                else if(s == this->directive_list.at(1)){
                    flag_const = 1;
                }
            }
        }
    }
    // cria o arquivo .obj e coloca nele a lista de objetos criada acima, separando os objetos com " "
    mounted_file.open(this->mounted_path);
    if (!mounted_file.is_open()){
        cerr << "Erro na abertura do arquivo .obj";
    }
    else{    
        for(size_t i = 0; i < object_list.size(); i++){
            if(i < object_list.size() - 1){
                mounted_file << object_list.at(i) << " ";
            }
            else{
                mounted_file << object_list.at(i) << "\n";
            }
        }
    }
    mounted_file.close();

    // for(size_t i = 0; i < object_list.size(); i++){
    //     if(i < object_list.size() - 1){
    //         cout << object_list.at(i) << " ";
    //     }
    //     else{
    //         cout << object_list.at(i) << "\n";
    //     }
    // }
}