use std::hash::poseidon;

struct OutStruct {
    dni_letter: u8,
    password_hash: Field,
    problem: [[u32;9];9],
}

fn main(dni : (u64, str<1>), pass: Field, solution: [[u32;9];9], problem: pub [[u32;9];9]) -> pub OutStruct{

    let list_letters:[u8] = [84, 82, 87, 65, 71, 77, 89, 70, 80, 68, 88, 66, 78, 74, 90, 83, 81, 86, 72, 76, 67, 75, 69];
    let index_array: [u32;9] = [0, 1, 2, 3, 4, 5, 6, 7, 8];
    let index_boxes: [u32;3] = [0, 3, 6];
    let acc: u32 = 0;
    let acc_1: u32 = 1;

    // DNI VALIDATE -----------------------

        // validate prohibited dni numbers
    assert(dni.0 > 1 & dni.0 < 99999999);

        // Validate letter of dni 
    let supose_letter = dni.1.as_bytes();

    let calculate_letter = dni.0 % 23;

    assert(list_letters[calculate_letter] == supose_letter[0]);
    

    // PASS VALIDATE -----------------------------------------

    let hash = poseidon::bn254::hash_1([pass]);

    // GAME VALIDATE -------------------------------------------

        // check consistency problen & solution
    let check_consistency = index_array.all(|index| {
        let row_consistency = index_array.all(|kindex| {
            (solution[index][kindex] == problem[index][kindex]) | (problem[index][kindex] == 0)
        });
        row_consistency
    });

    assert(check_consistency);

        // check rows if are OK
    let check_rows = solution.all(|sol| {
        (sol.fold(acc, |a, b| a + b) == 45) &
        (sol.fold(acc_1, |a, b| a * b) == 362880) &
        (sol.fold(acc, |a, b| (b * b) + a) == 285)
    });

    assert(check_rows);


        // check cols if are OK
    let check_cols = index_array.all(|index| {
        let col = solution.map(|sol| sol[index]);
        (col.fold(acc, |a, b| a + b) == 45) &
        (col.fold(acc_1, |a, b| a * b) == 362880) &
        (col.fold(acc, |a, b| (b * b) + a) == 285)
    });

    assert(check_cols);


        // check boxes if are OK
    let check_boxes = index_boxes.all(|i|{

        let row_boxes = solution.map(|sol| {
            [sol[0 + i] + sol[1 + i] + sol[2 + i],
             sol[0 + i] * sol[1 + i] * sol[2 + i],
             sol[0 + i] * sol[0 + i] + sol[1 + i] * sol[1 + i] + sol[2 + i] * sol[2 + i]]
        });

        let check_row_box = index_boxes.all(|k|{
            (row_boxes[0 + k][0] + row_boxes[1 + k][0] + row_boxes[2 + k][0] == 45) &
            (row_boxes[0 + k][1] * row_boxes[1 + k][1] * row_boxes[2 + k][1] == 362880) &
            (row_boxes[0 + k][2] + row_boxes[1 + k][2] + row_boxes[2 + k][2] == 285)
        });

        check_row_box
    });

    assert(check_boxes);


    // OUTPUTS -------------------------------------------

    OutStruct {
        dni_letter: dni.1.as_bytes()[0],
        password_hash: hash,
        problem: problem
    }

}
