#ifndef _OFDM_H_
#define _OFDM_H_

#include <assert.h>
#define OFDM_IQ_WIDTH        8
#define OFDM_MAX_NDBPS       216
#define OFDM_MAX_NCBPS       288

#include <hls_stream.h>
#define _XKEYCHECK_H
#include "ap_int.h"
using namespace hls;

typedef struct _OFDM_CFG_
{
  unsigned int LENGTH;
  unsigned int DATARATE;
}ofdm_cfg_t;

typedef ap_uint <OFDM_MAX_NDBPS>   ofdm_databits_t;
typedef ap_uint <OFDM_MAX_NCBPS>   ofdm_codedbits_t;

typedef struct _OFDM_CONV_DATA_
{
  ap_uint<6> A;
  ap_uint<6> B;
}ofdm_conv_data_t;
//typedef struct _OFDM_SIGDAT_
//{
//  bool                  IsSig   ;
//  ap_uint <MAX_NDBPS>   DataBits;
//}ofdm_databits_t;

#endif