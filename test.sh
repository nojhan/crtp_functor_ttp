#!/bin/bash

cmd="g++ -O3 -std=c++0x "
$cmd inheritance_functor.cpp -o inheritance_functor
echo "inheritance_functor"
time ./inheritance_functor

echo ""
echo "crtp_functor_ttp"
$cmd crtp_functor_ttp.cpp -o crtp_functor_ttp
time ./crtp_functor_ttp

