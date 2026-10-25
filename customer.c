
#include "customer.h"
#include "storage.h"

#include <stdio.h>
#include <string.h>

int add_customer(const char *name, const char *email)
{
    Customer customers[500];
    int count = load_customers(customers, 500);

    if (name == NULL || email == NULL ||
        name[0] == '\0' || email[0] == '\0' ||
        strlen(name) >= CUSTOMER_NAME_SIZE ||
        strlen(email) >= CUSTOMER_EMAIL_SIZE) {
        fprintf(stderr, "Enter a valid name and email.\n");
        return -1;
    }

    if (count < 0 || count >= 500) {
        fprintf(stderr, "Could not load customers or customer limit reached.\n");
        return -1;
    }

    for (int i = 0; i < count; i++) {
        if (strcmp(customers[i].email, email) == 0) {
            fprintf(stderr, "That email is already registered.\n");
            return -1;
        }
    }

    Customer customer;
    customer.id = 1;

    for (int i = 0; i < count; i++) {
        if (customers[i].id >= customer.id) {
            customer.id = customers[i].id + 1;
        }
    }

    snprintf(customer.name, sizeof(customer.name), "%s", name);
    snprintf(customer.email, sizeof(customer.email), "%s", email);
    customer.active = 1;

    if (save_customer(&customer) != 0) {
        fprintf(stderr, "Could not save customer.\n");
        return -1;
    }

    printf("Customer added. ID: %d\n", customer.id);
    return customer.id;
}

int list_customers(void)
{
    Customer customers[500];
    int count = load_customers(customers, 500);

    if (count < 0) {
        fprintf(stderr, "Could not load customers.\n");
        return -1;
    }

    if (count == 0) {
        printf("No customers found.\n");
        return 0;
    }

    for (int i = 0; i < count; i++) {
        printf("ID: %d | Name: %s | Email: %s | Status: %s\n",
               customers[i].id,
               customers[i].name,
               customers[i].email,
               customers[i].active ? "Active" : "Inactive");
    }

    return count;
}

int find_customer(int id, Customer *result)
{
    Customer customers[500];
    int count = load_customers(customers, 500);

    if (id <= 0 || result == NULL || count < 0) {
        return -1;
    }

    for (int i = 0; i < count; i++) {
        if (customers[i].id == id) {
            *result = customers[i];
            return 0;
        }
    }

    return -1;
}

int update_customer(int id, const char *name, const char *email)
{
    return update_customer_record(id, name, email);
}

int delete_customer(int id)
{
    return delete_customer_record(id);
}

