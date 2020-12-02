/****************************************
 * Trabalho 2 - Software Basico         *
 *                                      *
 * Ian Nery Bandeira                    *
 * 170144739                            *
 *                                      *
 * Versao do cmpilador:                *
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
    this->link();
    this->translate_data();
    this->translate_text();
    this->insert_procedures();
    this->translation_to_file();
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

/* > link()
 * A função "liga" o arquivo preprocessado às respectivas listas das seções DATA e TEXT, para depois
 * serem tratadas e se tornarem as seções .data, .bss e .text .
 */
void Tradutor::link(){
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
            // coloca na lista de texto tudo pertencente à section text
            this->text_list.push_back(command_list.at(i));
        }
        else if(flag_section_data){
            // coloca na lista de data tudo pertencente à section data
            this->data_list.push_back(command_list.at(i));
        }
    }
}

/* > translate_data()
 * A função começa o processo de traducao, fazendo a tradução da lista de elementos presentes
 * na seção de dados para as duas listas de elementos que pertencerão às seções .data e .bss,
 * dependendo se a label foi declarada como CONST ou SPACE, respectivamente.
 */
void Tradutor::translate_data(){
    string  data_line       = "",
            new_command     = "";
    vector<string> data_line_list;
    // nova seção de data, populada com os CONST
    this->new_data_list.push_back("section .data");
    // nova seção de data, populada com os SPACE
    this->bss_list.push_back("section .bss");
    this->bss_list.push_back("\tNBUFFER resb 12"); // necessario para procedures
    this->bss_list.push_back("\tAUX_INPUT resd 1"); // necessario para procedures
    for(size_t i = 0; i < this->data_list.size(); i++){
        data_line = this->data_list.at(i);
        istringstream iss(data_line);
        data_line_list.clear();
        for(string s; iss >> s;){
            data_line_list.push_back(s);
        }
        // se a linha possuir um CONST, ou seja, se a label declarada é uma constante:
        if(data_line.find("CONST") != std::string::npos){
            for(auto& s: data_line_list){ // para cada token
                if(s.find(":") != std::string::npos){
                    // se acha uma label, retira o char ":" dela para guardar apenas o token da label
                    new_command = s.substr(0, s.find(":"));
                }
                // se o token for o proprio CONST, concatena ao token da label a string dd
                else if(s == "CONST"){
                    // dd = declare double, pois todos os elementos são 32 bits.
                    new_command += " dd ";
                }
                else{
                    // se o token não é uma label ou a diretiva CONST, só pode ser um número,
                    // então o concatena na string de comando.
                    new_command += s;
                }
            }
            // coloca a string de novo comando em uma nova lista que consiste apenas dos dados da seção .data
            this->new_data_list.push_back("\t" + new_command);
            new_command = "";
        }
        // procedimento análogo ao da linha 255, mas para labels declaradas com SPACE:
        else if(data_line.find("SPACE") != std::string::npos){
            // se o tamanho é maior que 3, isso significa que existe um operando
            // com o tamanho daquele space, entao grava o numero logo apos o resd.
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
                // se não possui operando, coloca 1 no final do resd
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
            // coloca a string de novo comando em uma nova lista que consiste apenas dos dados da seção .bss
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

/* > translate_text()
 * A função continua o processo de traducao, fazendo a tradução da lista de elementos presentes
 * na seção de texto para a lista de elementos do .text, fazendo a propria tradução de cada opcode
 * do assembly inventado em suas equivalências de IA32.
 */
void Tradutor::translate_text(){
    int opcode_found_flag = 0;
    string text_line, token, label, opcode;
    vector<string> text_line_list, operands, ia32_line_list;
    // inicializa a lista com os comandos necessarios para o funcionamento do programa
    this->new_text_list.push_back("section .text");
    this->new_text_list.push_back("_start:");
    for(size_t i = 0; i < this->text_list.size(); i++){
        opcode_found_flag = 0;
        label = "";
        opcode = "";
        operands.clear();
        text_line = this->text_list.at(i);
        istringstream iss(text_line);
        text_line_list.clear();
        // separa as linhas da section text em tokens
        for(string s; iss >> s;){
            text_line_list.push_back(s);
        }
        // para cada token
        for(size_t j = 0; j < text_line_list.size(); j++){
            token = text_line_list.at(j);
            // se é o primeiro token, testa se ele é uma label
            if(j == 0){
                // se for label, guarda ela
                if(token.find(":") != std::string::npos){
                    label = token;
                }
            }
            // enquanto o comando não foi achado como token
            if(!opcode_found_flag){
                // percorre a lista com todos os opcodes para ver se o opcode é algum deles
                for(size_t k = 1; k < this->opcode_list.size(); k++){
                    // checa se a substring eh um opcode, se for, salva ele,
                    // habilita a flag que achou o opcode, e sai do loop.
                    if(token == this->opcode_list.at(k).first){
                        opcode = token;
                        opcode_found_flag = 1;
                        break;
                        getchar();
                    }
                }
            }
            else{
                // se achou o operador, coloca os proximos elementos na lista de operandos
                operands.push_back(token);
            }
        }
        // leva os parâmetros de opcode, a lista de operandos e o label para a tradução
        // destes comandos para a sua variante em IA32, e retorna uma lista com os comandos
        // resultantes.
        ia32_line_list = opcode_to_ia32(opcode, operands, label);
        // insere esses comandos traduzidos na nova lista da seção .text
        for(size_t j = 0; j < ia32_line_list.size(); j++){
            this->new_text_list.push_back(ia32_line_list.at(j));
        }
    }
}

/* > translation_to_file()
 * A função insere os valores resultantes da tradução do arquivo .pre para o arquivo .s,
 * com os valores já traduzidos presentes nas listas new_data_list, bss_list e new_text_list.
 */

void Tradutor::translation_to_file(){
    ofstream ia32_file;
    ia32_file.open(this->ia32_path);
    if (!ia32_file.is_open()){
        cerr << "Erro na abertura do arquivo .obj";
    }
    else{
        ia32_file << "global _start\n";
        for(size_t i = 0; i < this->bss_list.size(); i++){
            ia32_file << this->bss_list.at(i) << "\n";
        }
        for(size_t i = 0; i < this->new_data_list.size(); i++){
            ia32_file << this->new_data_list.at(i) << "\n";
        }
        for(size_t i = 0; i < this->new_text_list.size(); i++){
            ia32_file << this->new_text_list.at(i) << "\n";
        }
    }
    ia32_file.close();
}

/* > insert_procedures()
 * A função chama as funções que inserem todos os procedimentos de input, output e overflow
 * na lista da seção .text .
 */
void Tradutor::insert_procedures(){
    this->new_text_list.push_back("\n");
    this->new_text_list.push_back("\n");
    this->new_text_list.push_back(";########### PROCEDURES ###########");
    this->new_text_list.push_back("\n");
    this->create_leerchar();
    this->create_escreverchar();
    this->create_leerstring();
    this->create_escreverstring();
    this->create_escreverint();
    this->create_converteint();
    this->create_overflow();
}


void Tradutor::create_overflow(){
    this->new_data_list.push_back("\t_msg_overflow db \'Overflow!!\', 0DH, 0AH");
    this->new_data_list.push_back("\t_msg_overflow_size equ $-_msg_overflow");

    this->new_text_list.push_back(";#### INICIO DA ROTINA DE OVERFLOW ####");
    // imprime a string da mensagem de overflow
    this->new_text_list.push_back("_overflow_err:");
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
    this->new_data_list.push_back("\t_msg_input db \'Numero de caracteres lidos: \'");
    this->new_data_list.push_back("\t_msg_input_size equ $-_msg_input");    
    this->new_data_list.push_back("\t_breakline db \'\', 0DH, 0AH");
    this->new_data_list.push_back("\t_breakline_size equ $-_breakline");

    // inicializa os registradores para fazer um read em um registrador de tamanho 2,
    // que consiste do char e um enter.
    this->new_text_list.push_back(";#### INICIO DA ROTINA PARA LER CHAR ####");
    this->new_text_list.push_back("LeerChar:");
    this->new_text_list.push_back("\tenter 0,0");
    this->new_text_list.push_back("\tmov eax, 3");
    this->new_text_list.push_back("\tmov ebx, 0");
    this->new_text_list.push_back("\tmov edx, 2");
    this->new_text_list.push_back("\tint 80h");
    // guarda o valor de retorno desse syscall para dizer quantos chars foram lidos.
    this->new_text_list.push_back("\tmov [AUX_INPUT], eax");
    // escreve a mensagem "Numero de caracteres lidos: " na tela.
    this->new_text_list.push_back("\tmov ecx, _msg_input");
    this->new_text_list.push_back("\tmov edx, _msg_input_size");
    this->new_text_list.push_back("\tcall EscreverString");
    // retorna o valor de retorno do primeiro syscall para o eax, e chama o procedure
    // "EscreveInteiro", junto de um buffer que está referenciado pelo ponteiro em ESI,
    // para transformar um int em uma string e printá-lo na tela.
    this->new_text_list.push_back("\tmov eax, [AUX_INPUT]");
    this->new_text_list.push_back("\tlea esi, [NBUFFER]");
    this->new_text_list.push_back("\tcall EscreverInteiro");
    // pede para escrever uma string que quebra linha com o procedure "EscreverString".
    this->new_text_list.push_back("\tmov ecx, _breakline");
    this->new_text_list.push_back("\tmov edx, _breakline_size");
    this->new_text_list.push_back("\tcall EscreverString");
    // volta o valor do retorno do primeiro syscall para eax e o retorna para o programa.
    this->new_text_list.push_back("\tmov eax, [AUX_INPUT]");
    this->new_text_list.push_back("\tleave");
    this->new_text_list.push_back("\tret");
    this->new_text_list.push_back(";########################################");
}

void Tradutor::create_escreverchar(){
    // Escreve um char, é um procedure bem simples. Apenas chama os registradores para executar
    // um syscall de write de tamanho 2, que seriam o char e um enter.
    this->new_text_list.push_back(";#### INICIO DA ROTINA P ESCREVER CHAR ####");
    this->new_text_list.push_back("EscreverChar:");
    this->new_text_list.push_back("\tenter 0,0");
    this->new_text_list.push_back("\tmov eax, 4");
    this->new_text_list.push_back("\tmov ebx, 1");
    this->new_text_list.push_back("\tmov edx, 2");
    this->new_text_list.push_back("\tint 80h");
    this->new_text_list.push_back("\tleave");
    // já retorna automaticamente o eax com o valor de caracteres lidos depois do syscall.
    this->new_text_list.push_back("\tret");
    this->new_text_list.push_back(";##########################################");
    this->new_text_list.push_back("\n");
}

void Tradutor::create_leerstring(){
    // inicializa os registradores para fazer um read em um registrador de tamanho
    // declarado por um imediato passado antes do call da função.
    this->new_text_list.push_back(";#### INICIO DA ROTINA PARA LER STRING ####");
    this->new_text_list.push_back("LeerString:");
    this->new_text_list.push_back("\tenter 0,0");
    this->new_text_list.push_back("\tmov eax, 3");
    this->new_text_list.push_back("\tmov ebx, 0");
    this->new_text_list.push_back("\tint 80h");
    // análogo a função LeerChar, guarda o valor de retorno desse syscall para dizer quantos chars foram lidos.
    this->new_text_list.push_back("\tmov [AUX_INPUT], eax");
    // escreve a mensagem "Numero de caracteres lidos: " na tela.
    this->new_text_list.push_back("\tmov ecx, _msg_input");
    this->new_text_list.push_back("\tmov edx, _msg_input_size");
    this->new_text_list.push_back("\tcall EscreverString");
    // retorna o valor de retorno do primeiro syscall para o eax, e chama o procedure
    // "EscreveInteiro", junto de um buffer que está referenciado pelo ponteiro em ESI,
    // para transformar um int em uma string e printá-lo na tela.
    this->new_text_list.push_back("\tmov eax, [AUX_INPUT]");
    this->new_text_list.push_back("\tlea esi, [NBUFFER]");
    this->new_text_list.push_back("\tcall EscreverInteiro");
    // pede para escrever uma string que quebra linha com o procedure "EscreverString".
    this->new_text_list.push_back("\tmov ecx, _breakline");
    this->new_text_list.push_back("\tmov edx, _breakline_size");
    this->new_text_list.push_back("\tcall EscreverString");
    // retorna o valor do retorno do primeiro syscall para o eax.
    this->new_text_list.push_back("\tmov eax, [AUX_INPUT]");
    this->new_text_list.push_back("\tleave");
    this->new_text_list.push_back("\tret");
    this->new_text_list.push_back(";########################################");
    this->new_text_list.push_back("\n");
}


void Tradutor::create_escreverstring(){
    // Escreve uma string, é um procedure análogo ao EscreverChar. Chama os registradores
    // para executar um syscall de write de tamanho passado para edx antes do call do procedure.
    this->new_text_list.push_back(";#### INICIO DA ROTINA P ESCREVER STRING ####");
    this->new_text_list.push_back("EscreverString:");
    this->new_text_list.push_back("\tenter 0,0");
    this->new_text_list.push_back("\tmov eax, 4");
    this->new_text_list.push_back("\tmov ebx, 1");
    this->new_text_list.push_back("\tint 80h");
    this->new_text_list.push_back("\tleave");
    this->new_text_list.push_back("\tret");
    this->new_text_list.push_back(";##########################################");
    this->new_text_list.push_back("\n");
}

void Tradutor::create_converteint(){
    // Transforma uma string referenciada como ponteiro pelo ESI antes do call ao procedure
    // em um inteiro, e o guarda em eax para depois ser repassado à variável que foi pedida.
    this->new_text_list.push_back(";#### INICIO DA ROTINA P CONVERTER STR P INT ####");
    this->new_text_list.push_back("ConverteInteiro:");
    this->new_text_list.push_back("\tmov eax, 0");
    this->new_text_list.push_back("\tmov ebx, 10"); // numero a ser multiplicado
    this->new_text_list.push_back("\tmov edx, 0");
    this->new_text_list.push_back("\t; flag numero negativo");
    this->new_text_list.push_back("\tmov edi, 0"); // flag de numero negativo
    this->new_text_list.push_back("\tmovzx edx, BYTE [esi]"); // coloca o primeiro byte referenciado por ESI em edx
    this->new_text_list.push_back("\tcmp edx, '-'"); // se for um menos, é um número negativo.
    this->new_text_list.push_back("\tjne _convert2"); // se não for um número negativo, redireciona para a conversão.
    this->new_text_list.push_back("\t; tratamento para negativo");
    this->new_text_list.push_back("\tinc esi"); // não tenta converter o símbolo de menos, se não da pau
    this->new_text_list.push_back("\tmov edi, 1"); // seta o registrador usado como flag de numero negativo

    this->new_text_list.push_back("_convert2:");
    // coloca o primeiro byte de ESI (ou segundo, se for numero negativo) em edx
    this->new_text_list.push_back("\tmovzx edx, BYTE [esi]");
    this->new_text_list.push_back("\tinc esi"); // passa ESI pro proximo byte referenciado

    // ROTINA PARA VER SE EDX É UM VALOR ENTRE OS "CHARS" 0 E 9, OU SEJA, ASCII ENTRE 48 E 57
    this->new_text_list.push_back("\tcmp edx, 48");
    this->new_text_list.push_back("\tjl _neg_test");
    this->new_text_list.push_back("\tcmp edx, 57");
    this->new_text_list.push_back("\tjg _neg_test");
    // FIM DA ROTINA, SE FOR ALGO MENOR QUE 48 OU MAIOR QUE 57, LEVA PRA TESTAR SE ERA UM NUM NEGATIVO

    this->new_text_list.push_back("\t; se ainda for um numero");
    this->new_text_list.push_back("\tsub edx, 48"); // subtrai o valor ascii equivalente ao '0' em ascii, para virar inteiro
    this->new_text_list.push_back("\timul eax, ebx"); // caso ja tenha algo em eax, multiplica ele por 10 pra caber mais um char
    this->new_text_list.push_back("\tadd eax, edx"); // coloca o resultado de edx - 48 em eax
    this->new_text_list.push_back("\tjmp _convert2"); // reinicia o loop

    // ROTINA PARA CHECAR SE É UM NÚMERO NEGATIVO
    this->new_text_list.push_back("_neg_test:");
    this->new_text_list.push_back("\tcmp edi, 0"); // testa a flag de numero negativo
    this->new_text_list.push_back("\tje _return"); // se não estiver setada, termina o prodecure e retorna o inteiro em eax
    this->new_text_list.push_back("\t; se for numero negativo");
    this->new_text_list.push_back("\tmov edx, 0"); // edx = 0
    this->new_text_list.push_back("\tsub edx, eax"); // edx => edx(que é 0) - eax, que resulta em -eax
    this->new_text_list.push_back("\tmov eax, edx"); // eax => - eax

    this->new_text_list.push_back("_return:");
    this->new_text_list.push_back("\tret");
    this->new_text_list.push_back(";##########################################");

}


void Tradutor::create_escreverint(){
    // Transforma um inteiro presente em eax em uma string, que será armazenada em um buffer
    // de 12 bytes (só a leitura do inteiro tem limite de -999 a 999, a escrita não)
    // que está sendo referenciado pelo ponteiro de ESI, e printa essa string na tela com um syscall de write.
    this->new_text_list.push_back(";#### INICIO DA ROTINA P ESCREVER INT ####");
    this->new_text_list.push_back("EscreverInteiro:");
    this->new_text_list.push_back("\tenter 0,0");
    // inicializacao do que está dentro do buffer para não ter lixo nele de outros inteiros printados
    this->new_text_list.push_back("\tmov BYTE [esi], 0");
    this->new_text_list.push_back("\tmov BYTE [esi+1], 0");
    this->new_text_list.push_back("\tmov BYTE [esi+2], 0");
    this->new_text_list.push_back("\tmov BYTE [esi+3], 0");
    this->new_text_list.push_back("\tmov BYTE [esi+4], 0");
    this->new_text_list.push_back("\tmov BYTE [esi+5], 0");
    this->new_text_list.push_back("\tmov BYTE [esi+6], 0");
    this->new_text_list.push_back("\tmov BYTE [esi+7], 0");
    this->new_text_list.push_back("\tmov BYTE [esi+8], 0");
    this->new_text_list.push_back("\tmov BYTE [esi+9], 0");
    this->new_text_list.push_back("\tmov BYTE [esi+10], 0");
    this->new_text_list.push_back("\tmov BYTE [esi+11], 0");

    this->new_text_list.push_back("\tmov ebx, 10"); // valor que sera usado para dividir o eax e pegar cada char
    this->new_text_list.push_back("\t; flag de numero negativo");
    this->new_text_list.push_back("\tmov edi, 0"); // flag numero negativo
    this->new_text_list.push_back("\tcmp eax, 0"); // testa se o inteiro é negativo
    this->new_text_list.push_back("\tjnl _init"); // se for positivo, pula pra inicialização
    this->new_text_list.push_back("\t; tratamento para negativo");
    this->new_text_list.push_back("\tmov edi, 1"); // sobe flag de negativo
    this->new_text_list.push_back("\tmov edx, 0"); // edx = 0
    this->new_text_list.push_back("\tsub edx, eax"); // edx => edx(que é 0) - eax, que resulta em -eax
    this->new_text_list.push_back("\tmov eax, edx"); // eax => - eax

    this->new_text_list.push_back("_init:");
    // vai para o primeiro elemento do buffer
    this->new_text_list.push_back("\tadd esi, 11");
    this->new_text_list.push_back("_convert:"); // começa o loop para cada elemento menos significativo do inteiro
    this->new_text_list.push_back("\txor edx, edx"); // zera edx
    // divide o inteiro presente em eax por 10, então o resto (que é o elemento menos significativo do inteiro)
    // vai para o registrador de edx, o qual utilizaremos.
    this->new_text_list.push_back("\tdiv ebx");
    // soma o valor do primeiro byte de edx 48, que equivale ao char '0', para transformar o int em um char.
    this->new_text_list.push_back("\tadd dl, 48");
    // referencia o proximo byte referenciado por ESI, pois o primeiro tem que conter a quebra de linha. 
    this->new_text_list.push_back("\tdec esi"); 
    // coloca o ascii presente no primeiro byte de edx no byte do buffer referenciado em ESI
    this->new_text_list.push_back("\tmov [esi], dl"); 
    this->new_text_list.push_back("\ttest eax, eax"); // testa para ver se ainda existe algo em eax
    this->new_text_list.push_back("\tjne _convert"); // se existir, roda o loop mais uma vez
    this->new_text_list.push_back("\tcmp edi, 0"); // testa para ver se o numero era negativo
    this->new_text_list.push_back("\tje _print_int"); // se não for, leva para o syscall de write
    this->new_text_list.push_back("\t; se o numero for negativo");
    this->new_text_list.push_back("\txor edx, edx"); // zera edx
    this->new_text_list.push_back("\tadd dl, '-'"); // coloca no ultimo byte de edx o ascii do simbolo de menos
    this->new_text_list.push_back("\tdec esi"); // vai para o proximo byte do buffer
    this->new_text_list.push_back("\tmov [esi], dl"); // coloca o menos dentro do endereço referenciado por ESI

    this->new_text_list.push_back("_print_int:");
    this->new_text_list.push_back("\txor edx, edx"); // zera edx
    this->new_text_list.push_back("\tmov dl, BYTE [_breakline + 1]"); // coloca 0AH no ultimo byte de edx
    this->new_text_list.push_back("\tmov [NBUFFER + 11], dl"); // coloca na primeira posição do buffer, que não foi utilizada acima
    this->new_text_list.push_back("\tmov eax, 4");
    this->new_text_list.push_back("\tmov ebx, 1");
    this->new_text_list.push_back("\tmov ecx, NBUFFER");
    this->new_text_list.push_back("\tmov edx, 12"); // tamanho do buffer
    this->new_text_list.push_back("\tint 80h");
    this->new_text_list.push_back("\tleave");
    this->new_text_list.push_back("\tret");
    this->new_text_list.push_back(";#######################################");
    this->new_text_list.push_back("\n");
}


/* > opcode_to_ia32()
 * A função recebe o operador na linguagem assembly inventada, os seus respectivos operandos,
 * e sua label (se houver), se retorna uma lista de comandos em IA32 que equivalham à estes.
 */
vector<string> Tradutor::opcode_to_ia32(string opcode, vector<string> operands, string label){
    int plus_found = 0; 
    size_t comma_position = -1, i;
    vector<string> new_opcode;
    new_opcode.clear();
    string aux_token = "", concat_operands = "";
    // checa se existe label
    if(!label.empty()){
        // se existir, coloca na lista do opcode traduzido
        new_opcode.push_back(label);
    }
    // tratar pra ver se tem '+'
    for(i = 0; i < operands.size(); i++){
        aux_token = operands.at(i);
        // se existir uma vírgula no final de algum operando, retira ela e marca onde ela estava
        if(aux_token.find(",") != std::string::npos){
            operands.at(i) = operands.at(i).substr(0, operands.at(i).find(","));
            aux_token = operands.at(i);
            comma_position = i;
        }
        // se existir um mais, sobe uma flag que achou
        if(aux_token == "+"){
            plus_found = 1;
            continue;
        }
        // se a flag retorna 1, multiplica por 4 o valor existente no operando,
        // pois em assembly inventado os endereços são considerados todos de 4 bytes, logo,
        // para traduzir corretamente, precisa-se arrumar o offset de soma.
        if(plus_found){
            operands.at(i) = to_string(stoi(aux_token) * 4);
            plus_found = 0;
        }
    }
    // se achou uma vírgula, significa que existem dois operandos
    if(static_cast<int>(comma_position) != -1){ // tratamento para dois operandos
        // concatena tudo que tem antes da posição da vírgula e coloca como um só operando
        for(i = 0; i <= comma_position; i++){
            concat_operands += operands.at(i);
        }
        // coloca esse operando concatenado na posição 1
        operands.erase(operands.begin(), operands.begin() + i);
        operands.insert(operands.begin(), concat_operands);
        concat_operands = "";
        // concatena tudo que tem depois do primeiro operando concatenado
        for(i = 1; i < operands.size(); i++){
            concat_operands += operands.at(i);
        }
        // guarda o segundo operando concatenado na lista de operandos
        operands.erase(operands.begin() + 1, operands.end());
        operands.push_back(concat_operands);
    }
    else{ // tratamento para um operando só
        for(i = 0; i < operands.size(); i++){
            concat_operands += operands.at(i);
        }
        operands.erase(operands.begin(), operands.end());
        operands.push_back(concat_operands);
    }
    // rotina para OPCODE ADD
    if(opcode == "ADD"){
        new_opcode.push_back("\tadd eax, [" + operands.at(0) + "]");
    }
    // rotina para OPCODE SUB
    else if(opcode == "SUB"){
        new_opcode.push_back("\tsub eax, [" + operands.at(0) + "]");
    }
    // rotina para OPCODE MULT
    else if(opcode == "MULT"){
        new_opcode.push_back("\tmov ecx, [" + operands.at(0) + "]");
        new_opcode.push_back("\timul ecx"); // multiplicação com sinal
        new_opcode.push_back("\tjo _overflow_err"); // testa para ver se houve overflow na multiplicação
    }
    // rotina para OPCODE DIV
    else if(opcode == "DIV"){
        new_opcode.push_back("\tcdq"); // aloca o edx para o resto
        new_opcode.push_back("\tmov ecx, [" + operands.at(0) + "]");
        new_opcode.push_back("\tidiv ecx"); // divisão com sinal, não precisa de checar underflow
    }
    // rotina para OPCODE JMP
    else if(opcode == "JMP"){
        new_opcode.push_back("\tjmp " + operands.at(0));
    }
    // rotina para OPCODE JMPN, se o valor no acumulador for positivo
    else if(opcode == "JMPN"){
        new_opcode.push_back("\tcmp eax, 0");
        new_opcode.push_back("\tjl " + operands.at(0));
    }
    // rotina para OPCODE JMPP, se o valor no acumulador for positivo
    else if(opcode == "JMPP"){
        new_opcode.push_back("\tcmp eax, 0");
        new_opcode.push_back("\tjg " + operands.at(0));
    }
    // rotina para OPCODE JMPZ, se o valor no acumulador for zero
    else if(opcode == "JMPZ"){
        new_opcode.push_back("\tcmp eax, 0");
        new_opcode.push_back("\tje " + operands.at(0));
    }
    // rotina para OPCODE COPY
    else if(opcode == "COPY"){
        new_opcode.push_back("\tmov ebx, [" + operands.at(0) + "]");
        new_opcode.push_back("\tmov [" + operands.at(1) + "], ebx");

    }
    // rotina para OPCODE LOAD
    else if(opcode == "LOAD"){
        new_opcode.push_back("\tmov eax, [" + operands.at(0) + "]");
    }
    // rotina para OPCODE STORE
    else if(opcode == "STORE"){
        new_opcode.push_back("\tmov [" + operands.at(0) + "], eax");
    }
    // rotina para OPCODE INPUT
    else if(opcode == "INPUT"){
        new_opcode.push_back("\tmov ecx, " + operands.at(0));
        // le apenas 5 caracteres, pois são 3 caracteres de números, um de sinal e um para o enter.
        new_opcode.push_back("\tmov edx, 5");
        // chama a função que lê string com esses parâmetros
        new_opcode.push_back("\tcall LeerString");
        // transforma a string lida acima em um inteiro
        new_opcode.push_back("\tmov esi, " + operands.at(0)); // utiliza ESI como ponteiro para o endereço do operando
        new_opcode.push_back("\tcall ConverteInteiro");
        new_opcode.push_back("\tmov [" + operands.at(0) + "], eax");
    }
    // rotina para OPCODE OUTPUT
    else if(opcode == "OUTPUT"){
        new_opcode.push_back("\tmov eax, [" + operands.at(0) + "]");
        new_opcode.push_back("\tlea esi, [NBUFFER]"); // utiliza ESI como ponteiro para o buffer de 12 posições
        new_opcode.push_back("\tcall EscreverInteiro");

    }
    // rotina para OPCODE STOP
    else if(opcode == "STOP"){
        // chama o syscall de encerrar o programa
        new_opcode.push_back("\tmov eax, 1");
        new_opcode.push_back("\tmov ebx, 0");
        new_opcode.push_back("\tint 80h");
    }
    // rotina para OPCODE C_INPUT
    else if(opcode == "C_INPUT"){
        new_opcode.push_back("\tmov ecx, " + operands.at(0));
        new_opcode.push_back("\tcall LeerChar");
    }
    // rotina para OPCODE C_OUTPUT
    else if(opcode == "C_OUTPUT"){
        new_opcode.push_back("\tmov ecx, " + operands.at(0));
        new_opcode.push_back("\tcall EscreverChar");
    }
    // rotina para OPCODE S_INPUT
    else if(opcode == "S_INPUT"){ // TEM DOIS OPERANDOS
        new_opcode.push_back("\tmov ecx, " + operands.at(0)); // local da string
        new_opcode.push_back("\tmov edx, " + operands.at(1)); // tamanho da string
        new_opcode.push_back("\tcall LeerString");
    }
    // rotina para OPCODE S_OUTPUT
    else if(opcode == "S_OUTPUT"){ // TEM DOIS OPERANDOS
        new_opcode.push_back("\tmov ecx, " + operands.at(0)); // local da string
        new_opcode.push_back("\tmov edx, " + operands.at(1)); // tamanho da string
        new_opcode.push_back("\tcall EscreverString");
    }
    // retorna a lista com as operações traduzidas para IA32.
    return new_opcode;
}