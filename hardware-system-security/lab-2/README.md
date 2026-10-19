# Lab 2 — Intel SGX Enclaves

Enclave exercises using Intel SGX debug simulation, followed by an in-enclave digital-wallet demonstration.

- `Lab1_HelloEnclave-1.txt`: HelloEnclave build and execution transcript. This is part of Lab 2 despite its filename.
- `Lab2_WriteEnclave.txt`: random-number ECALL exercise transcript.
- `Bonus_DigitalWallet.txt`: wallet development and execution transcript.
- [digital-wallet/](digital-wallet/): host source, enclave implementation, EDL interfaces, and build support.

The wallet demonstration deposits 200 and pays 50 against an initial balance of 1000, then prints a balance of 1150 and the recorded transactions. The original logs show successful simulation-mode execution; they do not establish hardware-mode isolation.

The downloaded host, enclave, and EDL files implement the wallet extension. Separate source trees for the earlier HelloEnclave and RNG stages are not included. See the wallet README for SDK requirements and build-file provenance.
