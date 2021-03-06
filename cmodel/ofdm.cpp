#include "ofdm.h"
unsigned int reverse(unsigned int src,int length){
  unsigned int data=0;
  for(int i=0;i<length;++i){
    data<<=1;
    data|=(src>>(length-i))&0x1;
  }
  return data;
}
unsigned int xor(unsigned int val){
  unsigned int data=0;
  for(int i=0;i<32;++i){
    data+=(val>>i)&0x1;
  }
  return (data&0x1);
}
void ofdm_sig_generator(
  unsigned int                      datarate,
  unsigned int                      length,
  stream <ap_uint<8>> &             SigBuffer
  ){
    ap_uint<8> SigData=0;
    ap_uint<12> DataLength=length;
    ap_uint<1>  parity=0;
    SigData.range(3,0)  =datarate; 
    SigData.range(7,5)  =DataLength.range(2,0);
    parity+=SigData[0]+SigData[1]+SigData[2]+SigData[3]+SigData[4]+SigData[5]+SigData[6]+SigData[7];
    SigBuffer.write(SigData);

    SigData=DataLength.range(10,3);
    parity+=SigData[0]+SigData[1]+SigData[2]+SigData[3]+SigData[4]+SigData[5]+SigData[6]+SigData[7];
    SigBuffer.write(SigData);
   
    SigData.range(0,0)=DataLength.range(11,11);
    parity+=SigData[0];
    SigData.range(7,1)=parity;
    SigBuffer.write(SigData);
}

/*
Scrambler 

for genpoly x^7+x^4+1, X(N+1)=M*X(N)=>X(N+8)=M^8*X(N)
with M^8= 
     0     1     0     0     1     0     0     0
     0     0     1     0     0     1     0     0
     0     0     0     1     0     0     1     0
     0     0     0     0     1     0     0     1
     0     1     0     0     1     1     0     0
     0     0     1     0     0     1     1     0
     0     0     0     1     0     0     1     1
     0     1     0     0     0     0     0     1
*/
void ofdm_data_generator(
      unsigned int              Nbyte,         //Bytes number of DATA
      unsigned int              Length,        //Data Length
      stream<unsigned char >  & DataBuffer,
      stream<ap_uint<8> >     & ScrambBuffer
  ){
    ap_uint<8> shift_reg=0x7f;
    for(int i=0;i<Nbyte;++i){
      ap_uint<8> data         ;
      ap_uint<8> shift_reg_pre= shift_reg;
      if((i>=2)&&(i<(Length+2))){
        data= DataBuffer.read();
      }else{
        data=0;
      }
      for(int j=0;j<=7;++j){
        data[j]=data[j]+shift_reg[j];
      }
      ScrambBuffer.write(data));
      shift_reg[7]  = shift_reg_pre[6]+shift_reg_pre[3];
      shift_reg[6]  = shift_reg_pre[5]+shift_reg_pre[2];
      shift_reg[5]  = shift_reg_pre[4]+shift_reg_pre[1];
      shift_reg[4]  = shift_reg_pre[3]+shift_reg_pre[0];
      shift_reg[3]  = shift_reg_pre[6]+shift_reg_pre[3]+shift_reg_pre[2];
      shift_reg[2]  = shift_reg_pre[5]+shift_reg_pre[2]+shift_reg_pre[1];
      shift_reg[1]  = shift_reg_pre[4]+shift_reg_pre[1]+shift_reg_pre[0];
      shift_reg[0]  = shift_reg_pre[6]+shift_reg_pre[0];
    }
}

void ofdm_conv_feeder(
    unsigned int Nbyte,
    stream <ap_uint<8>>   & SigDataBuffer,
    stream <ap_uint<6>>   & ConvolFeederDataBuffer
){
    unsigned int src_bit_cnt=0;
    unsigned int dst_bit_cnt=6;
    ap_uint<8> cur_src=0;
    ap_uint<8> old_src=0;
    ap_uint<6> data;
    while(src_bit_cnt<(Nbyte*8)){
      unsigned int flag=(dst_bit_cnt-6)%8;
      if(src_bit_cnt<dst_bit_cnt) {
          old_src =cur_src;
          cur_src=SigDataBuffer.read();
          src_bit_cnt+=8;
      }
      if(flag==0){
        data=cur_src.range(5,0);
      }else if(flag==6){
        data.range(1,0)=old_src.range(7,6);
        data.range(5,2)=cur_src.range(3,0);
      }else if(flag==4){
        data.range(3,0)=old_src.range(7,4);
        data.range(5,4)=cur_src.range(1,0);
      }else if(flag==2){
        data.range(5,0)=cur_src.range(7,2);
      }else{
        assert(!"Invalid Flag!\n");
      }
      ConvolFeederDataBuffer.write(data);
      dst_bit_cnt+=6;
    }
}
/*
void ofdm_data_generator(
   unsigned int                           Length        ,                      //Data Byte Length 
   unsigned int                           Ndbps         ,                      //Data Bits per symbol
   stream <unsigned char> &               DataBuffer    ,
   stream <ap_uint<6>> &                  ConvolFeederDataBuffer               //Data To Convolutional Encoder 
   ){
    unsigned int Nsym = ceil((16+8*Length+6)/Ndbps); // number of OFDM symbols
    unsigned int Ndata = Nsym*Ndbps;                 // number of bits in the DATA field
    stream <unsigned char> ScramblerDataBuffer;
    ofdm_data_scrambler(
        Ndata/8,          //Bytes number of Service Bits+PSDU+TAIL+PAD
        Length,           //Original Data Length
        DataBuffer,
        ScramblerDataBuffer
    );
    ofdm_conv_data_feeder(
            Ndata/8,
            ScramblerDataBuffer,
            ConvolFeederDataBuffer
    );
}
*/
void ofdm_sigdat_generator(
  //ofdm_cfg_t                TXVECTOR,
  unsigned int              datarate,
  unsigned int              length,
  unsigned int              Nbyte,

  stream<unsigned char > &  DataBuffer,
  stream <ap_uint<8>>   &   SigDataBuffer
){
    //unsigned int Ndbps=0; // Data bits per symbol
    //unsigned int datarate=0;
    stream<ap_uint<8>> SigBuffer;
    stream<ap_uint<8>> ScrambBuffer; 
    //switch (TXVECTOR.DATARATE){
    //case 6: datarate=0xb;Ndbps=24;break;
    //case 9: datarate=0xf;Ndbps=36;break;
    //case 12:datarate=0xa;Ndbps=48;break;
    //case 18:datarate=0xe;Ndbps=72;break;
    //case 24:datarate=0x9;Ndbps=116;break;
    //case 36:datarate=0xd;Ndbps=144;break;
    //case 48:datarate=0x8;Ndbps=192;break;
    //case 54:datarate=0xc;Ndbps=216;break;
    //default:;
    //}
    //unsigned int Nsym = ceil((16+8*TXVECTOR.LENGTH+6)/Ndbps); // number of OFDM symbols
    //unsigned int Ndata = Nsym*Ndbps;                 // number of bits in the DATA field
    ofdm_sig_generator(
      datarate,
      length,
      SigBuffer
    );
    ofdm_data_generator(
      Nbyte,         //Bytes number of Service+DATA+TAIL+PAD
      length,        //Data Length
      DataBuffer,
      ScrambBuffer
      );
    for(int i=0;i<3;++i){
      ap_uint<8> data=SigBuffer.read();
      SigDataBuffer.write(data);
    }
    for(int i=0;i<Nbyte;++i){
      ap_uint<8> data=ScrambBuffer.read();
      SigDataBuffer.write(data);
    }
}
void ofdm_conv_encoder(
  unsigned int Nconv                                  ,
  stream <ap_uint<6>>         & ConvolFeederDataBuffer,
  stream <ofdm_conv_data_t>   & ConvolEncDataBuffer
){
  ap_uint<6> shift_data=0;
  ap_uint<6> cur_data=0;
  ofdm_conv_data_t EncData;
  for(int i=0;i<Nconv;++i){
    shift_data=cur_data;
    cur_data=ConvolFeederDataBuffer.read();
    EncData.A[0]=cur_data[0]+shift_data[1]+shift_data[2]+shift_data[4]+shift_data[5];
    EncData.A[1]=cur_data[1]+shift_data[0]+shift_data[1]+shift_data[3]+shift_data[4];
    EncData.A[2]=cur_data[2]+cur_data[0]+shift_data[0]+shift_data[2]+shift_data[3];
    EncData.A[3]=cur_data[3]+cur_data[1]+cur_data[0]+shift_data[1]+shift_data[2];
    EncData.A[4]=cur_data[4]+cur_data[2]+cur_data[1]+shift_data[0]+shift_data[1];
    EncData.A[5]=cur_data[5]+cur_data[3]+cur_data[2]+cur_data[0]+shift_data[0];

    EncData.B[0]=cur_data[0]+shift_data[0]+shift_data[1]+shift_data[2]+shift_data[5];
    EncData.B[1]=cur_data[1]+cur_data[0]+shift_data[0]+shift_data[1]+shift_data[4];
    EncData.B[2]=cur_data[2]+cur_data[1]+cur_data[0]+shift_data[0]+shift_data[3];
    EncData.B[3]=cur_data[3]+cur_data[2]+cur_data[1]+cur_data[0]+shift_data[2];
    EncData.B[4]=cur_data[4]+cur_data[3]+cur_data[2]+cur_data[1]+shift_data[1];
    EncData.B[5]=cur_data[5]+cur_data[4]+cur_data[3]+cur_data[2]+shift_data[0];
    ConvolEncDataBuffer.write(EncData);
  }
}
void ofdm_conv_punc(
  unsigned int mode,    //0:1/2 1:2/3 2:3/4
  unsigned int Nconv,
  stream <ofdm_conv_data_t>   & ConvolEncDataBuffer,
  stream <ap_uint<12>>  &PuncDataBuffer 
){
  int k=0;
  ofdm_conv_data_t  ConvolData;
  for(int i=0;i<Nconv;++i){
    ap_uint<12> PuncData;
    ofdm_conv_data_t PreConvolData=ConvolData;
    ConvolData=ConvolEncDataBuffer.read();
    if(mode==0||(i<=3)){ //1/2 also for 24bits signals.
      PuncData[0]=ConvolData.A[0];
      PuncData[1]=ConvolData.B[0];
      PuncData[2]=ConvolData.A[1];
      PuncData[3]=ConvolData.B[1];
      PuncData[4]=ConvolData.A[2];
      PuncData[5]=ConvolData.B[2];
      PuncData[6]=ConvolData.A[3];
      PuncData[7]=ConvolData.B[3];
      PuncData[8]=ConvolData.A[4];
      PuncData[9]=ConvolData.B[4];
      PuncData[10]=ConvolData.A[5];
      PuncData[11]=ConvolData.B[5];
      PuncDataBuffer.write(PuncData);
    }else if(mode==1){
      PuncData[0]=ConvolData.A[0];
      PuncData[1]=ConvolData.B[0];
      PuncData[2]=ConvolData.A[1];
      PuncData[3]=ConvolData.A[2];
      PuncData[4]=ConvolData.B[2];
      PuncData[5]=ConvolData.A[3];
      PuncData[6]=ConvolData.A[4];
      PuncData[7]=ConvolData.B[4];
      PuncData[8]=ConvolData.A[5];
      PuncData[9]=0;
      PuncData[10]=0;
      PuncData[11]=0;
      PuncDataBuffer.write(PuncData);
    }else{
      if(k==1){
        PuncData[0]=PreConvolData.A[0];
        PuncData[1]=PreConvolData.B[0];
        PuncData[2]=PreConvolData.A[1];
        PuncData[3]=PreConvolData.B[2];
        PuncData[4]=PreConvolData.A[3];
        PuncData[5]=PreConvolData.B[3];
        PuncData[6]=PreConvolData.A[4];
        PuncData[7]=PreConvolData.B[5];
        PuncData[8]=ConvolData.A[0];
        PuncData[9]=ConvolData.B[0];
        PuncData[10]=ConvolData.A[1];
        PuncData[11]=ConvolData.B[2];
        PuncDataBuffer.write(PuncData);
      }else if(k==2){
        PuncData[0]=PreConvolData.A[3];
        PuncData[1]=PreConvolData.B[3];
        PuncData[2]=PreConvolData.A[4];
        PuncData[3]=PreConvolData.B[5];
        PuncData[4]=ConvolData.A[0];
        PuncData[5]=ConvolData.B[0];
        PuncData[6]=ConvolData.A[1];
        PuncData[7]=ConvolData.B[2];
        PuncData[8]=ConvolData.A[3];
        PuncData[9]=ConvolData.B[3];
        PuncData[10]=ConvolData.A[4];
        PuncData[11]=ConvolData.B[5];
        PuncDataBuffer.write(PuncData);
      }
      if(k==2){
        k=0;
      }else{
        ++k;
      }
    }
  }
}
void ofdm_databits_reshape(
  unsigned int Ncbps,                    //Coded Bits Per symbol
  unsigned int NPunc,                    //Punc Numbers
  unsigned int PuncMode,
  stream <ap_uint<12>>  &                PuncDataBuffer, 
  stream <ofdm_codedbits_t> &            CodedBitsBuffer 
){
  ofdm_codedbits_t  CodedBits;
  ap_uint<12> PuncData;
  int k=0;
  for(int i=0;i<4;++i){
    PuncData=PuncDataBuffer.read();
    CodedBits.range(i*12+5,i*12)=PuncData;
  }
  for(int i=4;i<NPunc;++i){
    PuncData=PuncDataBuffer.read();
    if(Ncbps==48){
      if(PuncMode==2){  //(48/12=4)
        CodedBits.range(12*k+11,12*k)=PuncData;
        if(k==3){
          CodedBitsBuffer.write(CodedBits);
          k=0;
        }else{
          ++k;
        }
      }else if(PuncMode==0){//(48/12=4)
        CodedBits.range(12*k+11,12*k)=PuncData;
        if(k==3){
          CodedBitsBuffer.write(CodedBits);
          k=0;
        }else{
          ++k;
        }
      }
    }else if(Ncbps==96){
      if(PuncMode==2){//(96/12=8)
        CodedBits.range(12*k+11,12*k)=PuncData;
        if(k==7){
          CodedBitsBuffer.write(CodedBits);
          k=0;
        }else{
          ++k;
        }
      }else if(PuncMode==0){//(96/12=8)
        CodedBits.range(12*k+11,12*k)=PuncData;
        if(k==7){
          CodedBitsBuffer.write(CodedBits);
          k=0;
        }else{
          ++k;
        }
      }
    }else if(Ncbps==192){
      if(PuncMode==2){//(192/12=16)
        CodedBits.range(12*k+11,12*k)=PuncData;
        if(k==15){
          CodedBitsBuffer.write(CodedBits);
          k=0;
        }else{
          ++k;
        }
      }else if(PuncMode==0){//(192/12=16)
        CodedBits.range(12*k+11,12*k)=PuncData;
        if(k==15){
          CodedBitsBuffer.write(CodedBits);
          k=0;
        }else{
          ++k;
        }
      }
    }else if(Ncbps==288){
      if(PuncMode==2){//(288/12=24)
        CodedBits.range(12*k+11,12*k)=PuncData;
        if(k==23){
          CodedBitsBuffer.write(CodedBits);
          k=0;
        }else{
          ++k;
        }
      }else if(PuncMode==1){//(288/9=32)
        CodedBits.range(9*k+11,9*k)=PuncData;
        if(k==31){
          CodedBitsBuffer.write(CodedBits);
          k=0;
        }else{
          ++k;
        }
      }
    }else{
      assert(0);
    }
  }
}
int interleaved_idx(
  int Ncbps,
  int k
){
  int i = (Ncbps/16)*(k%16)+k/16;
  int s = (Ncbps>=96)?(Ncbps/96):1;
  int j = (i/s)*s+(i+Ncbps-16*i/Ncbps)%s;
  return j;
}
void ofdm_interleaver(
  unsigned int Ncbps,                    //Coded Bits Per symbol
  unsigned int Nsym,
  stream <ofdm_codedbits_t>&   CodedBitsBuffer,
  stream <ofdm_codedbits_t>&   InterleavedBitsBuffer
){
  ofdm_codedbits_t coded_data;
  ofdm_codedbits_t interleaved_data;
  for(int i=0;i<Nsym;++i){
    coded_data=CodedBitsBuffer.read();
    if((i==0)||Ncbps==48){
      for(int k=0;k<48;++k){
        interleaved_data[interleaved_idx(48,k)]=coded_data[k];
      }
    }else if(Ncbps==96){
      for(int k=0;k<96;++k){
        interleaved_data[interleaved_idx(96,k)]=coded_data[k];
      }
    }else if(Ncbps==128){
      for(int k=0;k<128;++k){
        interleaved_data[interleaved_idx(128,k)]=coded_data[k];
      }
    }else if(Ncbps==192){
      for(int k=0;k<192;++k){
        interleaved_data[interleaved_idx(192,k)]=coded_data[k];
      }
    }else if(Ncbps==288){
      for(int k=0;k<288;++k){
        interleaved_data[interleaved_idx(288,k)]=coded_data[k];
      }
    }
    InterleavedBitsBuffer.write(interleaved_data);
  }
}
void ofdm_modulation_mapping(
  unsigned int Ncbps,                    //Coded Bits Per symbol
  unsigned int Nsym,
  stream <ofdm_codedbits_t>&   InterleavedBitsBuffer,
){

}
void ofdm_transmitter(
  ofdm_cfg_t                        TXVECTOR,
  stream <unsigned char> &          DataBuffer,
  stream <ap_int<OFDM_IQ_WIDTH>> &  ISymbolBuffer,
  stream <ap_int<OFDM_IQ_WIDTH>> &  QSymbolBuffer
  ) {
    unsigned int Ndbps=0; // Data bits per symbol
    unsigned int Ncbps=0;
    unsigned int datarate=0;
    unsigned int PuncMode=0;
    switch (TXVECTOR.DATARATE){
      case 6: datarate=0xb;Ndbps=24;Ncbps=48;break;
      case 9: datarate=0xf;Ndbps=36;Ncbps=48;PuncMode=2;break;
      case 12:datarate=0xa;Ndbps=48;Ncbps=96;break;
      case 18:datarate=0xe;Ndbps=72;Ncbps=96;PuncMode=2;break;
      case 24:datarate=0x9;Ndbps=116;Ncbps=192;break;
      case 36:datarate=0xd;Ndbps=144;Ncbps=192;PuncMode=2;break;
      case 48:datarate=0x8;Ndbps=192;Ncbps=288;PuncMode=1;break;
      case 54:datarate=0xc;Ndbps=216;Ncbps=288;PuncMode=0;break;
      default:;
    }
    unsigned int Nsym  = ceil((16+8*TXVECTOR.LENGTH+6)/Ndbps); // number of OFDM symbols
    unsigned int Ndata = Nsym*Ndbps;                 // number of bits in the DATA field
    unsigned int Nbyte = Ndata/8;
    unsigned int Nconv = Ndata/6+4;
    unsigned int NPunc =0;
    stream <ap_uint<8>>   SigDataBuffer;
    stream <ap_uint<6>>   ConvolFeederDataBuffer;
    stream <ofdm_conv_data_t>   ConvolEncDataBuffer;
    stream <ap_uint<12>>  PuncDataBuffer;
    stream <ofdm_codedbits_t>   CodedBitsBuffer;
    stream <ofdm_codedbits_t>   InterleavedBitsBuffer;
    ofdm_sigdat_generator(
      datarate,
      TXVECTOR.LENGTH,
      Nbyte,
      DataBuffer,
      SigDataBuffer
    );
    ofdm_conv_feeder(
      Nbyte+3,
      SigDataBuffer,
      ConvolFeederDataBuffer
    );
    ofdm_conv_encoder(
      Nconv ,
      ConvolFeederDataBuffer,
      ConvolEncDataBuffer
    );
    ofdm_conv_punc(
      PuncMode,    //0:1/2 1:2/3 2:3/4
      Nconv,
      ConvolEncDataBuffer,
      PuncDataBuffer 
    );
    if(PuncMode==0){
      NPunc=Nconv;
    }else if(PuncMode==1){
      NPunc=4+(Nconv-4)/2;
    }else if(PuncMode==2){
      NPunc=4+(Nconv-4)*2/3;
    }
    ofdm_databits_reshape(
      Ncbps,                    //Coded Bits Per symbol
      NPunc,                    //Punc Numbers
      PuncMode,
      PuncDataBuffer, 
      CodedBitsBuffer
    );
    ofdm_interleaver(
      Ncbps,                    //Coded Bits Per symbol
      Nsym+1,
      CodedBitsBuffer,
      InterleavedBitsBuffer
  );
}