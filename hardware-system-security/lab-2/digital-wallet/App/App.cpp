#include <stdio.h>
#include <unistd.h>

#include "sgx_urts.h"
#include "Enclave_u.h"

// Enclave ID
sgx_enclave_id_t global_eid = 0;

/**
 * Initialize an SGX enclave.
 * @return error state
 */
int initialize_enclave(void) {
	sgx_status_t state = SGX_ERROR_UNEXPECTED;

	char enclavefile[256];
	getcwd(enclavefile, sizeof(enclavefile));
	strcat(enclavefile, "/enclave.signed.so");

	state = sgx_create_enclave(enclavefile, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);

	if (state != SGX_SUCCESS) {
		printf("Can not create enclave, error-code: %d\n", state);
		return -1;
	}

	return 0;
}

/**
 * OCALL used by the enclave to print text outside the enclave.
 */
void ocall_print_string(const char *str) {
	printf("%s", str);
}

/**
 * Main application.
 */
int SGX_CDECL main() {
	int retval = 0;
	sgx_status_t state = SGX_SUCCESS;

	if (initialize_enclave() != 0) {
		return -1;
	}

	state = deposit(global_eid, &retval, 200);
	if (state != SGX_SUCCESS || retval != 0) {
		printf("Deposit failed\n");
		sgx_destroy_enclave(global_eid);
		return -1;
	}

	state = pay(global_eid, &retval, 50);
	if (state != SGX_SUCCESS || retval != 0) {
		printf("Payment failed\n");
		sgx_destroy_enclave(global_eid);
		return -1;
	}

	state = view_balance(global_eid, &retval);
	if (state != SGX_SUCCESS || retval != 0) {
		printf("View balance failed\n");
		sgx_destroy_enclave(global_eid);
		return -1;
	}

	state = view_transactions(global_eid, &retval);
	if (state != SGX_SUCCESS || retval != 0) {
		printf("View transactions failed\n");
		sgx_destroy_enclave(global_eid);
		return -1;
	}

	sgx_destroy_enclave(global_eid);

	return 0;
}


