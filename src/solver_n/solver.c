#include "solver.h"
#include <stdio.h>


/** La cantidad de decisiones que es necesario tomar para resolver */
size_t decisions_required;

/** La cantidad de posibles rutas */
size_t routes_count;

/** La cantidad de rutas que se han cerrado */
size_t routes_closed;

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

/* Encuentra el siguiente nodo sin asignar. Toma el primero que ve */
Client* find_unnasigned_location(Layout* layout)
{
    for(int i = 0; i < layout -> zone_count; i++)
    {
        for(int j = 0; j < layout -> zones[i] -> building_count; j++)
        {
            if(!city_client_is_taken(layout -> zones[i] -> buildings[j]))
            {
                return layout -> zones[i] -> buildings[j];
            }
        }
    }
    for(int i = 0; i < layout -> zone_count; i++)
    {
        for(int j = 0; j < layout -> zones[i] -> building_count; j++)
        {
            if(city_client_is_blank(layout -> zones[i] -> buildings[j]))
            {
                return layout -> zones[i] -> buildings[j];
            }
        }
    }
    return NULL;
}

/*#########################################################################*/
/*                            Solver-specific                              */
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
                // fprintf(stderr, "ZONA %zu INRESOLVIBLE\n", zone -> index);
                return false;
            }
        }
    }
    return true;
}

/* Pinta recursivamente del color escogido, revisando si deja algo mal */
bool client_paint_check_zone(Client* target, Color color)
{
    bool ret = true;
    if(target -> color != color)
    {
        target -> color = color;

        /* Si una celda quedó inresolvible */
        bool ok = zone_is_solvable(target -> zone);
        if(!ok) return false;

        for(int i = 0; i < target -> link_count; i++)
        {
            ret = ret && client_paint_check_zone(target -> linked[i], color);
        }
    }
    return ret;
}

/* Pinta recursivamente del color escogido */
void client_paint(Client* target, Color color)
{
    if(target -> color != color)
    {
        target -> color = color;
        for(int i = 0; i < target -> link_count; i++)
        {
            client_paint(target -> linked[i], color);
        }
    }
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
            ret = client_paint_check_zone(val, next -> color);
        }
        else
        {
            /* Sino, pintamos al que tomo la decision */
            choice -> tinted = next;
            ret = client_paint_check_zone(next, val -> color);
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

    /* Registramos la devolución */
    undo_count++;

    return choice -> choser;
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
            heap_insert(client -> options, pal, -i);
        }
    }

    return !heap_is_empty(client -> options);
}

/** Inicializa los elementos del solver dentro del problema */
void solver_init(Layout* layout)
{
    decision_count = 0;
    routes_closed = 0;

    /* Contamos la cantidad de decisiones que hay que tomar */
    decisions_required = 0;
    for(int i = 0; i < layout -> zone_count; i++)
    {
        decisions_required += layout -> zones[i] -> building_count;
    }
    decisions_required /= 2;

    routes_count = 0;
    for(int i = 0; i < layout -> core_count; i++)
    {
        routes_count += city_core_get_capacity(layout -> cores[i]);
    }
    routes_count /= 2;


    for(int i = 0; i < layout -> zone_count; i++)
    {
        Zone* zone = layout -> zones[i];
        for(int j = 0; j < zone -> building_count; j++)
        {
            Client* client = zone -> buildings[j];
            client -> options = heap_init(zone -> building_count - 1);
        }
    }
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
