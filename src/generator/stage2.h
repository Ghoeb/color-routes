#ifndef T1_GEN_STAGE2_H
#define T1_GEN_STAGE2_H

#include "../common/city.h"
#include "qdbmp.h"

/** Expande las celdas de 8 segun lo indicado por el canal azul del BMP */
/* Cada byte de este correspode a un indice en la celda */
/* El byte se lee de izquierda a derecha: */
/* El bit más significativo indica que hacer con el indice 0 de la celda */
/* El bit menos significativo indica que hacer con el indice 7 de la celda */
void gen_expand_cells(Zone*** grid, BMP* bmp);

#endif /* end of include guard: T1_GEN_STAGE2_H */
