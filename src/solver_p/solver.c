#include "solver.h"
#include <stdio.h>
#include <limits.h>

/** La cantidad de decisiones que es necesario tomar para resolver */
size_t decisions_required;

/** La cantidad de posibles rutas */
size_t routes_count;

/** La cantidad de rutas que se han cerrado */
size_t routes_closed;

/** Cores de la ciudad, agrupados por color */
Core*** cores;

/** Cuantos core hay de cada color */
int core_count[8] = {0,0,0,0,0,0,0,0};

/*#########################################################################*/
/*                                  Easy                                   */
/*#########################################################################*/

/** La cantidad de decisiones que se han tomado */
size_t decision_count;

/* Indica si un par de nodos son pareja válida */
bool is_valid_pair(Client* node1, Client* node2)
{
    if(node1 -> color == node2 -> color) return true;
    if(!node1 -> color || !node2 -> color) return true;
    return false;
}

/* Indica que emparejar estos dos nodos es una jugada legal */
bool is_legal(Client* node1, Client* node2)
{
    return node1 != node2 &&
           !city_client_is_taken(node2) &&
           is_valid_pair(node1, node2);
}

/* Pinta recursivamente del color escogido */
void client_paint(Client* target, Color color)
{
    if(target -> color != color)
    {
        target -> color = color;
        if(!color) target -> abastecedor = NULL;

        for(int i = 0; i < target -> link_count; i++)
        {
            client_paint(target -> linked[i], color);
        }
    }
}


/*#########################################################################*/
/*                                Normal                                   */
/*#########################################################################*/

/* Obtiene el cliente que está al final de la cadena que parte en start */
/* Solo debe ser usado por clientes incoloros */
Client* chain_end(Client* start)
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

/* Revisa que la disposicion de colores de la celda sea resolvible */
bool zone_is_solvable(Zone* zone)
{
    /* Contamos los colores de los nodos que quedan por emparejar */
    uint8_t color_count[8] = {0,0,0,0,0,0,0,0};

    for(int i = 0; i < zone -> building_count; i++)
    {
        /* Para cada cliente sin pareja */
        if(!city_client_is_taken(zone -> buildings[i]))
        {
            /* Marcamos de que color es */
            color_count[zone -> buildings[i] -> color]++;
        }
    }
    /* Para cada color, desde el ultimo al primero */
    for(int i = 7; i > 0; i--)
    {
        /* Si hay una cantidad impar de ese color */
        if(color_count[i] % 2)
        {
            /* Lo compensamos con un incoloro */
            if(color_count[0] > 0)
            {
                color_count[0]--;
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}

/* Pinta recursivamente del color escogido, revisando si deja algo mal */
bool client_paint_check_zone(Client* target, Color color , Core* core)
{
    bool ret = true;
    if(target -> color != color)
    {
        target -> color = color;
        target -> abastecedor = core;

        if(!zone_is_solvable(target -> zone))
        {
            return false;
        }

        for(int i = 0; i < target -> link_count; i++)
        {
            Client* pal = target -> linked[i];
            ret = ret && client_paint_check_zone(pal, color, core);
        }
    }
    return ret;
}

/* Toma la siguiente decision, y retorna false si queda algo inresolvible */
bool assign_next(Client* next, Choice* steps)
{
    /* El valor que retornaremos */
    bool ret = true;

    /* Tomamos el siguiente candidato a pareja */
    Client* val = heap_extract(next -> options);

    /* Almacenamos la decision */
    Choice* choice = &steps[decision_count++];

    /* El que toma la decision */
    choice -> choser = next;
    /* La pareja que escogió */
    choice -> chosen = val;
    /* Cual de los dos cambió de color */
    choice -> tinted = NULL;

    /* Si son de distinto color significa que uno tiene y otro no */
    if(next -> color != val -> color)
    {
        /* PODA 1 : Si hacemos que una celda sea inresolvible al pintar */
        /* Si el que toma la decision tiene color */
        if(next -> color)
        {
            /* Pintamos al escogido */
            choice -> tinted = val;
            ret = client_paint_check_zone(val, next -> color , next -> abastecedor);
        }
        else
        {
            /* Sino, pintamos al que tomo la decision */
            choice -> tinted = next;
            ret = client_paint_check_zone(next, val -> color, val -> abastecedor);
        }
    }
    /* Ambos están sin color */
    else if(!next -> color)
    {
        /* PODA 2 : Si cerramos un ciclo incoloro */

        if(next -> zone == chain_end(next) -> zone)
        {
            ret = false;
        }
    }
    /* Ambos son del mismo color */
    else
    {
        /* PODA 3 : Si cerramos todas las rutas y aun queda por resolver */

        /* Cerramos los extremos de la ruta */
        next -> abastecedor -> avaliable_ports--;
        val -> abastecedor -> avaliable_ports--;

        /* Cerramos una ruta */
        routes_closed++;

        /* Si cerramos todas las rutas */
        if(routes_closed == routes_count)
        {
            /* Y aun nos quedan decisiones por tomar */
            if(decision_count < decisions_required)
            {
                ret = false;
            }
        }
    }

    /* Efectuamos la conexión */
    city_client_link(next, val);

    /* Si se quiere hacer seguimiento */
    if(step)
    {
        city_client_link_print(next, val);
    }

    return ret;
}

/* Desconecta dos clientes */
Client* choice_undo(Choice* steps)
{
    Choice* choice = &steps[--decision_count];

    city_client_link_undo(choice -> choser, choice -> chosen);
    /* Si se quiere hacer seguimiento */
    if(step)
    {
        city_client_link_undo_print(choice -> choser, choice -> chosen);
    }

    /* Le sacamos el color a los clientes que habian cambiado */
    if(choice -> tinted)
    {
        client_paint(choice -> tinted, none);
    }
    else if(choice -> choser -> color)
    {
        choice -> chosen -> abastecedor -> avaliable_ports++;
        choice -> choser -> abastecedor -> avaliable_ports++;

        routes_closed--;
    }

    /* Registramos la devolución */
    undo_count++;

    return choice -> choser;
}

/*#########################################################################*/
/*                                 Hard                                    */
/*#########################################################################*/

/** Matriz que indica el orden en que se deben probar los nodos */
int scores[4][8] =
{
    {10000},
    {9000, 5000, 1000},
    {8000, 6000, 4000, 2000, 1000},
    {7000, 6000, 5000, 4000, 3000, 2000, 1000}
};

/** Prioridad que tiene cada color */
int color_score[8] = {0,0,0,0,0,0,0,0};

int distance_squared(Core* core, Zone* zone)
{
    int dx = core -> x - zone -> x;
    int dy = core -> y - zone -> y;

    return dx*dx + dy*dy;
}

int pal_score(Client* master, Client* servant)
{
    /* Si el maestro no tiene color, entonces todos le valen lo mismo */
    if(!master -> color) return 0;

    /** Arreglo de cores del color del maestro */
    Core** colores = cores[master -> color];

    /** Cuantos cores hay de ese color */
    int color_count = core_count[master -> color];

    /* La distancia al core mas cercano del color del maestro */
    int best_dist = INT32_MAX;

    for(int i = 0; i < color_count; i++)
    {
        /* Un posible candidato */
        Core* core = colores[i];

        /* Si no le quedan puertos no es un candidato valido */
        if(!core -> avaliable_ports) continue;

        /* Calculamos la distancia de la zona vecina a el core */
        int dist = distance_squared(core, servant -> vecino);

        /* Si está mas cerca */
        if(dist < best_dist) best_dist = dist;
    }

    /* El puntaje es mayor mientras menor sea la distancia */
    return -best_dist;
}

/** Calcula las opciones de un cliente */
bool options(Client* client)
{
    heap_clear(client -> options);

    Zone* zone = client -> zone;
    /* Revisamos los posibles valores en orden */
    for (int i = 0; i < zone -> building_count; i++)
    {
        if(i == client -> index) continue;

        Client* pal = zone -> buildings[i];

        /* Si es legal unir esa pareja */
        if(is_legal(client,pal))
        {
            // /* HEURISTICA 1: Preferir los con color y del mismo color */
            // if(pal -> color && client -> color == pal -> color)
            // {
            //     heap_insert(client -> options, pal, 1);
            // }
            // else
            // {
            //     heap_insert(client -> options, pal, 10);
            // }
            heap_insert(client -> options, pal, pal_score(client, pal));
        }
    }

    return !heap_is_empty(client -> options);
}

/* Calcula el puntaje de un nodo. Mientras mayor sea, antes se evaluará */
int client_score(Client* client)
{
    /* Solo si no ha sido emparejado tiene puntaje */
    if(city_client_is_taken(client)) return 0;

    /* El hecho de que sea no emparejado le da alta prioridad */
    int score = 100000;

    /* Luego le asignamos puntaje segun cuantas opciones tiene */
    int total_options = client -> zone -> building_count / 2 - 1;
    int actual_options = client -> options -> count - 1;

    /* Entre 1000 y 10000 */
    score += scores[total_options][actual_options];

    /* Mientras menos hay de su color, más prioridad tiene */
    score += color_score[client -> color];

    /* Entrega el puntaje final del nodo */
    return score;
}

/* Encuentra el siguiente nodo sin asignar. Toma el con mejor puntaje */
Client* find_unnasigned_location(Layout* layout)
{
    Client* selected = NULL;

    double score = 0;

    for(int i = 0; i < layout -> zone_count; i++)
    {
        for(int j = 0; j < layout -> zones[i] -> building_count; j++)
        {
            Client* challenger = layout -> zones[i] -> buildings[j];

            if(!city_client_is_taken(challenger))
            {
                /* Si no tenemos ninguno, esta es la mejor opcion */
                if(!selected)
                {
                    selected = challenger;

                    options(selected);

                    score = client_score(selected);
                }
                else
                {
                    /* Calculamos las opciones */
                    options(challenger);

                    int challenscore = client_score(challenger);
                    if(challenscore > score)
                    {
                        selected = challenger;
                        score = challenscore;
                    }
                }
            }
        }
    }
    for(int i = 0; i < layout -> zone_count; i++)
    {
        for(int j = 0; j < layout -> zones[i] -> building_count; j++)
        {
            Client* challenger = layout -> zones[i] -> buildings[j];

            if(city_client_is_blank(challenger))
            {
                /* Si no tenemos ninguno, esta es la mejor opcion */
                if(!selected)
                {
                    selected = challenger;

                    options(selected);

                    score = client_score(selected);
                }
                else
                {
                    /* Calculamos las opciones */
                    options(challenger);

                    int challenscore = client_score(challenger);
                    if(challenscore > score)
                    {
                        selected = challenger;
                        score = challenscore;
                    }
                }
            }
        }
    }

    return selected;
}

/** Marca los edificios conectados como distribuidos por el mismo core */
void distribute(Building* building, Core* provider)
{
    if(!building -> abastecedor)
    {
        building -> abastecedor = provider;
        for(int i = 0; i < building -> link_count; i++)
        {
            distribute(building -> linked[i], provider);
        }
    }
}

/** Inicializa los elementos del solver dentro del problema */
void solver_init(Layout* layout)
{
    /* Inicializamos las variables que usa el solver */
    for(int i = 0; i < layout -> zone_count; i++)
    {
        Zone* zone = layout -> zones[i];
        for(int j = 0; j < zone -> building_count; j++)
        {
            Client* client = zone -> buildings[j];
            client -> options = heap_init(zone -> building_count - 1);
            client -> abastecedor = NULL;
            /* Obtenemos el vecino de este nodo */
            client -> vecino = client -> linked[0] -> zone;
        }
    }

    /* Contamos la cantidad de decisiones que hay que tomar */
    decisions_required = 0;
    for(int i = 0; i < layout -> zone_count; i++)
    {
        decisions_required += layout -> zones[i] -> building_count;
    }
    decisions_required /= 2;
    /* Inicialmente no se ha tomado ninguna */
    decision_count = 0;

    /* Contamos la cantidad de rutas de cada color */
    int color_count[8] = {0,0,0,0,0,0,0,0};

    /* Contamos la cantidad de rutas que existen en el sistema */
    routes_count = 0;
    for(int i = 0; i < layout -> core_count; i++)
    {
        /* El core actual */
        Core* core = layout -> cores[i];

        /* La capacidad de este core */
        uint8_t capacity = city_core_get_capacity(core);

        /* El color de este core */
        Color color = city_core_get_color(core);

        /* La agregamos a la cantidad total de rutas */
        routes_count += capacity;

        /* La agregamos a la cantidad de rutas de ese color */
        color_count[color] += capacity;

        /* Contamos un core mas de ese color */
        core_count[color]++;

        /* Inicialmente todos los puertos del core están disponibles */
        layout -> cores[i] -> avaliable_ports = capacity;

        /* Marcamos los edificios ya conectados a este core */
        for(int i = 0; i < core -> building_count; i++)
        {
            Central* central = core -> buildings[i];

            central -> abastecedor = NULL;

            central -> vecino = NULL;

            distribute(central, core);
        }
    }
    routes_count /= 2;
    /* Inicialmente no hemos cerrado ninguna */
    routes_closed = 0;

    /* Inicializamos el grupo de cores por color */
    cores = malloc(sizeof(Core**) * 8);
    for(int i = 0; i < 8; i++)
    {
        cores[i] = malloc(sizeof(Core*) * core_count[i]);
        core_count[i] = 0;
    }

    /* Separamos los cores por color */
    for(int i = 0; i < layout -> core_count; i++)
    {
        Core* core = layout -> cores[i];
        Color color = city_core_get_color(core);
        /* Indice 1: Color */
        /* Indice 2: Usamos el contador de ese color */
        cores[color][core_count[color]++] = core;
    }

    /* Calculamos la prioridad que va a tener cada color */
    Color colors[8] = {0,1,2,3,4,5,6,7};
    Heap* color_heap = heap_init(7);
    for(int i = 1; i < 8; i++)
    {
        heap_insert(color_heap, &colors[i], color_count[i]);
    }
    /* Mientras menos haya del mismo color, mas prioridad tiene */
    int score = 100;
    while(!heap_is_empty(color_heap))
    {
        Color* c = heap_extract(color_heap);
        color_score[*c] = score;
        score += 100;
    }
    heap_destroy(color_heap);

    // fprintf(stderr, "Prioridad de colores: \n");
    //
    // for(int i = 0; i < 8; i++)
    // {
    //     fprintf(stderr, "COLOR %d: %d PUERTOS, PRIORIDAD %d\n", i, color_count[i], color_score[i]);
    // }
}

/** Libera los recursos que se usaron solo para el solver */
void solver_release(Layout* layout)
{
    for(int i = 0; i < layout -> zone_count; i++)
    {
        Zone* zone = layout -> zones[i];
        for(int j = 0; j < zone -> building_count; j++)
        {
            Client* client = zone -> buildings[j];
            heap_destroy(client -> options);
        }
    }

    for(int i = 0; i < 8; i++)
    {
        free(cores[i]);
    }
    free(cores);
}


bool solve(Layout* layout, Choice* steps)
{
    solver_init(layout);

    do
    {
        /* Obtenemos el siguiente cliente a asignar */
        Client* next = find_unnasigned_location(layout);

        /* Si no nos quedan clientes por asignar, ya terminamos el puzzle */
        if(!next)
        {
            solver_release(layout);
            return true;
        }

        if(!options(next) || !assign_next(next, steps))
        {
            do
            {
                /* Recuperamos la celda que tomo la ultima desicion */
                do
                {
                    /* Si no nos queda a donde volver */
                    if(decision_count == 0)
                    {
                        solver_release(layout);
                        /* El puzzle no se puede resolver */
                        return false;
                    }
                    /* Volvemos al nodo anterior */
                    next = choice_undo(steps);

                /* Si a esa celda no le quedan opciones */
                /* volvemos a retroceder */
                } while(heap_is_empty(next -> options));

            /* Retrocedemos hasta poder tomar una opcion valida */
            } while(!assign_next(next, steps));
        }

    } while(decision_count > 0);

    /* Ya probamos todos los valores y ninguno servia. */
    /* Entonces este estado no puede llegar a la solución */
    solver_release(layout);
    return false;
}
