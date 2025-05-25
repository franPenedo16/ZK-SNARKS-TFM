#ifndef POSEIDON_CONSTANTS_TCC_
#define POSEIDON_CONSTANTS_TCC_

#include <libff/common/profiling.hpp>
#include <libff/common/utils.hpp>


namespace libsnark {

template<typename FieldT>
std::vector<std::vector<FieldT>> poseidon_constants<FieldT>::getM(std::size_t t){
    switch (t) {
        case 2:
            return M_t2;
        default:
            throw std::invalid_argument("PoseidonParameters: ancho t no soportado");
    }
}

template<typename FieldT>
std::vector<std::vector<FieldT>> poseidon_constants<FieldT>::getP(std::size_t t){
    switch (t) {
        case 2:
            return P_t2;
        default:
            throw std::invalid_argument("PoseidonParameters: ancho t no soportado");
    }
}

template<typename FieldT>
std::vector<FieldT> poseidon_constants<FieldT>::getC(std::size_t t){
    switch (t) {
        case 2:
            return C_t2;
        default:
            throw std::invalid_argument("PoseidonParameters: ancho t no soportado");
    }
}

template<typename FieldT>
std::vector<FieldT> poseidon_constants<FieldT>::getS(std::size_t t){
    switch (t) {
        case 2:
            return S_t2;
        default:
            throw std::invalid_argument("PoseidonParameters: ancho t no soportado");
    }
}
}

#endif