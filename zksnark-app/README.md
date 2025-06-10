# ZKSNARK-APP

## Description
ZKSNARK-APP is a web application that demonstrates the use of zero-knowledge proofs (ZKPs), built with the help of Circom, to showcase how a basic client-server architecture can implement zero-trust authentication. Additionally, it features peer-to-peer verification for task completion.

The application includes a web-based authentication system and allows users to solve daily Sudoku puzzles at easy, medium, and hard difficulty levels. After solving a puzzle, users can generate a zero-knowledge proof of their solution. Other users can then verify these proofs, enabling a decentralized trust model. The platform also includes a blocking mechanism, allowing users to block others who attempt to validate incorrect solutions. Each user maintains their own block list, and verification attempts from blocked users are hidden—ensuring that users only see verifications from trusted participants and helping maintain the integrity of the network.

Sudoku puzzles are generated using the [API](https://www.api-ninjas.com/api/sudoku) Ninjas service.

## Components

The web application consists of the following components:

- FrontEnd: Built with Node and Vue.js, using Vuetify for crafting responsive and modern UI components.
  
- BackEnd: Developed with Node.js and Express to handle HTTP requests and manage communication with the database.
  
- Database: A lightweight SQLite database structured with the following tables:
  
  - user (id, username, dni, hash)
  - sudokus (id, problem, solution, date, level)
  - resolved (id, FK user, FK sudoku, proof, public_inputs)
  - verifications (id, FK verifier, FK solution)
  - blocks (id, FK user, FK blocked_user)
  
- Proofs: Implemented using Circom, with dedicated circuits for:
  - Authentication
  - Sudoku solution verification

### Proof

The circuit compilation is executed before launching both the front end and back end of the application, following the Powers of Tau ceremony.

The resulting files are placed in the front end’s public folder:

- .zkey – proving key

- .wasm – compiled circuit

- verification_key.json – for public verification

### Authentication

During login, the user generates a zero-knowledge proof demonstrating knowledge of the correct password associated with a hashed value stored in the database.

The inputs for the circuits are:
- DNI
- Password
- Stored user hash
- Control date (timestamp of proof generation)

Circuit Output (Public):
- Hash calculated within the circuit

The **user hash** and **control date** are public inputs to the circuit.

The hash is generated using the Poseidon hash function, following these steps:

1. Compute Poseidon hash of the password.

2. Concatenate the DNI number (excluding the letter).

3. Compute Poseidon hash again with the result from step 2.

The control date ensures the proof is valid for only 5 minutes. Even if the proof verifies correctly, the server will reject it if the timestamp is outside the allowed time window.

## Sudoku solution

On the main page, users can solve a Sudoku puzzle and generate a proof that the solution is correct.

The inputs for the circuit are:
- Password
- DNI
- User hash
- Sudoku problem
- Sudoku solution

Circuit Output (Public):
- Sudoku problem
- User hash

The hash is calculated using the same method as in the authentication circuit.

The circuit checks:

- That the provided solution is valid for the given Sudoku puzzle.
- That the proof is bound to a specific user by including the hash, ensuring uniqueness.

The circuit validates that the solution is correct and correspond to the problem.

This proves not only that the user solved the puzzle correctly but also that they are authenticated as the rightful owner of the associated password and user hash.

## Flows
### Login
- ### Login Screen
  
![Login Screen](/Images/LoginScreen.png)

- Login Flow
  
![Login Flow](/Images/Login.png)

### Sudoku
- ### Sudoku Screen
  
![Sudoku Solve Screen](/Images/SudokuScreen.png)

- ### Sudoku Password Screen
  
![Sudoku Pass Screen](/Images/SudokuPassScreen.png)

- ### Sudoku Solve Flow
  
![Sudoku Solve Flow](/Images/Sudoku.png)

### Verifications
- ###  Verifications Screen
  
![Verify Screen](/Images/VerifyScreen.png)

- ### Verifications Block screen
  
![Verify Block Screen](/Images/BlockScreen.png)

- ### Verifications Flow
  
![Verify Flow](/Images/Verify.png)


### Setup

To launch the application, follow these steps:

1. Configure Environment Variables
Create a `.env` file in the backend directory. You can use the provided `.env-test` file as a reference.

2. Install Dependencies and Run Servers
   
- In the backend directory (`zksnark-app/backend`), run:
  
    `npm install`

    `node server.js`


- In the frontend (`zksnark-app`) run:
  
    `npm install`
  
    `npm run dev`

3. Recompile Circuits (If Needed)

    If you need to modify the Circom circuits, follow the instructions provided in [/circom/README.md](/circom/README.md)

    After recompiling, make sure to place the output files (`.wasm`, `.zkey`, and `verification_key.json`) in the following directories:

   - `zksnark-app/backend/circuits/auth` && `zksnark-app/public/circuits/auth`
   - `zksnark-app/backend/circuits/sudokus` && `zksnark-app/public/circuits/sudokus`

4. Access the Database (If Needed)

    To inspect or modify the SQLite database directly, use the following command:

    `sqlite3 database.db`

