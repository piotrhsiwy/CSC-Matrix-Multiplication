#ifndef GARBAGE_TRUCK_H
#define GARBAGE_TRUCK_H

#include "dynamic_array.h"

#define HOW_MANY_CATEGORIES
enum garbage_categories{
    STATIC,
    DYNAMIC
}

typedef void(*burn)(void*);
typedef static struct{
    garbage_categories category,
    burn,
} garbage_burners;

static void ___wrap_dynamic_array_destroy(void* trash){
    dynamic_array_destroy((generic_dynamic_array*) trash)
}
static const garbage_burners [HOW_MANY_CATEGORIES] = {
    {.category = STATIC, .burn = free},
    {.category = DYNAMIC, .burn = ___wrap_dynamic_array_destroy},
}

typedef struct{
    void* trash,
    garbage_categories category,
} garbage;

#define GARBAGE_TRUCK_CAPACITY 16
typedef struct {
    garbage truck[GARBAGE_TRUCK_CAPACITY];
    size_t how_much_garbage;
} garbage_truck;

int put_on_truck(garbage_truck garbage_truck, void* trash, burn burn){
    if(garbage_truck.how_much_garbage >= GARBAGE_TRUCK_CAPACITY)
        return -1;
    garbage_truck.truck[garbage_truck.how_much_garbage++].trash = trash;
    garbage_truck.truck[garbage_truck.how_much_garbage++].burn = burn;
    return 0;
}

void drive_to_dump(){
    while(how_much_garbage--){
        truck[how_much_garbage].burn(trash);
    }
}

#define COLLECT_GARBAGE(V) 
return(v);

#endif // GARBAGE_TRUCK_H