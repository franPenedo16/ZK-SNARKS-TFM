# ZK-SNARKS-TFM
*A EVALUATION OF ZK-SNARKS*

---

## SCOPE
The scope of this project is to compare different implementations for building zero-knowledge proofs—including two domain-specific languages for zk-SNARKs (Circom and Noir), a C++ library for zk-SNARKs (libsnark), and a zk-VM based on zk-STARKs (RISC Zero)—and to evaluate the time required to generate each proof. Although RISC Zero follows a different proof paradigm, it offers a valuable point of comparison across diverse zero-knowledge methods.

## THE PROOF
The proof consists in three parts:

- ### **Validate a Spanish DNI**
  
    Given a DNI as input, the circuit must verify the    consistency between the numerical part and the corresponding letter.

    A valid DNI consists of exactly 8 digits followed by a single letter. The control letter is determined by calculating the remainder of the 8-digit number modulo 23.
    The letter obtained from this calculation must match the letter provided in the input.

    **TABLE OF VALID DNI LETTERS**
    |   |   |   |   |   |   |   |   |   |   |    |    |    |    |    |    |    |    |    |    |    |    |    |
    | - | - | - | - | - | - | - | - | - | - | -  | -  | -  | -  | -  | -  | -  | -  | -  | -  | -  | -  | -  |
    | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 |
    | T | R | W | A | G | M | Y | F | P | D | X  | B  | N  | J  | Z  | S  | Q  | V  | H  | L  | C  | K  | E  |


    Additionally, the circuit must ensure that the DNI is not one of the explicitly invalid values:

    - 00000000T

    - 00000001R

    - 99999999R


- ### **Create the hash of a password**
  
    Given a password as input, the circuit must compute its hash using the Poseidon hash function.

    Poseidon is a cryptographic hash function specifically designed for efficient use in zero-knowledge proof systems. It operates over finite fields and is optimized for performance within zk-SNARK and zk-STARK circuits, offering a good balance between security and efficiency.

    For more details, refer to the official Poseidon specification:
    https://www.poseidon-hash.info/


- ### **Validate the solution for a 9x9 sudoku**: 
    Given a Sudoku puzzle and its proposed solution as input, the circuit must verify that the solution is both consistent with the original puzzle and valid according to standard Sudoku rules.

    **Basic 9x9 Sudoku Rules:**

    - Each row must contain the digits 1 through 9 with no repetition.
    - Each column must contain the digits 1 through 9 with no repetition.
    - Each of the nine 3×3 subgrids (also called boxes) must contain the digits 1 through 9 with no repetition.
  
    **For each cell in the solution:**

    If the corresponding cell in the original puzzle contains a non-zero digit, the solution must match it.

    For each row, column, and 3×3 box:

    - The sum of all digits must be exactly 45 (since 1+2+…+9 = 45).

    - The product of all digits must be 362,880 (since 1×2×…×9 = 9! = 362,880).

    - The sum of the squares of all digits must be 285 (since 1² + 2² + … + 9² = 285).

## PUBLIC AND PRIVATE INPUTS

### Public Inputs
Public inputs are the data that are visible to anyone and are not hidden within the generated proof.

The public inputs in this project are:

- DNI letter

- Hash of the password

- Sudoku problem

### Private Inputs
Private inputs are the data that are verified inside the circuit and remain hidden from the outside.

The exact number of private inputs depends on the specific implementation, optimizations, and the number of constraints generated. However, the main private inputs are generally:

- DNI number

- Password in plain text

- Sudoku solution

### Inputs to the Circuit
All zk-SNARK circuits in this project are initialized using the same structure: a JSON input that includes the DNI (number and letter), the Sudoku puzzle (both the problem and its proposed solution), and the password. Note that Noir uses a TOML file instead of JSON, but it requires the same fields.


## FINITE FIELD

All computations within zk-SNARK circuits are performed over a finite field, which is a mathematical structure with a limited set of elements where addition, subtraction, multiplication, and division (except by zero) are defined and behave consistently.

In this project, all circuits use the same finite field, defined by the following large prime number:

    p = 21888242871839275222246405745257275088548364400416034343698204186575808495617


This is the prime field used by Circom, Noir, and Libsnark, and corresponds to the BN254 elliptic curve.

    ⚠️ Note on RISC Zero:
    Although RISC Zero does not use this prime for its core zero-knowledge proof system (since it is based on zk-STARKs and operates differently), it does use this same field specifically for computing the Poseidon hash, ensuring compatibility in that part of the logic.

Using a consistent prime field for Poseidon ensures interoperability of the hash component across different proving systems.

## THE IMPLEMENTATIONS

### [Circom](circom/README.md)
https://docs.circom.io

Circom is a domain-specific language designed for writing arithmetic circuits used in zk-SNARKs. It provides a simple syntax to define constraints and supports modularity through templates. It is widely used with the snarkJS toolkit.

### [RISC Zero](risczero/README.md)
https://www.risczero.com

RISC Zero is a zkVM (zero-knowledge virtual machine) that allows developers to generate zero-knowledge proofs for programs written in Rust. It uses a custom RISC-V-compatible virtual machine and STARK-based proof system, enabling general-purpose ZK computing.

### [Noir](noir/README.md)
https://noir-lang.org

Noir is a Rust-inspired programming language for writing zero-knowledge circuits. Developed by the Aztec team, it focuses on readability, developer experience, and seamless integration with backends like Barretenberg.

### [Libsnark](libsnark/README.md)
https://github.com/scipr-lab/libsnark

Libsnark is one of the first C++ libraries developed for zk-SNARKs. It provides low-level tools for constructing and verifying proofs, serving as the foundation for many early ZK projects. While older, it remains a reference in the space.

## THE TESTS

This section provides an overview of the benchmarking and evaluation tests conducted across four different zero-knowledge proof (ZKP) implementations: Circom, Libsnark, Noir, and RISC Zero. The goal of these tests is to assess and compare the performance, memory usage, and artifact sizes when generating zero-knowledge proofs for the same logic (DNI validation, password hashing, and Sudoku solving).

Included Benchmark Files:

[Elapsed Time Metrics](Elapsed%20Time%20Metrics.md)
Measures the total execution time needed to generate a proof in each implementation. Noir performs the fastest, while RISC Zero is significantly slower due to its architecture and execution model.

[Proof and Verification Key Sizes Comparison](Proof%20and%20Verification%20Key%20Sizes.md)
Reports the size (in bytes) of the proof files and verification keys produced by each framework. This helps evaluate storage and transmission efficiency. Note: RISC Zero uses a code image ID instead of a traditional verification key.

[PSS and RSS Metrics](Proof%20and%20Verification%20Key%20Sizes.md)
Details memory usage statistics for each implementation:

- PSS (Proportional Set Size): Reflects the exclusive and proportionate use of shared memory.

- RSS (Resident Set Size): Reflects the total physical memory used.


These tests collectively provide a comparative perspective on the trade-offs between performance, memory usage, and artifact size among leading ZKP frameworks.

The script **stats.py** is intended to quickly summarize the performance metrics for each framework using real benchmark data. It provides a command-line interface where the user specifies the implementation to analyze. It reads memory and execution time logs from the corresponding stats/ directory and computes: Average and peak memory usage & Average elapsed execution time.

## [DEMO APP](/zksnark-app/README.md)

An application built with Vuetify, Express.js, and Circom to demonstrate how Zero-Knowledge Proofs (ZKP) can enhance the development of secure authentication and verification systems.