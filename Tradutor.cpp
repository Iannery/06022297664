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
    this->ia32_path = this->asm_path.substr(0, this->asm_path.find(".")) + ".s"; // utiliza o asm_path para criar um path do arquivo .s
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
    this->opcode_list.push_back(make_pair("S_INPUT", 3));
    this->opcode_list.push_back(make_pair("S_OUTPUT", 3));
    this->preprocess();
    this->link_1();
    this->translate_data();
    this->translate_text();
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
    int flag_section_text   = 0,
        flag_section_data   = 0;
    vector<string> command_list, command_line;
    ifstream preprocessed_file;
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
        if(command_list.at(i).find("SECTION TEXT") != std::string::npos){
            flag_section_text = 1;
            flag_section_data = 0;
            continue;
        }
        // se a linha mostrar que esta na secao de dados, sobe a flag que esta nela
        else if(command_list.at(i).find("SECTION DATA") != std::string::npos){
            flag_section_text = 0;
            flag_section_data = 1;
            continue;
        }
        if(flag_section_text){
            this->text_list.push_back(command_list.at(i));
        }
        else if(flag_section_data){
            this->data_list.push_back(command_list.at(i));
        }
    }


    // istringstream iss(command_list.at(i));
    // command_line.clear();
    // for(string s; iss >> s;){
    //     command_line.push_back(s);
    // }
    // // fim da subrotina
    // for(auto& s: command_line){ // para cada substring da linha de comando
    // }

}

void Tradutor::translate_data(){
    string  data_line       = "",
            new_command     = "";
    vector<string> data_line_list;
    this->new_data_list.push_back("section .data");
    this->bss_list.push_back("section .bss");
    this->bss_list.push_back("\tNBUFFER: resb 5");
    for(size_t i = 0; i < this->data_list.size(); i++){

        data_line = this->data_list.at(i);
        istringstream iss(data_line);
        data_line_list.clear();
        for(string s; iss >> s;){
            data_line_list.push_back(s);
        }
        if(data_line.find("CONST") != std::string::npos){
            for(auto& s: data_line_list){ // para cada substring da linha de dados
                if(s.find(":") != std::string::npos){
                    new_command = s.substr(0, s.find(":"));
                }
                else if(s == "CONST"){
                    new_command += " dd ";
                }
                else{
                    new_command += s;
                }
            }
            this->new_data_list.push_back("\t" + new_command);
            new_command = "";
        }
        else if(data_line.find("SPACE") != std::string::npos){
            if(data_line_list.size() >= 3){
                for(auto& s: data_line_list){ // para cada substring da linha de dados
                    if(s.find(":") != std::string::npos){
                        new_command = s.substr(0, s.find(":"));
                    }
                    else if(s == "SPACE"){
                        new_command += " resd ";
                    }
                    else{
                        new_command += s;
                    }
                }
            }
            else{
                for(auto& s: data_line_list){ // para cada substring da linha de dados
                    if(s.find(":") != std::string::npos){
                        new_command = s.substr(0, s.find(":"));
                    }
                    else if(s == "SPACE"){
                        new_command += " resd 1";
                    }
                    else{
                        new_command += s;
                    }
                }
            }
            this->bss_list.push_back("\t" + new_command);
            new_command = "";
        }
    }
    // for(size_t i = 0; i < this->bss_list.size(); i++){
    //     cout << this->bss_list.at(i) << endl;
    // }
    // for(size_t i = 0; i < this->new_data_list.size(); i++){
    //     cout << this->new_data_list.at(i) << endl;
    // }
}


void Tradutor::translate_text(){
    int no_label_flag = 0,
        opcode_found_flag = 0;
    string text_line, token, label, opcode;
    vector<string> text_line_list, operands, ia32_line_list;
    this->new_text_list.push_back("section .text");
    this->new_text_list.push_back("_start:");
    for(size_t i = 0; i < this->text_list.size(); i++){
        cout << this->text_list.at(i) << endl;
        no_label_flag = 0;
        opcode_found_flag = 0;
        label = "";
        opcode = "";
        operands.clear();
        text_line = this->text_list.at(i);
        istringstream iss(text_line);
        text_line_list.clear();
        for(string s; iss >> s;){
            text_line_list.push_back(s);
        }
        for(size_t j = 0; j < text_line_list.size(); j++){
            token = text_line_list.at(j);
            if(j == 0){
                if(token.find(":") == std::string::npos){
                    no_label_flag = 1;
                }
                else{
                    label = token;
                }
            }
            if(!opcode_found_flag){
                for(size_t k = 1; k < this->opcode_list.size(); k++){
                    // checa se a substring eh um opcode
                    if(token == this->opcode_list.at(k).first){
                        opcode = token;
                        opcode_found_flag = 1;
                        break;
                        getchar();
                    }
                }
            }
            else{
                operands.push_back(token);
            }
        }
        ia32_line_list = opcode_to_ia32(opcode, operands);
        this->new_text_list.push_back("\t" + ia32_line_list);
    }
}

void Tradutor::create_overflow(){
    this->new_data_list.push_back("\t_msg_overflow db \"Overflow!!\", 0DH, 0AH");
    this->new_data_list.push_back("\t_msg_overflow_size equ $-_msg_overflow");
    this->new_text_list.push_back(";#### INICIO DA ROTINA DE OVERFLOW ####");
    // imprime a string da mensagem de overflow
    this->new_text_list.push_back("\tmov eax, 4");
    this->new_text_list.push_back("\tmov ebx, 1");
    this->new_text_list.push_back("\tmov ecx, _msg_overflow");
    this->new_text_list.push_back("\tmov edx, _msg_overflow_size");
    this->new_text_list.push_back("\tint 80h");
    // termina o programa
    this->new_text_list.push_back("\tmov eax, 1");
    this->new_text_list.push_back("\tmov ebx, 0");
    this->new_text_list.push_back("\tint 80h");
    this->new_text_list.push_back(";######################################");
}

void Tradutor::create_leerchar(){
    this->new_data_list.push_back("\t_msg_c_input db \"Numero de caracteres lidos: \", 0DH, 0AH");
    this->new_data_list.push_back("\t_msg_c_input_size equ $-_msg_c_input");
    this->new_text_list.push_back(";#### INICIO DA ROTINA PARA LER CHAR ####");
    this->new_text_list.push_back("LeerChar:");
    this->new_text_list.push_back("\tenter 0,0");
    this->new_text_list.push_back("\tmov eax, 3");
    this->new_text_list.push_back("\tmov ebx, 0");
    this->new_text_list.push_back("\tmov ecx, [ESP+8]");
    this->new_text_list.push_back("\tmov edx, 1");
    this->new_text_list.push_back("\tint 80h");

    this->new_text_list.push_back("\tmov eax, 1");
    this->new_text_list.push_back("\tleave");
    this->new_text_list.push_back("\tret 4"); // retorna 4 bytes para o esp estar apontando para o eax
    this->new_text_list.push_back(";########################################");
    // FALTA ROTINA PRA PRINTAR O VALOR DE CARACTERES LIDOS
}

void Tradutor::create_escreverchar(){
    this->new_data_list.push_back("\t_msg_c_output db \"Numero de caracteres escritos: \", 0DH, 0AH");
    this->new_data_list.push_back("\t_msg_c_output_size equ $-_msg_c_output");

    this->bss_list.push_back("\t_leitor resb 5");
    this->bss_list.push_back("\t_leitor_size equ $-_leitor");

    this->new_text_list.push_back(";#### INICIO DA ROTINA P ESCREVER CHAR ####");
    this->new_text_list.push_back("EscreverChar:");
    this->new_text_list.push_back("\tenter 0,0");
    this->new_text_list.push_back("\tmov eax, 4");
    this->new_text_list.push_back("\tmov ebx, 1");
    this->new_text_list.push_back("\tmov ecx, [ESP+8]");
    this->new_text_list.push_back("\tmov edx, 1");
    this->new_text_list.push_back("\tint 80h");
    this->new_text_list.push_back("\n");

    this->new_text_list.push_back("\tmov [_leitor], eax");
    this->new_text_list.push_back("\n");

    this->new_text_list.push_back("\tmov eax, 4");
    this->new_text_list.push_back("\tmov ebx, 1");
    this->new_text_list.push_back("\tmov ecx, _msg_c_output");
    this->new_text_list.push_back("\tmov edx, _msg_c_output_size");
    this->new_text_list.push_back("\tint 80h");
    this->new_text_list.push_back("\n");


    this->new_text_list.push_back("\tmov eax, 4");
    this->new_text_list.push_back("\tmov ebx, 1");
    this->new_text_list.push_back("\tmov ecx, _leitor");
    this->new_text_list.push_back("\tmov edx, _leitor_size");
    this->new_text_list.push_back("\tint 80h");
    this->new_text_list.push_back("\n");


    this->new_text_list.push_back("\tmov eax, [_leitor]");
    this->new_text_list.push_back("\tleave");
    this->new_text_list.push_back("\tret 4"); // retorna 4 bytes para o esp estar apontando para o eax
    this->new_text_list.push_back(";##########################################");
    this->new_text_list.push_back("\n");

}



vector<string> Tradutor::opcode_to_ia32(string opcode, vector<string> operands){
    int plus_found = 0; 
    size_t comma_position = -1, i;
    vector<string> new_opcode;
    new_opcode.clear();
    string aux_token = "", concat_operands = "";

    // tratar pra ver se tem '+'
    for(i = 0; i < operands.size(); i++){
        aux_token = operands.at(i);
        if(aux_token.find(",") != std::string::npos){
            operands.at(i) = operands.at(i).substr(0, operands.at(i).find(","));
            aux_token = operands.at(i);
            comma_position = i;
        }
        if(aux_token == "+"){
            plus_found = 1;
            continue;
        }
        if(plus_found){
            operands.at(i) = to_string(stoi(aux_token) * 4);
            plus_found = 0;
        }
    }
    if(comma_position != -1){ // tratamento para dois operandos
        for(i = 0; i <= comma_position; i++){
            concat_operands += operands.at(i);
        }
        operands.erase(operands.begin(), operands.begin() + i);
        operands.insert(operands.begin(), concat_operands);
        concat_operands = "";
        for(i = 1; i < operands.size(); i++){
            concat_operands += operands.at(i);
        }
        operands.erase(operands.begin() + 1, operands.end());
        operands.push_back(concat_operands);
    }
    else{ // tratamento para um operando
        for(i = 0; i < operands.size(); i++){
            concat_operands += operands.at(i);
        }
        operands.erase(operands.begin(), operands.end());
        operands.push_back(concat_operands);
    }
    cout << "OPERANDS = " << endl;
    for(size_t a = 0; a < operands.size(); a++){
        cout << operands.at(a) << endl;
    }
    getchar();

    // ADD
    if(opcode == "ADD"){
        new_opcode.push_back("\tadd eax, [" + operands.at(0) + "]");
    }
    else if(opcode == "SUB"){
        new_opcode.push_back("\tsub eax, [" + operands.at(0) + "]");
    }
    else if(opcode == "MULT"){
        new_opcode.push_back("\tmov ecx, [" + operands.at(0) + "]");
        new_opcode.push_back("\timul ecx");
        new_opcode.push_back("\tjo _overflow_err");
    }
    else if(opcode == "DIV"){
        new_opcode.push_back("\tcdq");
        new_opcode.push_back("\tmov ecx, [" + operands.at(0) + "]");
        new_opcode.push_back("\tidiv ecx");
    }
    else if(opcode == "JMP"){
        new_opcode.push_back("\tjmp [" + operands.at(0) + "]");
    }
    else if(opcode == "JMPN"){
        new_opcode.push_back("\tcomp eax, 0");
        new_opcode.push_back("\tjl [" + operands.at(0) + "]");
    }
    else if(opcode == "JMPP"){
        new_opcode.push_back("\tcomp eax, 0");
        new_opcode.push_back("\tjg [" + operands.at(0) + "]");
    }
    else if(opcode == "JMPZ"){
        new_opcode.push_back("\tcomp eax, 0");
        new_opcode.push_back("\tje [" + operands.at(0) + "]");
    }
    else if(opcode == "COPY"){
        new_opcode.push_back("\tmov ebx, [" + operands.at(0) + "]");
        new_opcode.push_back("\tmov [" + operands.at(1) + "], ebx");

    }
    else if(opcode == "LOAD"){
        new_opcode.push_back("\tmov eax, [" + operands.at(0) + "]");
    }
    else if(opcode == "STORE"){
        new_opcode.push_back("\tmov [" + operands.at(0) + "], eax");
    }
    else if(opcode == "INPUT"){
    }
    else if(opcode == "OUTPUT"){
    }
    else if(opcode == "STOP"){
        new_opcode.push_back("\tmov eax, 1");
        new_opcode.push_back("\tmov ebx, 0");
        new_opcode.push_back("\tint 80h");
    }
    else if(opcode == "C_INPUT"){
        //pilha: 
        //  4 - eax
        //  8 - operando
        new_opcode.push_back("\tpush eax");
        new_opcode.push_back("\tpush " + operands.at(0));
        new_opcode.push_back("\tcall LeerChar");
        new_opcode.push_back("\tpop eax");
    }
    else if(opcode == "C_OUTPUT"){
        //pilha: 
        //  4 - eax
        //  8 - operando
        new_opcode.push_back("\tpush eax");
        new_opcode.push_back("\tpush " + operands.at(0));
        new_opcode.push_back("\tcall EscreverChar");
        new_opcode.push_back("\tpop eax"); // retorna o valor resultante da pilha pra eax
    }
    else if(opcode == "S_INPUT"){ // TEM DOIS OPERANDOS
        new_opcode.push_back("\tpush eax");
        new_opcode.push_back("\tpush " + operands.at(1)); // tamanho da string
        new_opcode.push_back("\tpush " + operands.at(0)); // local da string
        new_opcode.push_back("\tcall LeerString");
        new_opcode.push_back("\tpop eax");
    }
    else if(opcode == "S_OUTPUT"){ // TEM DOIS OPERANDOS
        new_opcode.push_back("\tpush eax");
        new_opcode.push_back("\tpush " + operands.at(1)); // tamanho da string
        new_opcode.push_back("\tpush " + operands.at(0)); // local da string
        new_opcode.push_back("\tcall EscreverString");
        new_opcode.push_back("\tpop eax");
    }
    
    return new_opcode;
}