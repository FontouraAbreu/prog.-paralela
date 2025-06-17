#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import random
import string
from argparse import ArgumentParser


def parse_args():
    parser = ArgumentParser(description="Gera testes para o projeto.", add_help=True)
    parser.add_argument(
        "base_string_size",
        type=int,
        help="Tamanho da string base.\nExemplo: 10. A string base terá tamanho 10.",
    )
    parser.add_argument(
        "step",
        type=int,
        help="Incremento de tamanho a cada teste.\nExemplo: 2. Testes com 10, 12, 14, etc.",
    )
    parser.add_argument(
        "num_testes",
        type=int,
        help="Número de testes a serem gerados.\nExemplo: 10.",
    )
    parser.add_argument(
        "-d",
        "--diretorio",
        default=".",
        help="Diretório onde os testes serão gerados (deve existir). Padrão: diretório atual.",
    )
    return parser.parse_args()


def random_string(length):
    return "".join(random.choices(string.ascii_letters + string.digits, k=length))


def main():
    args = parse_args()

    for i in range(args.num_testes):
        test_size = args.base_string_size + (i * args.step)
        string_a = random_string(test_size)
        string_b_size = int(test_size / 3)
        string_b = random_string(string_b_size)

        path_a = f"{args.diretorio}/{test_size}_A.in"
        path_b = f"{args.diretorio}/{test_size}_B.in"

        with open(path_a, "w") as f:
            f.write(f"{string_a}\n")
            print(f"Teste {i + 1} A ({test_size} chars) gerado com sucesso!")

        with open(path_b, "w") as f:
            f.write(f"{string_b}\n")
            print(f"Teste {i + 1} B ({string_b_size} chars) gerado com sucesso!")

    print("Todos os testes foram gerados com sucesso!")
    return 0


if __name__ == "__main__":
    main()
