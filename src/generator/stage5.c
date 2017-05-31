#include "stage5.h"

/** Absorbe el color del nodo y de todos los alcanzables por el */
void node_decolor(Client* node)
{
    if(node -> color)
    {
        node -> color = none;
        for(int i = 0; i < node -> link_count; i++)
        {
            node_decolor(node -> linked[i]);
        }
    }
}

/** Limpia todas las conexiones internas de una celda */
void zone_reset(Zone* zone)
{
    for(int i = 0; i < zone -> building_count; i++)
    {
        zone -> buildings[i] -> linked[--zone -> buildings[i] -> link_count] = NULL;
        if(!zone -> buildings[i] -> linked[0] -> zone -> core)
        {
            zone -> buildings[i] -> color = none;
        }
    }
}

/* Pinta recursivamente al nodo objetivo y a los que esten conectados a el */
void stage5_node_paint(Client* node, Color color, int* color_count)
{
    /* Solo si el nodo no tiene color */
    if(!node -> color)
    {
        /* Se lo pinta */
        node -> color = color;
        /* Contamos como que se agregó un nodo a este color */
        if(node -> zone -> core)
        {
            color_count[color - 1]+=2;
        }
        else
        {
            color_count[color - 1]++;
        }
        /* Y se extiende la pintura a cada uno de sus vecinos */
        for(int i = 0; i < node -> link_count; i++)
        {
            stage5_node_paint(node -> linked[i], color, color_count);
        }
    }
}

void gen_color_and_clear(Layout* graph, BMP* bmp)
{
    /* Se lava todo del color placeholder usado para completar el grafo */
    for(int i = 0; i < graph -> core_count; i++)
    {
        node_decolor(graph -> cores[i] -> buildings[0]);
    }

    /* Cuenta cuantas veces ha sido usado cada color */
    int color_count[7] = {0,0,0,0,0,0,0};
    /* Se asignan los colores, tratando de ser equitativo */
    for(int i = 0; i < graph -> core_count; i++)
    {
        Color chosen = red;
        for(Color j = red; j <= purple; j++)
        {
            if(color_count[j - 1] < color_count[chosen - 1])
            {
                chosen = j;
            }
            else if(color_count[j - 1] == color_count[chosen - 1] && rand()%2)
            {
                chosen = j;
            }
        }

        stage5_node_paint(graph -> cores[i] -> buildings[0], chosen, color_count);
    }

    uint8_t G = 0;

    for(int i = 0; i < graph -> zone_count; i++)
    {
        Zone* zone = graph -> zones[i];

        BMP_GetPixelRGB(bmp, (zone -> x / 2) - 1, (zone -> y / 2) - 1, NULL, &G, NULL);

        if(zone -> x % 2 == 0 && zone -> y % 2 == 0 && G)
        {
            continue;
        }
        else
        {
            zone_reset(graph -> zones[i]);
        }
    }

    for(int i = 0; i < graph -> zone_count; i++)
    {
        Zone* zone = graph -> zones[i];

        BMP_GetPixelRGB(bmp, (zone -> x / 2) - 1, (zone -> y / 2) - 1, NULL, &G, NULL);

        if(zone -> x % 2 == 0 && zone -> y % 2 == 0 && G)
        {
            for(int j = 0; j < zone -> building_count; j++)
            {
                Client* cli = zone -> buildings[j];

                for(int k = 0; k < cli -> link_count; k++)
                {
                    cli -> linked[k] -> color = cli -> color;
                }
            }
        }
    }




    fprintf(stderr, "%s\n", "Limpiado y pintado puzzle");
}
