use risc0_zkvm::guest::env;
use json_core::{Inputs, Outputs};
use light_poseidon::{Poseidon, PoseidonHasher};
use ark_bn254::Fr;

fn main() {
    let letters_list = ["T","R","W","A","G","M","Y","F","P","D","X","B","N","J","Z","S","Q","V","H","L","C","K","E"];

    let input: Inputs = env::read();
    let input_dni: String = input.dni;
    let input_pass: String = input.password;
    let input_sol: [[u8; 9]; 9] = input.solution;
    let input_prob: [[u8; 9]; 9] = input.problem;

    // DNI CHECK --------------------------------------------------------
    if input_dni.chars().count() != 9 {
        panic!("Invalid number of characters");
    }

    // Invalid DNI's List
    if input_dni == "00000000T" || input_dni == "00000001R" || input_dni == "99999999R"  {
        panic!("Not valid DNI");
    }

    let (numbers, supose_letter) = input_dni.split_at(input_dni.len() - 1);

    let numbers_int: u32 = numbers
        .parse()
        .unwrap_or_else(|_| panic!("DNI must have 8 numbers in a row"));

    let real_letter = numbers_int % 23;

    // Invalid digits module
    if  real_letter > 22 {
        panic!("Not valid DNI digits");
    }

    // Incorrect letter of DNI
    if supose_letter != letters_list[real_letter as usize] {
        panic!("Invalid DNI letter");
    }

    // PASSWORD CHECK --------------------------------------------------------

    let input_pass_bytes = input_pass.as_bytes();

    let bytes_pass_string = input_pass_bytes.iter().map(|b| format!("{}", b)).collect::<String>();

    let chunk_size = 32;

    let chunks: Vec<String> = bytes_pass_string
        .as_bytes()
        .chunks(chunk_size)
        .map(|chunk| String::from_utf8_lossy(chunk).to_string())
        .collect();

    let field_elements: Vec<Fr> = chunks.iter()
        .map(|chunk| Fr::from(chunk.parse::<u128>().unwrap()))
        .collect();


    let mut poseidon = Poseidon::<Fr>::new_circom(field_elements.len()).unwrap();

    let hash_result = poseidon.hash(&field_elements).unwrap();

    let hash_str = format!("{}", hash_result);

    // SUDOKU CHECK --------------------------------------------------------

    let check_consistency = input_sol.iter().zip(input_prob.iter()).all(|(row_sol, row_prob)| {
        row_sol.iter().zip(row_prob.iter()).all(|(&sol, &prob)| sol == prob || prob == 0 )
    });

    if !check_consistency {
        panic!("The solution is not consistency with the problem");
    }

    let check_rows = input_sol.iter().all(|row|{

        let row_vec: Vec<u32> = row.iter().map(|&row| row as u32).collect();
        
        row_vec.iter().sum::<u32>() == 45 && row_vec.iter().product::<u32>() == 362880
            && row_vec.iter().map(|&v| v * v).sum::<u32>() == 285
    });

    if !check_rows {
        panic!("The solution rows are not correct");
    }

    let check_cols = (0..9).all(|col|{
    
        let col_vec: Vec<u32> = input_sol.iter().map(|row| row[col] as u32).collect();
        
        col_vec.iter().sum::<u32>() == 45 && col_vec.iter().product::<u32>() == 362880
            && col_vec.iter().map(|&v| v * v).sum::<u32>() == 285
    });
    
    if !check_cols {
        panic!("The solution columns are not correct");
    }

    let check_boxes = (0..9).step_by(3).all(|i| 
        (0..9).step_by(3).all(|j| {
            let sqr_vec: Vec<u32> = (0..3)
                .flat_map(|x| (0..3).map(move |y| input_sol[i + x][j + y] as u32))
                .collect();

            sqr_vec.iter().sum::<u32>() == 45 &&
            sqr_vec.iter().product::<u32>() == 362880 &&
            sqr_vec.iter().map(|&v| v * v).sum::<u32>() == 285
        })
    );

    if !check_boxes {
        panic!("The solution boxes are not correct");
    }


    let output = Outputs {
        dni_letter: hash_str.to_string(),
        password_hash: supose_letter.to_string(),
        problem: input_prob,
    };

    env::commit(&output);

}
