# LIBSNARK

## Description  
Libsnark is a C++ library for constructing and verifying zk-SNARK proofs. It provides low-level APIs to define Rank-1 Constraint Systems (R1CS), perform the trusted setup, generate proofs, and verify them.

## Code base  
The code in this project are based on the tutorials from the [libsnark-tutorial repository](https://github.com/coder5876/libsnark-tutorial), which demonstrate:
- Defining constraints in C++  
- Running the trusted setup (key generation)  
- Creating and exporting proofs  
- Verifying proofs against a public key

## Execution Environment
To ensure compatibility with libsnark and its dependencies (GMP, protobuf, etc.), all code runs inside a Docker container built from `ubuntu:16.04`. This guarantees a consistent build and runtime environment for compiling and executing libsnark-based circuits.

## Files

- **check-ops/**:  
  Directory containing a custom gadget used to verify whether an array satisfies addition, multiplication, and exponentiation constraints as specified by the gadget parameters. It is used to validate Sudoku rules.

- **compare-or/**:  
  Directory with a custom gadget that checks if each element in array A is equal to the corresponding element in array B or array C. It is used to ensure consistency between the Sudoku problem and its solution, where array C represents the zeros (empty cells).

- **poseidon/**:  
  Directory containing the gadget and constants required to compute a Poseidon hash. The implementation is tailored to support a single input, which is sufficient for this analysis since the password is treated as a single input.

- **proof/**:  
  Directory where the resulting zk-SNARK proof and the verification key are stored after execution.

- **stats/**:
  Directory with shell scripts that repeatedly run the proof-generation process to collect memory usage metrics (PSS and RSS) and measure elapsed execution time.

- **data.json**:  
  JSON file containing the inputs used for generating the proof, including the password, DNI, and Sudoku data.

- **Dockerfile**:  
  File defining the instructions for building the Docker container with the required dependencies and environment setup for running libsnark.

- **game.cpp**:  
  Main C++ file containing the logic to define the circuit, generate the constraints, perform the trusted setup, and produce the zk-SNARK proof.

- **Parent-CMakeLists.txt**:  
  Modified version of the root-level `CMakeLists.txt` file to support the build process with the added gadgets and custom logic.

- **Src-CMakeLists.txt**:  
  Modified version of the `src/CMakeLists.txt` file to include and compile the custom gadgets and the `game.cpp` logic.


## Commands to generate the proof

`docker build -t libsnark-sudoku .` => Builds the Docker container using the specifications defined in the Dockerfile. The container includes all necessary dependencies to compile and run libsnark.

`docker run -v "$(pwd)/proof:/proof" -w /root/libsnark-tutorial/src -it libsnark-sudoku /bin/bash -c "../build/src/game"` => Executes the proof generation process inside the container. The command runs the compiled game binary, which generates the zk-SNARK proof and verification key. The -v flag mounts the ./proof directory from the host machine into the container, allowing the output files (proof and verification key) to be saved and accessed from outside the container.

- `docker run -v "$(pwd)/stats:/stats" -w /root/libsnark-tutorial/src -it libsnark-sudoku /bin/bash -c "./memory-pss-usage.sh"`

- `docker run -v "$(pwd)/stats:/stats" -w /root/libsnark-tutorial/src -it libsnark-sudoku /bin/bash -c "./memory-rss-usage.sh"`

- `docker run -v "$(pwd)/stats:/stats" -w /root/libsnark-tutorial/src -it libsnark-sudoku /bin/bash -c "./elapsed_secs.sh"` 
  
  - This commands runs the tests inside the container to measure memory usage in terms of RSS (Resident Set Size), PSS (Proportional Set Size) and elapses excecution time. The -v flag mounts the ./stats directory from the host machine into the container, allowing the output json to be saved and accessed from outside the container.
