#include "stage3.h"

/* Limpia los nodos solitarios, entrega cuantos quedaron */
int cell_clean(Zone* cell)
{
    /* Inicialmente se consideran todos activos */
    int active = cell -> building_count;

    for(int k = 0; k < cell -> building_count; k++)
    {
        /* Si un nodo no tiene vecinos */
        if(!cell -> buildings[k] -> link_count)
        {
            /* Se elimina */
            // node_destroy(cell -> buildings[k]);
            free(cell -> buildings[k] -> linked);
            free(cell -> buildings[k]);
            cell -> buildings[k] = NULL;
            /* Ya no se considera activo */
            active--;
        }
    }

    if(active < cell -> building_count)
    {
        /* Se guarda el arreglo antiguo como variable auxiliar */
        Building** aux = cell -> buildings;
        uint8_t old_count = cell -> building_count;
        /* Se crea un nuevo arreglo para los nodos activos */
        cell -> buildings = malloc(sizeof(Building*) * active);
        cell -> building_count = 0;

        /* Traspasamos los nodos activos al nuevo arreglo */
        for(int k = 0; k < old_count; k++)
        {
            if(aux[k])
            {
                aux[k] -> index = cell -> building_count;
                cell -> buildings[cell -> building_count++] = aux[k];
            }
        }
        /* Eliminamos el arreglo antiguo */
        free(aux);
    }

    return active;
}

/** Procesa la celda para que cumpla las reglas de los nucleos */
void cell_encore(Zone* cell)
{
    /* Inicializamos un nodo que tendra tantos vecinos como nodos hayan */
    Central* node = malloc(sizeof(Central));
    node -> color = none;
    node -> capacity = cell -> building_count;
    node -> linked = malloc(sizeof(Building*) * node -> capacity);
    node -> link_count = 0;
    for(int i = 0; i < node -> capacity; i++)
    {
        node -> linked[i] = NULL;
    }
    node -> index = 0;
    node -> direction = 0;
    node -> drawn = 0;

    for(int k = 0; k < cell -> building_count; k++)
    {
        /* El nodo absorbe los vecinos de los nodos inicales */
        node -> linked[node -> link_count++] = cell -> buildings[k] -> linked[0];
        /* El nodo toma el lugar del vecino original */
        cell -> buildings[k] -> linked[0] -> linked[0] = node;
        /* Destruimos el nodo original */
        // node_destroy(cell -> buildings[k]);
        free(cell -> buildings[k] -> linked);
        free(cell -> buildings[k]);

    }
    /* Eliminamos el anterior arreglo de nodos */
    free(cell -> buildings);
    /* Solicitamos uno nuevo, de largo 1 */
    cell -> buildings = malloc(sizeof(Zone));
    /* Guardamos nuestro unico nodo al inicio del arreglo */
    cell -> buildings[0] = node;
    /* Indicamos que esta celda tiene un solo nodo */
    cell -> building_count = 1;
    /* Asignamos el padre de este nodo */
    node -> zone = cell;
}

/** Limpia las conexiones inactivas de las celdas y genera el grafo */
Layout* gen_cleanup(Zone*** grid, BMP* bmp)
{
    /* Solicitamos la memoria para el grafo */
    Layout* graph = malloc(sizeof(Layout));
    /* Asignamos las dimensiones del grafo */
    graph -> width = BMP_GetWidth(bmp)+2;
    graph -> height = BMP_GetHeight(bmp)+2;

    /* Inicialmente no se sabe ni cuantas celdas ni cuantos cores se tiene */
    graph -> zone_count = 0;
    graph -> core_count = 0;

    /* Dimensiones de la matriz */
    int x = 2 * graph -> width - 1;
    int y = 2 * graph -> height - 1;

    /* Tercera pasada */
    /* Se eliminan las celdas que quedaron sin vecinos */
    /* Se clasifican las celdas segun core o no */



    /* El canal rojo india que hay que forzar core */
    uint8_t R = 0;

    for(int i = 0; i < x; i++)
    {
        for(int j = 0; j < y; j++)
        {
            if(grid[i][j])
            {
                /* Se limpian los nodos solitarios de la celda */
                int active = cell_clean(grid[i][j]);

                BMP_GetPixelRGB(bmp, (i / 2) - 1, (j / 2) - 1, &R, NULL, NULL);

                /* Si la celda quedó sin ningun nodo, se destruye */
                /* TODO Allow core */
                if(!active)
                {
                    // cell_destroy(grid[i][j]);
                    /* Libera cada uno de los nodos */
                    for(int k = 0; k < grid[i][j] -> building_count; k++)
                    {
                        // node_destroy(grid[i][j] -> buildings[k]);
                        free(grid[i][j] -> buildings[k] -> linked);
                        free(grid[i][j] -> buildings[k]);
                    }
                    /* El arreglo de nodos */
                    free(grid[i][j] -> buildings);
                    /* Y a si misma */
                    free(grid[i][j]);


                    grid[i][j] = NULL;
                    continue;
                }

                /* Si quedó una cantidad impar de nodos en la celda */
                /* TODO force core */
                if(active % 2 || (i % 2 == 0 && j % 2 == 0 && R))
                // if(active % 2)
                {
                    /* Significa que esta celda corresponde a un nucleo */
                    grid[i][j] -> core = true;

                    /* Procesamos el core */
                    cell_encore(grid[i][j]);

                    graph -> core_count++;
                }
                else
                {
                    graph -> zone_count++;
                }
            }
        }
    }

    /* Se preparan los arreglos para guardar las celdas */
    graph -> cores = malloc(sizeof(Core*) * graph -> core_count);
    graph -> core_count = 0;
    graph -> zones = malloc(sizeof(Zone*) * graph -> zone_count);
    graph -> zone_count = 0;

    /* Cuarta pasada */
    /* Se separan los cores de las celdas normales en 2 listas (arreglos) */
    for(int i = 0; i < x; i++)
    {
        for(int j = 0; j < y; j++)
        {
            if(!grid[i][j]) continue;

            if(grid[i][j] -> core)
            {
                grid[i][j] -> index = graph -> core_count;
                graph -> cores[graph -> core_count++] = grid[i][j];
            }
            else
            {
                grid[i][j] -> index = graph -> zone_count;
                graph -> zones[graph -> zone_count++] = grid[i][j];
            }
        }
    }

    for(int i = 0; i < x; i++)
    {
        free(grid[i]);
    }
    free(grid);

    fprintf(stderr, "%s\n", "Configurada grid y cores");

    return graph;
}
