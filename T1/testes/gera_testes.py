#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import random
import string

from argparse import ArgumentParser

def parse_args():
    """
    Função para analisar os argumentos da linha de comando.
    """
    parser = ArgumentParser(description="Gera testes para o projeto.", add_help=True)
    parser.add_argument(
        "base_string_size",
        type=int,
        help="Tamanho da string base.\n" \
        "Exemplo: 10. A string base terá tamanho 10.\n" \
    )
    
    parser.add_argument(
        "step",
        type=int,
        help="Multiplicador  usado para gerar os testes.\n" \
        "Exemplo: 2. Serão gerados testes com tamanhos de 2, 4, 6, 8, 10, 12, 14 e 16.\n" \
    )

    parser.add_argument(
        "num_testes",
        type=int,
        help="Número de testes a serem gerados.\n" \
        "Exemplo: 10. Serão gerados 10 testes.\n" \
    )
        

    parser.add_argument(
        "-d", "--diretorio",
        default=".",
        help="Diretório onde os testes serão gerados."
        "Exemplo: ./testes. Os testes serão gerados no diretório ./testes.\n" \
        "O diretório deve existir.\n" \
        "Se não for especificado, o diretório padrão é o diretório atual.\n" \
    )
    return parser.parse_args()

def main():
    """
    Função principal que executa os testes.
    """
    args = parse_args()
    # print(f"String base: {args.base_string_size}")
    # print(f"Diretório: {args.diretorio}")
    # print(f"Multiplicador: {args.step}")
    # print(f"Número de testes: {args.num_testes}")

    # Gera a string base aleatoria de tamanho base_string_size
    random_string = ''.join(random.choices(string.ascii_letters + string.digits, k=args.base_string_size))
    # print(f"String base: {random_string}")
    
    string_a = random_string

    # Gera uma substring aleatoria de string_a
    start = random.randint(0, len(string_a) - 1)
    end = random.randint(start + 1, len(string_a))
    string_b = string_a[start:end]

    with open(f"{args.diretorio}/test{args.base_string_size}.in", "w") as f:
        f.write(f"{string_a}\n{string_b}\n")
        print(f"Teste 1 gerado com sucesso!")


    # Gera os testes com base na string base
    # de forma que os testes são apenas multiplicadores da string base
    for i in range(1, args.num_testes):
        # Gera o tamanho do teste
        test_size = args.base_string_size + (i * args.step)
        string_a += string_a
        string_b += string_b
        with open(f"{args.diretorio}/test{test_size}.in", "w") as f:
            f.write(f"{string_a}\n{string_b}\n")
            print(f"Teste {i+1} gerado com sucesso!")

    print("Todos os testes foram gerados com sucesso!")
    
    return 0

        


    
    

if __name__ == "__main__":
    main()