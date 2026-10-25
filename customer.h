
#ifndef CUSTOMER_H
#define CUSTOMER_H

#define CUSTOMER_NAME_SIZE 100
#define CUSTOMER_EMAIL_SIZE 100

typedef struct {
    int id;
    char name[CUSTOMER_NAME_SIZE];
    char email[CUSTOMER_EMAIL_SIZE];
    int active;
} Customer;

int add_customer(const char *name, const char *email);
int list_customers(void);
int find_customer(int id, Customer *result);
int update_customer(int id, const char *name, const char *email);
int delete_customer(int id);

#endif

