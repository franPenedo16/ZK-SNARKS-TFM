#include <stdlib.h>
#include <iostream> 
#include <string>
#include <cmath>

#include "libsnark/common/default_types/r1cs_ppzksnark_pp.hpp"
#include "libsnark/gadgetlib1/pb_variable.hpp"
#include "libsnark/gadgetlib1/gadgets/basic_gadgets.hpp"

#include "rapidjson/document.h" 
#include "rapidjson/filereadstream.h" 

#include "compare-or/compare_or.hpp" 
#include "compare-or/compare_or.tcc" 
#include "check-ops/check_ops.hpp" 
#include "check-ops/check_ops.tcc" 
#include "poseidon/poseidon.hpp" 
#include "poseidon/poseidon.tcc" 

#include "util.hpp"

using namespace libsnark;
using namespace std;
  
int main()
{
  // Open the file for reading 
  FILE* fp = fopen("data.json", "r"); 
  
  // Use a FileReadStream to 
  // read the data from the file 
  char readBuffer[65536]; 
  rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer)); 

  // Parse the JSON data  
  // using a Document object 
  rapidjson::Document data; 
  data.ParseStream(is); 

  // Close the file 
  fclose(fp); 

  // Access the data in the JSON document 
  default_r1cs_ppzksnark_pp::init_public_params();
  typedef libff::Fr<default_r1cs_ppzksnark_pp> FieldT;

  string dni = data["DNI"].GetString();

  int dni_numbers = stoi(dni.substr(0,8));
  int dni_letter_ascii  = dni[8];
  int dni_letter = 100;

  int letter_list[23] = {84, 82, 87, 65, 71, 77, 89, 70, 80, 68, 88, 66, 78, 74, 90, 83, 81, 86, 72, 76, 67, 75, 69};

  for (int i = 0; i < 23; ++i) {
        if (letter_list[i] == dni_letter_ascii) {
            dni_letter = i;
            break;
        }
  }

  int pass = data["pass"].GetInt();

  std::vector<std::vector<FieldT>> problem_data;
  for (const auto& rowValue : data["problem"].GetArray()) {
      std::vector<FieldT> row;
      for (const auto& v : rowValue.GetArray()) {
          row.push_back(FieldT(v.GetInt()));
      }
      problem_data.push_back(std::move(row));
  }

  std::vector<std::vector<FieldT>> solution_data;
  for (const auto& rowValue : data["solution"].GetArray()) {
      std::vector<FieldT> row;
      for (const auto& v : rowValue.GetArray()) {
          row.push_back(v.GetInt());
      }
      solution_data.push_back(std::move(row));
  }

  protoboard<FieldT> pb;

  std::vector<pb_variable_array<FieldT>> problem, solution;

  pb_variable_array<FieldT> zeros, consistency, password;

  pb_variable<FieldT> letter, numbers, diff1, invdiff1, diff2, invdiff2, diff3, invdiff3, quotient, pre_result, real_letter, hashpass;


  // allocate public inputs
  letter.allocate(pb, "letter");
  hashpass.allocate(pb, "hashpass");
  problem.resize(9);
  solution.resize(9);
  for (int i = 0; i < 9; ++i){
    problem[i].allocate(pb, 9, "problem_" + std::to_string(i));
  }
   for (int i = 0; i < 9; ++i){
    solution[i].allocate(pb, 9, "solution_" + std::to_string(i));
  }

  // allocate private inputs
  zeros.allocate(pb, 9, "zeros-");
  consistency.allocate(pb, 9, "consistency-");
  numbers.allocate(pb, "numbers");
  diff1.allocate(pb, "diff1");
  invdiff1.allocate(pb, "invdiff1");
  diff2.allocate(pb, "diff2");
  invdiff2.allocate(pb, "invdiff2");
  diff3.allocate(pb, "diff3");
  invdiff3.allocate(pb, "invdiff3");
  quotient.allocate(pb, "quotient");
  pre_result.allocate(pb, "pre_result");
  real_letter.allocate(pb, "real_letter");
  password.allocate(pb, 1, "password");

  pb.set_input_sizes(83);

  // =========================DNI==============================================

  // Add Contraints DNI is different from 99999999
  pb.add_r1cs_constraint(r1cs_constraint<FieldT>(1, numbers - FieldT(99999999), diff1));
  pb.add_r1cs_constraint(r1cs_constraint<FieldT>(diff1, invdiff1, FieldT::one()));

  // Add Contraints DNI is different from 00000000
  pb.add_r1cs_constraint(r1cs_constraint<FieldT>(1, numbers - FieldT(00000000), diff2));
  pb.add_r1cs_constraint(r1cs_constraint<FieldT>(diff2, invdiff2, FieldT::one()));

  // Add Contraints DNI is different from 00000001
  pb.add_r1cs_constraint(r1cs_constraint<FieldT>(1, numbers - FieldT(00000001), diff3));
  pb.add_r1cs_constraint(r1cs_constraint<FieldT>(diff3, invdiff3, FieldT::one()));

  // Add mod 23
  FieldT mod23 = FieldT(23);
  pb.add_r1cs_constraint(r1cs_constraint<FieldT>(quotient, mod23, pre_result));
  pb.add_r1cs_constraint(r1cs_constraint<FieldT>(1, numbers - pre_result, real_letter));

  // Add Constraint Letters are equal
  pb.add_r1cs_constraint(r1cs_constraint<FieldT>(1, real_letter - letter, FieldT::zero()));

  
  pb.val(letter) = FieldT(dni_letter);
  pb.val(numbers) = FieldT(dni_numbers);

  // Add Validation DNI is different from 99999999
  FieldT diff1_val = pb.val(numbers) - FieldT(99999999);
  pb.val(diff1) = diff1_val;
  pb.val(invdiff1) = diff1_val.inverse();

  // Add Validation DNI is different from 00000000
  FieldT diff2_val = pb.val(numbers) - FieldT(00000000);
  pb.val(diff2) = diff2_val;
  pb.val(invdiff2) = diff2_val.inverse();

  // Add Validation DNI is different from 00000001
  FieldT diff3_val = pb.val(numbers) - FieldT(00000001);
  pb.val(diff3) = diff3_val;
  pb.val(invdiff3) = diff3_val.inverse();

  // Add mod 23
  int quotient_val = dni_numbers * 0.043478261;
  pb.val(quotient) = FieldT(quotient_val);
  pb.val(pre_result) = pb.val(quotient) * mod23;
  pb.val(real_letter) = FieldT(dni_numbers % 23);

  // =========================PASSWORD==============================================

  poseidon_gadget<FieldT> gad(pb, password, hashpass, 1);

  gad.generate_r1cs_constraints();

  pb.val(password[0]) = pass;
  
  gad.generate_r1cs_witness();


  // =========================SUDOKU==============================================

  for (int j = 0; j < 9; j++){
    for (int i = 0; i < 9; i++){
      pb.val(problem[j][i]) = problem_data[j][i];
      pb.val(solution[j][i]) = solution_data[j][i];
    }
    pb.val(zeros[j]) = FieldT::zero();
  }

  // consistency problem sudoku equal solution sudoku or 0's
  for (int i = 0; i < 9; i++){
    compare_or_gadget<FieldT> gad(pb, 9, problem[i], solution[i], zeros, consistency[i]);
    gad.generate_r1cs_constraints();
    gad.generate_r1cs_witness();
    pb.add_r1cs_constraint(r1cs_constraint<FieldT>(consistency[i], 1, FieldT::one()));
  }

  int box_control = 0;

  for (int i = 0; i < 9; i++){
    // check filas sum45, mul362880, exp285
    check_ops_gadget<FieldT> gadRows(pb, 9, solution_data[i], 45, 362880, 285);
    gadRows.generate_r1cs_constraints();
    gadRows.generate_r1cs_witness(); 

    // check columnas sum45, mul362880, exp285
    std::vector<FieldT> cols(9);
    cols[0] = solution_data[0][i];
    cols[1] = solution_data[1][i];
    cols[2] = solution_data[2][i];
    cols[3] = solution_data[3][i];
    cols[4] = solution_data[4][i];
    cols[5] = solution_data[5][i];
    cols[6] = solution_data[6][i];
    cols[7] = solution_data[7][i];
    cols[8] = solution_data[8][i];

    check_ops_gadget<FieldT> gadCols(pb, 9, cols, 45, 362880, 285);
    gadCols.generate_r1cs_constraints();
    gadCols.generate_r1cs_witness(); 

    // check boxes sum45, mul362880, exp285
    std::vector<FieldT> box(9);
    if (i < 3){
      box[0] = solution_data[0][i + box_control];
      box[1] = solution_data[0][i + 1 + box_control];
      box[2] = solution_data[0][i + 2 + box_control];
      box[3] = solution_data[1][i + box_control];
      box[4] = solution_data[1][i + 1 + box_control];
      box[5] = solution_data[1][i + 2 + box_control];
      box[6] = solution_data[2][i + box_control];
      box[7] = solution_data[2][i + 1 + box_control];
      box[8] = solution_data[2][i + 2 + box_control];
    }
    if (i > 2 && i < 6){
      box[0] = solution_data[3][i - 3 + box_control];
      box[1] = solution_data[3][i - 2 + box_control];
      box[2] = solution_data[3][i - 1 + box_control];
      box[3] = solution_data[4][i - 3 + box_control];
      box[4] = solution_data[4][i - 2 + box_control];
      box[5] = solution_data[4][i - 1 + box_control];
      box[6] = solution_data[5][i - 3 + box_control];
      box[7] = solution_data[5][i - 2 + box_control];
      box[8] = solution_data[5][i - 1 + box_control];
    }
    if (i > 5) {
      box[0] = solution_data[6][i - 6 + box_control];
      box[1] = solution_data[6][i - 5 + box_control];
      box[2] = solution_data[6][i - 4 + box_control];
      box[3] = solution_data[7][i - 6 + box_control];
      box[4] = solution_data[7][i - 5 + box_control];
      box[5] = solution_data[7][i - 4 + box_control];
      box[6] = solution_data[8][i - 6 + box_control];
      box[7] = solution_data[8][i - 5 + box_control];
      box[8] = solution_data[8][i - 4 + box_control];
    }

    check_ops_gadget<FieldT> gadBoxs(pb, 9, box, 45, 362880, 285);
    gadBoxs.generate_r1cs_constraints();
    gadBoxs.generate_r1cs_witness(); 

    box_control = box_control + 2;
    box_control = box_control > 4 ? 0 : box_control;
  }


  const r1cs_constraint_system<FieldT> constraint_system = pb.get_constraint_system();

  const r1cs_ppzksnark_keypair<default_r1cs_ppzksnark_pp> keypair = r1cs_ppzksnark_generator<default_r1cs_ppzksnark_pp>(constraint_system);

  const r1cs_ppzksnark_proof<default_r1cs_ppzksnark_pp> proof = r1cs_ppzksnark_prover<default_r1cs_ppzksnark_pp>(keypair.pk, pb.primary_input(), pb.auxiliary_input());

  bool verified = r1cs_ppzksnark_verifier_strong_IC<default_r1cs_ppzksnark_pp>(keypair.vk, pb.primary_input(), proof);

  cout << "Number of R1CS constraints: " << constraint_system.num_constraints() << endl;
  cout << "Primary (public) input: " << pb.primary_input() << endl;
  cout << "Auxiliary (private) input: " << pb.auxiliary_input() << endl;
  cout << "Verification status: " << verified << endl;

  const r1cs_ppzksnark_verification_key<default_r1cs_ppzksnark_pp> vk = keypair.vk;

  print_vk_to_file<default_r1cs_ppzksnark_pp>(vk, "/proof/vk_data");
  print_proof_to_file<default_r1cs_ppzksnark_pp>(proof, "/proof/proof_data");

  return 0;
}