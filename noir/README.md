## NOIR

### Description  
Noir is a domain-specific language (DSL) designed for writing zero-knowledge circuits in a simple and readable syntax. It abstracts the complexity of constraint systems and focuses on developer productivity and circuit portability across proving systems.

### Prover: Barretenberg  
Noir uses the Barretenberg (bb) proving system to compile, prove, and verify the circuits. While Noir handles circuit definition, Barretenberg performs the actual cryptographic operations, including generating and verifying the zk-SNARK proofs.

### Version Used

- nargo version: 1.0.0-beta.3  
  
- noirc version: 1.0.0-beta.3+ceaa1986628197bd1170147f6a07f0f98d21030a  
  
- barretenberg version: 0.82.2  

### Files

- **src/**: Contains the Noir source file (.nr) with the circuit logic.
  
- **target/**: Output directory containing:
  - **proof**: The generated zk-SNARK proof.
  
  - **sudoku_game.json**: Compiled circuit bytecode.
  
  - **vk**: The verification key.
  
- **Nargo.toml**: Project configuration file for Noir.
  
- **Prover.toml**:  Input configuration for the circuit execution, equivalent to the JSON used by other frameworks to supply public and private inputs.

### Commands to Generate the Proof


`nargo check` => Command to generate the Prover.toml file.

`nargo execute` => Command to compile and execute nr file and also generate the witness that is need to feed to our proving backend.

`bb prove -b ./target/sudoku_game.json -w ./target/sudoku_game.gz -o ./target` => Generates the zk-SNARK proof using Barretenberg.

`bb write_vk -b ./target/sudoku_game.json -o ./target` => Creates the verification key (vk).

`bb verify -k ./target/vk -p ./target/proof` => Verifies the generated proof using the verification key.

