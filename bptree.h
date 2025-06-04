#ifndef SHOWROOM_H
#define SHOWROOM_H

#define MAX 4   // B+ Tree Order
#define NAME_LEN 50
#define ADDRESS_LEN 100

//  Car Structure 
typedef struct Car {
    int vin;
    char name[NAME_LEN];
    char color[NAME_LEN];
    float price;
    char fuel[NAME_LEN];
    char type[NAME_LEN]; // Hatchback, Sedan, SUV
    // Customer Details (added after sale)
    char cust_name[NAME_LEN];
    char cust_mobile[NAME_LEN];
    char cust_address[ADDRESS_LEN];
    char reg_no[NAME_LEN]; // Car registration number
    int d_o_prchse;
    char payment_method[NAME_LEN]; // Cash or Loan
    int payment_code;
} Car;

//  B+ Tree Node
typedef struct BPTreeNode {
    int is_leaf;
    int num_keys;
    int keys[MAX + 1];
    void* ptr[MAX + 2]; // can be Car*, Salesperson*, or node
    struct BPTreeNode* parent;
    struct BPTreeNode* next; // Used in leaf nodes
} BPTreeNode;

//  Salesperson Structure 
typedef struct Salesperson {
    int id;
    char name[NAME_LEN];
    float target;
    float achieved;
    float commission;

    BPTreeNode* soldCarsRoot; // B+ tree of sold cars (Car nodes)
} Salesperson;

//  Showroom Structure
typedef struct Showroom {
    int showroom_id;
    BPTreeNode* available_stock;   // VIN-based car tree
    BPTreeNode* sold_stock;        // VIN-based sold cars tree
    BPTreeNode* salespersons;      // Salesperson tree (ID-based)
} Showroom;

// Function Declarations 
BPTreeNode* create_bptree();
void bptree_insert(BPTreeNode** root, int key, void* data);
void* bptree_search(BPTreeNode* root, int key);
void bptree_delete(BPTreeNode** root, int key);
void bptree_traverse(BPTreeNode* root, int is_car);

void load_showroom_data(Showroom* s, const char* filename);
void load_salespersons(Showroom* s, const char* filename);
void load_customers(Showroom* s, const char* filename);

void sell_car(Showroom* showroom, int vin, Salesperson* sp, Car* customer_details);
Salesperson* get_salesperson(BPTreeNode* root, int id);
void display_car(Car* c);
void display_salesperson(Salesperson* s);

#endif
