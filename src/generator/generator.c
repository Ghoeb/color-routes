#include <stdlib.h>
#include "qdbmp.h"
#include "../common/city.h"
#include "../common/queue.h"
#include "../common/stack.h"
#include "stage1.h"
#include "stage2.h"
#include "stage3.h"
#include "stage4.h"
#include "stage5.h"

/** Inicializa un grafo a partir de un BMP */
/* El canal azul indica como expandir las celdas */
/* TODO El canal rojo indica que hay que forzar que una celda sea core */
/* TODO El canal verde indica que esa celda no hay que deshacerla */
Layout* graph_from_bmp(BMP* bmp)
{
    /* Inicializa la grilla del grafo */
    Zone*** grid = gen_initialize_grid(bmp);
    /* Expande las celdas que hay que expandir */
    gen_expand_cells(grid, bmp);
    /* Limpia y reduce las celdas, obtiene el grafo y destruye grid */
    Layout* graph = gen_cleanup(grid, bmp);
    /* Completa el grafo para asegurar que sea resolvible */
    bool status = gen_solve_fill(graph); // TODO OPTIMIZAR URGENTE
    /* Pinta y limpia el grafo */
    if(status) gen_color_and_clear(graph, bmp);

    return graph;
}

/* Obtiene el cliente que está al final de la cadena que parte en start */
/* Solo debe ser usado por clientes incoloros */
Client* chain_end_d(Client* start)
{
    /* Dado que es un extremo, tiene solo un edificio conectado */
    Client* next = start -> linked[0];
    Client* prev = start;

    /* Mientras no estemos en un extremo */
    while(city_client_is_taken(next))
    {
        /* El nodo actual tiene 2 vecinos: uno es el anterior */
        for(int i = 0; i < next -> link_count; i++)
        {
            /* El otro es el siguiente */
            if(next -> linked[i] != prev)
            {
                Client* aux = next;
                next = next -> linked[i];
                prev = aux;
                break;
            }
        }
    }

    return next;
}

int main(int argc, char** argv)
{
    // size_t n = 100;
    // Client** clients = malloc(sizeof(Client*) * n);
    // for(int i = 0; i < n; i++)
    // {
    //     /* Solicitamos memoria para un nodo */
    //     Building* node = malloc(sizeof(Building));
    //     /* Inicialmente los nodos no tienen color */
    //     node -> color = none;
    //     /* Cuantas conexiones desea tener este nodo */
    //     node -> capacity = 2;
    //     node -> linked = malloc(sizeof(Building*) * node -> capacity);
    //     /* Pero inicialmente no tiene ninguna */
    //     node -> link_count = 0;
    //     for(int i = 0; i < node -> capacity; i++)
    //     {
    //         node -> linked[i] = NULL;
    //     }
    //
    //     node -> index = i;
    //
    //     // node -> direction = i;
    //
    //     // cell -> buildings[i] = node;
    //
    //     clients[i] = node;
    // }
    //
    // for(int i = 0; i < n - 1; i++)
    // {
    //     city_client_link(clients[i], clients[i+1]);
    // }
    //
    // Client* start = chain_end_d(clients[n-1]);
    // Client* end = chain_end_d(clients[0]);
    //
    // printf("CLIENTS[0]: %p\n", clients[0]);
    // printf("CLIENTS[n-1]: %p\n", clients[n-1]);
    // printf("START: %p\n",start);
    // printf("END: %p\n", end);
    //
    //
    //
    //
    //
    //
    //
    //
    // for(int i = 0; i < n; i++)
    // {
    //     free(clients[i] -> linked);
    //     free(clients[i]);
    // }
    // free(clients);
    //
    // return 0;


    debug = false;
    char* filename;
    if(argc >= 2)
    {
        filename = argv[1];
        if(argc >= 3)
        {
            srand(atoi(argv[2]));
            if(argc >= 4)
            {
                debug = true;
            }
        }
    }
    else
    {
        printf("Usage: %s <bmp> <seed?> <debug?>\n", argv[0]);
        return 1;
    }

    BMP* bmp = BMP_ReadFile(filename);
    if(!bmp)
    {
        fprintf(stderr, "%s\n", "Invalid BMP file");
        return 2;
    }
    Layout* graph = graph_from_bmp(bmp);
    if(!debug)
    {
        city_layout_print(graph);
        printf("END\n");
    }

    BMP_Free(bmp);
    city_layout_destroy(graph);
    fprintf(stderr, "%s\n", "Generator Ready");
    return 0;
}
