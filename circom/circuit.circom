pragma circom 2.0.0;

// Import Circuits of circomlib 
include "node_modules/circomlib/circuits/poseidon.circom";
include "node_modules/circomlib/circuits/comparators.circom";
include "node_modules/circomlib/circuits/gates.circom";


// Component to get if the letter is valid
// Input a digit
template getLetterNumber() {

   signal input in;
   signal output out[25];
   component eq[25];
   // List of DNI letters in their ASCII form
   var listLetters[23] = [84, 82, 87, 65, 71, 77, 89, 70, 80, 68, 88, 66, 78, 74, 90, 83, 81, 86, 72, 76, 67, 75, 69];
   var i = 0;
   var j = 0;
   var isFind = 0;
   var index = 0;
   signal find[25];

   for(var k = 0; k < 23; k++){
      eq[k] = IsEqual();
      eq[k].in[0] <== in;
      eq[k].in[1] <== listLetters[k];
      find[k] <== eq[k].out;
      // xor - isFind
      index += (find[k] + 1 - 2*find[k]*1) - (isFind);
      isFind += find[k];
   }
   find[23] <== isFind;
   find[24] <== index;

   out <== find;
}

// Component to see if the solution is consistent with the problem
// Input 2 arrays of 9 digits
template Consistency(){
   signal input a[9];
   signal input b[9];

   component eq[9];
   component zero[9];
   component or[9];

   for(var i=0; i<9; i++){
      eq[i] = IsEqual();
      eq[i].in[0] <== a[i];
      eq[i].in[1] <== b[i];
      zero[i] = IsZero();
      zero[i].in <== a[i];
      or[i] = OR();
      or[i].a <== eq[i].out;
      or[i].b <== zero[i].out;
      or[i].out === 1;
   }

}


// Component to see if every array of 9 inputs follow the rules of sum, mul & exp
// Input array of 9 digits
template Rules(){
   signal input a[9];
   signal sum;
   signal por[9];
   signal a2[9];
   signal exp;

   sum <== a[0] + a[1] + a[2] + a[3] + a[4] + a[5] + a[6] + a[7] + a[8];
   sum === 45;

   for (var i = 0; i < 9; i++){
      a2[i] <== a[i] * a[i];
      if (i == 0){
         por[0] <== 1 * a[0];
      }
      else{
         por[i] <== por[i-1] * a[i];
      }
   }

   por[8] === 362880;

   exp <== a2[0] + a2[1] + a2[2] + a2[3] + a2[4] + a2[5] + a2[6] + a2[7] + a2[8];
   exp === 285;
}

template circuit() {  

   // Declaration of signals - Inputs
   signal input pass;
   signal input dni[9];
   signal input problem[9][9];
   signal input solution[9][9];

   // Declaration of signals - Intermediate
   signal sumdigits[8];
   signal checks[3];
   signal digits;
   signal letter;
   signal suposeletter[25];
   signal module23;

   // Declaration of the custom components
   component gln = getLetterNumber();
   component consis[9];
   component rule[27];

   // Declaration of outputs
   signal output hash;
   signal output letterdniarray[25];


   // ============================ Firts step check valid DNI ============================
   // ====================================================================================

   for (var i = 0; i < 8; i++) {
      if(i == 0){
         sumdigits[0] <== dni[i];
      }
      else{
         sumdigits[i] <== sumdigits[i-1] * 10 + dni[i];
      }
   }
   digits <== sumdigits[7];

   letter <== dni[8];
   // validate prohibited dni(missing)
   // 00000000T
   component zero = IsZero();
   zero.in <== digits;
   checks[0] <== zero.out;
   checks[0] === 0;

   // 00000001R 
   component isNot[2];
   isNot[0] = IsEqual();
   isNot[0].in[0] <== digits;
   isNot[0].in[1] <== 1;
   checks[1] <== isNot[0].out;
   checks[1] === 0;

   // 99999999R
   isNot[1] = IsEqual();
   isNot[1].in[0] <== digits;
   isNot[1].in[1] <== 99999999;
   checks[2] <== isNot[1].out;
   checks[2] === 0;

   // obtener supuesta letra DNI
   gln.in <== letter;
   suposeletter <== gln.out;

   // validar si es una letra existente
   suposeletter[23] === 1;

   // valildar si la letra es coherente con el modulo 23 del digito
   module23 <-- digits % 23;
   suposeletter[24] === module23;
   letterdniarray <== suposeletter;

   // #################################### END STEP ####################################
   // ##################################################################################



   // ============================ Second step check password ============================
   // ====================================================================================

   component pos = Poseidon(1);
   pos.inputs[0] <== pass;
   hash <== pos.out;

   // #################################### END STEP ####################################
   // ##################################################################################


   
   // ============================ Third step check valid Sudoku solution ============================
   // ================================================================================================

   for (var i = 0; i < 9; i++){
      consis[i] = Consistency();
      consis[i].a <== problem[i];
      consis[i].b <== solution[i];
   }

   var box_control = 0;
   for (var i = 0; i < 9; i++){
      // // // ROWS
      rule[i] = Rules();
      rule[i].a <== solution[i];

      // // COLS
      rule[i+9] = Rules();
      rule[i+9].a <== [solution[0][i],solution[1][i],solution[2][i],solution[3][i],solution[4][i],solution[5][i],solution[6][i],solution[7][i],solution[8][i]];

      // // BOXES
      // Superior Boxes
      if (i < 3){
         rule[i+18] = Rules();
         rule[i+18].a <== [solution[0][i + box_control], solution[0][i + 1 + box_control], solution[0][i + 2 + box_control],
                           solution[1][i + box_control], solution[1][i + 1 + box_control], solution[1][i + 2 + box_control],
                           solution[2][i + box_control], solution[2][i + 1 + box_control], solution[2][i + 2 + box_control]];
      }
      // Intermediate Boxes
      if (i > 2 && i < 6){
         rule[i+18] = Rules();
         rule[i+18].a <== [solution[3][i - 3 + box_control], solution[3][i - 2 + box_control], solution[3][i - 1 + box_control],
                           solution[4][i - 3 + box_control], solution[4][i - 2 + box_control], solution[4][i - 1 + box_control],
                           solution[5][i - 3 + box_control], solution[5][i - 2 + box_control], solution[5][i - 1 + box_control]];
      }
      // Inferior Boxex
      if (i > 5) {
         rule[i+18] = Rules();
         rule[i+18].a <== [solution[6][i - 6 + box_control], solution[6][i - 5 + box_control], solution[6][i - 4 + box_control],
                           solution[7][i - 6 + box_control], solution[7][i - 5 + box_control], solution[7][i - 4 + box_control],
                           solution[8][i - 6 + box_control], solution[8][i - 5 + box_control], solution[8][i - 4 + box_control]];
      }
      box_control = box_control + 2;
      box_control = box_control > 4 ? 0 : box_control;
   }

   // #################################### END STEP ####################################
   // ##################################################################################

}
component main {public [problem]} = circuit();
