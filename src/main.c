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
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include "fleet.h"

bool read_ints(int32_t *i1, int32_t *i2);
bool build_fleet(Fleet *fleet);
bool print_stat(Fleet *fleet);
bool print_adtm(Fleet *fleet);

int main(int argc, char *argv[])
{
    Fleet fleet;

    if (!build_fleet(&fleet)) return EXIT_FAILURE;
    if (!print_stat(&fleet)) return EXIT_FAILURE;
    if (!print_adtm(&fleet)) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

bool read_ints(int32_t *i1, int32_t *i2)
{   /* assume que i1 e i2 apontam para objetos válidos */

    if (scanf("%" PRId32 " %" PRId32, i1, i2) != 2) {
        printf("Erro ao ler uma entrada de par de inteiros\n");
        return false;
    }
    return true;
}

bool build_fleet(Fleet *fleet)
{   /* assume que fleet aponta para um objeto válido */

    int32_t npost, ntp;
    int32_t u, v;
    int32_t ret;

    if (!read_ints(&npost, &ntp)) return false;

    if ((ret = fleet_init(fleet, npost, ntp)) < 0) {
        printf("Erro ao inicializar a frota: %" PRId32 "\n", ret);
        return false;
    }
    for (int32_t i = 0; i < ntp; i++) {
        if (!read_ints(&u, &v)) return false;
        u--; v--; /* corrigindo a base do índice para 0 */
        if ((ret = fleet_add(fleet, i, u, v)) < 0) {
            printf("Erro ao adicionar o teleporte (%" PRId32
                    ", %" PRId32 "): %" PRId32 "\n", u, v, ret);
            return false;
        }
    }
    return true;
}

bool print_stat(Fleet *fleet)
{   /* assume que fleet aponta para um objeto Fleet inicializado e que a frota 
       ainda não foi explorada */

    int32_t stat[FLEET_NTYPE];
    int32_t ret;

    if ((ret = fleet_scan(fleet)) < 0) {
        printf("Erro ao explorar a frota: %" PRId32 "\n", ret);
        return false;
    }
    if ((ret = fleet_stat(fleet, stat)) < 0) {
        printf("Erro ao obter as estatísticas da frota : %" PRId32 "\n", ret);
        return false;
    }

    for (int32_t i = 0; i < FLEET_NTYPE; i++) {
        printf("%d ", stat[i]);
    }
    putchar('\n');

    return true;
}

bool print_adtm(Fleet *fleet)
{   /* assume que fleet aponta para um objeto Fleet inicializado e que a frota 
       já tenha sido explorada */
    
    int32_t *p1, *p2;
    int64_t ret;

    p1 = malloc(2 * fleet->npost * sizeof(int32_t));
    if (p1 == NULL) {
        printf("Erro ao alocar memória\n");
        return false;
    }
    p2 = &p1[fleet->npost];
    for (int32_t i = 0; i < fleet->npost; i++) {
        if (!read_ints(&p1[i], &p2[i])) return false;
        p1[i]--, p2[i]--; /* corrigindo a base do índice para 0 */
    }
    ret = fleet_adtm(fleet, p1, p2);
    free(p1);
    if (ret < 0) {
        printf("Erro ao calcular o tempo de vantagem: %" PRId64 "\n", ret);
        return false;
    }
    printf("%" PRId64 "\n", ret);

    return true;
}