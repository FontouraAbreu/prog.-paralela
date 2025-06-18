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

    plt.figure(figsize=(12, 7))

    # Plot sequencial
    plt.plot(
        df.index,
        df["sequential"],
        marker="o",
        linestyle="--",
        color="black",
        label="Sequencial",
        linewidth=2,
    )

    # Plot versões paralelas
    colors = ["blue", "green", "orange", "red"]
    for idx, threads in enumerate([2, 4, 8, 16]):
        avg_col = f"{threads}-avg"
        if avg_col in df.columns:
            plt.plot(
                df.index,
                df[avg_col],
                marker="o",
                color=colors[idx],
                label=f"{threads} threads",
                linewidth=2,
            )

    plt.title("Tempo de Execução vs Tamanho da Entrada", fontsize=16)
    plt.xlabel("Tamanho da Entrada", fontsize=14)
    plt.ylabel("Tempo (s)", fontsize=14)
    plt.legend(fontsize=12)
    plt.grid(True, linestyle="--", alpha=0.7)
    plt.tight_layout()

    plt.savefig("grafico_tempo.png", dpi=300)
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
