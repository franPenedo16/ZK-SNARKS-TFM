FROM ubuntu:16.04

WORKDIR /root

RUN apt-get update && \
    apt-get install -y \
    wget unzip curl \
    build-essential cmake git libgmp3-dev libprocps4-dev python-markdown libboost-all-dev libssl-dev pkg-config

RUN apt-get install bc

RUN git clone https://github.com/coder5876/libsnark-tutorial.git \
  && cd libsnark-tutorial \
  && git submodule init && git submodule update


RUN cd libsnark-tutorial/depends/libsnark \
  && git checkout master \
  && git pull origin master \
  && git submodule init && git submodule update 


RUN git clone https://github.com/Tencent/rapidjson.git \
  && cd rapidjson \
  && cp -r include/rapidjson ../libsnark-tutorial/src/ 


COPY game.cpp libsnark-tutorial/src/
COPY Src-CMakeLists.txt libsnark-tutorial/src/CMakeLists.txt
COPY Parent-CMakeLists.txt libsnark-tutorial/CMakeLists.txt
COPY compare-or/compare_or.hpp libsnark-tutorial/src/compare-or/
COPY compare-or/compare_or.tcc  libsnark-tutorial/src/compare-or/compare_or.tcc
COPY check-ops/check_ops.hpp libsnark-tutorial/src/check-ops/
COPY check-ops/check_ops.tcc  libsnark-tutorial/src/check-ops/check_ops.tcc
COPY poseidon/poseidon.hpp libsnark-tutorial/src/poseidon/
COPY poseidon/poseidon_constants.hpp libsnark-tutorial/src/poseidon/
COPY poseidon/poseidon_constants.tcc libsnark-tutorial/src/poseidon/poseidon_constants.tcc
COPY poseidon/poseidon.tcc libsnark-tutorial/src/poseidon/poseidon.tcc


RUN cd libsnark-tutorial \
 && mkdir build && cd build && cmake .. \
 && make

COPY data.json libsnark-tutorial/src/


RUN mkdir /stats
COPY  stats/memory-pss-usage.sh libsnark-tutorial/src/
COPY  stats/memory-rss-usage.sh libsnark-tutorial/src/
COPY  stats/elapsed_secs.sh libsnark-tutorial/src/




