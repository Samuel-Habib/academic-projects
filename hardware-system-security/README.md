# Hardware & Systems Security
Samuel Habib · Rutgers University

Labs exploring trusted execution environments on ARM and Intel platforms, with reports, execution logs, and enclave source.

| Lab | Work | Technical focus |
| --- | --- | --- |
| [Lab 1 — OP-TEE](lab-1/) | QEMU execution of a trusted application, OP-TEE regression tests, and entry-point analysis | ARM TrustZone, secure/normal-world interfaces, TA lifecycle |
| [Lab 2 — Intel SGX](lab-2/) | Enclave initialization, random-number exercise, and a digital-wallet extension | EDL interfaces, ECALLs, OCALLs, enclave state |

The recorded OP-TEE regression summary reports 143 test cases and 41,201 subtests with zero failures. These are results from the supplied transcript, not a new test run. Intel SGX execution logs use debug simulation mode.
