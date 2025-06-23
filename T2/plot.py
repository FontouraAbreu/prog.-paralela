import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Ler os CSVs principais
speedup_df = pd.read_csv("speedup.csv")
time_df = pd.read_csv("time.csv")
strong_df = pd.read_csv("strong_efficiency.csv")
weak_df = pd.read_csv("weak_efficiency.csv")
comm_df = pd.read_csv("comm_time.csv")  # Novo CSV com total e comunicação juntos
deviation_df = pd.read_csv("time.csv")  # Mesmo CSV de tempos contém os desvios

# Converter entradas para int
time_df["Entry"] = time_df["Entry"].astype(int)
speedup_df["Entry"] = speedup_df["Entry"].astype(int)
comm_df["Entry"] = comm_df["Entry"].astype(int)
deviation_df["Entry"] = deviation_df["Entry"].astype(int)

# --------- PLOT 1: TEMPO TOTAL vs TAMANHO DA ENTRADA ---------
plt.figure(figsize=(9, 6))
entries = time_df["Entry"]

for processes in [2, 4, 8, 10, 12]:
    times = []
    for entry in entries:
        row = time_df[time_df["Entry"] == entry].iloc[0]
        times.append(row[f"{processes}-avg"])
    plt.plot(entries, times, marker="o", label=f"{processes} Processos")

# Linha pontilhada para o tempo sequencial
seq_times = time_df["sequential"]
plt.plot(entries, seq_times, linestyle="--", color="black", label="Sequencial")

plt.xlabel("Tamanho da Entrada")
plt.ylabel("Tempo Médio (s)")
plt.title("Tempo Total de Execução vs Tamanho da Entrada")
plt.legend(title="Configuração")
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.savefig("tempo_vs_entry.png")
plt.close()

# --------- PLOT 2: SPEEDUP vs TAMANHO DA ENTRADA ---------
plt.figure(figsize=(9, 6))
for processes in [2, 4, 8, 10, 12]:
    speedups = []
    for entry in speedup_df["Entry"]:
        row = speedup_df[speedup_df["Entry"] == entry].iloc[0]
        speedups.append(row[str(processes)])
    plt.plot(speedup_df["Entry"], speedups, marker="o", label=f"{processes} Processos")

plt.xlabel("Tamanho da Entrada")
plt.ylabel("Speedup")
plt.title("Speedup vs Tamanho da Entrada")
plt.legend(title="Configuração")
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.savefig("speedup_vs_entry.png")
plt.close()

# --------- PLOT 3: EFICIÊNCIA FORTE ---------
plt.figure(figsize=(8, 6))
plt.plot(
    strong_df["Threads"],
    strong_df["Efficiency"],
    marker="o",
    color="green",
    label="Eficiência Observada",
)
plt.axhline(1.0, color="black", linestyle="--", label="Eficiência Ideal (1.0)")
plt.xlabel("Número de Processos")
plt.ylabel("Eficiência")
plt.title("Eficiência da Escalabilidade Forte")
plt.legend()
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.savefig("strong_efficiency.png")
plt.close()

# --------- PLOT 4: EFICIÊNCIA FRACA ---------
plt.figure(figsize=(8, 6))
plt.plot(
    weak_df["Threads"],
    weak_df["Efficiency"],
    marker="o",
    color="blue",
    label="Eficiência Observada",
)
plt.axhline(1.0, color="black", linestyle="--", label="Eficiência Ideal (1.0)")
plt.xlabel("Número de Processos")
plt.ylabel("Eficiência")
plt.title("Eficiência da Escalabilidade Fraca")
plt.legend()
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.savefig("weak_efficiency.png")
plt.close()

# --------- PLOT 5: TEMPO TOTAL x TEMPO DE COMUNICAÇÃO (EMPILHADO) ---------
plt.figure(figsize=(10, 6))
width = 3000  # Largura das barras
x = comm_df["Entry"]

# Definir pares de cores: (cálculo - claro, comunicação - escuro)
color_pairs = {
    2: ("lightgreen", "darkgreen"),
    4: ("mediumspringgreen", "forestgreen"),
    8: ("palegreen", "seagreen"),
    10: ("aquamarine", "teal"),
    12: ("lightcyan", "darkcyan"),
}

for processes in [2, 4, 8, 10, 12]:
    total = comm_df[f"{processes}-total"]
    comm = comm_df[f"{processes}-comm"]
    calc = total - comm

    calc_color, comm_color = color_pairs[processes]

    plt.bar(
        x + (processes - 8) * width / 5,
        calc,
        width=width / 5,
        label=f"{processes} Processos - Cálculo",
        bottom=comm,
        color=calc_color,
        edgecolor="black",
        alpha=0.9,
    )
    plt.bar(
        x + (processes - 8) * width / 5,
        comm,
        width=width / 5,
        label=f"{processes} Processos - Comunicação",
        color=comm_color,
        edgecolor="black",
        alpha=0.9,
    )

plt.xlabel("Tamanho da Entrada")
plt.ylabel("Tempo Total (s)")
plt.title("Composição do Tempo Total (Cálculo x Comunicação)")
plt.legend()
plt.tight_layout()
plt.savefig("stacked_calc_comm.png")
plt.close()


# --------- PLOT 6: CRESCIMENTO DOS DESVIOS PADRÃO COM O NÚMERO DE PROCESSOS ---------
plt.figure(figsize=(9, 6))

# Colunas de desvio
threads_list = ["sequential-dev", "2-dev", "4-dev", "8-dev", "10-dev", "12-dev"]
threads_labels = [
    "Sequencial",
    "2 Processos",
    "4 Processos",
    "8 Processos",
    "10 Processos",
    "12 Processos",
]

for idx, dev_col in enumerate(threads_list):
    plt.plot(
        deviation_df["Entry"],
        deviation_df[dev_col],
        marker="o",
        label=threads_labels[idx],
    )

plt.xlabel("Tamanho da Entrada")
plt.ylabel("Desvio Padrão (s)")
plt.title("Crescimento do Desvio Padrão com o Número de Processos")
plt.legend(title="Configuração")
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.savefig("std_dev_growth.png")
plt.close()

# --------- TABELA DE TEMPOS COM DESVIO PADRÃO ---------
print("\nTabela de Tempos com Desvio Padrão:\n")
print(time_df.to_string(index=False))

print("\nGráficos salvos:")
print("- tempo_vs_entry.png")
print("- speedup_vs_entry.png")
print("- strong_efficiency.png")
print("- weak_efficiency.png")
print("- stacked_calc_comm.png")
print("- std_dev_growth.png")
