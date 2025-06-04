#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bptree.h"

BPTreeNode* create_bptree() {
    BPTreeNode* node = (BPTreeNode*)malloc(sizeof(BPTreeNode));
    node->is_leaf = 1;
    node->num_keys = 0;
    for (int i = 0; i < MAX + 2; i++) node->ptr[i] = NULL;
    node->next = NULL;
    return node;
}

BPTreeNode* create_node(int is_leaf) {
    BPTreeNode* node = (BPTreeNode*)malloc(sizeof(BPTreeNode));
    if (!node) {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    node->is_leaf = is_leaf;
    node->num_keys = 0;
    node->parent = NULL;

    for (int i = 0; i < MAX + 1; i++)
        node->ptr[i] = NULL;

    if (is_leaf) {
        node->next = NULL;  // For leaf node chaining
    } else {
        node->next = NULL;  // Not needed for internal nodes but kept for uniformity
    }

    return node;
}

void bptree_insert(BPTreeNode** root, int key, void* data) {
    if (!(*root)) {
        *root = create_node(1);
        (*root)->keys[0] = key;
        (*root)->ptr[0] = data;
        (*root)->num_keys = 1;
        return;
    }

    BPTreeNode* node = *root;
    BPTreeNode* parent_stack[10];  // Up to 10 levels deep
    int index_stack[10];
    int height = 0;

    // Traverse to the correct leaf
    while (!node->is_leaf) {
        parent_stack[height] = node;
        int i;
        for (i = 0; i < node->num_keys && key >= node->keys[i]; i++);
        index_stack[height++] = i;
        node = (BPTreeNode*)node->ptr[i];
    }

    // Insert into leaf node
    int i;
    for (i = node->num_keys - 1; i >= 0 && node->keys[i] > key; i--) {
        node->keys[i + 1] = node->keys[i];
        node->ptr[i + 1] = node->ptr[i];
    }
    node->keys[i + 1] = key;
    node->ptr[i + 1] = data;
    node->num_keys++;

    // If no overflow, done
    if (node->num_keys <= MAX) return;

    // Leaf split
    BPTreeNode* new_leaf = create_node(1);
    int mid = (MAX + 1) / 2;

    new_leaf->num_keys = node->num_keys - mid;
    node->num_keys = mid;

    for (i = 0; i < new_leaf->num_keys; i++) {
        new_leaf->keys[i] = node->keys[mid + i];
        new_leaf->ptr[i] = node->ptr[mid + i];
    }

    // Fix leaf next pointer
    new_leaf->next = node->next;
    node->next = new_leaf;

    int up_key = new_leaf->keys[0];

    // Now propagate up
    while (height > 0) {
        node = parent_stack[--height];
        int pos = index_stack[height];

        for (i = node->num_keys; i > pos; i--) {
            node->keys[i] = node->keys[i - 1];
            node->ptr[i + 1] = node->ptr[i];
        }

        node->keys[pos] = up_key;
        node->ptr[pos + 1] = new_leaf;
        node->num_keys++;

        if (node->num_keys <= MAX) return;

        // Internal node split
        BPTreeNode* new_internal = create_node(0);
        mid = (MAX + 1) / 2;

        up_key = node->keys[mid];

        new_internal->num_keys = node->num_keys - mid - 1;
        for (i = 0; i < new_internal->num_keys; i++) {
            new_internal->keys[i] = node->keys[mid + 1 + i];
            new_internal->ptr[i] = node->ptr[mid + 1 + i];
        }
        new_internal->ptr[i] = node->ptr[node->num_keys];

        node->num_keys = mid;
        new_leaf = new_internal;
    }

    // New root creation
    BPTreeNode* new_root = create_node(0);
    new_root->keys[0] = up_key;
    new_root->ptr[0] = *root;
    new_root->ptr[1] = new_leaf;
    new_root->num_keys = 1;
    *root = new_root;
}

void* bptree_search(BPTreeNode* root, int key) {
    if (root == NULL) return NULL;
    BPTreeNode* node = root;

    while (!node->is_leaf) {
        int i;
        for (i = 0; i < node->num_keys && key >= node->keys[i]; i++);
        node = (BPTreeNode*)node->ptr[i];
    }

    for (int i = 0; i < node->num_keys; i++) {
        if (node->keys[i] == key) return node->ptr[i];
    }
    return NULL;
}

void bptree_traverse(BPTreeNode* root, int is_car) {
    if (!root) return;
    BPTreeNode* node = root;
    while (!node->is_leaf) node = (BPTreeNode*)node->ptr[0];

    while (node) {
        for (int i = 0; i < node->num_keys; i++) {
            if (is_car)
                display_car((Car*)node->ptr[i]);
            else
                display_salesperson((Salesperson*)node->ptr[i]);
        }
        node = node->next;
    }
}

void display_car(Car* c) {
    
    int current_day = c->d_o_prchse / 1000000;
    int current_month = (c->d_o_prchse / 10000) % 100;
    int current_year = c->d_o_prchse % 10000;
    printf("    VIN: %d | Name: %s | Color: %s | Price: %.2f | Fuel: %s | Type: %s\n", c->vin, c->name, c->color, c->price, c->fuel, c->type);
    if (strcmp(c->cust_name, "N/A") != 0) {
        printf("        Sold To: %s | Mobile: %s | Address: %s | Reg No: %s | Payment: %s | Payment date: %d-%d-%d\n\n",
               c->cust_name, c->cust_mobile, c->cust_address, c->reg_no, c->payment_method, current_day, current_month, current_year);
    }
}

void display_salesperson(Salesperson* s) {
    printf("Salesperson ID: %d | Name: %s | Target: %.2f | Achieved: %.2f | Commission: %.2f\n",
           s->id, s->name, s->target, s->achieved, s->commission);
    printf("  Cars Sold:\n");
    bptree_traverse(s->soldCarsRoot, 1);
}

Salesperson* get_salesperson(BPTreeNode* root, int id) {
    return (Salesperson*)bptree_search(root, id);
}

void sell_car(Showroom* showroom, int vin, Salesperson* sp, Car* cust) {
    Car* c = (Car*)bptree_search(showroom->available_stock, vin);
    if (!c) {
        printf("Car with VIN %d not found in showroom %d\n", vin, showroom->showroom_id);
        return;
    }

    strcpy(c->cust_name, cust->cust_name);
    strcpy(c->cust_mobile, cust->cust_mobile);
    strcpy(c->cust_address, cust->cust_address);
    strcpy(c->reg_no, cust->reg_no);
    c->d_o_prchse = cust->d_o_prchse;
    strcpy(c->payment_method, cust->payment_method);
    c->payment_code = cust->payment_code;

    bptree_delete(&(showroom->available_stock), vin);
    bptree_insert(&(showroom->sold_stock), vin, c);
    printf("Inserted car with VIN: %d\n", c->vin);
    bptree_insert(&(sp->soldCarsRoot), vin, c);
    printf("Inserted car with VIN: %d\n", c->vin);

    sp->achieved += c->price / 100000.0f;
    sp->commission = 0.02 * sp->achieved;
}

void bptree_delete(BPTreeNode** root, int key) {
    if (!(*root)) return;

    BPTreeNode* node = *root;
    BPTreeNode* parent = NULL;
    int parent_index = -1;

    // Traverse to the leaf node
    while (!node->is_leaf) {
        parent = node;
        int i;
        for (i = 0; i < node->num_keys && key >= node->keys[i]; i++);
        parent_index = i;
        node = (BPTreeNode*)node->ptr[i];
    }

    // Find the key in the leaf
    int found = 0, i;
    for (i = 0; i < node->num_keys; i++) {
        if (node->keys[i] == key) {
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Key %d not found.\n", key);
        return;
    }

    // Remove the key and data
    for (int j = i; j < node->num_keys - 1; j++) {
        node->keys[j] = node->keys[j + 1];
        node->ptr[j] = node->ptr[j + 1];
    }
    node->num_keys--;

    // Root with no keys left
    if (node == *root && node->num_keys == 0) {
        free(node);
        *root = NULL;
        return;
    }

    // If enough keys remain or it's root, nothing else needed
    if (node->num_keys >= (MAX + 1) / 2 || node == *root) return;

    // Handle underflow: borrow or merge
    BPTreeNode* left_sibling = NULL;
    BPTreeNode* right_sibling = NULL;
    int left_index = parent_index - 1;
    int right_index = parent_index + 1;

    if (parent_index > 0)
        left_sibling = (BPTreeNode*)parent->ptr[left_index];
    if (parent_index < parent->num_keys)
        right_sibling = (BPTreeNode*)parent->ptr[right_index];

    // Try to borrow from left
    if (left_sibling && left_sibling->num_keys > (MAX + 1) / 2) {
        // Shift node to right
        for (int j = node->num_keys; j > 0; j--) {
            node->keys[j] = node->keys[j - 1];
            node->ptr[j] = node->ptr[j - 1];
        }
        node->keys[0] = left_sibling->keys[left_sibling->num_keys - 1];
        node->ptr[0] = left_sibling->ptr[left_sibling->num_keys - 1];
        left_sibling->num_keys--;
        node->num_keys++;

        parent->keys[parent_index - 1] = node->keys[0];
    }
    // Try to borrow from right
    else if (right_sibling && right_sibling->num_keys > (MAX + 1) / 2) {
        node->keys[node->num_keys] = right_sibling->keys[0];
        node->ptr[node->num_keys] = right_sibling->ptr[0];
        node->num_keys++;

        // Shift right sibling
        for (int j = 0; j < right_sibling->num_keys - 1; j++) {
            right_sibling->keys[j] = right_sibling->keys[j + 1];
            right_sibling->ptr[j] = right_sibling->ptr[j + 1];
        }
        right_sibling->num_keys--;

        parent->keys[parent_index] = right_sibling->keys[0];
    }
    // Merge with left or right
    else if (left_sibling) {
        int idx = left_sibling->num_keys;
        for (int j = 0; j < node->num_keys; j++) {
            left_sibling->keys[idx + j] = node->keys[j];
            left_sibling->ptr[idx + j] = node->ptr[j];
        }
        left_sibling->num_keys += node->num_keys;
        left_sibling->next = node->next;
        free(node);

        // Remove key from parent
        for (int j = parent_index - 1; j < parent->num_keys - 1; j++) {
            parent->keys[j] = parent->keys[j + 1];
            parent->ptr[j + 1] = parent->ptr[j + 2];
        }
        parent->num_keys--;

        // If parent becomes empty and is root
        if (parent == *root && parent->num_keys == 0) {
            *root = left_sibling;
            free(parent);
        }
    } else if (right_sibling) {
        int idx = node->num_keys;
        for (int j = 0; j < right_sibling->num_keys; j++) {
            node->keys[idx + j] = right_sibling->keys[j];
            node->ptr[idx + j] = right_sibling->ptr[j];
        }
        node->num_keys += right_sibling->num_keys;
        node->next = right_sibling->next;
        free(right_sibling);

        for (int j = parent_index; j < parent->num_keys - 1; j++) {
            parent->keys[j] = parent->keys[j + 1];
            parent->ptr[j + 1] = parent->ptr[j + 2];
        }
        parent->num_keys--;

        if (parent == *root && parent->num_keys == 0) {
            *root = node;
            free(parent);
        }
    }
}

void load_showroom_data(Showroom* showroom, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return;

    while (!feof(f)) {
        Car* car = (Car*)malloc(sizeof(Car));
        if (fscanf(f, "%d %s %s %s %s %f", &car->vin, car->name, car->color, car->fuel, car->type, &car->price) != 6)
            break;
        strcpy(car->cust_name, "N/A");
        strcpy(car->cust_mobile, "N/A");
        strcpy(car->cust_address, "N/A");
        strcpy(car->reg_no, "N/A");
        car->d_o_prchse = 0;
        car->payment_code = 0;
        strcpy(car->payment_method, "N/A");

        bptree_insert(&showroom->available_stock, car->vin, car);
    }

    fclose(f);
}

void load_salespersons(Showroom* showroom, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return;

    while (!feof(f)) {
        int srid;
        Salesperson* s = (Salesperson*)malloc(sizeof(Salesperson));
        if (fscanf(f, "%d %s %f %f %f", &s->id, s->name, &s->target, &s->achieved, &s->commission) != 5)
            break;

        s->soldCarsRoot = NULL;

        printf("Inserted salesperson car with id: %d\n", s->id);
        bptree_insert(&showroom->salespersons, s->id, s);
    }

    fclose(f);
}

void process_customer_purchases(Showroom* showroom, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return;

    while (!feof(f)) {
        int spid, vin;
        Car* c = (Car*)malloc(sizeof(Car));
        if (fscanf(f, "%d %s %s %s %d %s %d %s %d", &spid, c->cust_name, c->cust_mobile, c->cust_address, &vin, c->reg_no, &c->d_o_prchse, c->payment_method, &c->payment_code) != 9)
            break;

        // Convert d_o_prchse to correct format
        int day = c->d_o_prchse / 1000000;
        int month = (c->d_o_prchse / 10000) % 100;
        int year = c->d_o_prchse % 10000;
        c->d_o_prchse = day * 1000000 + month * 10000 + year;

        Salesperson* sp = get_salesperson(showroom->salespersons, spid);
        if (sp){
            printf("Getting customers too :) ");
            sell_car(showroom, vin, sp, c);
        }
        
    }

    fclose(f);
}

void add_new_salesperson(Showroom* showroom) {
    Salesperson* s = (Salesperson*)malloc(sizeof(Salesperson));
    printf("Enter Salesperson ID: ");
    scanf("%d", &s->id);
    printf("Enter Name: ");
    scanf("%s", s->name);
    s->target = 50.0;
    s->achieved = 0.0;
    s->commission = 0.0;
    s->soldCarsRoot = NULL;
    bptree_insert(&showroom->salespersons, s->id, s);
    printf("Inserted car with VIN: %d\n", s->id);
    printf("Salesperson added.\n");
}

void add_new_customer(Showroom* showroom) {
    Car* cust = (Car*)malloc(sizeof(Car));
    int spid, vin;
    printf("Enter Customer Name, Mobile, Address:\n");
    scanf("%s %s %s", cust->cust_name, cust->cust_mobile, cust->cust_address);
    printf("Enter VIN of car, Registration No.,date of purchase, Payment Method and payment code(0 for Cash, 1 for 9.00% rate of interest for 84 months EMI,/n 2 for 8.75% rate of interest for 60 months EMI,/n 3 for 8.50% rate of interest for 36 months EMI.):\n");
    scanf("%d %s %d %s %d", &vin, cust->reg_no,&cust->d_o_prchse, cust->payment_method,&cust->payment_code);
    printf("Enter Salesperson ID:\n");
    scanf("%d", &spid);

    Salesperson* sp = get_salesperson(showroom->salespersons, spid);
    if (!sp) {
        printf("Salesperson not found.\n");
        return;
    }

    sell_car(showroom, vin, sp, cust);
}

void find_most_popular_car(Showroom* showrooms) {
    char most_popular_car[100] = "";
    int max_sales = 0;

    // Iterate through each showroom
    for (int i = 0; i < 3; i++) {
        // Iterate through the sold stock of the current showroom
        BPTreeNode* node = showrooms[i].sold_stock;
        while (node) {
            // Iterate through each car in the current node
            for (int j = 0; j < node->num_keys; j++) {
                // Get the car's brand and sales data
                Car* car = (Car*)node->ptr[j];
                char car_name[100];
                strcpy(car_name, car->name);

                // Check if the car's brand has higher sales than the current max sales
                int car_sales = 0;
                BPTreeNode* temp_node = showrooms[i].sold_stock;
                while (temp_node) {
                    for (int k = 0; k < temp_node->num_keys; k++) {
                        Car* temp_car = (Car*)temp_node->ptr[k];
                        char temp_car_name[100];
                        strcpy(temp_car_name, temp_car->name);
                        if (strcmp(temp_car_name, car_name) == 0) {
                            car_sales += temp_car->price;
                        }
                    }
                    temp_node = temp_node->next;
                }

                if (car_sales > max_sales) {
                    max_sales = car_sales;
                    strcpy(most_popular_car, car_name);
                }
            }
            node = node->next;
        }
    }

    // Print the most popular car's details
    printf("Most popular car: %s\n", most_popular_car);
    for (int i = 0; i < 3; i++) {
        BPTreeNode* node = showrooms[i].sold_stock;
        while (node) {
            for (int j = 0; j < node->num_keys; j++) {
                Car* car = (Car*)node->ptr[j];
                char car_name[100];
                strcpy(car_name, car->name);
                if (strcmp(car_name, most_popular_car) == 0) {
                    display_car(car);
                }
            }
            node = node->next;
        }
    }
}

void find_most_successful_sales_person(Showroom* showrooms) {
    int max_sales = 0;
    Salesperson* most_successful_sales_person = NULL;

    // Iterate through each showroom
    for (int i = 0; i < 3; i++) {
        // Iterate through the sales persons of the current showroom
        BPTreeNode* node = showrooms[i].salespersons;
        while (node) {
            // Get the sales person's sales data
            Salesperson* sales_person = (Salesperson*)node->ptr[0];

            // Check if the sales person's sales are higher than the current max sales
            if (sales_person->achieved > max_sales) {
                max_sales = sales_person->achieved;
                most_successful_sales_person = sales_person;
            }

            node = node->next;
        }
    }

    // Print the most successful sales person's details
    if (most_successful_sales_person != NULL) {
        printf("Most successful sales person: %s\n", most_successful_sales_person->name);
        printf("Sales achieved: %.2f\n", most_successful_sales_person->achieved);
        printf("Commission: %.2f\n", most_successful_sales_person->commission);

        // Award 1% extra incentives
        float extra_incentives = most_successful_sales_person->achieved * 0.01;
        most_successful_sales_person->commission += extra_incentives;
        printf("Extra incentives: %.2f\n", extra_incentives);
        printf("New commission: %.2f\n", most_successful_sales_person->commission);
    } else {
        printf("No sales person found.\n");
    }
}

void display_car_info(Showroom* showrooms, int vin) {
    // Read the showrooms text files
    char* srfiles[3] = { "showroom1.txt", "showroom2.txt", "showroom3.txt" };
    for (int i = 0; i < 3; i++) {
        FILE* file = fopen(srfiles[i], "r");
        if (file == NULL) {
            printf("Error opening file!\n");
            return;
        }

        // Read the file line by line
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            // Parse the line
            int vin_num;
            char name[256];
            char color[256];
            char fuel[256];
            char type[256];
            float price;
            sscanf(line, "%d %s %s %s %s %f", &vin_num, name, color, fuel, type, &price);

            // Check if the VIN number matches
            if (vin_num == vin) {
                // Print the car information
                printf("VIN: %d\n", vin_num);
                printf("Name: %s\n", name);
                printf("Color: %s\n", color);
                printf("Fuel: %s\n", fuel);
                printf("Type: %s\n", type);
                printf("Price: %.2f\n", price);
                return;
            }
        }

        // Close the file
        fclose(file);
    }

    // If the car is not found, print a message
    printf("Car with VIN %d not found.\n", vin);
}

void search_sales_person_by_sales_range(Showroom* showrooms, float min_sales, float max_sales) {
    // Iterate through each showroom
    for (int i = 0; i < 3; i++) {
        // Iterate through the sales persons of the current showroom
        BPTreeNode* node = showrooms[i].salespersons;
        while (node) {
            // Iterate through each sales person in the current node
            for (int j = 0; j < node->num_keys; j++) {
                // Get the sales person's sales data
                Salesperson* sales_person = (Salesperson*)node->ptr[j];

                // Check if the sales person's sales fall within the specified range
                if (sales_person->achieved >= min_sales && sales_person->achieved <= max_sales) {
                    printf("Showroom: %d | Salesperson ID: %d | Name: %s | Target: %.2f | Achieved: %.2f | Commission: %.2f\n",
                           i+1,sales_person->id, sales_person->name, sales_person->target, sales_person->achieved, sales_person->commission);
                }
            }
            node = node->next;
        }
    }
}

void merge_and_sort_database(Showroom* showrooms) {
    FILE* merge_file = fopen("merge.txt", "w");
    if (merge_file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    for (int i = 0; i < 3; i++) {
        BPTreeNode* node = showrooms[i].available_stock;

        // Move to the leftmost leaf node
        while (node != NULL && !node->is_leaf) {
            node = (BPTreeNode*)node->ptr[0];
        }

        // Now traverse all leaf nodes using linked list
        while (node != NULL) {
            for (int j = 0; j < node->num_keys; j++) {
                Car* car = (Car*)node->ptr[j];
                fprintf(merge_file, "%d %s %s %s %s %.2f\n", car->vin, car->name, car->color, car->fuel, car->type, car->price);
            }
            node = node->next;
        }
    }

    fclose(merge_file);
}


void predict_next_month_sales_helper(BPTreeNode* node, int previous_month, int current_month, int current_year, int* previous_month_sales, int* current_month_sales) {
    if (node->is_leaf) {
        for (int j = 0; j < node->num_keys; j++) {
            Car* car = (Car*)node->ptr[j];
            int purchase_day = car->d_o_prchse / 1000000;
            int purchase_month = (car->d_o_prchse / 10000) % 100;
            int purchase_year = car->d_o_prchse % 10000;

            // Check if the purchase month is the previous month
            if (purchase_month == previous_month && purchase_year == current_year) {
                (*previous_month_sales)++;
            }

            // Check if the purchase month is the current month
            if (purchase_month == current_month && purchase_year == current_year) {
                (*current_month_sales)++;
            }
        }
    } else {
        // Traverse the child nodes
        for (int j = 0; j <= node->num_keys; j++) {
            BPTreeNode* child_node = (BPTreeNode*)node->ptr[j];
            if (child_node != NULL) {
                predict_next_month_sales_helper(child_node, previous_month, current_month, current_year, previous_month_sales, current_month_sales);
            }
        }
    }
}


void predict_next_month_sales(Showroom* showrooms, int today_date) {
    int current_day = today_date / 1000000;
    int current_month = (today_date / 10000) % 100;
    int current_year = today_date % 10000;

    int previous_month = current_month - 1;
    if (previous_month == 0) {
        previous_month = 12;
        current_year--;
    }

    int previous_month_sales = 0;
    int current_month_sales = 0;

    // Traverse all three showrooms
    for (int i = 0; i < 3; i++) {
        // Traverse the sold cars tree
        BPTreeNode* sold_cars_node = showrooms[i].sold_stock;
        while (sold_cars_node != NULL) {
            // Traverse the leaf nodes
            if (sold_cars_node->is_leaf) {
                for (int j = 0; j < sold_cars_node->num_keys; j++) {
                    Car* car = (Car*)sold_cars_node->ptr[j];
                    int purchase_day = car->d_o_prchse / 1000000;
                    int purchase_month = (car->d_o_prchse / 10000) % 100;
                    int purchase_year = car->d_o_prchse % 10000;

                    // Check if the purchase month is the previous month
                    if (purchase_month == previous_month && purchase_year == current_year) {
                        previous_month_sales++;
                    }

                    // Check if the purchase month is the current month
                    if (purchase_month == current_month && purchase_year == current_year) {
                        current_month_sales++;
                    }
                }
            } else {
                // Traverse the child nodes
                for (int j = 0; j <= sold_cars_node->num_keys; j++) {
                    BPTreeNode* child_node = (BPTreeNode*)sold_cars_node->ptr[j];
                    if (child_node != NULL) {
                        predict_next_month_sales_helper(child_node, previous_month, current_month, current_year, &previous_month_sales, &current_month_sales);
                    }
                }
            }

            sold_cars_node = sold_cars_node->next;
        }
    }

    // Predict the next month's sales using a simple average algorithm
    float next_month_sales = (previous_month_sales + current_month_sales) / 2;

    printf("Predicted sales for next month: %.2f\n", next_month_sales);
}

void print_customers_with_36_months_emi_loan(Showroom* showrooms) {
    // Traverse all three showrooms
    for (int i = 0; i < 3; i++) {
        BPTreeNode* sold_cars_node = showrooms[i].sold_stock;
        while (sold_cars_node != NULL) {
            if (sold_cars_node->is_leaf) {
                for (int j = 0; j < sold_cars_node->num_keys; j++) {
                    Car* car = (Car*)sold_cars_node->ptr[j];
                    if (car->payment_code == 4) {
                        printf("Customer Name: %s\n", car->cust_name);
                        printf("Customer Mobile: %s\n", car->cust_mobile);
                        printf("Customer Address: %s\n", car->cust_address);
                        printf("Registration Number: %s\n", car->reg_no);
                        printf("Payment Method: %s\n", car->payment_method);
                        printf("Payment Code: %d\n", car->payment_code);
                        printf("\n");
                    }
                }
            } else {
                for (int j = 0; j <= sold_cars_node->num_keys; j++) {
                    BPTreeNode* child_node = (BPTreeNode*)sold_cars_node->ptr[j];
                    if (child_node != NULL) {
                        if (child_node->is_leaf) {
                            for (int k = 0; k < child_node->num_keys; k++) {
                                Car* car = (Car*)child_node->ptr[k];
                                if (car->payment_code == 4) {
                                    printf("Customer Name: %s\n", car->cust_name);
                                    printf("Customer Mobile: %s\n", car->cust_mobile);
                                    printf("Customer Address: %s\n", car->cust_address);
                                    printf("Registration Number: %s\n", car->reg_no);
                                    printf("Payment Method: %s\n", car->payment_method);
                                    printf("Payment Code: %d\n", car->payment_code);
                                    printf("\n");
                                }
                            }
                        } else {
                            for (int k = 0; k <= child_node->num_keys; k++) {
                                BPTreeNode* grandchild_node = (BPTreeNode*)child_node->ptr[k];
                                if (grandchild_node != NULL) {
                                    for (int l = 0; l < grandchild_node->num_keys; l++) {
                                        Car* car = (Car*)grandchild_node->ptr[l];
                                        if (car->payment_code == 4) {
                                            printf("Customer Name: %s\n", car->cust_name);
                                            printf("Customer Mobile: %s\n", car->cust_mobile);
                                            printf("Customer Address: %s\n", car->cust_address);
                                            printf("Registration Number: %s\n", car->reg_no);
                                            printf("Payment Method: %s\n", car->payment_method);
                                            printf("Payment Code: %d\n", car->payment_code);
                                            printf("\n");
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            sold_cars_node = sold_cars_node->next;
        }
    }
}


void menu(Showroom* showrooms) {
    int opt;
    while (1) {
        printf("\nMenu:\n");
        printf("1. View Showroom Available Stock\n");
        printf("2. View Sold Cars\n");
        printf("3. View Salespersons\n");
        printf("4. Add New Salesperson\n");
        printf("5. Add New Customer (Buy Car)\n");
        printf("6. Merge the data of all showrooms.\n");
        printf("7. View the most popular car among all the showrooms.\n");
        printf("8. Predict the most successful sales person among all the showrooms.\n");
        printf("9. Predict the next month's sales.\n");
        printf("10. View the information of a car based on VIN.\n");
        printf("11. View the sales persons who has achieved the sales target within a range of vales.\n");
        printf("12. View list of customers having EMI plan for less than 48 month but greater than 36 months.\n");
        printf("13. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &opt);

        int id;
        int today_date;
        int vin;
        int mins,maxs;
        switch (opt) {
            case 1:
                printf("Enter Showroom ID (1-3): ");
                scanf("%d", &id);
                bptree_traverse(showrooms[id - 1].available_stock, 1);
                break;
            case 2:
                printf("Enter Showroom ID (1-3): ");
                scanf("%d", &id);
                bptree_traverse(showrooms[id - 1].sold_stock, 1);
                break;
            case 3:
                printf("Enter Showroom ID (1-3): ");
                scanf("%d", &id);
                bptree_traverse(showrooms[id - 1].salespersons, 0);
                break;
            case 4:
                printf("Enter Showroom ID (1-3): ");
                scanf("%d", &id);
                add_new_salesperson(&showrooms[id - 1]);
                break;
            case 5:
                printf("Enter Showroom ID (1-3): ");
                scanf("%d", &id);
                add_new_customer(&showrooms[id - 1]);
                break;
            case 6:
                merge_and_sort_database(showrooms);
                break;
            case 7:
                find_most_popular_car(showrooms);
                break;
            case 8:
                find_most_successful_sales_person(showrooms); 
                break;  
            case 9:
                printf("Enter the date to predict the slaes for next month: ");
                scanf("%d", &today_date);
                predict_next_month_sales(showrooms, today_date);
                break;
            case 10:
                printf("Enter the VIN of a car to view its details: ");
                scanf("%d", &vin);
                display_car_info(showrooms, vin);
                break;
            case 11:
                printf("Enter the range of the sales person's target to view: ");
                scanf("%d %d",&mins,&maxs);
                search_sales_person_by_sales_range(showrooms, mins, maxs);
                break;
            case 12:
                print_customers_with_36_months_emi_loan(showrooms);
                break;
            case 13:
                printf("Exiting the car Showroom Management 2.");
                return;
            default:
                printf("Invalid option.\n");
                break;
        }
    }
}
