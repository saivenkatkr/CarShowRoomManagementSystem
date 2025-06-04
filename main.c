#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bptree.c"


int main() {
    Showroom showrooms[3];
    char* srfiles[3] = { "showroom1.txt", "showroom2.txt", "showroom3.txt" };
    char* spfiles[3] = { "Salesperson1.txt", "Salesperson2.txt", "Salesperson3.txt" };
    char* cfiles[3] = { "Customers1.txt", "Customers2.txt", "Customers3.txt" };
    

    for (int i = 0; i < 3; i++) {
        showrooms[i].showroom_id = i + 1;
        showrooms[i].available_stock = NULL;
        showrooms[i].sold_stock = NULL;
        showrooms[i].salespersons = NULL;

        load_showroom_data(&showrooms[i], srfiles[i]);
        load_salespersons(&showrooms[i], spfiles[i]);
        process_customer_purchases(&showrooms[i], cfiles[i]);
    }

    menu(showrooms);
    return 0;
}
