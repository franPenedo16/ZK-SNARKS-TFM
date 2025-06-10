# RISC-ZERO

## Description  
RISC Zero is a zero-knowledge Virtual Machine (zkVM) framework that lets you write general-purpose Rust programs and generate zk-STARK proofs of their execution. Instead of defining arithmetic circuits by hand, you compile standard Rust code to RISC-V instructions and run them inside the zkVM, which transparently produces a proof of correct execution.

## ZKVM  
A zero-knowledge VM (ZKVM) is an execution environment that records the entire instruction trace of a program and then generates a succinct proof that the trace was followed correctly. In RISC Zero, the zkVM implements a RISC-V subset, allowing unmodified Rust code to be proven without manual constraint generation.

## Host & Guest  
- **Guest**: The Rust program want to prove. It is compiled to a RISC-V binary (methods/) and executed inside the zkVM.  
- **Host**: The application that drives the zkVM. It loads the guest binary, provides inputs, runs the zkVM, and collects the resulting proof (receipt). The host code lives in host/.

## Receipt  
A **receipt** is the proof artifact output by the zkVM. It contains:  
- A Merkle-root commitment to the full execution trace  
- Public inputs and outputs  
- The zk-STARK proof itself  
  
The receipt can be share to any verifier, who uses it to confirm that the guest code ran correctly on the given inputs.

## Version Used

cargo-risczero: 2.0.2

## Files

- **core/**: 
  Definitions of Rust structs for input and output data. 

- **host/**: Host directory
  - **src/**: 
    Host-side Rust code that invokes the zkVM and generates the receipt.  

  - **stats/**: 
    Host-side Rust code that invokes the zkVM multiple times to collect memory usage metrics (PSS and RSS) and measure elapsed execution time.  

- **methods/**: 
  Guest-side Rust code (compiled to RISC-V) containing the circuit logic.  

- **res/**: 
  JSON file with public and private inputs for the guest program.  

- **stats**:
  Directory with json files with the collection of memory usage metrics (PSS and RSS) and measure elapsed execution time.

- **Cargo.toml**: 
  Workspace configuration.

- **code_id.bin**: 
  Identifier for the compiled guest code used by the host at runtime.  

- **receipt.bin**: 
  The zk-STARK proof (“receipt”) produced by the zkVM.  

- **rust-toolchain.toml**: 
  Specifies the Rust toolchain version used for reproducible builds.

## Commands to Generate the Proof

`cargo run -p host --bin normal --release`=> Compiles and runs the host application, which in turn compiles the guest to RISC-V, executes it in the zkVM, and writes receipt.bin (the proof) along with the code_id.bin to the project root.

`cargo run -p host --bin stats --release` => Compiles and runs the host application, which in turn compiles the guest to RISC-V, executes it multiple times in the zkVM, to collect memory usage metrics (PSS and RSS) and measure elapsed execution time. 
