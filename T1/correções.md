# Correcoes do T1

## Calculo da lei de ahmdal

A lei de Ahmdal deve ser calculada a partir do código sequencial.

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

Para desabilitar a interface gráfica, o comando `systemctl set-default multi-user.target` foi utilizado. Isso faz com que o sistema inicialize no modo texto, sem carregar a interface gráfica.

