pragma circom 2.0.0;

include "../../node_modules/circomlib/circuits/poseidon.circom";
template circuit() {  

   signal input pass;
   signal input dni[9];
   signal input hashBD;
   signal input controlDate;
   signal sumdigits[8];
   signal digits;
   signal hashPass;
   signal hashSum;
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
}

component main {public [controlDate, hashBD]}  = circuit();