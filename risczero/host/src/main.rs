
// Import guest methods
use methods::{
    GUEST_CODE_FOR_TFM_ELF, GUEST_CODE_FOR_TFM_ID
};
// Import risk zero
use risc0_zkvm::{default_prover, ExecutorEnv};

// Import packages to manage json and export files to validate receipt
use json::parse;
use json_core::{Inputs, Outputs};
use std::time::Instant;
use std::fs::File;
use std::io::{Write};

fn main() {
    // Include json file with the inputs for the program
    let data = include_str!("../../res/input.json");

    let data = parse(&data).unwrap();

    let mut data_array_sol: [[u8; 9]; 9] = [[0; 9]; 9];
    let mut data_array_prob: [[u8; 9]; 9] = [[0; 9]; 9];

    for i in 0..9 {
        for j in 0..9 {
            data_array_prob[i][j] = data["problem"][i][j].as_u8().unwrap();
            data_array_sol[i][j] = data["solution"][i][j].as_u8().unwrap();
        }
    }

    let input = Inputs {
        dni: data["dni"].to_string(),
        password: data["password"].to_string(),
        problem: data_array_prob,
        solution: data_array_sol,
    };

    // Prepare input values for the prove
    let env = ExecutorEnv::builder()
        .write(&input)
        .unwrap()
        .build()
        .unwrap();

    let prover = default_prover();

    // Launch the excecution of the prove & take a snapshot of the time it takes to run
    let start = Instant::now();

    let prove_info = prover
        .prove(env, GUEST_CODE_FOR_TFM_ELF)
        .unwrap();

    let last = start.elapsed();
    println!("The proof took {:?}", last);

    // Get the receipt serialized it & write in file
    let receipt = prove_info.receipt;

    let serialized = bincode::serialize(&receipt).unwrap();

    let _saved_file = match std::fs::write("./receipt.bin", serialized){
        Ok(()) => println!("Receipt saved and serialized as receipt.bin"),
        Err(_) => println!("Something went wrong !!"),
    };

    // Get the id of the code & send it to file 
    let serialized_code = bincode::serialize(&GUEST_CODE_FOR_TFM_ID).expect("Failed to serialize");

    let file = File::create("code_id.bin");    

    let _ = file.expect("Failed to serialize").write_all(&serialized_code);

    // Show the public outputs in terminal
    let _output: Outputs = receipt.journal.decode().unwrap();

    println!("I am someone with a DNI letter {:?} and a password related to {:?} ", _output.password_hash, _output.dni_letter);

    println!("And I know {:?}, and I can prove it!", _output.problem);

    // Chack verification of the prove 
    receipt.verify(GUEST_CODE_FOR_TFM_ID).expect(
        "Something wrong",
    );
}

