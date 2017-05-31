#ifndef T1_GEN_STAGE3_H
#define T1_GEN_STAGE3_H

#include "qdbmp.h"
#include "../common/city.h"

/** Limpia las conexiones inactivas de las celdas y genera el grafo */
/* Destruye grid */
Layout* gen_cleanup(Zone*** grid, BMP* bmp);

#endif /* end of include guard: T1_GEN_STAGE3_H */
