# Space Fleet

Este repositório contém a [solução](https://github.com/leandrolcampos/space_fleet/blob/master/solução.pdf) que eu apresentei para o primeiro [trabalho prático](https://github.com/leandrolcampos/space_fleet/blob/master/problema.pdf) da disciplina "Projetos e Análise de Algoritmos", que é ofertada pelo Programa de Pós-Graduação em Ciência da Computação da Universidade Federal de Minas Gerais.

O problema proposto neste trabalho é ambientado em um cenário fictício no qual duas raças estão em guerra por toda a galáxia. Como membros de uma delas, nós recebemos a missão de desenvolver um programa capaz de identificar, classificar e contar todas as naves da frota inimiga e estimar o tempo de vantagem para a realização de um ataque surpresa. Este programa tem como entrada uma massa de dados relacionados à estrutura do sistema de teleportes e aos estados de ocupação inicial e planejado de cada embarcação inimiga.

Para compilar o programa utilizando o GCC, execute os seguintes comandos:

```bash
chmod +x ./compilar.sh
./compilar.sh
```

E para executá-lo, execute os seguintes outros comandos:

```bash
chmod +x ./executar.sh
./executar.sh [arquivo de entrada] [arquivo de saída]
```

Os parâmetros `[arquivo de entrada]` e `[arquivo de saída]` se referem, respectivamente, ao arquivo existente que contém os dados de entrada e ao arquivo que será criado e no qual serão escritos os dados de saída do programa. A pasta [test\in](https://github.com/leandrolcampos/space_fleet/blob/master/test/in) contém 12 exemplos de arquivo de entrada. Os arquivos de saída correspondentes estão na pasta [test\out](https://github.com/leandrolcampos/space_fleet/blob/master/test/out).

Tanto a compilação quanto a execução foram testadas em uma máquina com as seguintes configurações:

- **Processador:** Intel Core i7-4720 2.6 GHz
- **Memória:** 16 GB DDR3L SDRAM 800 MHz
- **OS:** Ubuntu 18.10
- **Compilador:** GCC 8.3.0
