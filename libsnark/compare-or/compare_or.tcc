#ifndef COMPARE_OR_TCC_
#define COMPARE_OR_TCC_

#include <libff/common/profiling.hpp>
#include <libff/common/utils.hpp>


namespace libsnark {

template<typename FieldT>
void compare_or_gadget<FieldT>::generate_r1cs_constraints()
{
    for (int i = 0; i < size; i++) {
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(inputs_A[i] - inputs_B[i], 1, diffAB[i]));
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(inputs_A[i] - inputs_C[i], 1, diffAC[i]));
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(diffAB[i], invdiffAB[i], resultsAB[i]));
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(diffAC[i], invdiffAC[i], resultsAC[i]));
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(resultsAB[i], resultsAC[i], resultsABC[i]));
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(FieldT::one() - resultsABC[i], results[i], results[i + 1]));
    }

    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(results[size], 1, output));

}

template<typename FieldT>
void compare_or_gadget<FieldT>::generate_r1cs_witness()
{
    this->pb.val(results[0]) = 1;
    for (int i = 0; i < size; i++) {
        FieldT diff = this->pb.val(inputs_A[i]) - this->pb.val(inputs_B[i]);
        FieldT diff1 = this->pb.val(inputs_A[i]) - this->pb.val(inputs_C[i]);
        this->pb.val(diffAB[i]) = diff;
        this->pb.val(diffAC[i]) = diff1;
        this->pb.val(invdiffAB[i]) = diff.inverse();
        this->pb.val(invdiffAC[i]) = diff1.inverse();
        this->pb.val(resultsAB[i]) =  this->pb.val(diffAB[i]) * this->pb.val(invdiffAB[i]);
        this->pb.val(resultsAC[i]) =  this->pb.val(diffAC[i]) * this->pb.val(invdiffAC[i]);
        this->pb.val(resultsABC[i]) = this->pb.val(resultsAB[i]) * this->pb.val(resultsAC[i]);
        this->pb.val(results[i + 1]) = (FieldT::one() - this->pb.val(resultsABC[i])) * this->pb.val(results[i]);
    }
    this->pb.val(output) = this->pb.val(results[size]);
    
}


}

#endif