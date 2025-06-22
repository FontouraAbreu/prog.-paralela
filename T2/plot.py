import pandas as pd
import matplotlib.pyplot as plt

# Ler os CSVs
speedup_df = pd.read_csv("speedup.csv")
time_df = pd.read_csv("time.csv")
strong_df = pd.read_csv("strong_efficiency.csv")
weak_df = pd.read_csv("weak_efficiency.csv")

# Converter entradas para int
time_df["Entry"] = time_df["Entry"].astype(int)
speedup_df["Entry"] = speedup_df["Entry"].astype(int)

# --------- PLOT 1: TEMPO vs TAMANHO DA ENTRADA ---------
plt.figure(figsize=(9, 6))
entries = time_df["Entry"]

# Tempo paralelo por threads
for threads in [2, 4, 8, 12]:
    times = []
    for entry in entries:
        row = time_df[time_df["Entry"] == entry].iloc[0]
        times.append(row[f"{threads}-avg"])
    plt.plot(entries, times, marker="o", label=f"{threads} Threads")

# Linha pontilhada do tempo sequencial
seq_times = time_df["sequential"]
plt.plot(entries, seq_times, linestyle="--", color="black", label="Sequencial")

plt.xlabel("Tamanho da Entrada")
plt.ylabel("Tempo Médio (s)")
plt.title("Tempo de Execução vs Tamanho da Entrada")
plt.legend(title="Configuração")
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.savefig("tempo_vs_entry.png")
plt.close()

# --------- PLOT 2: SPEEDUP vs TAMANHO DA ENTRADA ---------
plt.figure(figsize=(9, 6))
for threads in [2, 4, 8, 12]:
    speedups = []
    for entry in speedup_df["Entry"]:
        row = speedup_df[speedup_df["Entry"] == entry].iloc[0]
        speedups.append(row[str(threads)])
    plt.plot(speedup_df["Entry"], speedups, marker="o", label=f"{threads} Threads")

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
plt.axhline(
    1.0, color="black", linestyle="--", label="Eficiência Ideal (1.0)"
)  # Linha pontilhada ideal
plt.xlabel("Número de Threads")
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
plt.axhline(
    1.0, color="black", linestyle="--", label="Eficiência Ideal (1.0)"
)  # Linha pontilhada ideal
plt.xlabel("Número de Threads")
plt.ylabel("Eficiência")
plt.title("Eficiência da Escalabilidade Fraca")
plt.legend()
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.savefig("weak_efficiency.png")
plt.close()

# --------- TABELA DE TEMPOS COM DESVIO PADRÃO ---------
print("\nTabela de Tempos com Desvio Padrão:\n")
print(time_df.to_string(index=False))

print("\nGráficos salvos:")
print("- tempo_vs_entry.png")
print("- speedup_vs_entry.png")
print("- strong_efficiency.png")
print("- weak_efficiency.png")
