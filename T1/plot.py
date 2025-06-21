import pandas as pd
import matplotlib.pyplot as plt

# ----- Speedup por entrada -----
speedup_df = pd.read_csv("speedup.csv")
plt.figure(figsize=(8, 5))
for threads in [2, 4, 8, 16]:
    plt.plot(
        speedup_df["Entry"],
        speedup_df[str(threads)],
        marker="o",
        label=f"{threads} Threads",
    )
plt.title("Speedup por Tamanho de Entrada")
plt.xlabel("Entrada")
plt.ylabel("Speedup")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("speedup.png")
plt.close()

# ----- Eficiência Forte -----
strong_df = pd.read_csv("strong_efficiency.csv")
plt.figure(figsize=(6, 4))
plt.plot(strong_df["Threads"], strong_df["Efficiency"], marker="s", color="blue")
plt.title("Eficiência Forte")
plt.xlabel("Número de Threads")
plt.ylabel("Eficiência")
plt.grid(True)
plt.tight_layout()
plt.savefig("strong_efficiency.png")
plt.close()

# ----- Eficiência Fraca -----
weak_df = pd.read_csv("weak_efficiency.csv")
plt.figure(figsize=(6, 4))
plt.plot(weak_df["Threads"], weak_df["Efficiency"], marker="^", color="green")
plt.title("Eficiência Fraca")
plt.xlabel("Número de Threads")
plt.ylabel("Eficiência")
plt.grid(True)
plt.tight_layout()
plt.savefig("weak_efficiency.png")
plt.close()

# ----- Tempo por Entrada e Threads -----
time_df = pd.read_csv("time.csv")
plt.figure(figsize=(8, 5))

# Plot para cada quantidade de threads
for threads in [2, 4, 8, 16]:
    col_name = f"{threads}-avg"
    plt.plot(
        time_df["Entry"], time_df[col_name], marker="o", label=f"{threads} Threads"
    )

# Plot do sequencial
plt.plot(
    time_df["Entry"],
    time_df["sequential"],
    marker="x",
    linestyle="--",
    color="black",
    label="Sequencial",
)

plt.title("Tempo de Execução por Entrada e Threads")
plt.xlabel("Entrada")
plt.ylabel("Tempo Médio (s)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("execution_time.png")
plt.close()

print(
    "✅ Gráficos gerados: speedup.png, strong_efficiency.png, weak_efficiency.png, execution_time.png"
)
