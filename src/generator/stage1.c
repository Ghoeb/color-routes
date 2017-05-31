#include "stage1.h"

/* Inicializa la grilla. Añade 2 filas y 2 columnas para envolver el BMP */
/* La grilla es una grilla con una columna entre cada columna del BMP */
/* Y una fila entre cada fila del BMP */
/* Las celdas con ambos indices par son celdas de 8 */
/* Corresponden a celdas del BMP:  bmp[i][j] <-> grid[(i+1)*2][(j+1)*2] */
/* Las celdas donde ambos indices son impares son celdas de 4 */
/* El resto no se inicializa */
Zone*** gen_initialize_grid(BMP* bmp)
{
    /* Tamaño del grafo. Dos filas y columnas extra */
    size_t width = BMP_GetWidth(bmp) + 2;
    size_t height = BMP_GetHeight(bmp) + 2;

    /* Tamaño de la grilla */
    size_t x = 2 * width - 1;
    size_t y = 2 * height - 1;


    Zone*** grid = malloc(sizeof(Zone**) * x);

    /* Inicializar celdas */
    for(int i = 0; i < x; i++)
    {
        grid[i] = malloc(sizeof(Zone*) * y);
        for(int j = 0; j < y; j++)
        {
            /* Si corresponde a una de las celdas activas */
            if(!((i+j)%2))
            {
                /* Si i es impar, j tambien lo es, asi que es celda de 4 */
                uint8_t sides = i % 2 ? 4 : 8;

                /* Solicitamos el espacio en memoria para la celda */
                Zone* cell    = malloc(sizeof(Zone));
                /* Por defecto las celdas no son nucleo */
                cell -> core  = false;
                /* Por defecto hay un nodo por lado */
                cell -> building_count = sides;
                /* Si tiene N lados, contiene N nodos */
                cell -> buildings = malloc(sizeof(Building*) * sides);
                /* Inicializamos cada uno de ellos */
                for(int i = 0; i < sides; i++)
                {
                    /* Solicitamos memoria para un nodo */
                    Building* node = malloc(sizeof(Building));
                    /* Inicialmente los nodos no tienen color */
                    node -> color = none;
                    /* Cuantas conexiones desea tener este nodo */
                    node -> capacity = 2;
                    node -> linked = malloc(sizeof(Building*) * node -> capacity);
                    /* Pero inicialmente no tiene ninguna */
                    node -> link_count = 0;
                    for(int i = 0; i < node -> capacity; i++)
                    {
                        node -> linked[i] = NULL;
                    }
                    node -> zone = cell;

                    node -> index = i;

                    node -> direction = i;

                    cell -> buildings[i] = node;
                }
                /* Asignamos la cantidad de lados */
                cell -> sides = sides;

                /* Asignamos coordenadas */
                cell -> x = i;
                cell -> y = j;

                grid[i][j] = cell;
            }
            else
            {
                /* Si no es una celda activa se marca como tal */
                grid[i][j] = NULL;
            }
        }
    }

    fprintf(stderr, "%s\n", "Inicializada grid");

    return grid;
}
