#!/usr/bin/env python
# encoding: utf-8

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import sys


def plot_speedup(csv_path):
    df = pd.read_csv(csv_path)
    df.set_index("Entry", inplace=True)

    plt.figure(figsize=(10, 6))
    for col in df.columns:
        plt.plot(df.index, df[col], marker="o", label=f"{col} threads")

    plt.title("Speedup vs Tamanho da Entrada")
    plt.xlabel("Tamanho da Entrada")
    plt.ylabel("Speedup")
    plt.legend(title="Threads")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig("grafico_speedup.png")
    print("Arquivo gerado: grafico_speedup.png")


def plot_time(csv_path):
    df = pd.read_csv(csv_path)
    df.set_index("Entry", inplace=True)

    # plt.figure(figsize=(10, 6))
    # plt.plot(df.index, df["sequential"], marker="o", label="Sequencial", linewidth=2)

    for threads in [2, 4, 8]:
        avg_col = f"{threads}-avg"
        plt.plot(df.index, df[avg_col], marker="o", label=f"{threads} threads")

    plt.title("Tempo de Execução vs Tamanho da Entrada")
    plt.xlabel("Tamanho da Entrada")
    plt.ylabel("Tempo (s)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig("grafico_tempo.png")
    print("Arquivo gerado: grafico_tempo.png")


def plot_weak_scalability(csv_path):
    df = pd.read_csv(csv_path)

    # Converte colunas numéricas
    for col in df.columns:
        if col != "Entry":
            df[col] = pd.to_numeric(df[col], errors="coerce")

    entries = df["Entry"].astype(float)
    threads = [2, 4, 8]

    plt.figure(figsize=(8, 6))

    for i, row in df.iterrows():
        tempos = [row[f"{t}-avg"] for t in threads]
        plt.plot(
            threads,
            tempos,
            marker="o",
            linestyle="-",
            label=f"Entrada {int(row['Entry'])}",
        )

    plt.title("Escalabilidade Fraca - Tempo vs Número de Threads")
    plt.xlabel("Número de Threads")
    plt.ylabel("Tempo de Execução (s)")
    plt.xticks(threads)
    plt.grid(True)
    plt.legend(title="Tamanho da Entrada", loc="upper right")
    plt.tight_layout()
    plt.savefig("grafico_escalabilidade_fraca_linhas_threads.png")
    print("Arquivo gerado: grafico_escalabilidade_fraca_linhas_threads.png")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python plot.py [spd|time|strong|both]")
        sys.exit(1)

    mode = sys.argv[1]
    if mode == "spd":
        plot_speedup("speedup.csv")
    elif mode == "time":
        plot_time("time.csv")
    elif mode == "strong":
        plot_strong_scalability("strong-scalability/time.csv")
    elif mode == "weak":
        plot_weak_scalability("time.csv")
    elif mode == "both":
        plot_speedup("speedup.csv")
        plot_time("time.csv")
    else:
        print("Modo inválido. Use 'spd', 'time', 'strong' ou 'both'.")
