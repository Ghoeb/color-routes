#include "stage4.h"
#include <stdio.h>
#include "solver.h"
#include <time.h>

void stage4_node_paint(Client* node, Color color)
{
    if(!node -> color)
    {
        node -> color = color;
        for(int i = 0; i < node -> link_count; i++)
        {
            stage4_node_paint(node -> linked[i], color);
        }
    }
}

bool gen_solve_fill(Layout* layout)
{
    bool ret = true;

    for(int i = 0; i < layout -> core_count; i++)
    {
        Central* node = layout -> cores[i] -> buildings[0];
        stage4_node_paint(node, teal);
    }

    size_t decision_count = 0;
    for(int i = 0; i < layout -> zone_count; i++)
    {
        decision_count += layout -> zones[i] -> building_count / 2;
    }

    Choice* steps = malloc(sizeof(Choice) * decision_count);

    if(debug)
    {
        city_layout_print(layout);
        step = true;
    }
    else
    {
        step = false;
    }

    /* Medimos el tiempo */
    clock_t start = clock();

    /* Resolvemos el problema */
    if(solve(layout, steps))
    {
        double time_used = ((double) (clock() - start)) / CLOCKS_PER_SEC;
        /* Imprimimos las estadisticas */
        fprintf(stderr, "Resuelto template en ");
        fprintf(stderr, "%lf segundos, ", time_used);
        fprintf(stderr, "volviendo %u veces\n", undo_count);
    }
    else
    {
        fprintf(stderr, "%s\n","El puzzle no tiene solución");
        ret = false;
    }

    if(debug) printf("END\n");

    free(steps);

    return ret;
}
