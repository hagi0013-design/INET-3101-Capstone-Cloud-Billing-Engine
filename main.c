

#include <stdio.h>
#include <string.h>
#include "customer.h"
#include "billing.h"

int run_server(void);
int run_client(void);

static int read_line(const char *prompt, char *buffer, size_t size)
{
    printf("%s", prompt);

    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }

    size_t length = strlen(buffer);

    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
    }

    return 1;
}

static int read_int(const char *prompt, int *value)
{
    char line[64];
    char extra;

    if (!read_line(prompt, line, sizeof(line))) {
        return 0;
    }

    if (sscanf(line, "%d %c", value, &extra) != 1) {
        printf("Please enter a valid whole number.\n");
        return -1;
    }

    return 1;
}

static int read_double(const char *prompt, double *value)
{
    char line[64];
    char extra;

    if (!read_line(prompt, line, sizeof(line))) {
        return 0;
    }

    if (sscanf(line, "%lf %c", value, &extra) != 1 ||
        *value < 0) {
        printf("Please enter a valid non-negative number.\n");
        return -1;
    }

    return 1;
}

static void show_customer(const Customer *customer)
{
    printf("ID: %d | Name: %s | Email: %s | Status: %s\n",
           customer->id, customer->name, customer->email,
           customer->active ? "Active" : "Inactive");
}

int main(void)
{
    int choice;

    while (1) {
        printf("\n=== Cloud Services Billing Engine ===\n");
        printf("1. Start server\n");
        printf("2. Run client\n");
        printf("3. Add customer\n");
        printf("4. List customers\n");
        printf("5. Find customer\n");
        printf("6. Update customer\n");
        printf("7. Delete customer\n");
        printf("8. Record usage\n");
        printf("9. Generate invoice\n");
        printf("10. Exit\n");

        int status = read_int("Choose an option: ", &choice);

        if (status == 0) {
            break;
        }
        if (status < 0) {
            continue;
        }

        char name[CUSTOMER_NAME_SIZE];
        char email[CUSTOMER_EMAIL_SIZE];
        int id;
        Customer customer;

        switch (choice) {
            case 1:
                run_server();
                break;

            case 2:
                run_client();
                break;

            case 3:
                if (read_line("Customer name: ", name, sizeof(name)) &&
                    read_line("Customer email: ", email, sizeof(email))) {
                    add_customer(name, email);
                }
                break;

            case 4:
                list_customers();
                break;

            case 5:
                if (read_int("Customer ID: ", &id) == 1) {
                    if (find_customer(id, &customer) == 0) {
                        show_customer(&customer);
                    } else {
                        printf("Customer not found.\n");
                    }
                }
                break;

            case 6:
                if (read_int("Customer ID to update: ", &id) != 1) {
                    break;
                }
                if (find_customer(id, &customer) != 0) {
                    printf("Customer not found.\n");
                    break;
                }
                if (read_line("New name: ", name, sizeof(name)) &&
                    read_line("New email: ", email, sizeof(email))) {
                    if (update_customer(id, name, email) == 0) {
                        printf("Customer updated.\n");
                    } else {
                        printf("Could not update customer.\n");
                    }
                }
                break;

            case 7:
                if (read_int("Customer ID to delete: ", &id) == 1) {
                    if (delete_customer(id) == 0) {
                        printf("Customer deleted.\n");
                    } else {
                        printf("Could not delete customer.\n");
                    }
                }
                break;

            case 8: {
                UsageRecord usage = {0};

                if (read_int("Customer ID: ", &usage.customer_id) != 1 ||
                    read_double("CPU hours: ", &usage.cpu_hours) != 1 ||
                    read_double("RAM GB-hours: ", &usage.ram_gb_hours) != 1 ||
                    read_double("Storage GB: ", &usage.storage_gb) != 1 ||
                    read_double("Inbound MB: ", &usage.inbound_mb) != 1 ||
                    read_double("Outbound MB: ", &usage.outbound_mb) != 1) {
                    printf("Usage entry cancelled or invalid.\n");
                    break;
                }

                if (find_customer(usage.customer_id, &customer) != 0) {
                    printf("Customer not found. Usage was not saved.\n");
                    break;
                }

                record_usage(&usage);
                break;
            }

            case 9: {
                char format[16];

                if (read_int("Customer ID: ", &id) != 1) {
                    break;
                }

                if (!read_line("Invoice format (csv or json): ",
                               format, sizeof(format))) {
                    break;
                }

                if (generate_invoice(id, format) != 0) {
                    printf("Invoice generation failed.\n");
                }
                break;
            }

            case 10:
                printf("Goodbye!\n");
                return 0;

            default:
                printf("Invalid option.\n");
        }
    }

    return 0;
}
