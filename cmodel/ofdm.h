#ifndef _OFDM_H_
#define _OFDM_H_

#include <assert.h>
#define OFDM_IQ_WIDTH   8
#define MAX_NDBPS       216

#include <hls_stream.h>
#define _XKEYCHECK_H
#include "ap_int.h"
using namespace hls;

typedef struct _OFDM_CFG_
{
  unsigned int LENGTH;
  unsigned int DATARATE;
}ofdm_cfg_t;

typedef ap_uint <MAX_NDBPS>   ofdm_databits_t;

//typedef struct _OFDM_SIGDAT_
//{
//  bool                  IsSig   ;
//  ap_uint <MAX_NDBPS>   DataBits;
//}ofdm_databits_t;

#endif