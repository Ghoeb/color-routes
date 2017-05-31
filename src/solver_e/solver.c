#include "solver.h"

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


void client_connect(Client* client1, Client* client2, Choice* choice)
{
    choice -> choser = client1;
    choice -> chosen = client2;

    if(client1 -> color != client2 -> color)
    {
        if(client1 -> color)
        {
            choice -> tinted = client2;
            client_paint(client2, client1 -> color);
        }
        else
        {
            choice -> tinted = client1;
            client_paint(client1, client2 -> color);
        }
    }
    else
    {
        choice -> tinted = NULL;
    }

    city_client_link(client1, client2);

    /* Si se quiere hacer seguimiento */
    if(step)
    {
        city_client_link_print(client1, client2);
    }
}

void client_disconnect(Client* client1, Client* client2, Choice* choice)
{
    city_client_link_undo(client1, client2);
    /* Si se quiere hacer seguimiento */
    if(step)
    {
        city_client_link_undo_print(client1, client2);
    }

    /* Le sacamos el color a los clientes que habian cambiado */
    if(choice -> tinted)
    {
        client_paint(choice -> tinted, none);
    }

    /* Registramos la devolución */
    undo_count++;
}

size_t decisions = 0;

bool solve(Layout* layout, Choice* steps)
{
    Client* next = find_unnasigned_location(layout);
    /* Si no nos quedan celdas por asignar, ya terminamos el puzzle */
    if(!next)
    {
        return true;
    }

    Zone* zone = next -> zone;

    /* Revisamos los posibles valores en orden */
    for (int i = 0; i < zone -> building_count; i++)
    {
        if(i == next -> index) continue;

        /* Si es legal asignar ese valor */
        if (is_legal(next, zone -> buildings[i]))
        {
            /* Lo asignamos */
            client_connect(next, zone -> buildings[i], &steps[decisions++]);

            /* Si el estado resultante puede ser resuelto, estamos listos! */
            if(solve(layout, steps))
            {
                return true;
            }

            /* Si no, debemos deshacer nuestra jugada */
            client_disconnect(next, zone -> buildings[i], &steps[--decisions]);
        }
    }
    /* Ya probamos todos los valores y ninguno servia. */
    /* Entonces este estado no puede llegar a la solución */
    return false;
}
