#  PROOF AND VERIFICATION KEY SIZES COMPARISON

## Description
This file summarizes the size of the generated zk-proofs and verification keys for each implementation used in the project. The goal is to highlight the relative footprint of the artifacts produced by different zero-knowledge frameworks when proving the same logic (DNI validation, password hashing, Sudoku solving).

## Proof
| IMPLEMENTATION | FILE          | SIZE        |
| -----------    | ------------- | ----------- |
| Circom         | `proof.json`  |  805 B      |
| libsnark       | `proof_data`  |  1398 B     |
| noir           | `proof`       |  19332 B    |
| risc-zero      | `receipt.bin` |  550204 B   |




## Verification key
| IMPLEMENTATION | FILE                     | SIZE         |
| -----------    | ------------------------ | ------------ |
| Circom         | `verification_key.json`  |  22332 B     |
| libsnark       | `vk_data`                |  14884 B     |
| noir           | `vk`                     |  1825 B      |
| risc-zero      | `code_id.bin`(image ID)  |  32 B        |


    Note: RISC Zero does not use a separate verification key. Instead, verification depends on the integrity of a precompiled image ID (code-id.bin), which is functionally equivalent.




