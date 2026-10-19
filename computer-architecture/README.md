# Computer Architecture & RISC-V Assembly
Rutgers University · Computer Architecture and Assembly Lab (14:332:333)

Assembly exercises and processor analysis covering procedure calls, stack memory, array addressing, matrix multiplication, and pipeline dependencies.

| Lab | Work | Concepts |
| --- | --- | --- |
| [Lab 4 — Registers & Procedures](lab-4/) | Array traversal, conditional procedures, polynomial evaluation through nested calls | Register usage, branches, arguments and return values, return-address storage |
| [Lab 5 — Procedures & Stacks](lab-5/) | Stack arithmetic, factorial, in-place array reversal, matrix multiplication | Stack addressing, loads and stores, loop control, row-major indexing |
| [Lab 6 — CPU Structure & Pipelining](lab-6/) | Datapath control tables and instruction-sequence analysis | Five-stage pipelines, data dependencies, stalls, forwarding, instruction scheduling |

Each lab includes its report PDF and assembly listings. Labs 4 and 5 target the **Venus RISC-V simulator**. Lab 6 uses **WebRISC-V** to study supplied instruction sequences and compare pipeline behavior with and without forwarding.

The `.s` listings were extracted from the submitted reports. Lab 6 files are analysis snippets rather than standalone applications.
