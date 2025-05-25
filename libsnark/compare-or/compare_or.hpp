#ifndef COMPARE_OR_HPP_
#define COMPARE_OR_HPP_

#include <cassert>
#include <memory>

#include <libsnark/gadgetlib1/gadget.hpp>

namespace libsnark {

template<typename FieldT>

    // Check IF A is equal to B or C --> 1 , IF A is no equal to B or C --> 0 
    // Every val in the array must be equal to val in b or val in c
    // a[i] == b[i] or a[i] == c[i]
    class compare_or_gadget : public gadget<FieldT> {
    private:
        pb_variable_array<FieldT> results;
        pb_variable_array<FieldT> diffAB;
        pb_variable_array<FieldT> invdiffAB;
        pb_variable_array<FieldT> resultsAB;
        pb_variable_array<FieldT> diffAC;
        pb_variable_array<FieldT> invdiffAC;
        pb_variable_array<FieldT> resultsABC;
        pb_variable_array<FieldT> resultsAC;

    public:
        const pb_variable_array<FieldT> inputs_A;
        const pb_variable_array<FieldT> inputs_B;
        const pb_variable_array<FieldT> inputs_C;
        const pb_variable<FieldT> output;
        const int size;

        compare_or_gadget(protoboard<FieldT>& pb,
                       const int size,
                       const pb_variable_array<FieldT> &inputs_A,
                       const pb_variable_array<FieldT> &inputs_B,
                       const pb_variable_array<FieldT> &inputs_C,
                       const pb_variable<FieldT> &output,
                       const std::string &annotation_prefix="") :
        gadget<FieldT>(pb, annotation_prefix), size(size), inputs_A(inputs_A), inputs_B(inputs_B), inputs_C(inputs_C), output(output)
        {
            results.allocate(pb, size + 1, "results");
            diffAB.allocate(pb, size, "diffAB");
            invdiffAB.allocate(pb, size, "invdiffAB");
            resultsAB.allocate(pb, size, "resultsAB");
            diffAC.allocate(pb, size, "diffAC");
            invdiffAC.allocate(pb, size, "invdiffAC");
            resultsAC.allocate(pb, size, "resultsAC");
            resultsABC.allocate(pb, size, "resultsABC");
        }

    void generate_r1cs_constraints();
    void generate_r1cs_witness();
};  
}

#include "compare-or/compare_or.tcc"

#endif 