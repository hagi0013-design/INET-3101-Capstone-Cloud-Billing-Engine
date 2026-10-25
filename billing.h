
#ifndef BILLING_H
#define BILLING_H

#define MAX_USAGE_RECORDS 500

typedef struct {
    int customer_id;
    double cpu_hours;
    double ram_gb_hours;
    double storage_gb;
    double inbound_mb;
    double outbound_mb;
} UsageRecord;

typedef struct {
    double compute_cost;
    double storage_cost;
    double network_cost;
    double total_cost;
} Invoice;

int record_usage(const UsageRecord *usage);
int calculate_invoice(int customer_id, Invoice *invoice);
int generate_invoice(int customer_id, const char *format);

#endif
