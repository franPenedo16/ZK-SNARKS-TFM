pragma circom 2.0.0;

include "../../node_modules/circomlib/circuits/poseidon.circom";
include "../../node_modules/circomlib/circuits/comparators.circom";
include "../../node_modules/circomlib/circuits/gates.circom";

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

   signal input pass;
   signal input dni[9];
   signal input hashBD;
   signal input problem[9][9];
   signal input solution[9][9];

   signal sumdigits[8];
   signal digits;
   signal hashPass;
   signal hashSum;

   component consis[9];
   component rule[27];

   signal output hash;

   for (var i = 0; i < 8; i++) {
      if(i == 0){
         sumdigits[0] <== dni[i];
      }
      else{
         sumdigits[i] <== sumdigits[i-1] * 10 + dni[i];
      }
   }
   digits <== sumdigits[7];

   component pos = Poseidon(1);
   pos.inputs[0] <== pass;
   hashPass <== pos.out;

   hashSum <== hashPass + digits;

   component pos1 = Poseidon(1);
   pos1.inputs[0] <== hashSum;
   hash <== pos1.out;


   hash === hashBD;

   for (var i = 0; i < 9; i++){
      consis[i] = Consistency();
      consis[i].a <== problem[i];
      consis[i].b <== solution[i];
   }

   var box_control = 0;
   for (var i = 0; i < 9; i++){
      rule[i] = Rules();
      rule[i].a <== solution[i];

      rule[i+9] = Rules();
      rule[i+9].a <== [solution[0][i],solution[1][i],solution[2][i],solution[3][i],solution[4][i],solution[5][i],solution[6][i],solution[7][i],solution[8][i]];

      if (i < 3){
         rule[i+18] = Rules();
         rule[i+18].a <== [solution[0][i + box_control], solution[0][i + 1 + box_control], solution[0][i + 2 + box_control],
                           solution[1][i + box_control], solution[1][i + 1 + box_control], solution[1][i + 2 + box_control],
                           solution[2][i + box_control], solution[2][i + 1 + box_control], solution[2][i + 2 + box_control]];
      }
      if (i > 2 && i < 6){
         rule[i+18] = Rules();
         rule[i+18].a <== [solution[3][i - 3 + box_control], solution[3][i - 2 + box_control], solution[3][i - 1 + box_control],
                           solution[4][i - 3 + box_control], solution[4][i - 2 + box_control], solution[4][i - 1 + box_control],
                           solution[5][i - 3 + box_control], solution[5][i - 2 + box_control], solution[5][i - 1 + box_control]];
      }
      if (i > 5) {
         rule[i+18] = Rules();
         rule[i+18].a <== [solution[6][i - 6 + box_control], solution[6][i - 5 + box_control], solution[6][i - 4 + box_control],
                           solution[7][i - 6 + box_control], solution[7][i - 5 + box_control], solution[7][i - 4 + box_control],
                           solution[8][i - 6 + box_control], solution[8][i - 5 + box_control], solution[8][i - 4 + box_control]];
      }
      box_control = box_control + 2;
      box_control = box_control > 4 ? 0 : box_control;

   }

}
component main {public [problem, hashBD]} = circuit();
