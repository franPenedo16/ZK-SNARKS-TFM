#ifndef CHECK_OPS_TCC_
#define CHECK_OPS_TCC_

#include <libff/common/profiling.hpp>
#include <libff/common/utils.hpp>


namespace libsnark {

template<typename FieldT>
void check_ops_gadget<FieldT>::sum()
{
    for (int i = 1; i < size; i++){
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(resultsSum[i - 1] + input[i], 1, resultsSum[i]));
    }
}

template<typename FieldT>
void check_ops_gadget<FieldT>::mult()
{
    for (int i = 1; i < size; i++){
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(resultsMult[i - 1], input[i], resultsMult[i]));
    }
}

template<typename FieldT>
void check_ops_gadget<FieldT>::exp()
{
    for (int i = 1; i < size; i++){
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(input[i], input[i], resultsExp[i]));
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(resultsExp[i] + resultsExp[i - 1 + size], 1, resultsExp[i + size]));
    }
}



template<typename FieldT>
void check_ops_gadget<FieldT>::sumW()
{
    for (int i = 1; i < size; i++){
        this->pb.val(resultsSum[i]) = this->pb.val(resultsSum[i - 1]) + input[i];
    }
}

template<typename FieldT>
void check_ops_gadget<FieldT>::multW()
{
    for (int i = 1; i < size; i++){
        this->pb.val(resultsMult[i]) = this->pb.val(resultsMult[i - 1]) * input[i];
    }
}

template<typename FieldT>
void check_ops_gadget<FieldT>::expW()
{
    for (int i = 1; i < size; i++){
        this->pb.val(resultsExp[i]) = input[i] * input[i];
        this->pb.val(resultsExp[i + size]) = this->pb.val(resultsExp[i]) + this->pb.val(resultsExp[i - 1 + size]);
    }
}


template<typename FieldT>
void check_ops_gadget<FieldT>::generate_r1cs_constraints()
{

    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(input[0], 1, resultsSum[0]));
    sum();
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(sumval, 1, resultsSum[size-1]));

    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(input[0], 1, resultsMult[0]));
    mult();
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(multval, 1, resultsMult[size-1]));

    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(input[0],  input[0], resultsExp[0]));
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(input[0],  input[0], resultsExp[size]));
    exp();
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(expval, 1, resultsExp[(size * 2) - 1]));

}

template<typename FieldT>
void check_ops_gadget<FieldT>::generate_r1cs_witness()
{
    this->pb.val(resultsSum[0]) = input[0];
    sumW();

    this->pb.val(resultsMult[0]) = input[0];
    multW();

    this->pb.val(resultsExp[0]) = input[0] * input[0];
    this->pb.val(resultsExp[size]) = input[0] * input[0];
    expW();

}


}

#endif