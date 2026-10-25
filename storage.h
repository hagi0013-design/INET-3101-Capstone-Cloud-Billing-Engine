
#ifndef STORAGE_H
#define STORAGE_H

#include "customer.h"

int load_customers(Customer customers[], int capacity);
int save_customer(const Customer *customer);
int update_customer_record(int id, const char *name, const char *email);
int delete_customer_record(int id);

#endif

