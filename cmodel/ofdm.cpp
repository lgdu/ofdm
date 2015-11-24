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
/*
void ofdm_sigdat_generator(
  ofdm_cfg_t                        TXVECTOR,
  stream <unsigned char> &          DataBuffer,
  stream <ofdm_databits_t> &        SigDataBuffer
  ){
    ofdm_databits_t SigData;
    unsigned int Ndbps=0; // Data bits per symbol
    SigData.IsSig=true;
    SigData.DataBits=0;
    switch (TXVECTOR.DATARATE){
    case 6: SigData.DataBits.range(3,0)=0xb;Ndbps=24;break;
    case 9: SigData.DataBits.range(3,0)=0xf;Ndbps=36;break;
    case 12:SigData.DataBits.range(3,0)=0xa;Ndbps=48;break;
    case 18:SigData.DataBits.range(3,0)=0xe;Ndbps=72;break;
    case 24:SigData.DataBits.range(3,0)=0x9;Ndbps=116;break;
    case 36:SigData.DataBits.range(3,0)=0xd;Ndbps=144;break;
    case 48:SigData.DataBits.range(3,0)=0x8;Ndbps=192;break;
    case 54:SigData.DataBits.range(3,0)=0xc;Ndbps=216;break;
    default:;
    }
    SigData.DataBits.range(16,5)=reverse(TXVECTOR.LENGTH,12);
    SigData.DataBits.range(17,17) =xor(SigData.DataBits.range(24,0).to_uint());
    SigDataBuffer.write(SigData);

    unsigned int Nsym = ceil((16+8*TXVECTOR.LENGTH+6)/Ndbps); // number of OFDM symbols
    unsigned int Ndata = Nsym*Ndbps; // number of bits in the DATA field
    unsigned int NdataByte=Ndata/8;
    SigData.IsSig=false;
    
}
*/

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
void ofdm_data_scrambler(
      unsigned int           Nbyte,         //bytes number of DATA
      unsigned int           Length,        //Data Length
      stream<unsigned char > & DataBuffer,
      stream<unsigned char > & ScrambBuffer
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
      ScrambBuffer.write(data.to_uint());
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
void ofdm_databits_reshape(
  unsigned int Ndbps,                     //Data Bits Per symbol
  unsigned int Nsym,                      //Symbol Number
  stream <unsigned char> &               ScramblerDataBuffer    ,
  stream <ofdm_databits_t> &             DataBitsBuffer 
  ){
    assert(Ndbps%8==0);
    for(int i=0;i<Nsym;++i){
      ofdm_databits_t DataBits=0;
      for(int j=0;j<Ndbps/8;++j){
        unsigned char ScramblerData=ScramblerDataBuffer.read();
        DataBits.range(j*8+7,j*8) = ScramblerData;
      }
      DataBitsBuffer.write(DataBits);
    }
}
void ofdm_data_generator(
   unsigned int                           Length        ,                      //Data Byte Length 
   unsigned int                           Ndbps         ,                      //Data Bits per symbol
   stream <unsigned char> &               DataBuffer    ,
   stream <ofdm_databits_t> &             DataBitsBuffer
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
    ofdm_databits_reshape(
        Ndbps,          //Data Bits Per symbol
        Nsym,           //Symbol Number
        ScramblerDataBuffer,
        DataBitsBuffer 
    );
}


void ofdm_transmitter(
  ofdm_cfg_t                        cfg,
  stream <unsigned char> &          DataBuffer,
  stream <ap_int<OFDM_IQ_WIDTH>> &  ISymbolBuffer,
  stream <ap_int<OFDM_IQ_WIDTH>> &  QSymbolBuffer
  ) {

}