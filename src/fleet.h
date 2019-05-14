/* ----------------------------------------------------------------------- *
 * 
 *   Universidade Federal de Minas Gerais
 *   Departamento de Ciência da Computação
 *   Programa de Pós-Graduação em Ciência da Computação
 *   Projeto e Análise de Algoritmos
 * 
 *   Trabalho Prático - Grafos
 * 
 *   Autor: Leandro Augusto Lacerda Campos
 * 
 * ----------------------------------------------------------------------- */

#ifndef _FLEET_H_
#define _FLEET_H_

#include <stdint.h>

/* limites para postos de combate por frota */
#define FLEET_MINPOST   10
#define FLEET_MAXPOST   100000

/* limites para teleportes por frota */
#define FLEET_MINTP     8
#define FLEET_MAXTP     1000000

/* representação para valor infinito */
#define FLEET_INF       INT64_MAX

/* tipos de nave da frota */
#define FLEET_SCOUT     0   /* reconhecimento */
#define FLEET_FRIGATE   1   /* fragata */
#define FLEET_BOMBER    2   /* bombardeiro */
#define FLEET_TRANSPORT 3   /* transportador */
#define FLEET_NTYPE     4   /* quantidades de tipos de nave */

typedef struct Fleet Fleet;
typedef struct Ship Ship;
typedef struct Post Post;
typedef struct Teleport Teleport;

struct Fleet {          /* frota de naves */
    int32_t nship;      /* número de naves */
    Ship *ship;         /* lista de naves */
    int32_t npost;      /* número de postos de combate */
    Post *post;         /* vetor de postos de combate */
    int32_t ntp;        /* número de teleportes possíveis */ 
    Teleport *tp;       /* vetor de teleportes possíveis */
};

struct Ship {       /* lista de naves de uma frota */
    int32_t id;     /* id da nave na frota: de 0 a nship - 1 */
    int32_t type;   /* tipo da nave */
    int32_t npost;  /* número de postos de combate na nave */
    Ship *next;     /* próxima nave na lista */

    /* atributos da árvore que representa a nave na floresta de busca em 
       profundidade */
    int32_t root;   /* raíz */
    int32_t height; /* altura */
};

struct Post {       /* posto de combate */
    Ship *ship;     /* nave a que pertence */
    Teleport *tp;   /* lista dos teleportes possíveis a partir deste posto */

    /* atributos do vértice que representa o posto na árvore da nave */
    int32_t pi;     /* pai do posto */
    int32_t depth;  /* profundidade do posto */
    int8_t group;   /* grupo do posto */

    /* atributo utilizado na decomposição SQRT da árvore da nave e na posterior
       obtenção de ancestral comum mais baixo */
    int32_t jump;
};

struct Teleport {   /* lista de teleportes possíveis a partir de um posto */
    int32_t to;     /* posto de combate que é destino do teleporte */
    Teleport *next; /* próximo teleporte na lista */
};

/*
 * fleet_init: inicializa o objeto apontado por fleet com npost postos de 
 * combate e ntp teleportes possíveis. Em caso de sucesso, a função retorna 
 * 0. Em caso de falha, ela retorna:
 *  -1: se fleet é NULL;
 *  -2: se npost ou ntp está fora dos limites suportados; ou
 *  -3: se não foi possível alocar memória.
 */
int32_t fleet_init(Fleet *fleet, int32_t npost, int32_t ntp);

/*
 * fleet_add: adiciona um teleporte possível entre os pontos de combate p1 e 
 * p2 a uma frota assumidamente inicializada apontada por fleet. O teleporte 
 * é adicionado na posição idx do vetor de teleportes possíveis. Em caso de 
 * sucesso, a função retorna idx + 1. Em caso de falha, ela retorna:
 *  -1: se fleet não é um objeto Fleet válido;
 *  -2: se idx está fora dos limites do vetor de teleportes;
 *  -3: se p1 ou p2 está fora dos limites do vetor de postos de combate; ou
 *  -4: se a frota já foi explorada.
 */
int32_t fleet_add(Fleet *fleet, int32_t idx, int32_t p1, int32_t p2);

/*
 * fleet_scan: explora a frota apontada por fleet. A função assume que fleet
 * foi inicializado e que todos os teleportes possíveis entre postos já foram 
 * adicionados. Em caso de sucesso, a função identifica, descreve e classifica 
 * todas as naves da frota, retornando a contagem delas. Em caso de falha, ela
 * retorna:
 *  -1: se fleet não é um objeto Fleet válido;
 *  -2: se a frota já foi explorada; ou
 *  -3: se não foi possível alocar memória.
 */
int32_t fleet_scan(Fleet *fleet);

/*
 * fleet_stat: retorna a contagem de naves por tipo. A função assume que fleet 
 * aponta para um objeto Fleet que já foi explorado por fleet_scan() e que stat 
 * aponta para um vetor de tamanho FLEET_NTYPE. Para i = 0, ..., FLEET_NTYPE-1,
 * a função grava a contagem de naves do tipo i em stat[i]. Em caso de sucesso,
 * ela retorna o número total de naves. Em caso de falha, ela retorna:
 *  -1: se fleet não é um objeto Fleet válido e já explorado; ou
 *  -2: se stat é NULL.
 */
int32_t fleet_stat(Fleet *fleet, int32_t *stat);

/* fleet_adtm: calcula uma cota inferior não-trivial para o tempo de 
 * vantagem em relação a uma frota. A função assume que fleet aponta para um 
 * objeto Fleet que já foi explorado por fleet_scan(); que os vetores apontados 
 * por p1 e p2 têm tamanho npost; que o tripulante no posto p1[i] deve retornar
 * ao seu posto correto p2[i] para todo i = 0, ..., npost-1; que para cada posto
 * i da frota existe um e apenas um par (j, k) tal que i = p1[j] = p2[k]; que um 
 * teleporte demora uma unidade de tempo; e que só pode ser realizado um único
 * teleporte por vez em cada nave. Em caso de sucesso, a funcão retorna uma 
 * cota inferior >= 0. Em caso de falha, ela returna:
 *  -1: se fleet não é um objeto Fleet válido;
 *  -2: se p1[i] ou p2[i] está fora dos limites do vetor de postos de combate
 *      para algum i; ou
 *  -3: se p1[i] e p2[i] não estão na mesma nave para algum i ou se a nave é 
 *      de tipo desconhecido.
 */
int64_t fleet_adtm(Fleet *fleet, int32_t *p1, int32_t *p2);

/*
 * fleet_free: libera a memória alocada dinamicamente para o objeto Fleet 
 * apontado por fleet.
 */
void    fleet_free(Fleet *fleet);

#endif /* !_FLEET_H_ */