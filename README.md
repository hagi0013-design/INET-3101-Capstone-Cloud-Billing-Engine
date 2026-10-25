# Cloud Services Billing Engine

## Problem Statement

Cloud services are allowing people and businesses to use computing resources without having to manage all the hardware themselves. However, providers still need a way to track how much each customer uses and how much they should be charged.

For this project, I created a Cloud Services Billing Engine using VirtualBox and C. The program lets users manage customer accounts, record cloud resource usage, calculate the billing costs, and generate invoices. I also made sure to include a TCP client and server to demonstrate basic network communication.

## Design/Architecture Details

I divided the program into separate source and header files so that each part has its own function. This makes the code easier to organize and keep up with.

- **main.c:** This displays the menu and also allows users to choose different operations.
- **storage.c & storage.h:** These both handle the saving, updating, and deleting customer information in files.
- **customer.c & customer.h:** Handles the adding, listing, searching for, updating and deleting customer records.
- **server.c:** Starts up a TCP server that listens for client connections and sends a response.
- **client.c:** Connects to the server and sends a test message making sure the program runs.
- **billing.c & billing.h:** Records the resource usage, calculates charges, and also generates invoices.
- **Makefile:** Compiles the source files into the `billing_engine` executable!

### Data Structures

In the program, there are three main structures:

- `UsageRecord` stores the CPU hours of the customer, their RAM usage, storage, and inbound/outbound traffic.
- `Invoice` stores the compute, storage, and network cost; as well as the total bill.
- `Customer` stores the customer's ID, email address, name, and additionally account status.

These structures are used in order to keep related information together and make it easier for the different parts of the program to use the same data.

### File Storage

- `data/customers.txt` stores customer records.
- `data/usage.txt` stores usage records.
- `invoices/` contains generated CSV and JSON invoices.

The program uses text files to store information between runs. The customer storage code uses file locking in order to help prevent conflicts when multiple processes access customer records. The billing code appends usage records to the usage file and reads those records off when calculating an invoice.

### Billing Calculations

The current billing system uses fixed rates for the calculations. These include:

- CPU usage: $0.05 per CPU hour
- Storage: $0.10 per whole GB
- RAM usage: $0.01 per GB-hour
- Inbound network traffic: Free
- Outbound network traffic: $0.02 per MB

The compute charge is a combination of the CPU and RAM costs. The storage charge is calculated using whole gigabytes, and the network charge is based on the outbound traffic. These charges are added together to calculate the total invoice amount.

The rates are examples for this project than actual prices from a cloud provider. Pricing tiers haven't been implemented.

### Network Communication

The project includes a basic TCP client and server. These two communicate through localhost on port 8080. The server listens for a connection, receives a test message, and sends a response to the client.

This demonstrates the basic process of connecting to a server and exchanging data through sockets. But, the current network implementation does not send actual cloud usage records into the billing system.

### Build and Run Instructions

To Run, open up a new terminal in the project directory and run:

```bash
make clean
make
./billing_engine
```

The Makefile will use Clang with the `-Wall`, `-Wextra`, and `-Wpedantic` compiler flags in order to help identify potential problems during compilation.

On my Mac, I encountered a linker issue with the default SDK. To fix it, specifying the available SDK resolved the build issue in the development environment:

```bash
SDKROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk make clean
SDKROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk make
```

The project was able to compile successfully without compiler warnings or errors using this configuration.

## Pros & Cons of the Solution

### Pros

- The program is seperated into differnet files, making it easier to understand and maintain.
- It shows basic TCP client-server communication.
- It helps support customer creation, searching, listing, updating, and deletion.
- Customer/usage information is stored in the files and not only in memory.
- It calculates billing costs from the recorded usage.
- The Makefile simplifies the build process.
- It also creates invoices in CSV and JSON formats.
- It demonstrates basic TCP client-server communication.
- File locking is used to help protect customer records.

### Cons

- The pricing model uses fixed rates, and doesn't support configurable pricing tiers.
- The TCP server handles a basic test message instead of receiving real usage records for billing.
- Usage-file writes do not currently have the same locking protection as customer record operations.
- The program uses text files rather than a database system.
- JSON output needs extra support for special characters in customer names.
- More testing is needed for errors like invalid input and network failures.

These problems show that the project have the basic structure needed for a billing application, but additional development would be needed before it could be used for real cloud billing.

## AI Audit Reflection

### Original Code and AI Assistance

## AI Audit Reflection

### AI Prompt and Review Process

For this project, I used ChatGPT as a senior code reviewer to help evaluate my C application. The goal was to review the program's architecture, identify possible concurrency and memory safety issues, and check whether the code was organized correctly.


### Architectural Verification

AI assistance helped me organize the application into separate source and header files. For example, customer management, file storage, and network communication were separated into different modules. The `Customer`, `UsageRecord`, and `Invoice` structures were used to group related information together.

This organization made the application easier to understand and also maintain. It also helped separate customer operations from the code that was responsible for saving records and calculating charges.

But, separate files did not automatically mean the architecture is complete. The application also need a stronger integration between the network server and billing functionality. The current TCP server receives a test message rather than actual cloud usage records, so the network component is not connected to the billing workflow yet.

### Concurrency and Memory Safety Audit

AI-assisted troubleshooting helped me investigate errors and review how the program handles files and network connections. I also tested the application by compiling it with `-Wall`, `-Wextra`, and `-Wpedantic`, then checking customer management, usage recording, invoice generation, and client-server communication.

The review identified several areas that need additional attention:

- **File concurrency:** Customer storage uses file locking, but the usage records are appended without equivalent locking. Meaning that simultaneous writes could cause problems when multiple processes access the usage file.
- **Input validation:** The program also checks several inputs, but additional testing was needed to confirm that unexpected input as well as boundary cases are handled safely.
- **JSON output:** Customer names containing quotation marks or other special characters may produce invalid JSON because the output does not fully escape those characters.
- **Memory management:** The current implementation is mainly used to fixed-size structures, and standard file/socket operations. A successful build does not prove that every resource is handled correctly, so file descriptors, sockets, and error paths still need to be reviewed.

These findings allowed me to see that reviewing code for memory safety actually involves more than just checking whether it works. It also requires examining how the program behaves when operations fail or happen simultaneously.

### Where AI Helped and Where Human Oversight Was Needed

AI was useful for explaining compiler errors, helping organize the project into modules, and identifying areas of the code that needed further review. One example was the macOS linker issue, where investigating the development environment and specifying a compatible SDK allowed the project to build successfully.

However, some of the issues AI found needed more than an AI review. For example, the project currently uses fixed pricing rates rather than configurable pricing tiers, and the TCP server hasn't been able to send usage data into the billing system. These are incomplete features rather than problems that can be solved by compiling the existing code.

I also needed to verify these suggestions by running the application and comparing the results with the project requirements. The final build completed without compiler warnings or errors, and I also tested several of the application's main features. These tests confirmed the behavior of these features, but did not prove that every concurrency issue, memory problem, or edge case has been eliminated.

### Conclusion

Overall, using AI as a code reviewer helped me think more carefully about software architecture, file safety, input validation, and error handling. I also learned that AI can help find potential issues, but its suggestions still need to be checked and tested.

The audit also highlighted areas for future improvement, like protecting concurrent usage-file writes, improving JSON escaping, and integrating actual usage ingestion with the billing system. This process showed me the importance of combining AI-assisted review with human judgement and testing!

https://mediaspace.umn.edu/media/t/1_amnwhxb4 

# Went way over the limit for the demonstration, apologies!
