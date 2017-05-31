#include "stage2.h"

/* Conecta dos celdas entre ellas, a traves de los nodos especificados */
void cell_link(Zone* c1, Zone* c2, uint8_t i1, uint8_t i2)
{
    Building* node1 = c1 -> buildings[i1];
    Building* node2 = c2 -> buildings[i2];
    node1 -> linked[node1 -> link_count++] = node2;
    node2 -> linked[node2 -> link_count++] = node1;
}

/* Expande la celda de 8 para conectarla con sus vecinos */
void expand_n_cell(Zone*** grid, int i, int j, uint8_t which[8])
{
    /*  TODO condensar esto, ta feo */

    /* Hacia la derecha */
    if(which[0] && !grid[i][j] -> buildings[0] -> link_count)
        cell_link(grid[i][j],grid[i+2][j],0,4);

    /* Hacia arriba y la derecha */
    if(which[1] && !grid[i][j] -> buildings[1] -> link_count)
        cell_link(grid[i][j],grid[i+1][j-1],1,2);

    /* Hacia arriba */
    if(which[2] && !grid[i][j] -> buildings[2] -> link_count)
        cell_link(grid[i][j],grid[i][j-2],2,6);

    /* Hacia arriba y la izquierda */
    if(which[3] && !grid[i][j] -> buildings[3] -> link_count)
        cell_link(grid[i][j],grid[i-1][j-1],3,3);

    /* Hacia la izquierda */
    if(which[4] && !grid[i][j] -> buildings[4] -> link_count)
        cell_link(grid[i][j],grid[i-2][j],4,0);

    /* Hacia abajo y la izquierda */
    if(which[5] && !grid[i][j] -> buildings[5] -> link_count)
        cell_link(grid[i][j],grid[i-1][j+1],5,0);

    /* Hacia abajo */
    if(which[6] && !grid[i][j] -> buildings[6] -> link_count)
        cell_link(grid[i][j],grid[i][j+2],6,2);

    /* Hacia abajo y la derecha */
    if(which[7] && !grid[i][j] -> buildings[7] -> link_count)
        cell_link(grid[i][j],grid[i+1][j+1],7,1);
}

/* Expande las celdas de 8 segun lo indicado por el canal azul del BMP */
/* El canal azul es un byte */
/* Esos 8 bits corresponden cada uno a un indice de la celda */
/* El byte se lee de izquierda a derecha: */
/* El bit más significativo indica que hacer con el indice 0 de la celda */
/* El bit menos significativo indica que hacer con el indice 7 de la celda */
void gen_expand_cells(Zone*** grid, BMP* bmp)
{
    /* Obtenemos las dimensiones de la imagen */
    size_t bmp_width = BMP_GetWidth(bmp);
    size_t bmp_height = BMP_GetHeight(bmp);

    uint8_t B = 0;
    for(int i = 0; i < bmp_width; i++)
    {
        for(int j = 0; j < bmp_height; j++)
        {
            /* Tomamos el canal azul del pixel (i,j) */
            BMP_GetPixelRGB(bmp, i, j, NULL, NULL, &B);

            /** Variable que indicará que nodos expandir */
            uint8_t which[8];

            /* Mascara para leer los bits de B */
            uint8_t mask = 128;

            /* Leemos bit a bit marcando cuales expandir */
            /* El bit más significativo equivale al nodo 0 */
            /* Avanza hasta el 7, en el bit menos significativo */
            for(int b = 0; b < 8; b++, mask /= 2)
            {
                which[b] = B & mask;
            }

            /* Expandimos la celda correspondiente segun lo dicho en STAGE1 */
            expand_n_cell(grid,  (i+1)*2, (j+1)*2, which);
        }
    }

    fprintf(stderr, "%s\n", "Expandida grid");

}
