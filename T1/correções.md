# Correcoes do T1

## Calculo da lei de ahmdal

A lei de Ahmdal deve ser calculada a partir do código sequencial.

Através de um teste com tamanhao 180k o tempode execução foi:

```text
TotalTime: 19.904740 seconds
SeqTime: 0.104797 seconds
ParTime: 19.799940 seconds
```

Isso nos mostra uma fração paralelizável de `0.9947`.

Ao utilizar a lei de Ahmdal, temos:

```text
| Processadores (p) | Speedup Teórico |
| ----------------- | --------------- |
| 2                 | ≈ 1,99x         |
| 4                 | ≈ 3,96x         |
| 8                 | ≈ 7,85x         |
| 16                | ≈ 15,4x         |
| inf               | ≈ 190,14        |
```

## Escalabilidades

Para saber se o código é fortemente ou fracamente escalável, é necessário calcular a eficiência do código paralelo. Dado o speedup e a quantidade de processadores.

## fixar frequencia da CPU

Quem cuida disso são os `governors` do sistema operacional, que definem os máximos e mínimos de frequência da CPU. O `governor` padrão do meu sistema é o `powersave` o utilitário `cpufreq-utils` foi utilizado para alterar o `governor` para `performance`, que fixa a frequência da CPU no máximo. O comando utilizado foi:

```bash
for i in {0..15}; do
    sudo cpufreq-set -c $i -g performance
done
```

## Desabilitar a interface gráfica

Para desabilitar a interface gráfica, o comando `sudo systemctl isolate multi-user.target` foi utilizado. Isso faz com que o sistema inicialize no modo texto, sem carregar a interface gráfica.
