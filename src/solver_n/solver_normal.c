#include <stdio.h>
#include <string.h>
#include <time.h>
#include "../common/city.h"
#include "solver.h"

/** Revisa los parametros */
void check_params(int argc, char const** argv)
{
    /* -s = seguimiento */
    if(argc == 2 && !strcmp(argv[1],"-s"))
    {
        step = true;
    }
    else
    {
        step = false;
    }
}

int main(int argc, char const *argv[])
{
    /* Revisamos si se quiere seguimiento */
    check_params(argc, argv);

    /* Leemos la ciudad del input */
    Layout* layout = city_layout_read(stdin);

    /* Lo imprimimos para darle el estado inicial al watcher / judge */
    city_layout_print(layout);

    /* Contamos la cantidad de decisiones que hay que tomar */
    size_t decision_count = 0;
    for(int i = 0; i < layout -> zone_count; i++)
    {
        decision_count +=  layout -> zones[i] -> building_count / 2;
    }

    /* Inicializamos un stack de ese tamaño */
    Choice* steps = malloc(sizeof(Choice) * decision_count);

    /* Medimos el tiempo */
    clock_t start = clock();

    /* Resolvemos el problema */
    if(solve(layout, steps))
    {
        double time_used = ((double) (clock() - start)) / CLOCKS_PER_SEC;
        /* Imprimimos las estadisticas */
        fprintf(stderr, "%s ha resuelto el problema en ", argv[0]);
        fprintf(stderr, "%lf segundos, ", time_used);
        fprintf(stderr, "volviendo %u veces\n", undo_count);

        /* Si no hicimos seguimiento entonces imprimimos los pasos */
        if(!step)
        {
            /* Imprimimos las decisiones */
            for(int i = 0; i < decision_count; i++)
            {
                city_client_link_print(steps[i].choser, steps[i].chosen);
            }
        }
    }
    else
    {
        /* Indicamos que no se pudo resolver */
        fprintf(stderr, "%s\n", "NO SOLUTION");
    }

    /* Indicamos al watcher y al judge que ya terminamos */
    printf("END\n");

    /* Liberamos memoria */
    free(steps);
    city_layout_destroy(layout);

    /* El programa terminó satisfactoriamente */
    return 0;
}
