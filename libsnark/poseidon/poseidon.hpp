#ifndef POSEIDON_HPP_
#define POSEIDON_HPP_

#include <cassert>
#include <memory>

#include <libsnark/gadgetlib1/gadget.hpp>

namespace libsnark {

template<typename FieldT>

    class poseidon_gadget : public gadget<FieldT> {
    private:
        const std::vector<int> N_ROUNDS_P = {56, 57, 56, 60, 60, 63, 64, 63, 60, 66, 60, 65, 70, 60, 64, 68};
        const int nRoundsF = 8;
        int t;
        int nRoundsP;
        std::vector<pb_variable_array<FieldT>> state;
        pb_variable_array<FieldT> placeholder_sigma;

    public:
        const pb_variable_array<FieldT> inputs;
        const pb_variable<FieldT> output;

        poseidon_gadget(protoboard<FieldT>& pb,
                       const pb_variable_array<FieldT> &inputs,
                       const pb_variable<FieldT> &output,
                       const int nInputs,
                       const std::string &annotation_prefix="") :
        gadget<FieldT>(pb, annotation_prefix), inputs(inputs), output(output)
        {
            state.resize(256);
            t = nInputs + 1;
            nRoundsP = N_ROUNDS_P[t - 2];
            placeholder_sigma.allocate(pb, 256, "sigmas");
            for (int i = 0; i < 256; i++){
                state[i].allocate(pb, t, "state_" + std::to_string(i));
            }
        }

    void generate_r1cs_constraints();
    void generate_r1cs_witness();
    void ark(const std::vector<FieldT> C, int r, int count);
    void arkW(const std::vector<FieldT> C, int r, int count);
    void sigma(int count, int sigma_count, int i);
    void sigmaW(int count, int sigma_count, int i);
    void mix(const std::vector<std::vector<FieldT>> M, int count);
    void mixW(const std::vector<std::vector<FieldT>> M, int count);
    void mixs(const std::vector<FieldT> S, int r, int count);
    void mixsW(const std::vector<FieldT> S, int r, int count);
    void mixlast(const std::vector<std::vector<FieldT>> M, int outs, int count);
    void mixlastW(const std::vector<std::vector<FieldT>> M, int outs, int count);
    
};  
}

#include "poseidon/poseidon.tcc"

#endif 