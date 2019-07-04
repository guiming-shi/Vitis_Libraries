/*
 * Copyright 2019 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file dot.h
 * @brief BLAS Level 1 asum template function implementation.
 *
 * This file is part of XF BLAS Library.
 */

#ifndef XF_BLAS_DOT_H
#define XF_BLAS_DOT_H


#ifndef __cplusplus
#error "BLAS Library only works with C++."
#endif

#include "ap_int.h"
#include "hls_stream.h"
#include "sum.h"
#include "xf_blas/utility.h"


namespace xf {
namespace linear_algebra {
namespace blas {

  namespace {
    template<typename t_DataType, 
      unsigned int t_ParEntries, 
      unsigned int t_DataWidth = sizeof(t_DataType) << 3, 
      typename t_IndexType=unsigned int
    >
    void mul(
        unsigned int p_n,
        hls::stream<WideType<t_DataType, t_ParEntries, t_DataWidth> > & p_x,
        hls::stream<WideType<t_DataType, t_ParEntries, t_DataWidth> > & p_y,
        hls::stream<WideType<t_DataType, t_ParEntries, t_DataWidth> > & p_res
    ){
      #ifndef __SYNTHESIS__
        assert(p_n % t_ParEntries == 0);
      #endif
      t_IndexType l_numParEntries = p_n / t_ParEntries;
      for (t_IndexType i=0; i<l_numParEntries; ++i) {
      #pragma HLS PIPELINE
        WideType<t_DataType, t_ParEntries, t_DataWidth> l_valX;
        WideType<t_DataType, t_ParEntries, t_DataWidth> l_valY;
        WideType<t_DataType, t_ParEntries, t_DataWidth> l_valRes;
        #pragma HLS ARRAY_PARTITION variable=l_valX complete dim=1
        #pragma HLS ARRAY_PARTITION variable=l_valY complete dim=1
        #pragma HLS ARRAY_PARTITION variable=l_valRes complete dim=1
        l_valX = p_x.read();
        l_valY = p_y.read();
        for (unsigned int j=0; j<t_ParEntries; ++j) {
          l_valRes[j] = l_valX[j] * l_valY[j];
        }
        p_res.write(l_valRes);
      }
    }
  }

  /**
   * @brief dot function that returns the dot product of vector x and y.
   *
   * @tparam t_DataType the data type of the vector entries
   * @tparam t_DataWidth the datawidth of the datatype t_DataType of the input vector 
   * @tparam t_LogParEntries log2 of the number of parallelly processed entries in the input vector 
   * @tparam t_IndexType the datatype of the index 
   *
   * @param p_n the number of entries in the input vector p_x, p_n % l_ParEntries == 0
   * @param p_x the input stream of packed vector entries
   * @param p_res the dot product of x and y
   */

  template<typename t_DataType, 
    unsigned int t_LogParEntries, 
    unsigned int t_DataWidth = sizeof(t_DataType) << 3, 
    typename t_IndexType=unsigned int
  >
  void dot(
      unsigned int p_n,
      hls::stream<WideType<t_DataType, 1<<t_LogParEntries, t_DataWidth> > & p_x,
      hls::stream<WideType<t_DataType, 1<<t_LogParEntries, t_DataWidth> > & p_y,
      t_DataType &p_res
      ) {
    #ifndef __SYNTHESIS__
    assert(p_n % ( 1 << t_LogParEntries) == 0);
    #endif
    #pragma HLS DATAFLOW
    hls::stream<WideType<t_DataType, 1<<t_LogParEntries, t_DataWidth> > l_mulStr;
    mul<t_DataType, 1<<t_LogParEntries, t_DataWidth, t_IndexType>(p_n,p_x,p_y,l_mulStr);
    sum<t_DataType, t_LogParEntries, t_DataWidth, t_IndexType>(p_n, l_mulStr, p_res);    
  }


} //end namespace blas
} //end namspace linear_algebra
} //end namespace xf

#endif
