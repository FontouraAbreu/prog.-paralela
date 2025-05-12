#!/usr/bin/env python
# encoding: utf-8

import pandas as pd
import matplotlib.pyplot as plt
import sys


def plot_speedup(csv_path):
    df = pd.read_csv(csv_path)
    df.set_index("Entry", inplace=True)

    plt.figure(figsize=(10, 6))
    for col in df.columns:
        plt.plot(df.index, df[col], marker="o", label=f"{col} threads")

    plt.title("Speedup vs Entry Size")
    plt.xlabel("Input Size (Entry)")
    plt.ylabel("Speedup")
    plt.legend(title="Threads")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig("speedup_plot.png")
    print("speedup file: speedup_plot.png")


def plot_time(csv_path):
    df = pd.read_csv(csv_path)
    df.set_index("Entry", inplace=True)

    plt.figure(figsize=(10, 6))
    plt.plot(df.index, df["sequential"], marker="o", label="Sequential", linewidth=2)

    for threads in [2, 4, 8, 16]:
        avg_col = f"{threads}-avg"
        plt.plot(df.index, df[avg_col], marker="o", label=f"{threads} threads")

    plt.title("Execution Time vs Entry Size")
    plt.xlabel("Input Size (Entry)")
    plt.ylabel("Time (s)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig("time_plot.png")
    print("time file: time_plot.png")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python plot.py [spd|time|both]")
        sys.exit(1)

    mode = sys.argv[1]
    if mode == "spd":
        plot_speedup("speedup.csv")
    elif mode == "time":
        plot_time("time.csv")
    elif mode == "both":
        plot_speedup("speedup.csv")
        plot_time("time.csv")
    else:
        print("Invalid mode. Use 'spd', 'time' or 'both'.")
