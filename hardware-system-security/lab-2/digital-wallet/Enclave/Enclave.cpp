#include "sgx_trts.h"
#include <stdio.h>
#include <string.h>
#include "Enclave_t.h"

static int balance = 1000;
static char transactions[20][64];
static int transaction_count = 0;

static void log_transaction(const char *type, int amount) {
	if(transaction_count < 20) {
		snprintf(transactions[transaction_count], 64, "%s: %d", type, amount);
		transaction_count++;
	}
}

int deposit(int amount){
	if(amount <= 0) {
		ocall_print_string("Deposit Failed: Amount must be positive\n");
		return -1;
	}
	balance += amount;
	log_transaction("Deposit", amount);
	return 0;
}

int pay(int amount){
	if(amount <= 0) {
		ocall_print_string("Payment Failed: Amount must be positive\n");
		return -1;
	}
	if(amount > balance) {
		ocall_print_string("Payment Failed: Insufficient funds\n");
		return -1;
	}
	balance -= amount;
	log_transaction("Pay", amount);
	return 0;
}

int view_balance() {
	char buffer[64];
	snprintf(buffer, 64, "Current Balance: %d\n", balance);
	ocall_print_string(buffer);
	return 0;
}

int view_transactions() {
	ocall_print_string("Recent Transactions:\n");
	int start = transaction_count -3;
	if(start < 0){
		start = 0;
	}
	for(int i = start; i< transaction_count; ++i){
		ocall_print_string(transactions[i]);
		ocall_print_string("\n");
	}
	return 0;
}
