#ifndef POSEIDON_TCC_
#define POSEIDON_TCC_

#include <libff/common/profiling.hpp>
#include <libff/common/utils.hpp>
#include "poseidon_constants.hpp"


namespace libsnark {

template<typename FieldT>
void poseidon_gadget<FieldT>::ark(const std::vector<FieldT> C, int r, int count){

    for (int j = 0; j < t; j++){
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(state[count][j] + C[j + r], 1, state[count+1][j]));
    };
}
    
template<typename FieldT>
void poseidon_gadget<FieldT>::arkW(const std::vector<FieldT> C, int r, int count){

    for (int j = 0; j < t; j++){
        this->pb.val(state[count + 1][j]) = this->pb.val(state[count][j]) + C[j + r];
    };   
}

template<typename FieldT>
void poseidon_gadget<FieldT>::sigma(int i, int sigma_count, int count){

    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(state[count][i], state[count][i], placeholder_sigma[sigma_count]));
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(placeholder_sigma[sigma_count], placeholder_sigma[sigma_count], placeholder_sigma[sigma_count + 1]));
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(placeholder_sigma[sigma_count + 1], state[count][i], state[count + 1][i]));

}
    
template<typename FieldT>
void poseidon_gadget<FieldT>::sigmaW(int i, int sigma_count, int count){

    this->pb.val(placeholder_sigma[sigma_count]) = this->pb.val(state[count][i]) * this->pb.val(state[count][i]);
    this->pb.val(placeholder_sigma[sigma_count + 1]) = this->pb.val(placeholder_sigma[sigma_count]) * this->pb.val(placeholder_sigma[sigma_count]);
    this->pb.val(state[count + 1][i]) = this->pb.val(placeholder_sigma[sigma_count + 1]) * this->pb.val(state[count][i]);

}

template<typename FieldT>
void poseidon_gadget<FieldT>::mix(const std::vector<std::vector<FieldT>> M, int count){

    for (int i = 0; i < t; i++){
        linear_combination<FieldT> lc(0);
        for (int j = 0; j < t; j++){
            lc.add_term(M[j][i] * state[count][j]);
        }
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(lc, 1, state[count + 1][i]));
    };
    
}

template<typename FieldT>
void poseidon_gadget<FieldT>::mixW(const std::vector<std::vector<FieldT>> M, int count){

    FieldT lc;
    for (int i = 0; i < t; i++){
        lc = FieldT::zero();
        for (int j = 0; j < t; j++){
            lc += M[j][i] * this->pb.val(state[count][j]);
        }
        this->pb.val(state[count + 1][i]) = lc;
    };
    
}

template<typename FieldT>
void poseidon_gadget<FieldT>::mixs(const std::vector<FieldT> S, int r, int count){
    
    linear_combination<FieldT> lc(0);
    for (int i = 0; i < t; i++){
        if (i == 0){
            lc.add_term(S[(t * 2 - 1) * r + 0] * state[count][0]);
        }else{
            lc.add_term(S[(t * 2 - 1) * r + i] * state[count - 2][i]);
        }
    }
    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(lc, 1, state[count + 1][0]));

    for (int i = 1; i < t; i++){
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(state[count - 2][i] + state[count][0] * S[(t * 2 - 1) * r + t + i - 1], 1,state[count + 1][i]));
    }
    
}

template<typename FieldT>
void poseidon_gadget<FieldT>::mixsW(const std::vector<FieldT> S, int r, int count){
    
    FieldT lc = FieldT::zero();
    for (int i = 0; i < t; i++){
        if (i == 0){
            lc += S[(t * 2 - 1) * r + 0] * this->pb.val(state[count][0]);
        }else{
            lc += S[(t * 2 - 1) * r + i] * this->pb.val(state[count - 2][i]);
        }
    }
    this->pb.val(state[count + 1][0]) = lc;

    for (int i = 1; i < t; i++){
        this->pb.val(state[count + 1][i]) =  this->pb.val(state[count - 2][i]) +  this->pb.val(state[count][0]) * S[(t * 2 - 1) * r + t + i - 1];
    }
    
}

template<typename FieldT>
void poseidon_gadget<FieldT>::mixlast(const std::vector<std::vector<FieldT>> M, int outs, int count){

    for (int s = 0; s < outs; s++){
        linear_combination<FieldT> lc(0);
        for (int i = 0; i < t; i++){
            lc.add_term(M[i][s] * state[count][i]);
        };

        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(lc, 1, state[count + 1 + s][0]));
    }
}

template<typename FieldT>
void poseidon_gadget<FieldT>::mixlastW(const std::vector<std::vector<FieldT>> M, int outs, int count){

    for (int s = 0; s < outs; s++){
        FieldT lc = FieldT::zero();
        for (int i = 0; i < t; i++){
            lc += M[i][s] * this->pb.val(state[count][i]);
        };

        this->pb.val(state[count + 1 + s][0]) = lc;
    }
}

template<typename FieldT>
void poseidon_gadget<FieldT>::generate_r1cs_constraints()
{
    poseidon_constants<FieldT> cons;
    auto const& C = cons.getC(2);
    auto const& M = cons.getM(2);
    auto const& P = cons.getP(2);
    auto const& S = cons.getS(2);
    int sigma_count = 0;
    int counting_steps = 0;

    for (int j = 0; j < t; j++){
        if (j==0){
            this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(FieldT::zero(), 1, state[0][0]));
        }else{
            this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(inputs[j - 1], 1, state[0][j]));
        }
    };

    ark(C, 0, counting_steps);
    counting_steps += 1;

    for (int j = 0; j < nRoundsF / 2 - 1; j++) {

        for (int i = 0; i < t; i++){
            sigma(i, sigma_count, counting_steps);
            sigma_count += 2;
        }
        counting_steps += 1;

        ark(C, (j + 1)*t, counting_steps);
        counting_steps += 1;

        mix(M, counting_steps);
        counting_steps += 1;
    }

    for (int i = 0; i < t; i++){
        sigma(i, sigma_count, counting_steps);
        sigma_count += 2;
    }
    counting_steps += 1;

    ark(C, nRoundsF / 2 * t, counting_steps);
    counting_steps += 1;
    
    mix(P, counting_steps);
    counting_steps += 1;

    for (int j = 0; j < nRoundsP; j++){
        sigma(0, sigma_count, counting_steps);
        sigma_count += 2;
        counting_steps += 1;
        
        this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(state[counting_steps][0] + C[(nRoundsF / 2 + 1) * t + j], 1, state[counting_steps + 1][0]));
        counting_steps += 1;

        mixs(S, j, counting_steps);
        counting_steps += 1;
    }

    for (int j = 0; j < nRoundsF / 2 - 1; j++){

        for (int i = 0; i < t; i++){
            sigma(i, sigma_count, counting_steps);
            sigma_count += 2;
        }
        counting_steps += 1;

        ark(C, (nRoundsF / 2 + 1) * t + nRoundsP + j * t, counting_steps);
        counting_steps += 1;

        mix(M, counting_steps);
        counting_steps += 1;
    }

    for (int i = 0; i < t; i++){
        sigma(i, sigma_count, counting_steps);
        sigma_count += 2;
    }
    counting_steps += 1;


    // CAMBIAR SI SE USAN MAS SALIDAS, EL VALOR 1 POR nOUTS, y cambiar salida hash a pb-array hash, cada counting_steps sig tendria la salida siguiente
    mixlast(M, 1, counting_steps);
    counting_steps += 1;

    this->pb.add_r1cs_constraint(r1cs_constraint<FieldT>(state[counting_steps][0], 1, output));
    
}

template<typename FieldT>
void poseidon_gadget<FieldT>::generate_r1cs_witness()
{
    poseidon_constants<FieldT> cons;
    auto const& C = cons.getC(2);
    auto const& M = cons.getM(2);
    auto const& P = cons.getP(2);
    auto const& S = cons.getS(2);
    int sigma_count = 0;
    int counting_steps = 0;

    for (int j = 0; j < t; j++){
        if (j==0){
            this->pb.val(state[0][0]) = FieldT::zero();
        }else{
            this->pb.val(state[0][j]) = this->pb.val(inputs[j - 1]);
        }
    };
    
    arkW(C, 0, counting_steps);
    counting_steps += 1;

    for (int j = 0; j < nRoundsF / 2 - 1; j++) {

        for (int i = 0; i < t; i++){
            sigmaW(i, sigma_count, counting_steps);
            sigma_count += 2;
        }
        counting_steps += 1;

        arkW(C, (j + 1)*t, counting_steps);
        counting_steps += 1;

        mixW(M, counting_steps);
        counting_steps += 1;
    }

    for (int i = 0; i < t; i++){
        sigmaW(i, sigma_count, counting_steps);
        sigma_count += 2;
    }
    counting_steps += 1;

    arkW(C, nRoundsF / 2 * t, counting_steps);
    counting_steps += 1;
    
    mixW(P, counting_steps);
    counting_steps += 1;

    for (int j = 0; j < nRoundsP; j++){
        sigmaW(0, sigma_count, counting_steps);
        sigma_count += 2;
        counting_steps += 1;
        
        this->pb.val(state[counting_steps + 1][0]) = this->pb.val(state[counting_steps][0]) + C[(nRoundsF / 2 + 1) * t + j];
        counting_steps += 1;

        mixsW(S, j, counting_steps);
        counting_steps += 1;
    }

    for (int j = 0; j < nRoundsF / 2 - 1; j++){

        for (int i = 0; i < t; i++){
            sigmaW(i, sigma_count, counting_steps);
            sigma_count += 2;
        }
        counting_steps += 1;

        arkW(C, (nRoundsF / 2 + 1) * t + nRoundsP + j * t, counting_steps);
        counting_steps += 1;

        mixW(M, counting_steps);
        counting_steps += 1;
    }

    for (int i = 0; i < t; i++){
        sigmaW(i, sigma_count, counting_steps);
        sigma_count += 2;
    }
    counting_steps += 1;

    // CAMBIAR SI SE USAN MAS SALIDAS, EL VALOR 1 POR nOUTS, y cambiar salida hash a pb-array hash, cada i step[counting_steps][i] tendria la salida seria la salida por posicion
    mixlastW(M, 1, counting_steps);
    counting_steps += 1;

    this->pb.val(output) = this->pb.val(state[counting_steps][0]);
    
}


}

#endif