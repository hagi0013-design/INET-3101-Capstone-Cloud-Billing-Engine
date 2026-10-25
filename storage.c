
#include "storage.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h>

#define CUSTOMER_FILE "data/customers.txt"
#define LOCK_FILE "data/customers.lock"
#define TEMP_FILE "data/customers.tmp"
#define MAX_LINE 512
#define MAX_CUSTOMERS 500

static int lock_data(int mode)
{
    int fd = open(LOCK_FILE, O_CREAT | O_RDWR, 0600);

    if (fd < 0) {
        perror("Could not open lock file");
        return -1;
    }

    if (flock(fd, mode) != 0) {
        perror("Could not lock customer data");
        close(fd);
        return -1;
    }

    return fd;
}

static void unlock_data(int fd)
{
    if (fd >= 0) {
        flock(fd, LOCK_UN);
        close(fd);
    }
}

static int valid_field(const char *value)
{
    if (value == NULL || value[0] == '\0') {
        return 0;
    }

    if (strchr(value, '|') != NULL ||
        strchr(value, '\n') != NULL ||
        strchr(value, '\r') != NULL) {
        return 0;
    }

    return 1;
}

static int load_unlocked(Customer customers[], int capacity)
{
    FILE *file = fopen(CUSTOMER_FILE, "r");

    if (file == NULL) {
        return 0;
    }

    int count = 0;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), file) != NULL) {
        Customer customer;

        int fields = sscanf(line, "%d|%99[^|]|%99[^|]|%d",
                            &customer.id,
                            customer.name,
                            customer.email,
                            &customer.active);

        if (fields != 4 || customer.id <= 0 ||
            (customer.active != 0 && customer.active != 1)) {
            fprintf(stderr, "Skipping invalid customer record.\n");
            continue;
        }

        if (count >= capacity) {
            fclose(file);
            return -1;
        }

        customers[count++] = customer;
    }

    if (ferror(file)) {
        fclose(file);
        return -1;
    }

    fclose(file);
    return count;
}

int load_customers(Customer customers[], int capacity)
{
    if (customers == NULL || capacity <= 0) {
        return -1;
    }

    int lock_fd = lock_data(LOCK_SH);

    if (lock_fd < 0) {
        return -1;
    }

    int count = load_unlocked(customers, capacity);
    unlock_data(lock_fd);

    return count;
}

int save_customer(const Customer *customer)
{
    if (customer == NULL || customer->id <= 0 ||
        !valid_field(customer->name) ||
        !valid_field(customer->email) ||
        (customer->active != 0 && customer->active != 1)) {
        return -1;
    }

    int lock_fd = lock_data(LOCK_EX);

    if (lock_fd < 0) {
        return -1;
    }

    Customer customers[MAX_CUSTOMERS];
    int count = load_unlocked(customers, MAX_CUSTOMERS);

    if (count < 0 || count >= MAX_CUSTOMERS) {
        unlock_data(lock_fd);
        return -1;
    }

    for (int i = 0; i < count; i++) {
        if (customers[i].id == customer->id ||
            strcmp(customers[i].email, customer->email) == 0) {
            fprintf(stderr, "Customer ID or email already exists.\n");
            unlock_data(lock_fd);
            return -1;
        }
    }

    FILE *file = fopen(CUSTOMER_FILE, "a");

    if (file == NULL) {
        perror("Could not open customer file");
        unlock_data(lock_fd);
        return -1;
    }

    int result = fprintf(file, "%d|%s|%s|%d\n",
                         customer->id, customer->name,
                         customer->email, customer->active);

    if (result < 0 || fflush(file) != 0) {
        result = -1;
    } else {
        result = 0;
    }

    if (fclose(file) != 0) {
        result = -1;
    }

    unlock_data(lock_fd);
    return result;
}

static int write_customers(Customer customers[], int count)
{
    FILE *file = fopen(TEMP_FILE, "w");

    if (file == NULL) {
        perror("Could not create temporary file");
        return -1;
    }

    for (int i = 0; i < count; i++) {
        if (fprintf(file, "%d|%s|%s|%d\n",
                    customers[i].id, customers[i].name,
                    customers[i].email, customers[i].active) < 0) {
            fclose(file);
            unlink(TEMP_FILE);
            return -1;
        }
    }

    if (fclose(file) != 0) {
        unlink(TEMP_FILE);
        return -1;
    }

    if (rename(TEMP_FILE, CUSTOMER_FILE) != 0) {
        perror("Could not replace customer file");
        unlink(TEMP_FILE);
        return -1;
    }

    return 0;
}

int update_customer_record(int id, const char *name, const char *email)
{
    if (id <= 0 || !valid_field(name) || !valid_field(email) ||
        strlen(name) >= CUSTOMER_NAME_SIZE ||
        strlen(email) >= CUSTOMER_EMAIL_SIZE) {
        return -1;
    }

    int lock_fd = lock_data(LOCK_EX);

    if (lock_fd < 0) {
        return -1;
    }

    Customer customers[MAX_CUSTOMERS];
    int count = load_unlocked(customers, MAX_CUSTOMERS);

    if (count < 0) {
        unlock_data(lock_fd);
        return -1;
    }

    int index = -1;

    for (int i = 0; i < count; i++) {
        if (customers[i].id == id) {
            index = i;
        } else if (strcmp(customers[i].email, email) == 0) {
            fprintf(stderr, "That email is already in use.\n");
            unlock_data(lock_fd);
            return -1;
        }
    }

    if (index < 0) {
        fprintf(stderr, "Customer not found.\n");
        unlock_data(lock_fd);
        return -1;
    }

    snprintf(customers[index].name, sizeof(customers[index].name),
             "%s", name);
    snprintf(customers[index].email, sizeof(customers[index].email),
             "%s", email);

    int result = write_customers(customers, count);
    unlock_data(lock_fd);

    return result;
}

int delete_customer_record(int id)
{
    if (id <= 0) {
        return -1;
    }

    int lock_fd = lock_data(LOCK_EX);

    if (lock_fd < 0) {
        return -1;
    }

    Customer customers[MAX_CUSTOMERS];
    int count = load_unlocked(customers, MAX_CUSTOMERS);

    if (count < 0) {
        unlock_data(lock_fd);
        return -1;
    }

    int index = -1;

    for (int i = 0; i < count; i++) {
        if (customers[i].id == id) {
            index = i;
            break;
        }
    }

    if (index < 0) {
        fprintf(stderr, "Customer not found.\n");
        unlock_data(lock_fd);
        return -1;
    }

    for (int i = index; i < count - 1; i++) {
        customers[i] = customers[i + 1];
    }

    int result = write_customers(customers, count - 1);
    unlock_data(lock_fd);

    return result;
}
