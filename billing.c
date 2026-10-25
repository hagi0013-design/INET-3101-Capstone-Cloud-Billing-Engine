
#include "billing.h"
#include "customer.h"

#include <stdio.h>
#include <string.h>

#define USAGE_FILE "data/usage.txt"
#define INVOICE_DIR "invoices"

#define CPU_RATE 0.05
#define RAM_RATE 0.01
#define STORAGE_RATE 0.10
#define OUTBOUND_RATE 0.02

int record_usage(const UsageRecord *usage)
{
    if (usage == NULL || usage->customer_id <= 0 ||
        usage->cpu_hours < 0 || usage->ram_gb_hours < 0 ||
        usage->storage_gb < 0 || usage->inbound_mb < 0 ||
        usage->outbound_mb < 0) {
        fprintf(stderr, "Invalid usage information.\n");
        return -1;
    }

    FILE *file = fopen(USAGE_FILE, "a");

    if (file == NULL) {
        perror("Could not open usage file");
        return -1;
    }

    int result = fprintf(file, "%d|%.2f|%.2f|%.2f|%.2f|%.2f\n",
                         usage->customer_id,
                         usage->cpu_hours,
                         usage->ram_gb_hours,
                         usage->storage_gb,
                         usage->inbound_mb,
                         usage->outbound_mb);

    if (fclose(file) != 0 || result < 0) {
        fprintf(stderr, "Could not save usage record.\n");
        return -1;
    }

    printf("Usage record saved.\n");
    return 0;
}

int calculate_invoice(int customer_id, Invoice *invoice)
{
    if (customer_id <= 0 || invoice == NULL) {
        return -1;
    }

    FILE *file = fopen(USAGE_FILE, "r");

    if (file == NULL) {
        invoice->compute_cost = 0;
        invoice->storage_cost = 0;
        invoice->network_cost = 0;
        invoice->total_cost = 0;
        return 0;
    }

    invoice->compute_cost = 0;
    invoice->storage_cost = 0;
    invoice->network_cost = 0;

    char line[256];
    int record_customer;
    double cpu, ram, storage, inbound, outbound;

    while (fgets(line, sizeof(line), file) != NULL) {
        int fields = sscanf(line, "%d|%lf|%lf|%lf|%lf|%lf",
                            &record_customer, &cpu, &ram,
                            &storage, &inbound, &outbound);

        if (fields != 6) {
            fprintf(stderr, "Skipping invalid usage record.\n");
            continue;
        }

        if (record_customer == customer_id) {
            invoice->compute_cost +=
                cpu * CPU_RATE + ram * RAM_RATE;

            invoice->storage_cost +=
                (double)((long)storage) * STORAGE_RATE;

            invoice->network_cost +=
                outbound * OUTBOUND_RATE;
        }
    }

    if (ferror(file)) {
        fclose(file);
        return -1;
    }

    fclose(file);

    invoice->total_cost = invoice->compute_cost +
                          invoice->storage_cost +
                          invoice->network_cost;

    return 0;
}

int generate_invoice(int customer_id, const char *format)
{
    if (customer_id <= 0 || format == NULL) {
        return -1;
    }

    Customer customer;

    if (find_customer(customer_id, &customer) != 0) {
        fprintf(stderr, "Customer not found.\n");
        return -1;
    }

    Invoice invoice;

    if (calculate_invoice(customer_id, &invoice) != 0) {
        fprintf(stderr, "Could not calculate invoice.\n");
        return -1;
    }

    char filename[200];

    if (strcmp(format, "csv") == 0) {
        snprintf(filename, sizeof(filename),
                 INVOICE_DIR "/invoice_%d.csv", customer_id);

        FILE *file = fopen(filename, "w");

        if (file == NULL) {
            perror("Could not create invoice");
            return -1;
        }

        fprintf(file, "Customer ID,Name,Compute Cost,Storage Cost,Network Cost,Total\n");
        fprintf(file, "%d,\"%s\",%.2f,%.2f,%.2f,%.2f\n",
                customer.id, customer.name,
                invoice.compute_cost, invoice.storage_cost,
                invoice.network_cost, invoice.total_cost);

        if (fclose(file) != 0) {
            return -1;
        }
    } else if (strcmp(format, "json") == 0) {
        snprintf(filename, sizeof(filename),
                 INVOICE_DIR "/invoice_%d.json", customer_id);

        FILE *file = fopen(filename, "w");

        if (file == NULL) {
            perror("Could not create invoice");
            return -1;
        }

        fprintf(file,
                "{\n"
                "  \"customer_id\": %d,\n"
                "  \"customer_name\": \"%s\",\n"
                "  \"compute_cost\": %.2f,\n"
                "  \"storage_cost\": %.2f,\n"
                "  \"network_cost\": %.2f,\n"
                "  \"total_cost\": %.2f\n"
                "}\n",
                customer.id, customer.name,
                invoice.compute_cost, invoice.storage_cost,
                invoice.network_cost, invoice.total_cost);

        if (fclose(file) != 0) {
            return -1;
        }
    } else {
        fprintf(stderr, "Format must be csv or json.\n");
        return -1;
    }

    printf("Invoice generated: %s\n", filename);
    return 0;
}
