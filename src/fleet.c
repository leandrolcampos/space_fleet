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

#include <stdlib.h>
#include <math.h>
#include "fleet.h"

/* para os algoritmos de exploração de frota e cálculo de tempo de vantagem */
#define NIL     (-1)  /* ausência de antecessor na árvore de BP de uma nave */
#define ND      (-2)  /* atributo jump não calculado */

/* Declaração de funções internas */
static inline Ship *add_ship(Fleet *fleet, int32_t id, int32_t root);
static void ship_visit(Fleet *fleet, Ship *ship);
static inline void add_post(Fleet *fleet, Ship *ship, Post *post, int32_t pi);
static inline void ship_class(Ship *ship, int32_t mdeg, int32_t nback);
static int64_t get_dist(Fleet *fleet, int32_t p1, int32_t p2);
static void set_jump(Fleet *fleet, int32_t p, int32_t block_sz);
static int32_t get_lca(Fleet *fleet, int32_t p1, int32_t p2);

/* ------------------------------------------------------------------------- *
 *
 * Definição de algumas macros para comparação de variáveis
 * 
 * ------------------------------------------------------------------------- */

/* typeof: retorna o tipo de dados de uma variável */
#ifndef typeof
#define typeof __typeof__
#endif

/* _cmp: compara duas variáveis do mesmo tipo ou de tipos compatíveis */
#define _cmp(x, y, op) ((x) op (y) ? (x) : (y))

/* _cmp: compara duas variáveis do mesmo tipo ou de tipos compatíveis,
   evitando avaliar cada variável mais de uma vez */
#define _cmp_once(x, y, op) ({      \
		typeof(x) _x = (x);         \
		typeof(y) _y = (y);         \
		_cmp(_x, _y, op); })

/* max: avalia duas variáveis e retorna o maior valor */
#define max(x, y) _cmp_once(x, y, >)

/* min: avalia duas variáveis e retorna o menor valor */
#define min(x, y) _cmp_once(x, y, <)

/* ------------------------------------------------------------------------- *
 *
 * Definição de funções declaradas e documentadas no arquivo fleet.h
 * 
 * ------------------------------------------------------------------------- */

int32_t fleet_init(Fleet *fleet, int32_t npost, int32_t ntp)
{
    Post *post;
    Teleport *tp;

    if (fleet == NULL) return -1;

    if (npost < FLEET_MINPOST || npost > FLEET_MAXPOST) return -2;

    if (ntp < FLEET_MINTP || ntp > FLEET_MAXTP) return -3;
    
    post = malloc(npost * sizeof(Post));
    if (post == NULL) return -4;

    /* 2 * ntp pois (u, v) implica v em Adj[u] e u em Adj[v] no grafo da frota */
    tp = malloc(2 * ntp * sizeof(Teleport));
    if (tp == NULL) { free(post); return -5; }
    
    for (int32_t i = 0; i < npost; i++) {
        post[i].tp = NULL;  /* necessário para criação de lista encadeada */
    }
    fleet->nship = 0;
    fleet->ship = NULL;
    fleet->npost = npost;
    fleet->post = post;
    fleet->ntp = ntp;
    fleet->tp = tp;

    return 0;
}

int32_t fleet_add(Fleet *fleet, int32_t idx, int32_t p1, int32_t p2)
{
    int32_t npost = fleet->npost;
    int32_t ntp = fleet->ntp;
    Teleport *tp;
    Post *post;

    if (fleet == NULL || fleet->post == NULL || fleet->tp == NULL) return -1;

    if (idx < 0 || idx >= ntp) return -2;

    if (p1 < 0 || p1 >= npost || p2 < 0 || p2 >= npost) return -3;

    if (fleet->ship != NULL) return -4;

    /* adiciona teleporte de p1 para p2 */
    tp = &fleet->tp[idx];
    post = &fleet->post[p1];
    tp->to = p2;
    tp->next = post->tp;
    post->tp = tp;

    /* adiciona teleporte de p2 para p1 */
    tp = &fleet->tp[ntp + idx];
    post = &fleet->post[p2];
    tp->to = p1;
    tp->next = post->tp;
    post->tp = tp;

    return idx + 1;
}

int32_t fleet_scan(Fleet *fleet)
{   /* baseado no algoritmo de busca em profundidade */

    Ship *ship;
    Post *post;
    int32_t counter = 0;    /* contagem de naves na frota */

    if (fleet == NULL || fleet->post == NULL || fleet->tp == NULL) return -1;

    if (fleet->ship != NULL) return -2;

    for (int32_t i = 0; i < fleet->npost; i++) {
        post = &fleet->post[i];
        post->ship = NULL;
        post->pi = NIL;
    }
    for (int32_t i = 0; i < fleet->npost; i++) {
        post = &fleet->post[i];
        if (post->ship == NULL) {
            /* uma nova nave encontrada */
            ship = add_ship(fleet, counter++, i);
            if (ship == NULL) return -3;
            /* explora a nave encontrada */
            ship_visit(fleet, ship);
        }
    }
    return counter;
}

int32_t fleet_stat(Fleet *fleet, int32_t *stat)
{
    if (fleet == NULL || fleet->ship == NULL) return -1;

    if (stat == NULL) return -2;

    for (int32_t i = 0; i < FLEET_NTYPE; i++) stat[i] = 0;

    for (Ship *ship = fleet->ship; ship != NULL; ship = ship->next) {
        stat[ship->type]++;
    }
    return fleet->nship;
}

int64_t fleet_adtm(Fleet *fleet, int32_t *p1, int32_t *p2)
{    
    int32_t r[fleet->nship];    /* n. de cálculos ainda a executar por nave */
    int64_t s[fleet->nship];    /* soma das distâncias por nave */
    int64_t m = FLEET_INF;      /* menor soma encontrada: dada a nave i, 
                                   se r[i] = 0 então m <= s[i] */
    int64_t d;                  /* distância entre dois postos */
    int32_t u, v;
    Ship *ship;

    if (fleet == NULL || fleet->post == NULL 
        || fleet->tp == NULL || fleet->ship == NULL) return -1;

    for (ship = fleet->ship; ship != NULL; ship = ship->next) {
        s[ship->id] = 0; r[ship->id] = ship->npost; 
    }
    for (int32_t i = 0; i < fleet->npost; i++) {
        u = p1[i]; v = p2[i];
        if (u < 0 || u >= fleet->npost || v < 0 || v >= fleet->npost) 
            return -2;

        ship = fleet->post[u].ship;
        if (s[ship->id] < m) {
            /* s[ship->id] >= m implica que s[ship->id] não poderá ser um 
               novo limitante inferior */
            if (u != v) {
                /* u = v implica d(u, v) = 0 */
                d = get_dist(fleet, u, v);
                if (d == -1 || d == FLEET_INF) {
                    /* u e v não estão na mesma nave ou a nave é de 
                       tipo desconhecido */
                    return -3;
                }
                s[ship->id] += d;
            }
            r[ship->id]--;
            if (r[ship->id] == 0 && s[ship->id] < m) m = s[ship->id];
            
            if (m <= 1) {
                /* m <= 1 é o menor limitante inferior não trivial possível */
                return 0;
            }
        }
    }
    return m / 2;
}

void fleet_free(Fleet *fleet)
{
    Ship *next;

    if (fleet == NULL) return;

    for ( ; fleet->ship != NULL; fleet->ship = next) {
        next = fleet->ship->next;
        free(fleet->ship);
    }
    if (fleet->post != NULL) {
        free(fleet->post);
        fleet->post = NULL;
    }
    if (fleet->tp != NULL) {
        free(fleet->tp);
        fleet->tp = NULL;
    }

    fleet->nship = 0;
    fleet->npost = 0;
    fleet->ntp = 0;
}

/* ------------------------------------------------------------------------- *
 *
 * Definições de funções internas
 * 
 * ------------------------------------------------------------------------- */

inline Ship *add_ship(Fleet *fleet, int32_t id, int32_t root)
{
    Ship *ship;

    ship = malloc(sizeof(Ship));
    if (ship == NULL) return NULL;

    ship->id = id;
    ship->npost = 0;
    ship->next = fleet->ship;
    fleet->ship = ship;
    fleet->nship++;
    ship->root = root;
    ship->height = 1;

    return ship;
}

void ship_visit(Fleet *fleet, Ship *ship)
{   /* baseado no algoritmo de busca em profundidade com pilha */

    Post *post = &fleet->post[ship->root];
    Post *child;
    int32_t stack[fleet->npost];    /* pilha */
    int32_t idx = 0;                /* índice da pilha */
    int32_t u, v;
    int32_t mdeg = 0;   /* grau máximo */
    int32_t nback = 0;  /* número de arestas de retorno */
    int32_t ntp;

    add_post(fleet, ship, post, NIL);
    stack[idx++] = ship->root;

    while (idx > 0) {
        /* retira um posto da fila */
        u = stack[--idx];
        post = &fleet->post[u];

        /* percorre a lista de adjacências do posto combate u */
        ntp = 0;
        for (Teleport *tp = post->tp; tp != NULL; tp = tp->next) {
            ntp++;
            v = tp->to;
            child = &fleet->post[v];
            if (child->ship == NULL) {
                add_post(fleet, ship, child, u);
                stack[idx++] = v;
            } else if (child->depth < post->depth && v != post->pi) {
                /* child é um ancestral do posto mas não é seu pai */
                nback++;
            }
        }
        if (ntp > mdeg) mdeg = ntp;
    }
    /* classifica a nave encontrada */
    ship_class(ship, mdeg, nback);
}

inline void add_post(Fleet *fleet, Ship *ship, Post *post, int32_t pi)
{
    Post *parent;

    /* adiciona o posto na nave */
    post->ship = ship;
    ship->npost++;

    /* monta a floresta de BP */
    post->pi = pi;
    if (pi == NIL) {
        post->depth = 0;
        post->group = 0;
    } else {
        parent = &fleet->post[pi];
        post->depth = 1 + parent->depth;
        post->group = 1 - parent->group;
    }
    post->jump = ND;

    /* atualiza a altura da árvore da nave, se necessário */
    if (post->depth + 1 > ship->height) ship->height = post->depth + 1;
}

inline void ship_class(Ship *ship, int32_t mdeg, int32_t nback)
{
    if (nback == 0) {
        if (mdeg == 2) ship->type = FLEET_SCOUT;
        else ship->type = FLEET_FRIGATE;
    } else if (nback == 1) {
        ship->type = FLEET_TRANSPORT;
    } else {
        ship->type = FLEET_BOMBER;
    }
}

int64_t get_dist(Fleet *fleet, int32_t p1, int32_t p2)
{   /* assume que todas as naves correspondem exatamente às características 
       até então conhecidas do seu tipo */
    
    Post *post1 = &fleet->post[p1];
    Post *post2 = &fleet->post[p2];
    Ship *ship = post1->ship;
    int32_t lca;
    Post *post_lca;
    int32_t i, j, k;

    /* se p1 e p2 não estão na mesma nave */
    if (ship != post2->ship) return FLEET_INF;

    switch (ship->type)
    {   /* Para entender as fórmulas, consulte a documentação */
        case FLEET_SCOUT:
        case FLEET_FRIGATE:
            if (post1->jump == ND) 
                set_jump(fleet, ship->root, sqrt(ship->height));
            lca = get_lca(fleet, p1, p2);
            post_lca = &fleet->post[lca];
            return post1->depth + post2->depth - 2 * post_lca->depth;

        case FLEET_TRANSPORT:
            i = min(post1->depth, post2->depth);
            j = max(post1->depth, post2->depth);
            k = ship->height;
            return min(j - i, k - j + i);

        case FLEET_BOMBER:
            if (post1->group == post2->group) {
                if (post1 == post2) return 0;
                else return 2;
            }
            return 1;

        default:
            /* Erro! Nave de tipo desconhecido */
            return -1;
    }
}

void set_jump(Fleet *fleet, int32_t p, int32_t block_sz)
{   /* configura o atributo jump dos postos com base na decomposição
       SQRT da árvore que representa a nave */

    Post *post = &fleet->post[p];
    int32_t pi;

    pi = post->pi;
    if (post->depth % block_sz == 0) {
        /* o posto está no primeiro nível do bloco */
        post->jump = pi;
    } else {
        /* post->pi = NIL implica que post está no primeiro nível do bloco */
        post->jump = fleet->post[pi].jump;
    }
    /* configura outros postos da nave a partir desse posto */
    for (Teleport *tp = post->tp; tp != NULL; tp = tp->next) {
        if (fleet->post[tp->to].pi == p) set_jump(fleet, tp->to, block_sz);
    }
}

int32_t get_lca(Fleet *fleet, int32_t p1, int32_t p2)
{   /* retorna o ancentral comum mais baixo entre p1 e p2 com base na 
       decomposição SQRT da árvore que representa a nave */

    Post *post = fleet->post;

    while (post[p1].jump != post[p2].jump) {
        if (post[p1].depth > post[p2].depth) p1 = post[p1].jump;
        else p2 = post[p2].jump;
    }
    while (p1 != p2) {
        if (post[p1].depth > post[p2].depth) p1 = post[p1].pi;
        else p2 = post[p2].pi;
    }
    return p1;
}