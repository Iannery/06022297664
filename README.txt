/****************************************
 * Trabalho 2 - Software Basico         *
 *                                      *
 * Ian Nery Bandeira                    *
 * 170144739                            *
 *                                      *
 * Versao do compilador:                *
 * g++ (Ubuntu 9.3.0-10ubuntu2) 9.3.0   *
 *                                      *
 ****************************************/

O trabalho consiste de um Programa que simula um tradutor.
O programa recebe um arquivo .asm em assembly inventado, e resulta criando um arquivo .s compatível com o assembly IA32.

Os comandos necessários são:

- Para compilar o programa:
g++ main.cpp Tradutor.cpp -o tradutor

- Para traduizir o arquivo .asm com o programa, gerando o arquivo .s:
./tradutor -p myprogram.asm
