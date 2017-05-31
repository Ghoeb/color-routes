#ifndef T1_GEN_STAGE1_H
#define T1_GEN_STAGE1_H

#include "qdbmp.h"
#include "../common/city.h"

/** Inicializa la grilla. Añade 2 filas y 2 columnas para envolver el BMP */
/* La grilla es una grilla con una columna entre cada columna del BMP */
/* Y una fila entre cada fila del BMP */
/* Las celdas con ambos indices par son celdas de 8 */
/* Corresponden a celdas del BMP:  bmp[i][j] <-> grid[(i+1)*2][(j+1)*2] */
/* Las celdas donde ambos indices son impares son celdas de 4 */
/* El resto no se inicializa */
Zone*** gen_initialize_grid(BMP* bmp);

#endif /* End of include guard: T1_GEN_STAGE1_H */
