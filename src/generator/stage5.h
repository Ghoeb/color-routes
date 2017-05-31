#ifndef T1_GEN_STAGE5_H
#define T1_GEN_STAGE5_H

#include "../common/city.h"
#include "qdbmp.h"

/** Asigna colores al grafo y limpia las aristas, excepto las especificadas
por el canal verde del BMP */
void gen_color_and_clear(Layout* graph, BMP* bmp);

#endif /* end of include guard: T1_GEN_STAGE5_H */
