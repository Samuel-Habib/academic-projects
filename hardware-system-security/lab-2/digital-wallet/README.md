# SGX Digital Wallet
A C++ enclave exercise that stores a balance and transaction history inside the enclave, exposes deposit and payment operations through ECALLs, and prints results through an OCALL.

The host starts with an enclave balance of 1000, deposits 200, pays 50, and displays the resulting balance of 1150 and transaction history.

## Build and run
Requires the Intel SGX SDK for Linux x86-64, a C/C++ compiler, GNU Make, and OpenSSL. Simulation mode does not require SGX hardware.

```sh
make SGX_SDK=/opt/intel/sgxsdk SGX_MODE=SIM
LD_LIBRARY_PATH=/opt/intel/sgxsdk/lib64:${LD_LIBRARY_PATH:-} ./secure_rng
```

If the SDK is installed elsewhere, substitute that path in both commands. Run the executable from this project directory so it can locate `enclave.signed.so`.

The Makefile invokes Intel's `sgx_edger8r` to generate `App/Enclave_u.c/.h` and `Enclave/Enclave_t.c/.h` from `Enclave/Enclave.edl`. It compiles the host and enclave, creates a local development signing key, and signs the enclave. Generated interfaces, binaries, and the development key are ignored by Git.

`make clean` removes generated build artifacts and the development signing key. The default configuration enables debug simulation. Hardware mode requires the SDK's hardware runtime and SGX-capable environment; it has not been validated here.

## Project files
- `App/App.cpp`: host initialization and ECALL sequence.
- `Enclave/Enclave.cpp`: balance and transaction operations.
- `Enclave/Enclave.edl`: trusted/untrusted interface definitions.
- `Enclave/Enclave.config.xml`: stack, heap, thread, and debug configuration.
- `Enclave/Enclave.lds`: enclave symbol exports.

## Build-file provenance
The three C++/EDL submission files are unchanged. Missing build support was reconstructed using Intel's [SampleEnclave](https://github.com/intel/confidential-computing.sgx.sdk/tree/main/SampleCode/SampleEnclave) Makefile, configuration, and linker script. Configuration values match those printed in the submitted execution log.

The Makefile defaults to simulation, names the host `secure_rng` to match the recorded lab commands, and supplies the standard string declaration needed by the submitted host code. Generated interface headers must come from the installed SDK rather than manually written declarations.

The SDK and container runtime are unavailable in the reconstruction environment, so a complete SGX build and execution have not been revalidated. Original execution logs are stored in the parent lab folder.

This is an in-memory classroom wallet demonstration: it does not implement persistent sealed storage, authentication, attestation, overflow protection, or synchronization for concurrent wallet operations.
