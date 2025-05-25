#ifndef CHECK_OPS_HPP_
#define CHECK_OPS_HPP_

#include <cassert>
#include <memory>

#include <libsnark/gadgetlib1/gadget.hpp>

namespace libsnark {

template<typename FieldT>

    // CHECK IF A ARRAY COMPLIES WITH A SUM [1] + [2] + .. [size - 1] = SUMVAL, MUL [1] * [2] * .. [size - 1] = MULVAL & EXP [1]^2 + [2]^2 + ... [size-1]^2 = EXPVAL
    // IF NOT COMPLAINT INVALID CIRCUIT
    class check_ops_gadget : public gadget<FieldT> {
    private:
        pb_variable_array<FieldT> resultsSum;
        pb_variable_array<FieldT> resultsMult;
        pb_variable_array<FieldT> resultsExp;

    public:
        const std::vector<FieldT> input;
        const int size;
        const int sumval;
        const int multval;
        const int expval;

        check_ops_gadget(protoboard<FieldT>& pb,
                       const int size,
                       const std::vector<FieldT> &input,
                       const int sumval,
                       const int multval,
                       const int expval,
                       const std::string &annotation_prefix="") :
        gadget<FieldT>(pb, annotation_prefix), size(size), input(input), sumval(sumval), multval(multval), expval(expval) 
        {
            resultsSum.allocate(pb, size, "resultsSum");
            resultsMult.allocate(pb, size, "resultsMult");
            resultsExp.allocate(pb, size * 2, "resultsExp");
        }

    void generate_r1cs_constraints();
    void generate_r1cs_witness();
    void sum();
    void sumW();
    void mult();
    void multW();
    void exp();
    void expW();
};  
}

#include "check-ops/check_ops.tcc"

#endif 