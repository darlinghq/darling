/*
 *  log.c
 *  cLibm
 *      by Jeff Kidder.
 *
 *  Copyright (c) 2007, Apple Inc.  All Rights Reserved.
 *
 *  Implementation for C99 compliant double log[|10|1p](double x)
 *
 *  Minimax polynomial by Ali Sazegari.
 */
extern double Max_c, Min_c;

#ifndef AVOID_64_BIT_INTS
  #define AVOID_64_BIT_INTS 1
#endif 

#ifndef DEBUG
  #define DEBUG 0
#endif

#include <math.h>
#include <float.h>
#include <stdint.h>
#if DEBUG
#include <stdio.h> // for debug only
#endif

// Control if you want to use a look-up to get exact powers of 10 correctly rounded
#ifndef EXACT_LOG10_10_N
#define EXACT_LOG10_10_N 1
#endif 

#if DEBUG
 extern int Debug_Verbose;
#endif

#ifndef ALWAYS_INLINE
  #if DEBUG
        #define ALWAYS_INLINE
  #else
        #define ALWAYS_INLINE __attribute__ ((always_inline))
  #endif
#endif

#ifdef ARMLIBM_SET_FLAGS 
  #include "fenv.h" 
  #include "required_arithmetic.h"
    //If fenvaccess need to raise inexact. 
  #define RAISE_INEXACT {const double dummy = lge; required_multiply_double(dummy,dummy);} // raise inexact.
  #define RAISE_INVALID {volatile double bigdummy = 1p1000; int dummy_int = required_convert_double_to_int(bigdummy);} // REQUIRED conversion
  #define RAISE_ZERO_DIVIDE {const double xone = one; double xzero = 0.0; required_divide_double(xone,xzero);} // raise zero divide
#else // if there is no fenv access, we don't need to raise the flag.
  #define RAISE_INEXACT
  #define RAISE_INVALID
  #define RAISE_ZERO_DIVIDE
#endif

//Note: a is unused at this point, but keeps the records on a good stride
//gcc logtable.c -o xlt -I./mpfr/include -L./mpfr/lib -lgmp -lmpfr -framework Accelerate -Wall && ./xlt 
typedef struct logdata_t {double ap1; double inv; double log1p; double a; } logdata_t;

static const logdata_t LOG_LOOKUP_BIG[] = { //ap1, va, log1pa, a: va = 1/(a+1), log1pa = log(1+a)+err
    {  0x1.0000000000000p+0,   0x1.0000000000000p+0,   0x0.0000000000000p+0,   0x0.0000000000000p+0}, //    0 * 0x1p-8 = 0x0.00p+0 (err =   0x0.0000000000000p+0)
    {  0x1.0100003936f5ap+0,   0x1.fe01fd90739d3p-1,   0x1.ff011c2700000p-9,   0x1.00003936f5a00p-8}, //    1 * 0x1p-8 = 0x1.00p-8 (err =  0x1.ec7567bf1f2c5p-64)
    {  0x1.0200003540a78p+0,   0x1.fc07efb6e4447p-1,   0x1.fe02db8800000p-8,   0x1.00001aa053c00p-7}, //    2 * 0x1p-8 = 0x1.00p-7 (err =  0x1.f21c6a889955ep-64)
    {  0x1.02ffffda17765p+0,   0x1.fa11caea31adap-1,   0x1.7dc4633c00000p-7,   0x1.7fffed0bbb280p-7}, //    3 * 0x1p-8 = 0x1.80p-7 (err =  0x1.fea966f30b702p-64)
    {  0x1.0400005692f79p+0,   0x1.f81f815042f54p-1,   0x1.fc0ab5aec0000p-7,   0x1.000015a4bde40p-6}, //    4 * 0x1p-8 = 0x1.00p-6 (err =  0x1.f212e674011cbp-64)
    {  0x1.04ffffb0b3706p+0,   0x1.f6310b62a2281p-1,   0x1.3cea30c280000p-6,   0x1.3fffec2cdc180p-6}, //    5 * 0x1p-8 = 0x1.40p-6 (err =  0x1.fe1d63326da7cp-64)
    {  0x1.0600000cd0b53p+0,   0x1.f44659cc2bd1ap-1,   0x1.7b91b39ec0000p-6,   0x1.800003342d4c0p-6}, //    6 * 0x1p-8 = 0x1.80p-6 (err =  0x1.ff9310b95d4c8p-64)
    {  0x1.0700003220a96p+0,   0x1.f25f63e333613p-1,   0x1.b9fc0eadc0000p-6,   0x1.c0000c882a580p-6}, //    7 * 0x1p-8 = 0x1.c0p-6 (err =  0x1.edc4ef8798bb3p-64)
    {  0x1.07ffffc763caep+0,   0x1.f07c1f723856dp-1,   0x1.f829a32e40000p-6,   0x1.fffff1d8f2b80p-6}, //    8 * 0x1p-8 = 0x1.00p-5 (err =  0x1.fb741c252ea0dp-64)
    {  0x1.0900002540747p+0,   0x1.ee9c7f3ed17c2p-1,   0x1.1b0d9d11d0000p-5,   0x1.200004a80e8e0p-5}, //    9 * 0x1p-8 = 0x1.20p-5 (err =  0x1.f78bcbfaa94c1p-64)
    {  0x1.0a000025d53cap+0,   0x1.ecc07aea095fbp-1,   0x1.39e8802d10000p-5,   0x1.400004baa7940p-5}, //   10 * 0x1p-8 = 0x1.40p-5 (err =  0x1.f71913624a53cp-64)
    {  0x1.0b000032ccd53p+0,   0x1.eae8074e396d3p-1,   0x1.58a5c11330000p-5,   0x1.600006599aa60p-5}, //   11 * 0x1p-8 = 0x1.60p-5 (err =  0x1.f91ea793c17f1p-64)
    {  0x1.0c00005056fa6p+0,   0x1.e9131a2c6eacfp-1,   0x1.774598faf0000p-5,   0x1.80000a0adf4c0p-5}, //   12 * 0x1p-8 = 0x1.80p-5 (err =  0x1.ec71d6e487c75p-64)
    {  0x1.0d00005a7a6ddp+0,   0x1.e741a9b591808p-1,   0x1.95c83baff0000p-5,   0x1.a0000b4f4dba0p-5}, //   13 * 0x1p-8 = 0x1.a0p-5 (err =  0x1.f19c7de6dffc4p-64)
    {  0x1.0dffffb0e53d1p+0,   0x1.e573ad1e58997p-1,   0x1.b42dcdb180000p-5,   0x1.bffff61ca7a20p-5}, //   14 * 0x1p-8 = 0x1.c0p-5 (err =  0x1.f4e2b5a966aa2p-64)
    {  0x1.0f000056e5441p+0,   0x1.e3a91702abf7ep-1,   0x1.d276c2f070000p-5,   0x1.e0000adca8820p-5}, //   15 * 0x1p-8 = 0x1.e0p-5 (err =  0x1.e8a37ae38ea34p-64)
    {  0x1.0fffffbc7973ap+0,   0x1.e1e1e259832fcp-1,   0x1.f0a3040f60000p-5,   0x1.fffff78f2e740p-5}, //   16 * 0x1p-8 = 0x1.00p-4 (err =  0x1.ed4033ca6af0bp-64)
    {  0x1.1100005342607p+0,   0x1.e01e014db1078p-1,   0x1.0759883ac0000p-4,   0x1.1000053426070p-4}, //   17 * 0x1p-8 = 0x1.10p-4 (err =  0x1.f3c7d9479fb47p-64)
    {  0x1.12000032d2bd8p+0,   0x1.de5d6de6cd82ap-1,   0x1.165371e1f8000p-4,   0x1.2000032d2bd80p-4}, //   18 * 0x1p-8 = 0x1.20p-4 (err =  0x1.ee8f57daea6d7p-64)
    {  0x1.13000032ad417p+0,   0x1.dca01d722cd47p-1,   0x1.253f65e370000p-4,   0x1.3000032ad4170p-4}, //   19 * 0x1p-8 = 0x1.30p-4 (err =  0x1.f6febd2d5eef1p-64)
    {  0x1.14000033ca6fdp+0,   0x1.dae607127b072p-1,   0x1.341d7c6258000p-4,   0x1.4000033ca6fd0p-4}, //   20 * 0x1p-8 = 0x1.40p-4 (err =  0x1.f17b32ba48e5ap-64)
    {  0x1.15000020b69f4p+0,   0x1.d92f21fa060ebp-1,   0x1.42edcdce20000p-4,   0x1.5000020b69f40p-4}, //   21 * 0x1p-8 = 0x1.50p-4 (err =  0x1.f3bcac0fd322fp-64)
    {  0x1.16000033f01dbp+0,   0x1.d77b64f36cd62p-1,   0x1.51b076eda0000p-4,   0x1.6000033f01db0p-4}, //   22 * 0x1p-8 = 0x1.60p-4 (err =  0x1.fba468240645ap-64)
    {  0x1.1700003638738p+0,   0x1.d5cac7ac0aac2p-1,   0x1.60658daf78000p-4,   0x1.7000036387380p-4}, //   23 * 0x1p-8 = 0x1.70p-4 (err =  0x1.ec787c7887fd7p-64)
    {  0x1.1800004d5e42bp+0,   0x1.d41d4152c46c8p-1,   0x1.6f0d2d1a20000p-4,   0x1.800004d5e42b0p-4}, //   24 * 0x1p-8 = 0x1.80p-4 (err =  0x1.f1384bce67ab4p-64)
    {  0x1.19000025564f6p+0,   0x1.d272ca01cb4b4p-1,   0x1.7da768f7f0000p-4,   0x1.9000025564f60p-4}, //   25 * 0x1p-8 = 0x1.90p-4 (err =  0x1.f9467c12bec5cp-64)
    {  0x1.19ffffdbfe8f2p+0,   0x1.d0cb593244471p-1,   0x1.8c345b5820000p-4,   0x1.9ffffdbfe8f20p-4}, //   26 * 0x1p-8 = 0x1.a0p-4 (err =  0x1.fc2d027876ac7p-64)
    {  0x1.1afffff933376p+0,   0x1.cf26e5cf6ceaap-1,   0x1.9ab423ff98000p-4,   0x1.afffff9333760p-4}, //   27 * 0x1p-8 = 0x1.b0p-4 (err =  0x1.feea15cc03a2ap-64)
    {  0x1.1c00004c54b75p+0,   0x1.cd8568142e92dp-1,   0x1.a926d7f190000p-4,   0x1.c00004c54b750p-4}, //   28 * 0x1p-8 = 0x1.c0p-4 (err =  0x1.e70f4409f8e5fp-64)
    {  0x1.1d0000539db20p+0,   0x1.cbe6d8d92e948p-1,   0x1.b78c876cc8000p-4,   0x1.d0000539db200p-4}, //   29 * 0x1p-8 = 0x1.d0p-4 (err =  0x1.e2809de338feap-64)
    {  0x1.1e000056075d7p+0,   0x1.ca4b2fcc134bcp-1,   0x1.c5e54dc5d0000p-4,   0x1.e000056075d70p-4}, //   30 * 0x1p-8 = 0x1.e0p-4 (err =  0x1.dd2acc60b7865p-64)
    {  0x1.1effffc6f8e47p+0,   0x1.c8b2660a77e26p-1,   0x1.d4313a38e8000p-4,   0x1.effffc6f8e470p-4}, //   31 * 0x1p-8 = 0x1.f0p-4 (err =  0x1.fe5663d116ba0p-64)
    {  0x1.20000027072ecp+0,   0x1.c71c71896fe8cp-1,   0x1.e270790dc0000p-4,   0x1.0000013839760p-3}, //   32 * 0x1p-8 = 0x1.00p-3 (err =  0x1.f0ad4fe93f4bdp-64)
    {  0x1.21000059603cbp+0,   0x1.c5894c8491f4dp-1,   0x1.f0a310f3d0000p-4,   0x1.080002cb01e58p-3}, //   33 * 0x1p-8 = 0x1.08p-3 (err =  0x1.da51af36dc1a5p-64)
    {  0x1.22000041b9312p+0,   0x1.c3f8efb5d1114p-1,   0x1.fec916be08000p-4,   0x1.1000020dc9890p-3}, //   34 * 0x1p-8 = 0x1.10p-3 (err =  0x1.f4c9a30431fedp-64)
    {  0x1.22ffffeff0d4bp+0,   0x1.c26b53abc5687p-1,   0x1.067150bba0000p-3,   0x1.17ffff7f86a58p-3}, //   35 * 0x1p-8 = 0x1.18p-3 (err =  0x1.fde499f003adep-64)
    {  0x1.240000537e0b5p+0,   0x1.c0e06fb7c2d3bp-1,   0x1.0d77ea16a0000p-3,   0x1.2000029bf05a8p-3}, //   36 * 0x1p-8 = 0x1.20p-3 (err =  0x1.fced1c98ed09ep-64)
    {  0x1.2500005ca4617p+0,   0x1.bf583e5af74cbp-1,   0x1.14785ace00000p-3,   0x1.280002e5230b8p-3}, //   37 * 0x1p-8 = 0x1.28p-3 (err =  0x1.dd1fc6c91e26bp-64)
    {  0x1.2600002fa2e70p+0,   0x1.bdd2b8510405bp-1,   0x1.1b72ae9ecc000p-3,   0x1.3000017d17380p-3}, //   38 * 0x1p-8 = 0x1.30p-3 (err =  0x1.f91683fab9397p-64)
    {  0x1.27000017eec54p+0,   0x1.bc4fd634781e9p-1,   0x1.2266f236cc000p-3,   0x1.380000bf762a0p-3}, //   39 * 0x1p-8 = 0x1.38p-3 (err =  0x1.fe7f209c6f43fp-64)
    {  0x1.2800001eec5d2p+0,   0x1.bacf911dd8ef3p-1,   0x1.29553057f4000p-3,   0x1.400000f762e90p-3}, //   40 * 0x1p-8 = 0x1.40p-3 (err =  0x1.faded11512c2dp-64)
    {  0x1.2900004307df7p+0,   0x1.b951e24df5a78p-1,   0x1.303d735c80000p-3,   0x1.480002183efb8p-3}, //   41 * 0x1p-8 = 0x1.48p-3 (err =  0x1.f5f08db3d551ep-64)
    {  0x1.2a00004f79581p+0,   0x1.b7d6c368561d2p-1,   0x1.371fc42418000p-3,   0x1.5000027bcac08p-3}, //   42 * 0x1p-8 = 0x1.50p-3 (err =  0x1.f2625f9dfae50p-64)
    {  0x1.2b000039effaap+0,   0x1.b65e2de6fd839p-1,   0x1.3dfc2c9ba4000p-3,   0x1.580001cf7fd50p-3}, //   43 * 0x1p-8 = 0x1.58p-3 (err =  0x1.ed8441b9cd15fp-64)
    {  0x1.2c00005fab0adp+0,   0x1.b4e81ac32e0acp-1,   0x1.44d2b959d0000p-3,   0x1.600002fd58568p-3}, //   44 * 0x1p-8 = 0x1.60p-3 (err =  0x1.fbf9bb43df5bep-64)
    {  0x1.2d00005991e29p+0,   0x1.b374842bebe12p-1,   0x1.4ba3719b14000p-3,   0x1.680002cc8f148p-3}, //   45 * 0x1p-8 = 0x1.68p-3 (err =  0x1.f8a48ef870d79p-64)
    {  0x1.2e00002c5bc7dp+0,   0x1.b20363c708522p-1,   0x1.526e5f66ec000p-3,   0x1.70000162de3e8p-3}, //   46 * 0x1p-8 = 0x1.70p-3 (err =  0x1.fb39bfccfed78p-64)
    {  0x1.2f00005801681p+0,   0x1.b094b29fedd79p-1,   0x1.59338fec58000p-3,   0x1.780002c00b408p-3}, //   47 * 0x1p-8 = 0x1.78p-3 (err =  0x1.f5d2afde4cfc0p-64)
    {  0x1.300000411fb8fp+0,   0x1.af286b6dbdbc1p-1,   0x1.5ff308c134000p-3,   0x1.80000208fdc78p-3}, //   48 * 0x1p-8 = 0x1.80p-3 (err =  0x1.f8227532c76cap-64)
    {  0x1.31000050c0dc1p+0,   0x1.adbe87878103ap-1,   0x1.66acd64568000p-3,   0x1.8800028606e08p-3}, //   49 * 0x1p-8 = 0x1.88p-3 (err =  0x1.f74f724b5b182p-64)
    {  0x1.31ffffa96bbfcp+0,   0x1.ac570225889a6p-1,   0x1.6d60fc2e28000p-3,   0x1.8ffffd4b5dfe0p-3}, //   50 * 0x1p-8 = 0x1.90p-3 (err =  0x1.eea60252a2d4ep-64)
    {  0x1.33000038a33b9p+0,   0x1.aaf1d2a9baadfp-1,   0x1.740f90cdd8000p-3,   0x1.980001c519dc8p-3}, //   51 * 0x1p-8 = 0x1.98p-3 (err =  0x1.ec0677b6a1612p-64)
    {  0x1.33ffffa36f9ddp+0,   0x1.a98ef6868b217p-1,   0x1.7ab88db990000p-3,   0x1.9ffffd1b7cee8p-3}, //   52 * 0x1p-8 = 0x1.a0p-3 (err =  0x1.f1eb03cd29c0fp-64)
    {  0x1.34ffffa6b29a5p+0,   0x1.a82e658da52d6p-1,   0x1.815c07c454000p-3,   0x1.a7fffd3594d28p-3}, //   53 * 0x1p-8 = 0x1.a8p-3 (err =  0x1.eda843ec79705p-64)
    {  0x1.35ffffcd8e7a1p+0,   0x1.a6d01ab1ce919p-1,   0x1.87fa0504cc000p-3,   0x1.affffe6c73d08p-3}, //   54 * 0x1p-8 = 0x1.b0p-3 (err =  0x1.fc2353b44b6d3p-64)
    {  0x1.3700002bf67d9p+0,   0x1.a574103af50edp-1,   0x1.8e928f0a08000p-3,   0x1.b800015fb3ec8p-3}, //   55 * 0x1p-8 = 0x1.b8p-3 (err =  0x1.fe7cd2027400ap-64)
    {  0x1.37ffffb9f00cbp+0,   0x1.a41a420270b39p-1,   0x1.9525a80360000p-3,   0x1.bffffdcf80658p-3}, //   56 * 0x1p-8 = 0x1.c0p-3 (err =  0x1.fa065a340e5adp-64)
    {  0x1.39000042e2bb8p+0,   0x1.a2c2a822d5699p-1,   0x1.9bb3649d84000p-3,   0x1.c800021715dc0p-3}, //   57 * 0x1p-8 = 0x1.c8p-3 (err =  0x1.f937431a1cf8bp-64)
    {  0x1.3a000042bc471p+0,   0x1.a16d3f3eed1d0p-1,   0x1.a23bc3b170000p-3,   0x1.d0000215e2388p-3}, //   58 * 0x1p-8 = 0x1.d0p-3 (err =  0x1.edfc157070baep-64)
    {  0x1.3b0000420f1c2p+0,   0x1.a01a0148d7269p-1,   0x1.a8bed17600000p-3,   0x1.d800021078e10p-3}, //   59 * 0x1p-8 = 0x1.d8p-3 (err =  0x1.f8366355ec430p-64)
    {  0x1.3c00006308d43p+0,   0x1.9ec8e8cf04dc4p-1,   0x1.af3c9769e4000p-3,   0x1.e000031846a18p-3}, //   60 * 0x1p-8 = 0x1.e0p-3 (err =  0x1.fbc9bb562e32fp-64)
    {  0x1.3d00004c70a8bp+0,   0x1.9d79f113024adp-1,   0x1.b5b51bd6d4000p-3,   0x1.e800026385458p-3}, //   61 * 0x1p-8 = 0x1.e8p-3 (err =  0x1.f0ffb9620e448p-64)
    {  0x1.3dffffcd39012p+0,   0x1.9c2d15301aba4p-1,   0x1.bc2865fbd4000p-3,   0x1.effffe69c8090p-3}, //   62 * 0x1p-8 = 0x1.f0p-3 (err =  0x1.fb7df07d7a8b7p-64)
    {  0x1.3f000007071e5p+0,   0x1.9ae24e9c43b89p-1,   0x1.c2968585e0000p-3,   0x1.f800003838f28p-3}, //   63 * 0x1p-8 = 0x1.f8p-3 (err =  0x1.ff627720067bcp-64)
    {  0x1.40000033ab3ebp+0,   0x1.9999995776ba1p-1,   0x1.c8ff7dc458000p-3,   0x1.000000ceacfacp-2}, //   64 * 0x1p-8 = 0x1.00p-2 (err =  0x1.ffae62069f174p-64)
    {  0x1.410000277105dp+0,   0x1.9852f0a6c045cp-1,   0x1.cf6355dc40000p-3,   0x1.0400009dc4174p-2}, //   65 * 0x1p-8 = 0x1.04p-2 (err =  0x1.ffb95f7836922p-64)
    {  0x1.4200004edb4e3p+0,   0x1.970e4f1d1bc57p-1,   0x1.d5c218aa88000p-3,   0x1.0800013b6d38cp-2}, //   66 * 0x1p-8 = 0x1.08p-2 (err =  0x1.f21dc04c3495dp-64)
    {  0x1.430000352875cp+0,   0x1.95cbb07b6eb7ep-1,   0x1.dc1bcb5bcc000p-3,   0x1.0c0000d4a1d70p-2}, //   67 * 0x1p-8 = 0x1.0cp-2 (err =  0x1.fed7978ec7f6dp-64)
    {  0x1.4400003e24673p+0,   0x1.948b0f7fd78acp-1,   0x1.e270786b7c000p-3,   0x1.100000f8919ccp-2}, //   68 * 0x1p-8 = 0x1.10p-2 (err =  0x1.fb7cdbe2775e3p-64)
    {  0x1.45000045e1f91p+0,   0x1.934c67a2facf9p-1,   0x1.e8c026e304000p-3,   0x1.1400011787e44p-2}, //   69 * 0x1p-8 = 0x1.14p-2 (err =  0x1.feadb0cadb0f1p-64)
    {  0x1.45ffffbfb71ddp+0,   0x1.920fb4ec56b0ap-1,   0x1.ef0adb29ec000p-3,   0x1.17fffefedc774p-2}, //   70 * 0x1p-8 = 0x1.18p-2 (err =  0x1.f7ae87b141064p-64)
    {  0x1.470000418b3c4p+0,   0x1.90d4f0cfc14fcp-1,   0x1.f550a6ff38000p-3,   0x1.1c0001062cf10p-2}, //   71 * 0x1p-8 = 0x1.1cp-2 (err =  0x1.f6302ea22f09ap-64)
    {  0x1.4800005c73e8cp+0,   0x1.8f9c18891e6acp-1,   0x1.fb91891728000p-3,   0x1.20000171cfa30p-2}, //   72 * 0x1p-8 = 0x1.20p-2 (err =  0x1.fda049e97050bp-64)
    {  0x1.49000032c09b3p+0,   0x1.8e6527719e597p-1,   0x1.00e6c4f8cc000p-2,   0x1.240000cb026ccp-2}, //   73 * 0x1p-8 = 0x1.24p-2 (err =  0x1.fb1153f0917c5p-64)
    {  0x1.4a0000617bec4p+0,   0x1.8d30185dac9cdp-1,   0x1.04025a79cc000p-2,   0x1.28000185efb10p-2}, //   74 * 0x1p-8 = 0x1.28p-2 (err =  0x1.fb9ee21ad2e65p-64)
    {  0x1.4b0000664fcf0p+0,   0x1.8bfce78bc9b18p-1,   0x1.071b87395a000p-2,   0x1.2c0001993f3c0p-2}, //   75 * 0x1p-8 = 0x1.2cp-2 (err =  0x1.f2d61b0e48ed5p-64)
    {  0x1.4c00005755c87p+0,   0x1.8acb908ee4a5fp-1,   0x1.0a324f3498000p-2,   0x1.3000015d5721cp-2}, //   76 * 0x1p-8 = 0x1.30p-2 (err =  0x1.e7d00c765df10p-64)
    {  0x1.4d00002bd4b75p+0,   0x1.899c0f2c4997cp-1,   0x1.0d46b60074000p-2,   0x1.340000af52dd4p-2}, //   77 * 0x1p-8 = 0x1.34p-2 (err =  0x1.fe22796a81b4bp-64)
    {  0x1.4dffffb681888p+0,   0x1.886e5f6114f44p-1,   0x1.1058beb992000p-2,   0x1.37fffeda06220p-2}, //   78 * 0x1p-8 = 0x1.38p-2 (err =  0x1.ee19182bb15e1p-64)
    {  0x1.4f0000568b9f9p+0,   0x1.87427b66f4becp-1,   0x1.13687131c6000p-2,   0x1.3c00015a2e7e4p-2}, //   79 * 0x1p-8 = 0x1.3cp-2 (err =  0x1.fa3b8197c4d31p-64)
    {  0x1.4fffffafd4f84p+0,   0x1.861861e32b92bp-1,   0x1.1675c9c668000p-2,   0x1.3ffffebf53e10p-2}, //   80 * 0x1p-8 = 0x1.40p-2 (err =  0x1.fdd5ba79baff6p-64)
    {  0x1.51000042b10f7p+0,   0x1.84f00bda880edp-1,   0x1.1980d3a7e8000p-2,   0x1.4400010ac43dcp-2}, //   81 * 0x1p-8 = 0x1.44p-2 (err =  0x1.fef0fb353faaep-64)
    {  0x1.5200003cf4e5dp+0,   0x1.83c977653c730p-1,   0x1.1c898ccf46000p-2,   0x1.480000f3d3974p-2}, //   82 * 0x1p-8 = 0x1.48p-2 (err =  0x1.efc01fcaeb1a5p-64)
    {  0x1.5300005d04b3fp+0,   0x1.82a49fae12f2ep-1,   0x1.1f8ffafd84000p-2,   0x1.4c00017412cfcp-2}, //   83 * 0x1p-8 = 0x1.4cp-2 (err =  0x1.e8e47f5c165a8p-64)
    {  0x1.5400005a07c12p+0,   0x1.8181811b6d057p-1,   0x1.229420cc1e000p-2,   0x1.500001681f048p-2}, //   84 * 0x1p-8 = 0x1.50p-2 (err =  0x1.e9bb184056793p-64)
    {  0x1.5500005cf2f1ap+0,   0x1.8060179d3bd46p-1,   0x1.2596022516000p-2,   0x1.54000173cbc68p-2}, //   85 * 0x1p-8 = 0x1.54p-2 (err =  0x1.f686289662d7cp-64)
    {  0x1.55ffffb18cb69p+0,   0x1.7f40602801a71p-1,   0x1.2895a05304000p-2,   0x1.57fffec632da4p-2}, //   86 * 0x1p-8 = 0x1.58p-2 (err =  0x1.fdadb05d8884cp-64)
    {  0x1.5700000c7d98dp+0,   0x1.7e225507ba85bp-1,   0x1.2b9303d0d4000p-2,   0x1.5c000031f6634p-2}, //   87 * 0x1p-8 = 0x1.5cp-2 (err =  0x1.fbd8350c5e070p-64)
    {  0x1.5800003c34df2p+0,   0x1.7d05f3d520addp-1,   0x1.2e8e2c614a000p-2,   0x1.600000f0d37c8p-2}, //   88 * 0x1p-8 = 0x1.60p-2 (err =  0x1.fc3ea6931335ep-64)
    {  0x1.59000017f47a5p+0,   0x1.7beb39087eec1p-1,   0x1.31871cdc5e000p-2,   0x1.6400005fd1e94p-2}, //   89 * 0x1p-8 = 0x1.64p-2 (err =  0x1.f843d3c26a416p-64)
    {  0x1.5a00002abcf47p+0,   0x1.7ad2205f44020p-1,   0x1.347dda2804000p-2,   0x1.680000aaf3d1cp-2}, //   90 * 0x1p-8 = 0x1.68p-2 (err =  0x1.fcec9c09b9cc4p-64)
    {  0x1.5b00005f50a20p+0,   0x1.79baa653a2212p-1,   0x1.3772674544000p-2,   0x1.6c00017d42880p-2}, //   91 * 0x1p-8 = 0x1.6cp-2 (err =  0x1.f8fc5a9db8104p-64)
    {  0x1.5bffffb96b0fep+0,   0x1.78a4c863ee7b2p-1,   0x1.3a64c486e4000p-2,   0x1.6ffffee5ac3f8p-2}, //   92 * 0x1p-8 = 0x1.70p-2 (err =  0x1.f9495a0ba8d22p-64)
    {  0x1.5d000015917acp+0,   0x1.7790810276a04p-1,   0x1.3d54fa9b68000p-2,   0x1.7400005645eb0p-2}, //   93 * 0x1p-8 = 0x1.74p-2 (err =  0x1.fe1da90d1fae7p-64)
    {  0x1.5dffffc252ddep+0,   0x1.767dce8548978p-1,   0x1.404307b3f8000p-2,   0x1.77ffff094b778p-2}, //   94 * 0x1p-8 = 0x1.78p-2 (err =  0x1.f71974f88f2dcp-64)
    {  0x1.5effffa3e67fcp+0,   0x1.756cac821325bp-1,   0x1.432ef1939e000p-2,   0x1.7bfffe8f99ff0p-2}, //   95 * 0x1p-8 = 0x1.7cp-2 (err =  0x1.fc617a1657314p-64)
    {  0x1.600000621f36ep+0,   0x1.745d16de05102p-1,   0x1.4618bd3f38000p-2,   0x1.800001887cdb8p-2}, //   96 * 0x1p-8 = 0x1.80p-2 (err =  0x1.f453c96f3d175p-64)
    {  0x1.60ffffcd57dd6p+0,   0x1.734f0c8968a73p-1,   0x1.490067710e000p-2,   0x1.83ffff355f758p-2}, //   97 * 0x1p-8 = 0x1.84p-2 (err =  0x1.f738f2bbfcb1fp-64)
    {  0x1.62000052fcffdp+0,   0x1.7242879da1196p-1,   0x1.4be5fa4786000p-2,   0x1.8800014bf3ff4p-2}, //   98 * 0x1p-8 = 0x1.88p-2 (err =  0x1.ffc8c26e6f432p-64)
    {  0x1.630000279d05ap+0,   0x1.713786b0949b4p-1,   0x1.4ec9739844000p-2,   0x1.8c00009e74168p-2}, //   99 * 0x1p-8 = 0x1.8cp-2 (err =  0x1.fdb5253cd0c19p-64)
    {  0x1.64000056860b9p+0,   0x1.702e05673c3a9p-1,   0x1.51aad96bc0000p-2,   0x1.9000015a182e4p-2}, //  100 * 0x1p-8 = 0x1.90p-2 (err =  0x1.f48756a3d6c56p-64)
    {  0x1.65000068164d8p+0,   0x1.6f2601041a3a3p-1,   0x1.548a2d656c000p-2,   0x1.940001a059360p-2}, //  101 * 0x1p-8 = 0x1.94p-2 (err =  0x1.e0ebf656d254bp-64)
    {  0x1.66000063fd1f4p+0,   0x1.6e1f764df192fp-1,   0x1.5767729256000p-2,   0x1.9800018ff47d0p-2}, //  102 * 0x1p-8 = 0x1.98p-2 (err =  0x1.e35ee3b9b1a11p-64)
    {  0x1.66ffffc1059b7p+0,   0x1.6d1a62a829001p-1,   0x1.5a42aa5baa000p-2,   0x1.9bffff04166dcp-2}, //  103 * 0x1p-8 = 0x1.9cp-2 (err =  0x1.ea63313f0a4afp-64)
    {  0x1.67ffffd9f79cfp+0,   0x1.6c16c1928dba9p-1,   0x1.5d1bdb8952000p-2,   0x1.9fffff67de73cp-2}, //  104 * 0x1p-8 = 0x1.a0p-2 (err =  0x1.fe2b390cdceb9p-64)
    {  0x1.69000044ea073p+0,   0x1.6b149064e1f25p-1,   0x1.5ff307cdf4000p-2,   0x1.a4000113a81ccp-2}, //  105 * 0x1p-8 = 0x1.a4p-2 (err =  0x1.f5e0548ea3197p-64)
    {  0x1.69ffffcb6c142p+0,   0x1.6a13cd49cdf53p-1,   0x1.62c82e96e2000p-2,   0x1.a7ffff2db0508p-2}, //  106 * 0x1p-8 = 0x1.a8p-2 (err =  0x1.ffe4fa3dada76p-64)
    {  0x1.6b00005b37110p+0,   0x1.6914734dd17f6p-1,   0x1.659b58318e000p-2,   0x1.ac00016cdc440p-2}, //  107 * 0x1p-8 = 0x1.acp-2 (err =  0x1.e4393e3c88983p-64)
    {  0x1.6c000025e9426p+0,   0x1.68168142958dbp-1,   0x1.686c825458000p-2,   0x1.b0000097a5098p-2}, //  108 * 0x1p-8 = 0x1.b0p-2 (err =  0x1.fc6a6ac300ce7p-64)
    {  0x1.6d0000686f162p+0,   0x1.6719f2f95769fp-1,   0x1.6b3bb34856000p-2,   0x1.b40001a1bc588p-2}, //  109 * 0x1p-8 = 0x1.b4p-2 (err =  0x1.e6a53c3a8f452p-64)
    {  0x1.6e000062935f4p+0,   0x1.661ec6449e1e3p-1,   0x1.6e08ebb686000p-2,   0x1.b800018a4d7d0p-2}, //  110 * 0x1p-8 = 0x1.b8p-2 (err =  0x1.e548bc63b24d5p-64)
    {  0x1.6f0000608950ap+0,   0x1.6524f7f5c3035p-1,   0x1.70d42f34e4000p-2,   0x1.bc00018225428p-2}, //  111 * 0x1p-8 = 0x1.bcp-2 (err =  0x1.f98ed7fca6ee0p-64)
    {  0x1.70000042866bep+0,   0x1.642c85504ef51p-1,   0x1.739d8024da000p-2,   0x1.c000010a19af8p-2}, //  112 * 0x1p-8 = 0x1.c0p-2 (err =  0x1.e9728409d46edp-64)
    {  0x1.71000036b91f3p+0,   0x1.63356b53fe859p-1,   0x1.7664e1bb7a000p-2,   0x1.c40000dae47ccp-2}, //  113 * 0x1p-8 = 0x1.c4p-2 (err =  0x1.f4ec6e3ce07f9p-64)
    {  0x1.71ffffce2f1cep+0,   0x1.623fa79fc81f3p-1,   0x1.792a5573f6000p-2,   0x1.c7ffff38bc738p-2}, //  114 * 0x1p-8 = 0x1.c8p-2 (err =  0x1.fa4f9d3b2fbf1p-64)
    {  0x1.72ffffa69821cp+0,   0x1.614b36d83e713p-1,   0x1.7beddfacb6000p-2,   0x1.cbfffe9a60870p-2}, //  115 * 0x1p-8 = 0x1.ccp-2 (err =  0x1.ebb89d19bdb6fp-64)
    {  0x1.73ffff93e0847p+0,   0x1.6058166bea515p-1,   0x1.7eaf828e8a000p-2,   0x1.cffffe4f8211cp-2}, //  116 * 0x1p-8 = 0x1.d0p-2 (err =  0x1.e6f90ed8988d4p-64)
    {  0x1.7500004fb5411p+0,   0x1.5f6642f77b4b5p-1,   0x1.816f42b4e0000p-2,   0x1.d400013ed5044p-2}, //  117 * 0x1p-8 = 0x1.d4p-2 (err =  0x1.ee5133e9c1e2ep-64)
    {  0x1.760000070d773p+0,   0x1.5e75bb86658acp-1,   0x1.842d1db538000p-2,   0x1.d800001c35dccp-2}, //  118 * 0x1p-8 = 0x1.d8p-2 (err =  0x1.ff8e50e91c4ccp-64)
    {  0x1.76ffff9a91e9fp+0,   0x1.5d867c9d584ffp-1,   0x1.86e9188e38000p-2,   0x1.dbfffe6a47a7cp-2}, //  119 * 0x1p-8 = 0x1.dcp-2 (err =  0x1.f5148e6a74d0bp-64)
    {  0x1.7800003cbfce3p+0,   0x1.5c988280deb04p-1,   0x1.89a3391186000p-2,   0x1.e00000f2ff38cp-2}, //  120 * 0x1p-8 = 0x1.e0p-2 (err =  0x1.f5e23e9e63edfp-64)
    {  0x1.79000003e9ab8p+0,   0x1.5babcc60e3ea8p-1,   0x1.8c5b7c902c000p-2,   0x1.e400000fa6ae0p-2}, //  121 * 0x1p-8 = 0x1.e4p-2 (err =  0x1.ff5630aa85652p-64)
    {  0x1.79ffffa8023cap+0,   0x1.5ac05700cd43fp-1,   0x1.8f11e78508000p-2,   0x1.e7fffea008f28p-2}, //  122 * 0x1p-8 = 0x1.e8p-2 (err =  0x1.fdfb91bb0e262p-64)
    {  0x1.7b000060cedb4p+0,   0x1.59d61eb9e681bp-1,   0x1.91c67fb9ea000p-2,   0x1.ec0001833b6d0p-2}, //  123 * 0x1p-8 = 0x1.ecp-2 (err =  0x1.fad7f84c4da52p-64)
    {  0x1.7c000016ce419p+0,   0x1.58ed22f3622b0p-1,   0x1.947941ff86000p-2,   0x1.f000005b39064p-2}, //  124 * 0x1p-8 = 0x1.f0p-2 (err =  0x1.ff95f647cfd01p-64)
    {  0x1.7d00002a012c4p+0,   0x1.58055fef92d79p-1,   0x1.972a34821a000p-2,   0x1.f40000a804b10p-2}, //  125 * 0x1p-8 = 0x1.f4p-2 (err =  0x1.f7782ead6e256p-64)
    {  0x1.7e0000616de6dp+0,   0x1.571ed36d835ccp-1,   0x1.99d95916aa000p-2,   0x1.f8000185b79b4p-2}, //  126 * 0x1p-8 = 0x1.f8p-2 (err =  0x1.edca12358dbbap-64)
    {  0x1.7effff981741ep+0,   0x1.56397c049deb6p-1,   0x1.9c86af17f0000p-2,   0x1.fbfffe605d078p-2}, //  127 * 0x1p-8 = 0x1.fcp-2 (err =  0x1.ee972e3e2f148p-64)
    {  0x1.81000016f6a2ap-1,   0x1.54725e5769d7fp+0,  -0x1.23ec5954d8000p-2,  -0x1.fbffffa425758p-3}, // -127 * 0x1p-9 = -0x1.fcp-3 (err =  0x1.fed7ff9ae4a06p-64)
    {  0x1.81ffff914737dp-1,   0x1.539094f0a7f11p+0,  -0x1.214457f6a6000p-2,  -0x1.f80001bae320cp-3}, // -126 * 0x1p-9 = -0x1.f8p-3 (err =  0x1.fe2f5c2dc84f6p-64)
    {  0x1.8300006a28068p-1,   0x1.52aff50d98ee7p+0,  -0x1.1e9e155fa6000p-2,  -0x1.f3fffe575fe60p-3}, // -125 * 0x1p-9 = -0x1.f4p-3 (err =  0x1.ff39d3660dbb3p-64)
    {  0x1.84000055cfbe4p-1,   0x1.51d07e637929dp+0,  -0x1.1bf995532e000p-2,  -0x1.effffea8c1070p-3}, // -124 * 0x1p-9 = -0x1.f0p-3 (err =  0x1.ee01118390a2dp-64)
    {  0x1.85000045796cap-1,   0x1.50f22dd4eed41p+0,  -0x1.1956d302da000p-2,  -0x1.ebfffeea1a4d8p-3}, // -123 * 0x1p-9 = -0x1.ecp-3 (err =  0x1.f0f53c10a041ep-64)
    {  0x1.85ffffabe2dd5p-1,   0x1.5015019891290p+0,  -0x1.16b5cd97aa000p-2,  -0x1.e8000150748acp-3}, // -122 * 0x1p-9 = -0x1.e8p-3 (err =  0x1.fac760ac55b72p-64)
    {  0x1.8700005f4e59bp-1,   0x1.4f38f5dc1ef24p+0,  -0x1.14167df9ce000p-2,  -0x1.e3fffe82c6994p-3}, // -121 * 0x1p-9 = -0x1.e4p-3 (err =  0x1.f4dff1f6a8d2ep-64)
    {  0x1.880000555fa38p-1,   0x1.4e5e0a2a1df31p+0,  -0x1.1178e7437a000p-2,  -0x1.dffffeaa81720p-3}, // -120 * 0x1p-9 = -0x1.e0p-3 (err =  0x1.fdd67627c4576p-64)
    {  0x1.890000545d5bap-1,   0x1.4d843ba62a54bp+0,  -0x1.0edd052fa6000p-2,  -0x1.dbfffeae8a918p-3}, // -119 * 0x1p-9 = -0x1.dcp-3 (err =  0x1.efc1d3824f001p-64)
    {  0x1.8a00005937c1dp-1,   0x1.4cab882706706p+0,  -0x1.0c42d58e36000p-2,  -0x1.d7fffe9b20f8cp-3}, // -118 * 0x1p-9 = -0x1.d8p-3 (err =  0x1.f95e19cbd10e4p-64)
    {  0x1.8b00005b67493p-1,   0x1.4bd3ed8d9ff16p+0,  -0x1.09aa564178000p-2,  -0x1.d3fffe9262db4p-3}, // -117 * 0x1p-9 = -0x1.d4p-3 (err =  0x1.e32cb2ab350c2p-64)
    {  0x1.8bffff93a606fp-1,   0x1.4afd6a5fbc467p+0,  -0x1.0713871d04000p-2,  -0x1.d00001b167e44p-3}, // -116 * 0x1p-9 = -0x1.d0p-3 (err =  0x1.fa3087120dc09p-64)
    {  0x1.8d00000ce1d39p-1,   0x1.4a27facca9846p+0,  -0x1.047e60acae000p-2,  -0x1.cbffffcc78b1cp-3}, // -115 * 0x1p-9 = -0x1.ccp-3 (err =  0x1.fd62244db073ep-64)
    {  0x1.8e00005b5836ep-1,   0x1.49539def97aadp+0,  -0x1.01eae47768000p-2,  -0x1.c7fffe929f248p-3}, // -114 * 0x1p-9 = -0x1.c8p-3 (err =  0x1.ee32af7515e72p-64)
    {  0x1.8f00006d0060ep-1,   0x1.488051c65689ep+0,  -0x1.feb2210f24000p-3,  -0x1.c3fffe4bfe7c8p-3}, // -113 * 0x1p-9 = -0x1.c4p-3 (err =  0x1.f81a92c6a38edp-64)
    {  0x1.8fffffc52570ep-1,   0x1.47ae14ab17ce3p+0,  -0x1.f991c7f890000p-3,  -0x1.c00000eb6a3c8p-3}, // -112 * 0x1p-9 = -0x1.c0p-3 (err =  0x1.fbe26227e0b3fp-64)
    {  0x1.90ffff9deb1f3p-1,   0x1.46dce39588c1bp+0,  -0x1.f474b329cc000p-3,  -0x1.bc00018853834p-3}, // -111 * 0x1p-9 = -0x1.bcp-3 (err =  0x1.fbccc391d8558p-64)
    {  0x1.92000028991cbp-1,   0x1.460cbc5e6f71ap+0,  -0x1.ef5add7efc000p-3,  -0x1.b7ffff5d9b8d4p-3}, // -110 * 0x1p-9 = -0x1.b8p-3 (err =  0x1.f99dac329a00ap-64)
    {  0x1.92ffffd7480d4p-1,   0x1.453d9e4d540f6p+0,  -0x1.ea444abf38000p-3,  -0x1.b40000a2dfcb0p-3}, // -109 * 0x1p-9 = -0x1.b4p-3 (err =  0x1.ff78223a8834cp-64)
    {  0x1.9400004af240cp-1,   0x1.446f8619fdf0ap+0,  -0x1.e530ee8284000p-3,  -0x1.affffed436fd0p-3}, // -108 * 0x1p-9 = -0x1.b0p-3 (err =  0x1.f2d0754cbeba6p-64)
    {  0x1.9500006c63939p-1,   0x1.43a272b421deap+0,  -0x1.e020ca3e1c000p-3,  -0x1.abfffe4e71b1cp-3}, // -107 * 0x1p-9 = -0x1.acp-3 (err =  0x1.eb29ea6f274fbp-64)
    {  0x1.960000523bd96p-1,   0x1.42d6621bee410p+0,  -0x1.db13d96e78000p-3,  -0x1.a7fffeb7109a8p-3}, // -106 * 0x1p-9 = -0x1.a8p-3 (err =  0x1.f1b3c9a52e187p-64)
    {  0x1.97000053b55e5p-1,   0x1.420b5223a94e7p+0,  -0x1.d60a1653cc000p-3,  -0x1.a3fffeb12a86cp-3}, // -105 * 0x1p-9 = -0x1.a4p-3 (err =  0x1.f4afcbcdc641ep-64)
    {  0x1.9800004aa5713p-1,   0x1.414141067aae8p+0,  -0x1.d1037dafa4000p-3,  -0x1.9ffffed56a3b4p-3}, // -104 * 0x1p-9 = -0x1.a0p-3 (err =  0x1.f1184575c7c62p-64)
    {  0x1.9900001b4e7fcp-1,   0x1.40782cfb80f99p+0,  -0x1.cc000c14f8000p-3,  -0x1.9bffff92c6010p-3}, // -103 * 0x1p-9 = -0x1.9cp-3 (err =  0x1.fe7c3cdf67207p-64)
    {  0x1.9a0000602dd08p-1,   0x1.3fb013b002f8bp+0,  -0x1.c6ffba8e94000p-3,  -0x1.97fffe7f48be0p-3}, // -102 * 0x1p-9 = -0x1.98p-3 (err =  0x1.e943a823e8585p-64)
    {  0x1.9affffb7212d5p-1,   0x1.3ee8f462e5e6ap+0,  -0x1.c2028c1c9c000p-3,  -0x1.940001237b4acp-3}, // -101 * 0x1p-9 = -0x1.94p-3 (err =  0x1.ff1163616f2f8p-64)
    {  0x1.9c0000315b837p-1,   0x1.3e22cba82dbdbp+0,  -0x1.bd08728e64000p-3,  -0x1.8fffff3a91f24p-3}, // -100 * 0x1p-9 = -0x1.90p-3 (err =  0x1.f28d8c72ac254p-64)
    {  0x1.9cffff9b6d4a2p-1,   0x1.3d5d9967f0282p+0,  -0x1.b81174fe3c000p-3,  -0x1.8c0001924ad78p-3}, //  -99 * 0x1p-9 = -0x1.8cp-3 (err =  0x1.e5ec64c3ed19fp-64)
    {  0x1.9e00003830411p-1,   0x1.3c995a1cc2a52p+0,  -0x1.b31d845fc8000p-3,  -0x1.87ffff1f3efbcp-3}, //  -98 * 0x1p-9 = -0x1.88p-3 (err =  0x1.f95b1752f51aap-64)
    {  0x1.9effffbdbbcf2p-1,   0x1.3bd60dc4a1326p+0,  -0x1.ae2ca83d78000p-3,  -0x1.8400010910c38p-3}, //  -97 * 0x1p-9 = -0x1.84p-3 (err =  0x1.f92f3fcdef0cep-64)
    {  0x1.a00000674d943p-1,   0x1.3b13b0ecd5e7bp+0,  -0x1.a93ed1cc1c000p-3,  -0x1.7ffffe62c9af4p-3}, //  -96 * 0x1p-9 = -0x1.80p-3 (err =  0x1.effbb08494ef2p-64)
    {  0x1.a0ffffa9ab8c3p-1,   0x1.3a5243c8bf208p+0,  -0x1.a45409d668000p-3,  -0x1.7c00015951cf4p-3}, //  -95 * 0x1p-9 = -0x1.7cp-3 (err =  0x1.f71eeffbd8e9ap-64)
    {  0x1.a200001f61b92p-1,   0x1.3991c2a9fd560p+0,  -0x1.9f6c3fd6c8000p-3,  -0x1.77ffff82791b8p-3}, //  -94 * 0x1p-9 = -0x1.78p-3 (err =  0x1.fb148b050f6abp-64)
    {  0x1.a2ffff9fa9a4cp-1,   0x1.38d22d7e4d310p+0,  -0x1.9a877ab59c000p-3,  -0x1.74000181596d0p-3}, //  -93 * 0x1p-9 = -0x1.74p-3 (err =  0x1.f5c4b8af73903p-64)
    {  0x1.a400004bb8d2fp-1,   0x1.381380ffcfd56p+0,  -0x1.95a5ac5e34000p-3,  -0x1.6ffffed11cb44p-3}, //  -92 * 0x1p-9 = -0x1.70p-3 (err =  0x1.ff57c93246ae4p-64)
    {  0x1.a500006a4e7c3p-1,   0x1.3755bccdf6e8bp+0,  -0x1.90c6d99aa8000p-3,  -0x1.6bfffe56c60f4p-3}, //  -91 * 0x1p-9 = -0x1.6cp-3 (err =  0x1.f367749e43ff3p-64)
    {  0x1.a600005b4ab44p-1,   0x1.3698defaaf54dp+0,  -0x1.8beafcf884000p-3,  -0x1.67fffe92d52f0p-3}, //  -90 * 0x1p-9 = -0x1.68p-3 (err =  0x1.feb9825f2baabp-64)
    {  0x1.a700004efd17fp-1,   0x1.35dce5c015ffdp+0,  -0x1.871211f6a0000p-3,  -0x1.63fffec40ba04p-3}, //  -89 * 0x1p-9 = -0x1.64p-3 (err =  0x1.fc33b40daae4bp-64)
    {  0x1.a7ffff92131e8p-1,   0x1.3521d002dca4ep+0,  -0x1.823c186810000p-3,  -0x1.600001b7b3860p-3}, //  -88 * 0x1p-9 = -0x1.60p-3 (err =  0x1.ee6ed6d4c3c81p-64)
    {  0x1.a900005ad8a59p-1,   0x1.34679a8c14d48p+0,  -0x1.7d69021530000p-3,  -0x1.5bfffe949d69cp-3}, //  -87 * 0x1p-9 = -0x1.5cp-3 (err =  0x1.f8943b7b742adp-64)
    {  0x1.aa000013862c7p-1,   0x1.33ae45a761d61p+0,  -0x1.7898d7f668000p-3,  -0x1.57ffffb1e74e4p-3}, //  -86 * 0x1p-9 = -0x1.58p-3 (err =  0x1.fc1e271c89d16p-64)
    {  0x1.ab00005d6e104p-1,   0x1.32f5ce9377be9p+0,  -0x1.73cb8eb4e0000p-3,  -0x1.53fffe8a47bf0p-3}, //  -85 * 0x1p-9 = -0x1.54p-3 (err =  0x1.f5dcee71d838cp-64)
    {  0x1.ac0000681b14fp-1,   0x1.323e345833a36p+0,  -0x1.6f0126c530000p-3,  -0x1.4ffffe5f93ac4p-3}, //  -84 * 0x1p-9 = -0x1.50p-3 (err =  0x1.ffa4e416ad790p-64)
    {  0x1.ad00005ac579fp-1,   0x1.3187754df9411p+0,  -0x1.6a399bfa68000p-3,  -0x1.4bfffe94ea184p-3}, //  -83 * 0x1p-9 = -0x1.4cp-3 (err =  0x1.f1f2fad6607d6p-64)
    {  0x1.ae0000599939ap-1,   0x1.30d18fd38958dp+0,  -0x1.6574ea3e04000p-3,  -0x1.47fffe999b198p-3}, //  -82 * 0x1p-9 = -0x1.48p-3 (err =  0x1.ffe0e2eb3a200p-64)
    {  0x1.af00006262d4fp-1,   0x1.301c8266d4736p+0,  -0x1.60b30e3788000p-3,  -0x1.43fffe7674ac4p-3}, //  -81 * 0x1p-9 = -0x1.44p-3 (err =  0x1.ffd26a1ab0b07p-64)
    {  0x1.b000004f65e84p-1,   0x1.2f684ba24f6dcp+0,  -0x1.5bf4053cdc000p-3,  -0x1.3ffffec2685f0p-3}, //  -80 * 0x1p-9 = -0x1.40p-3 (err =  0x1.f27139e8192dep-64)
    {  0x1.b1000062d3611p-1,   0x1.2eb4e9dad67eep+0,  -0x1.5737cabcac000p-3,  -0x1.3bfffe74b27bcp-3}, //  -79 * 0x1p-9 = -0x1.3cp-3 (err =  0x1.f883ed07e0110p-64)
    {  0x1.b200006e2d9fap-1,   0x1.2e025bb80c7bdp+0,  -0x1.527e5c4230000p-3,  -0x1.37fffe4749818p-3}, //  -78 * 0x1p-9 = -0x1.38p-3 (err =  0x1.f9822a95452efp-64)
    {  0x1.b30000383ca79p-1,   0x1.2d509febe0cb0p+0,  -0x1.4dc7b78ef8000p-3,  -0x1.33ffff1f0d61cp-3}, //  -77 * 0x1p-9 = -0x1.34p-3 (err =  0x1.f9bad0bf57021p-64)
    {  0x1.b400006e9a4fep-1,   0x1.2c9fb48bcff36p+0,  -0x1.4913d62bb4000p-3,  -0x1.2ffffe4596c08p-3}, //  -76 * 0x1p-9 = -0x1.30p-3 (err =  0x1.f6f4ae2688688p-64)
    {  0x1.b4ffffc8d14d1p-1,   0x1.2bef990b83555p+0,  -0x1.4462badf38000p-3,  -0x1.2c0000dcbacbcp-3}, //  -75 * 0x1p-9 = -0x1.2cp-3 (err =  0x1.fe8cdc7d23969p-64)
    {  0x1.b5ffffaeed29ap-1,   0x1.2b404b0776dc4p+0,  -0x1.3fb45bd4a8000p-3,  -0x1.280001444b598p-3}, //  -74 * 0x1p-9 = -0x1.28p-3 (err =  0x1.f509370c6d194p-64)
    {  0x1.b6fffff242ad4p-1,   0x1.2a91c938943d7p+0,  -0x1.3b08b6b598000p-3,  -0x1.24000036f54b0p-3}, //  -73 * 0x1p-9 = -0x1.24p-3 (err =  0x1.ffa062b5e0917p-64)
    {  0x1.b80000393ad75p-1,   0x1.29e41277822e6p+0,  -0x1.365fc9f6f8000p-3,  -0x1.1fffff1b14a2cp-3}, //  -72 * 0x1p-9 = -0x1.20p-3 (err =  0x1.f72ac383f10a6p-64)
    {  0x1.b9000056b20d0p-1,   0x1.29372581126f7p+0,  -0x1.31b9934108000p-3,  -0x1.1bfffea537cc0p-3}, //  -71 * 0x1p-9 = -0x1.1cp-3 (err =  0x1.eafa2ceec47ccp-64)
    {  0x1.ba00003d5fbc4p-1,   0x1.288b00ff5dd12p+0,  -0x1.2d160fac08000p-3,  -0x1.17ffff0a810f0p-3}, //  -70 * 0x1p-9 = -0x1.18p-3 (err =  0x1.fe66ab477c823p-64)
    {  0x1.bb00003c849c9p-1,   0x1.27dfa361b197bp+0,  -0x1.28753aa954000p-3,  -0x1.13ffff0ded8dcp-3}, //  -69 * 0x1p-9 = -0x1.14p-3 (err =  0x1.fd3c1172e0083p-64)
    {  0x1.bc00005f97310p-1,   0x1.27350b488404ap+0,  -0x1.23d710ebb0000p-3,  -0x1.0ffffe81a33c0p-3}, //  -68 * 0x1p-9 = -0x1.10p-3 (err =  0x1.eb569040e5b60p-64)
    {  0x1.bd00006615b7cp-1,   0x1.268b3789ce435p+0,  -0x1.1f3b908954000p-3,  -0x1.0bfffe67a9210p-3}, //  -67 * 0x1p-9 = -0x1.0cp-3 (err =  0x1.e8b3f9976162bp-64)
    {  0x1.be00005e968b4p-1,   0x1.25e226c9b575ep+0,  -0x1.1aa2b62fe8000p-3,  -0x1.07fffe85a5d30p-3}, //  -66 * 0x1p-9 = -0x1.08p-3 (err =  0x1.ec8abe6ffb859p-64)
    {  0x1.beffffbdd5afap-1,   0x1.2539d8147ec93p+0,  -0x1.160c8153d8000p-3,  -0x1.04000108a9418p-3}, //  -65 * 0x1p-9 = -0x1.04p-3 (err =  0x1.fd1f8a2da6194p-64)
    {  0x1.bfffff8e37bfcp-1,   0x1.2492496ee0d13p+0,  -0x1.1178ea2aa4000p-3,  -0x1.000001c721010p-3}, //  -64 * 0x1p-9 = -0x1.00p-3 (err =  0x1.e665f8bb19781p-64)
    {  0x1.c100007296fe8p-1,   0x1.23eb7926f5b6fp+0,  -0x1.0ce7ead220000p-3,  -0x1.f7fffc6b480c0p-4}, //  -63 * 0x1p-9 = -0x1.f8p-4 (err =  0x1.f269c7d2b86dbp-64)
    {  0x1.c1ffffe1c2830p-1,   0x1.2345679d3ea18p+0,  -0x1.08598be384000p-3,  -0x1.f00000f1ebe80p-4}, //  -62 * 0x1p-9 = -0x1.f0p-4 (err =  0x1.fec9b01c6b38bp-64)
    {  0x1.c300005a9b654p-1,   0x1.22a011ef9dfa4p+0,  -0x1.03cdbf09ac000p-3,  -0x1.e7fffd2b24d60p-4}, //  -61 * 0x1p-9 = -0x1.e8p-4 (err =  0x1.fd3d0aef1454fp-64)
    {  0x1.c3ffffa7aa76dp-1,   0x1.21fb784acb842p+0,  -0x1.fe8916be38000p-4,  -0x1.e00002c2ac498p-4}, //  -60 * 0x1p-9 = -0x1.e0p-4 (err =  0x1.fe77f30ecdf85p-64)
    {  0x1.c500003d5d0dep-1,   0x1.215797dd539cbp+0,  -0x1.f57bc5ae28000p-4,  -0x1.d7fffe1517910p-4}, //  -59 * 0x1p-9 = -0x1.d8p-4 (err =  0x1.f6570c073bcffp-64)
    {  0x1.c60000261a066p-1,   0x1.20b470ae41559p+0,  -0x1.ec7396d8e0000p-4,  -0x1.cffffecf2fcd0p-4}, //  -58 * 0x1p-9 = -0x1.d0p-4 (err =  0x1.fd41b0afffdc7p-64)
    {  0x1.c6ffff939ad5ep-1,   0x1.20120164b299ap+0,  -0x1.e37082b2d0000p-4,  -0x1.c800036329510p-4}, //  -57 * 0x1p-9 = -0x1.c8p-4 (err =  0x1.fd87e212a406cp-64)
    {  0x1.c7ffff8b8e61ep-1,   0x1.1f70482578646p+0,  -0x1.da727a4e38000p-4,  -0x1.c00003a38cf10p-4}, //  -56 * 0x1p-9 = -0x1.c0p-4 (err =  0x1.e37b015ac7d96p-64)
    {  0x1.c900002bea282p-1,   0x1.1ecf43ac6c077p+0,  -0x1.d17976f880000p-4,  -0x1.b7fffea0aebf0p-4}, //  -55 * 0x1p-9 = -0x1.b8p-4 (err =  0x1.fba4437118897p-64)
    {  0x1.c9ffffcedcbb7p-1,   0x1.1e2ef3d2afbdep+0,  -0x1.c88581d338000p-4,  -0x1.b00001891a248p-4}, //  -54 * 0x1p-9 = -0x1.b0p-4 (err =  0x1.fbe88ec707d22p-64)
    {  0x1.caffffc416915p-1,   0x1.1d8f56982a9f7p+0,  -0x1.bf968980a0000p-4,  -0x1.a80001df4b758p-4}, //  -53 * 0x1p-9 = -0x1.a8p-4 (err =  0x1.f700674c84e42p-64)
    {  0x1.cbffffa09253cp-1,   0x1.1cf06b15449cdp+0,  -0x1.b6ac8c2c90000p-4,  -0x1.a00002fb6d620p-4}, //  -52 * 0x1p-9 = -0x1.a0p-4 (err =  0x1.fce2790d9013fp-64)
    {  0x1.cd000022febc6p-1,   0x1.1c522fac38bc3p+0,  -0x1.adc77daec0000p-4,  -0x1.97fffee80a1d0p-4}, //  -51 * 0x1p-9 = -0x1.98p-4 (err =  0x1.fc049bc64055cp-64)
    {  0x1.ce00003928d4bp-1,   0x1.1bb4a3e15512ap+0,  -0x1.a4e7621058000p-4,  -0x1.8ffffe36b95a8p-4}, //  -50 * 0x1p-9 = -0x1.90p-4 (err =  0x1.f33e1897dc360p-64)
    {  0x1.cf00002e58a37p-1,   0x1.1b17c662d543bp+0,  -0x1.9c0c313ad0000p-4,  -0x1.87fffe8d3ae48p-4}, //  -49 * 0x1p-9 = -0x1.88p-4 (err =  0x1.f5d8260aa36c4p-64)
    {  0x1.d000005783ed5p-1,   0x1.1a7b95dc603c2p+0,  -0x1.9335e2d108000p-4,  -0x1.7ffffd43e0958p-4}, //  -48 * 0x1p-9 = -0x1.80p-4 (err =  0x1.fd8747dbdd634p-64)
    {  0x1.d0ffff9fbe60fp-1,   0x1.19e011d85a714p+0,  -0x1.8a647af900000p-4,  -0x1.780003020cf88p-4}, //  -47 * 0x1p-9 = -0x1.78p-4 (err =  0x1.f5db2ba9e50efp-64)
    {  0x1.d2000043ad88fp-1,   0x1.194537dff0bd1p+0,  -0x1.8197e0a130000p-4,  -0x1.6ffffde293b88p-4}, //  -46 * 0x1p-9 = -0x1.70p-4 (err =  0x1.fc22ca5b24d14p-64)
    {  0x1.d30000581e4f3p-1,   0x1.18ab08040d24fp+0,  -0x1.78d01f5ef8000p-4,  -0x1.67fffd3f0d868p-4}, //  -45 * 0x1p-9 = -0x1.68p-4 (err =  0x1.fe5240e76f4dbp-64)
    {  0x1.d3ffffbd63b6ap-1,   0x1.1811813fee37fp+0,  -0x1.700d32f5a8000p-4,  -0x1.60000214e24b0p-4}, //  -44 * 0x1p-9 = -0x1.60p-4 (err =  0x1.fc52bb27a79d5p-64)
    {  0x1.d500007774e9dp-1,   0x1.1778a14a8ea00p+0,  -0x1.674f048020000p-4,  -0x1.57fffc4458b18p-4}, //  -43 * 0x1p-9 = -0x1.58p-4 (err =  0x1.f629867124176p-64)
    {  0x1.d600004046b7bp-1,   0x1.16e0686e03bcbp+0,  -0x1.5e95a2a950000p-4,  -0x1.4ffffdfdca428p-4}, //  -42 * 0x1p-9 = -0x1.50p-4 (err =  0x1.f171b4210108cp-64)
    {  0x1.d700005a97418p-1,   0x1.1648d4da3cdc2p+0,  -0x1.55e0fd3d10000p-4,  -0x1.47fffd2b45f40p-4}, //  -41 * 0x1p-9 = -0x1.48p-4 (err =  0x1.fdf6e85b175bdp-64)
    {  0x1.d800002d6d719p-1,   0x1.15b1e5dc98699p+0,  -0x1.4d311447d0000p-4,  -0x1.3ffffe9494738p-4}, //  -40 * 0x1p-9 = -0x1.40p-4 (err =  0x1.ffd467d8e8088p-64)
    {  0x1.d900005b64a35p-1,   0x1.151b9a0a52673p+0,  -0x1.4485dd2650000p-4,  -0x1.37fffd24dae58p-4}, //  -39 * 0x1p-9 = -0x1.38p-4 (err =  0x1.f761a9b1ed017p-64)
    {  0x1.da000063b5837p-1,   0x1.1485f0a681b1dp+0,  -0x1.3bdf571f80000p-4,  -0x1.2ffffce253e48p-4}, //  -38 * 0x1p-9 = -0x1.30p-4 (err =  0x1.f82a34f04caaap-64)
    {  0x1.db00007797b18p-1,   0x1.13f0e88dcaeabp+0,  -0x1.333d7b7a40000p-4,  -0x1.27fffc4342740p-4}, //  -37 * 0x1p-9 = -0x1.28p-4 (err =  0x1.f984921657485p-64)
    {  0x1.dc000061bcda0p-1,   0x1.135c80dad22fcp+0,  -0x1.2aa046fb68000p-4,  -0x1.1ffffcf219300p-4}, //  -36 * 0x1p-9 = -0x1.20p-4 (err =  0x1.e754e30cf4744p-64)
    {  0x1.dd00004e49233p-1,   0x1.12c8b871c2ffep+0,  -0x1.2207b32748000p-4,  -0x1.17fffd8db6e68p-4}, //  -35 * 0x1p-9 = -0x1.18p-4 (err =  0x1.ff32ed4dc00a8p-64)
    {  0x1.ddffff9e32928p-1,   0x1.12358eadedf82p+0,  -0x1.1973c05a78000p-4,  -0x1.1000030e6b6c0p-4}, //  -34 * 0x1p-9 = -0x1.10p-4 (err =  0x1.fd23e474a42c9p-64)
    {  0x1.df00006fe6458p-1,   0x1.11a3015a87d96p+0,  -0x1.10e4577fd0000p-4,  -0x1.07fffc80cdd40p-4}, //  -33 * 0x1p-9 = -0x1.08p-4 (err =  0x1.fd4afd5e9fee0p-64)
    {  0x1.e000003be08cdp-1,   0x1.111110ef00d33p+0,  -0x1.0859895af0000p-4,  -0x1.fffffc41f7330p-5}, //  -32 * 0x1p-9 = -0x1.00p-4 (err =  0x1.fcbd59e74c2bbp-64)
    {  0x1.e0ffffb437e4ap-1,   0x1.107fbc0affb1cp+0,  -0x1.ffa6962560000p-5,  -0x1.f00004bc81b60p-5}, //  -31 * 0x1p-9 = -0x1.f0p-5 (err =  0x1.f45422bbe3ed7p-64)
    {  0x1.e200003a7272bp-1,   0x1.0fef00eef586ap+0,  -0x1.eea3181f10000p-5,  -0x1.dffffc58d8d50p-5}, //  -30 * 0x1p-9 = -0x1.e0p-5 (err =  0x1.fd5259662b96ap-64)
    {  0x1.e300002086f7dp-1,   0x1.0f5edf98ebe55p+0,  -0x1.dda8ab9ed0000p-5,  -0x1.cffffdf790830p-5}, //  -29 * 0x1p-9 = -0x1.d0p-5 (err =  0x1.fbe5e8a90cb50p-64)
    {  0x1.e3ffff9615511p-1,   0x1.0ecf56f9ad1e5p+0,  -0x1.ccb743de90000p-5,  -0x1.c000069eaaef0p-5}, //  -28 * 0x1p-9 = -0x1.c0p-5 (err =  0x1.f204a69397a6dp-64)
    {  0x1.e500007a6965dp-1,   0x1.0e406513f0308p+0,  -0x1.bbceb7b2f0000p-5,  -0x1.affff85969a30p-5}, //  -27 * 0x1p-9 = -0x1.b0p-5 (err =  0x1.fd81b6ca0701bp-64)
    {  0x1.e6000019cdad2p-1,   0x1.0db20a7aa2bb0p+0,  -0x1.aaef2b5cc0000p-5,  -0x1.9ffffe63252e0p-5}, //  -26 * 0x1p-9 = -0x1.a0p-5 (err =  0x1.fb8ac955c0435p-64)
    {  0x1.e7000024b75b2p-1,   0x1.0d24454f0f503p+0,  -0x1.9a1878eda0000p-5,  -0x1.8ffffdb48a4e0p-5}, //  -25 * 0x1p-9 = -0x1.90p-5 (err =  0x1.fb945ee89d6b7p-64)
    {  0x1.e800003deb22ep-1,   0x1.0c9714d9b9567p+0,  -0x1.894a9d3a90000p-5,  -0x1.7ffffc214dd20p-5}, //  -24 * 0x1p-9 = -0x1.80p-5 (err =  0x1.fbf410b742e39p-64)
    {  0x1.e900004f09c6ep-1,   0x1.0c0a783d61212p+0,  -0x1.7885907740000p-5,  -0x1.6ffffb0f63920p-5}, //  -23 * 0x1p-9 = -0x1.70p-5 (err =  0x1.f7953467e9e8ep-64)
    {  0x1.ea00004294785p-1,   0x1.0b7e6e9e012f6p+0,  -0x1.67c94ad430000p-5,  -0x1.5ffffbd6b87b0p-5}, //  -22 * 0x1p-9 = -0x1.60p-5 (err =  0x1.fb85782349931p-64)
    {  0x1.eaffff8b33a48p-1,   0x1.0af2f7626f27bp+0,  -0x1.5715cc5cf0000p-5,  -0x1.5000074cc5b80p-5}, //  -21 * 0x1p-9 = -0x1.50p-5 (err =  0x1.f95afd23cae08p-64)
    {  0x1.ebffff8c7ded1p-1,   0x1.0a6810e50c8b8p+0,  -0x1.466af4c620000p-5,  -0x1.40000738212f0p-5}, //  -20 * 0x1p-9 = -0x1.40p-5 (err =  0x1.fdfee350f4aa4p-64)
    {  0x1.ed00001f03372p-1,   0x1.09ddba5a3ebbcp+0,  -0x1.35c8bda6c0000p-5,  -0x1.2ffffe0fcc8e0p-5}, //  -19 * 0x1p-9 = -0x1.30p-5 (err =  0x1.fffed1d8eb562p-64)
    {  0x1.ee00006af8497p-1,   0x1.0953f3569c740p+0,  -0x1.252f2c0af0000p-5,  -0x1.1ffff9507b690p-5}, //  -18 * 0x1p-9 = -0x1.20p-5 (err =  0x1.ea7e7c47e665bp-64)
    {  0x1.ef0000448da39p-1,   0x1.08cabb12aa7bep+0,  -0x1.149e39d180000p-5,  -0x1.0ffffbb725c70p-5}, //  -17 * 0x1p-9 = -0x1.10p-5 (err =  0x1.f242df0264b22p-64)
    {  0x1.f0000074850a3p-1,   0x1.084210460cc40p+0,  -0x1.0415d11a00000p-5,  -0x1.fffff16f5eba0p-6}, //  -16 * 0x1p-9 = -0x1.00p-5 (err =  0x1.ea7af9087796cp-64)
    {  0x1.f1000071a6ca7p-1,   0x1.07b9f25f3febep+0,  -0x1.e72be3dea0000p-6,  -0x1.dffff1cb26b20p-6}, //  -15 * 0x1p-9 = -0x1.e0p-6 (err =  0x1.f23b931cd4e19p-64)
    {  0x1.f200007403693p-1,   0x1.073260672f1f2p+0,  -0x1.c63d1fd880000p-6,  -0x1.bffff17f92da0p-6}, //  -14 * 0x1p-9 = -0x1.c0p-6 (err =  0x1.f5110e3c8382ep-64)
    {  0x1.f2ffff88d88ffp-1,   0x1.06ab5a0649ef9p+0,  -0x1.a55f63d4a0000p-6,  -0x1.a0000ee4ee020p-6}, //  -13 * 0x1p-9 = -0x1.a0p-6 (err =  0x1.fc497685e13dap-64)
    {  0x1.f400001ebe810p-1,   0x1.0624dd1efc36ap+0,  -0x1.84924e9d20000p-6,  -0x1.7ffffc282fe00p-6}, //  -12 * 0x1p-9 = -0x1.80p-6 (err =  0x1.fa07b4c0fe02cp-64)
    {  0x1.f4ffffb574859p-1,   0x1.059eea2e14aedp+0,  -0x1.63d6210c60000p-6,  -0x1.600009516f4e0p-6}, //  -11 * 0x1p-9 = -0x1.60p-6 (err =  0x1.f2dc2f821bd85p-64)
    {  0x1.f600007cfca45p-1,   0x1.05197f3c7132dp+0,  -0x1.432a826a40000p-6,  -0x1.3ffff0606b760p-6}, //  -10 * 0x1p-9 = -0x1.40p-6 (err =  0x1.fe82cf7799199p-64)
    {  0x1.f700003f0374bp-1,   0x1.04949ca0c15a9p+0,  -0x1.228fa9fa20000p-6,  -0x1.1ffff81f916a0p-6}, //   -9 * 0x1p-9 = -0x1.20p-6 (err =  0x1.fbf8f391aaf74p-64)
    {  0x1.f7ffff8dfafd8p-1,   0x1.0410413ee5c10p+0,  -0x1.02057403c0000p-6,  -0x1.00000e40a0500p-6}, //   -8 * 0x1p-9 = -0x1.00p-6 (err =  0x1.fb1bfd125ff74p-64)
    {  0x1.f8ffffd897ae5p-1,   0x1.038c6b8c65774p+0,  -0x1.c317424980000p-7,  -0x1.c00009da146c0p-7}, //   -7 * 0x1p-9 = -0x1.c0p-7 (err =  0x1.feb3b2f03a6d4p-64)
    {  0x1.fa00007d0c904p-1,   0x1.03091b11f1c0fp+0,  -0x1.82446a9680000p-7,  -0x1.7fffe0bcdbf00p-7}, //   -6 * 0x1p-9 = -0x1.80p-7 (err =  0x1.e8fce6f296b71p-64)
    {  0x1.fb00003f20acap-1,   0x1.02864fa7421efp+0,  -0x1.41928fa680000p-7,  -0x1.3ffff037d4d80p-7}, //   -5 * 0x1p-9 = -0x1.40p-7 (err =  0x1.ff5bf6c8b0eb7p-64)
    {  0x1.fc0000366d813p-1,   0x1.020407f47b5afp+0,  -0x1.010149a1c0000p-7,  -0x1.ffffe4c93f680p-8}, //   -4 * 0x1p-9 = -0x1.00p-7 (err =  0x1.fdfe732f2033ep-64)
    {  0x1.fd000062dde0ep-1,   0x1.018243331315fp+0,  -0x1.8120ef8c00000p-8,  -0x1.7fffce910f900p-8}, //   -3 * 0x1p-9 = -0x1.80p-8 (err =  0x1.fc0b400596190p-64)
    {  0x1.fe00006877316p-1,   0x1.010100cc5c53ap+0,  -0x1.0080212580000p-8,  -0x1.ffff9788cea00p-9}, //   -2 * 0x1p-9 = -0x1.00p-8 (err =  0x1.f8d73abd7a656p-64)
    {  0x1.ff0000315e9cep-1,   0x1.0080400747f7cp+0,  -0x1.003fe3e600000p-9, -0x1.ffff9d42c6400p-10}, //   -1 * 0x1p-9 = -0x1.00p-9 (err =  0x1.fc642421b0226p-64)
    {  0x1.0000000000000p+0,   0x1.0000000000000p+0,   0x0.0000000000000p+0,   0x0.0000000000000p+0}, //    0 * 0x1p-9 = 0x0.00p+0 (err =   0x0.0000000000000p+0)
};// Worst deviation:  ap1 =   0x1.73ffff93e0847p+0, log1pa =   0x1.7eaf828e8a000p-2, a =   0x1.cffffe4f8211cp-2,  err =  0x1.e6f90ed8988d4p-64, d = 0x1.b07dee4p-26

static const logdata_t LOG2_LOOKUP_BIG[] = { //ap1, va, log1pa, a: va = 1/(a+1), log1pa = log2(1+a)+err
    {  0x1.0000000000000p+0,   0x1.0000000000000p+0,   0x0.0000000000000p+0,   0x0.0000000000000p+0}, //    0 * 0x1p-8 = 0x0.00p+0 (err =   0x0.0000000000000p+0)
    {  0x1.00ffffff861bfp+0,   0x1.fe01fe02efe56p-1,   0x1.709c462880000p-8,   0x1.ffffff0c37e00p-9}, //    1 * 0x1p-8 = 0x1.00p-8 (err =  0x1.b1fda027dbf73p-67)
    {  0x1.0200000036ebfp+0,   0x1.fc07f01f54597p-1,   0x1.6fe50b9640000p-7,   0x1.0000001b75f80p-7}, //    2 * 0x1p-8 = 0x1.00p-7 (err =  0x1.85e67279db04fp-64)
    {  0x1.03000001838afp+0,   0x1.fa11ca9d2a654p-1,   0x1.13631204c0000p-6,   0x1.800000c1c5780p-7}, //    3 * 0x1p-8 = 0x1.80p-7 (err =  0x1.0bcac2309190ap-65)
    {  0x1.03ffffff01040p+0,   0x1.f81f81fa0de7ep-1,   0x1.6e796801a0000p-6,   0x1.ffffff8082000p-7}, //    4 * 0x1p-8 = 0x1.00p-6 (err =  0x1.c3c2e68078da7p-65)
    {  0x1.04ffffff63fd1p+0,   0x1.f6310acb39ea1p-1,   0x1.c9363b7120000p-6,   0x1.3fffffd8ff440p-6}, //    5 * 0x1p-8 = 0x1.40p-6 (err =  0x1.25ae56c1ddb5cp-64)
    {  0x1.05fffffe66f68p+0,   0x1.f44659e7b12fdp-1,   0x1.11cd1d0920000p-5,   0x1.7fffff99bda00p-6}, //    6 * 0x1p-8 = 0x1.80p-6 (err =  0x1.29330ae196d34p-64)
    {  0x1.0700000088991p+0,   0x1.f25f64412dd29p-1,   0x1.3ed3095e80000p-5,   0x1.c000002226440p-6}, //    7 * 0x1p-8 = 0x1.c0p-6 (err =  0x1.3920abe71b2a5p-64)
    {  0x1.08000000eb2cep+0,   0x1.f07c1f0607a9cp-1,   0x1.6bad378210000p-5,   0x1.0000001d659c0p-5}, //    8 * 0x1p-8 = 0x1.00p-5 (err =  0x1.dc3f597f11f9ep-65)
    {  0x1.08ffffffcf741p+0,   0x1.ee9c7f84b37c5p-1,   0x1.985bfc2c20000p-5,   0x1.1ffffff9ee820p-5}, //    9 * 0x1p-8 = 0x1.20p-5 (err =  0x1.035cbe923609ap-64)
    {  0x1.09ffffffc961dp+0,   0x1.ecc07b3083f95p-1,   0x1.c4dfab8730000p-5,   0x1.3ffffff92c3a0p-5}, //   10 * 0x1p-8 = 0x1.40p-5 (err =  0x1.0880304cef024p-64)
    {  0x1.0b000000a2e72p+0,   0x1.eae807aa749b5p-1,   0x1.f138984f50000p-5,   0x1.600000145ce40p-5}, //   11 * 0x1p-8 = 0x1.60p-5 (err =  0x1.0784bc3e23523p-66)
    {  0x1.0bfffffec9565p+0,   0x1.e9131ac14264ap-1,   0x1.0eb389df68000p-4,   0x1.7fffffd92aca0p-5}, //   12 * 0x1p-8 = 0x1.80p-5 (err =  0x1.12b4ca8f922dcp-64)
    {  0x1.0cfffffdb1c87p+0,   0x1.e741aa5da2272p-1,   0x1.24b5b7ae90000p-4,   0x1.9fffffb6390e0p-5}, //   13 * 0x1p-8 = 0x1.a0p-5 (err =  0x1.55c09368b8b07p-64)
    {  0x1.0dffffffb173dp+0,   0x1.e573ac90ab912p-1,   0x1.3aa2fdcbc8000p-4,   0x1.bffffff62e7a0p-5}, //   14 * 0x1p-8 = 0x1.c0p-5 (err =  0x1.ea3e82ac1f5afp-64)
    {  0x1.0efffffffce4dp+0,   0x1.e3a9179dc7326p-1,   0x1.507b835ff0000p-4,   0x1.dfffffff9c9a0p-5}, //   15 * 0x1p-8 = 0x1.e0p-5 (err =  0x1.0fc20bb5cc893p-67)
    {  0x1.0ffffffebf368p+0,   0x1.e1e1e1e41a328p-1,   0x1.663f6f9158000p-4,   0x1.ffffffd7e6d00p-5}, //   16 * 0x1p-8 = 0x1.00p-4 (err =  0x1.e1998189f9ae6p-65)
    {  0x1.11000001174f2p+0,   0x1.e01e01de32cb6p-1,   0x1.7beee98328000p-4,   0x1.1000001174f20p-4}, //   17 * 0x1p-8 = 0x1.10p-4 (err =  0x1.377ab8eed73b6p-66)
    {  0x1.120000016219dp+0,   0x1.de5d6e3d1e330p-1,   0x1.918a170238000p-4,   0x1.20000016219d0p-4}, //   18 * 0x1p-8 = 0x1.20p-4 (err =  0x1.627d246f76bb6p-71)
    {  0x1.12ffffff881b8p+0,   0x1.dca01dcad1a86p-1,   0x1.a7111de938000p-4,   0x1.2ffffff881b80p-4}, //   19 * 0x1p-8 = 0x1.30p-4 (err =  0x1.696f504d4b2efp-64)
    {  0x1.13ffffffd18a3p+0,   0x1.dae6076be80ecp-1,   0x1.bc842406f8000p-4,   0x1.3ffffffd18a30p-4}, //   20 * 0x1p-8 = 0x1.40p-4 (err =  0x1.c2d6232507c68p-64)
    {  0x1.15000001fd9f1p+0,   0x1.d92f222e8169ap-1,   0x1.d1e34e6030000p-4,   0x1.5000001fd9f10p-4}, //   21 * 0x1p-8 = 0x1.50p-4 (err =  0x1.2db4184c9ea60p-69)
    {  0x1.160000008e875p+0,   0x1.d77b654a91098p-1,   0x1.e72ec123d0000p-4,   0x1.60000008e8750p-4}, //   22 * 0x1p-8 = 0x1.60p-4 (err =  0x1.8c08390d044d6p-64)
    {  0x1.16fffffee8deap+0,   0x1.d5cac8092d2e2p-1,   0x1.fc66a0d998000p-4,   0x1.6fffffee8dea0p-4}, //   23 * 0x1p-8 = 0x1.70p-4 (err =  0x1.4956ad40969d2p-64)
    {  0x1.180000007cbfap+0,   0x1.d41d41d34cb2bp-1,   0x1.08c588d2cc000p-3,   0x1.80000007cbfa0p-4}, //   24 * 0x1p-8 = 0x1.80p-4 (err =  0x1.5206602382370p-64)
    {  0x1.18fffffea74ddp+0,   0x1.d272ca4201e01p-1,   0x1.134e1b3a68000p-3,   0x1.8fffffea74dd0p-4}, //   25 * 0x1p-8 = 0x1.90p-4 (err =  0x1.61218325c3d12p-67)
    {  0x1.19fffffef6ca6p+0,   0x1.d0cb58f8a1260p-1,   0x1.1dcd196a78000p-3,   0x1.9fffffef6ca60p-4}, //   26 * 0x1p-8 = 0x1.a0p-4 (err =  0x1.01788fca34b7ap-64)
    {  0x1.1affffff96cb6p+0,   0x1.cf26e5c4f829cp-1,   0x1.284294ac30000p-3,   0x1.affffff96cb60p-4}, //   27 * 0x1p-8 = 0x1.b0p-4 (err =  0x1.e53ed31965bb5p-64)
    {  0x1.1bfffffeeebe0p+0,   0x1.cd856891f5c12p-1,   0x1.32ae9e1c70000p-3,   0x1.bfffffeeebe00p-4}, //   28 * 0x1p-8 = 0x1.c0p-4 (err =  0x1.e5c42adfd1084p-70)
    {  0x1.1d000000da51cp+0,   0x1.cbe6d95ebc71bp-1,   0x1.3d1146e280000p-3,   0x1.d000000da51c0p-4}, //   29 * 0x1p-8 = 0x1.d0p-4 (err =  0x1.6388c7753f429p-65)
    {  0x1.1dfffffd5ba2cp+0,   0x1.ca4b305a29ebep-1,   0x1.476a9f7cf4000p-3,   0x1.dfffffd5ba2c0p-4}, //   30 * 0x1p-8 = 0x1.e0p-4 (err =  0x1.e48b8e0081f7cp-65)
    {  0x1.1f0000007afe9p+0,   0x1.c8b265aef4ec4p-1,   0x1.51bab90c98000p-3,   0x1.f0000007afe90p-4}, //   31 * 0x1p-8 = 0x1.f0p-4 (err =  0x1.7f66fadba406cp-65)
    {  0x1.1fffffff5709cp+0,   0x1.c71c71c827723p-1,   0x1.5c01a398f8000p-3,   0x1.fffffff5709c0p-4}, //   32 * 0x1p-8 = 0x1.00p-3 (err =  0x1.9422812f5f925p-64)
    {  0x1.2100000150bfbp+0,   0x1.c5894d0ec4201p-1,   0x1.663f6fba04000p-3,   0x1.0800000a85fd8p-3}, //   33 * 0x1p-8 = 0x1.08p-3 (err =  0x1.7acd18e3b6fb2p-66)
    {  0x1.21ffffff31de4p+0,   0x1.c3f8f01d80d20p-1,   0x1.70742d46bc000p-3,   0x1.0ffffff98ef20p-3}, //   34 * 0x1p-8 = 0x1.10p-3 (err =  0x1.caf10cc32f90ap-65)
    {  0x1.22ffffff45798p+0,   0x1.c26b53940ab79p-1,   0x1.7a9fec75a0000p-3,   0x1.17fffffa2bcc0p-3}, //   35 * 0x1p-8 = 0x1.18p-3 (err =  0x1.772b3981042c2p-66)
    {  0x1.23ffffff5c519p+0,   0x1.c0e0703917ac8p-1,   0x1.84c2bcfc78000p-3,   0x1.1ffffffae28c8p-3}, //   36 * 0x1p-8 = 0x1.20p-3 (err =  0x1.1ddcce6d50f34p-64)
    {  0x1.25000000f217cp+0,   0x1.bf583ee6f739cp-1,   0x1.8edcae8cdc000p-3,   0x1.2800000790be0p-3}, //   37 * 0x1p-8 = 0x1.28p-3 (err =  0x1.f3e70eb3a77f3p-64)
    {  0x1.25ffffffba70ep+0,   0x1.bdd2b899a9ea4p-1,   0x1.98edd0752c000p-3,   0x1.2ffffffdd3870p-3}, //   38 * 0x1p-8 = 0x1.30p-3 (err =  0x1.df4b975cbb561p-64)
    {  0x1.27000000d21dfp+0,   0x1.bc4fd65747707p-1,   0x1.a2f6323a44000p-3,   0x1.3800000690ef8p-3}, //   39 * 0x1p-8 = 0x1.38p-3 (err =  0x1.ab386fc3c0391p-64)
    {  0x1.27ffffff6fdc4p+0,   0x1.bacf914cf34e6p-1,   0x1.acf5e2d5b0000p-3,   0x1.3ffffffb7ee20p-3}, //   40 * 0x1p-8 = 0x1.40p-3 (err =  0x1.a02e7471645aap-66)
    {  0x1.29000000ab8cep+0,   0x1.b951e2b091090p-1,   0x1.b6ecf17ca4000p-3,   0x1.480000055c670p-3}, //   41 * 0x1p-8 = 0x1.48p-3 (err =  0x1.83a8eaba81e29p-71)
    {  0x1.29ffffffa1824p+0,   0x1.b7d6c3de2eb01p-1,   0x1.c0db6cd9ec000p-3,   0x1.4ffffffd0c120p-3}, //   42 * 0x1p-8 = 0x1.50p-3 (err =  0x1.c9c9fba84bc08p-66)
    {  0x1.2affffffd1c46p+0,   0x1.b65e2e3c32a8bp-1,   0x1.cac163c5a8000p-3,   0x1.57fffffe8e230p-3}, //   43 * 0x1p-8 = 0x1.58p-3 (err =  0x1.10a0c84d846fbp-64)
    {  0x1.2c0000010494cp+0,   0x1.b4e81b4d06354p-1,   0x1.d49ee4cd2c000p-3,   0x1.6000000824a60p-3}, //   44 * 0x1p-8 = 0x1.60p-3 (err =  0x1.9b659bbdd4b27p-64)
    {  0x1.2cffffff251fcp+0,   0x1.b37484aebd127p-1,   0x1.de73fe32b0000p-3,   0x1.67fffff928fe0p-3}, //   45 * 0x1p-8 = 0x1.68p-3 (err =  0x1.2bbad631b96c8p-65)
    {  0x1.2e000000b9c18p+0,   0x1.b2036405bd190p-1,   0x1.e840be7c00000p-3,   0x1.70000005ce0c0p-3}, //   46 * 0x1p-8 = 0x1.70p-3 (err =  0x1.d1d471cef1daep-67)
    {  0x1.2effffffda931p+0,   0x1.b094b31dc7989p-1,   0x1.f20533909c000p-3,   0x1.77fffffed4988p-3}, //   47 * 0x1p-8 = 0x1.78p-3 (err =  0x1.239ec0788eeafp-68)
    {  0x1.2ffffffdf57a5p+0,   0x1.af286bcd00081p-1,   0x1.fbc16b7c50000p-3,   0x1.7fffffefabd28p-3}, //   48 * 0x1p-8 = 0x1.80p-3 (err =  0x1.a46fbf781628dp-64)
    {  0x1.30fffffd3dd77p+0,   0x1.adbe87fd2bffap-1,   0x1.02baba1774000p-2,   0x1.87ffffe9eebb8p-3}, //   49 * 0x1p-8 = 0x1.88p-3 (err =  0x1.7b1189690a58ep-65)
    {  0x1.31ffffffe759dp+0,   0x1.ac5701ac7982bp-1,   0x1.0790adba8c000p-2,   0x1.8fffffff3ace8p-3}, //   50 * 0x1p-8 = 0x1.90p-3 (err =  0x1.4b0835369bc3dp-64)
    {  0x1.33000000bb4e7p+0,   0x1.aaf1d2f77a433p-1,   0x1.0c629758c8000p-2,   0x1.98000005da738p-3}, //   51 * 0x1p-8 = 0x1.98p-3 (err =  0x1.407850d92596cp-64)
    {  0x1.33ffffff9af19p+0,   0x1.a98ef60731dcap-1,   0x1.11307dab4c000p-2,   0x1.9ffffffcd78c8p-3}, //   52 * 0x1p-8 = 0x1.a0p-3 (err =  0x1.b944d7330acbap-65)
    {  0x1.350000006f934p+0,   0x1.a82e651274eb3p-1,   0x1.15fa676dc6000p-2,   0x1.a80000037c9a0p-3}, //   53 * 0x1p-8 = 0x1.a8p-3 (err =  0x1.d4723100c535dp-65)
    {  0x1.35ffffff5e354p+0,   0x1.a6d01a6dde526p-1,   0x1.1ac05b261c000p-2,   0x1.affffffaf1aa0p-3}, //   54 * 0x1p-8 = 0x1.b0p-3 (err =  0x1.7137e3355dc21p-65)
    {  0x1.36fffffcac7c9p+0,   0x1.a574107b0a938p-1,   0x1.1f825f5dbc000p-2,   0x1.b7ffffe563e48p-3}, //   55 * 0x1p-8 = 0x1.b8p-3 (err =  0x1.0b6c3ad51ce7fp-66)
    {  0x1.380000006ce16p+0,   0x1.a41a41a387a6ap-1,   0x1.24407ab2e4000p-2,   0x1.c0000003670b0p-3}, //   56 * 0x1p-8 = 0x1.c0p-3 (err =  0x1.42af68ae794bdp-64)
    {  0x1.38fffffce63f8p+0,   0x1.a2c2a88077be7p-1,   0x1.28fab34c90000p-2,   0x1.c7ffffe731fc0p-3}, //   57 * 0x1p-8 = 0x1.c8p-3 (err =  0x1.1c9f5ff950409p-64)
    {  0x1.3a000000946dbp+0,   0x1.a16d3f96df5e9p-1,   0x1.2db10fc794000p-2,   0x1.d0000004a36d8p-3}, //   58 * 0x1p-8 = 0x1.d0p-3 (err =  0x1.b1bbcad472c8bp-65)
    {  0x1.3afffffff7c9cp+0,   0x1.a01a01a024daap-1,   0x1.326396368c000p-2,   0x1.d7ffffffbe4e0p-3}, //   59 * 0x1p-8 = 0x1.d8p-3 (err =  0x1.e08b72b5af3c2p-64)
    {  0x1.3bfffffd09524p+0,   0x1.9ec8e954e7170p-1,   0x1.37124cdc72000p-2,   0x1.dfffffe84a920p-3}, //   60 * 0x1p-8 = 0x1.e0p-3 (err =  0x1.694bbef85bebdp-65)
    {  0x1.3d00000045905p+0,   0x1.9d79f1765bc6bp-1,   0x1.3bbd3a0b62000p-2,   0x1.e80000022c828p-3}, //   61 * 0x1p-8 = 0x1.e8p-3 (err =  0x1.d023add321d7dp-64)
    {  0x1.3e000000780dfp+0,   0x1.9c2d14edae744p-1,   0x1.406463b3de000p-2,   0x1.f0000003c06f8p-3}, //   62 * 0x1p-8 = 0x1.f0p-3 (err =  0x1.0e6115df27f57p-65)
    {  0x1.3f00000055ba6p+0,   0x1.9ae24ea4e2a1ep-1,   0x1.4507cfef62000p-2,   0x1.f8000002add30p-3}, //   63 * 0x1p-8 = 0x1.f8p-3 (err =  0x1.6da2d1fcfddf7p-65)
    {  0x1.40000000b602fp+0,   0x1.99999998b0a01p-1,   0x1.49a784c01a000p-2,   0x1.00000002d80bcp-2}, //   64 * 0x1p-8 = 0x1.00p-2 (err =  0x1.8ea3c07657241p-64)
    {  0x1.41000000c8664p+0,   0x1.9852f0d7ed259p-1,   0x1.4e4388122a000p-2,   0x1.0400000321990p-2}, //   65 * 0x1p-8 = 0x1.04p-2 (err =  0x1.54be0eacc057bp-65)
    {  0x1.41fffffeab3bdp+0,   0x1.970e4f827a4ebp-1,   0x1.52dbdfbeae000p-2,   0x1.07fffffaacef4p-2}, //   66 * 0x1p-8 = 0x1.08p-2 (err =  0x1.07a5e3682b28cp-66)
    {  0x1.430000011a24ap+0,   0x1.95cbb0bcd503ap-1,   0x1.577091b842000p-2,   0x1.0c00000468928p-2}, //   67 * 0x1p-8 = 0x1.0cp-2 (err =  0x1.079871d892368p-64)
    {  0x1.44000001004a8p+0,   0x1.948b0fcc2e9d6p-1,   0x1.5c01a3a44e000p-2,   0x1.10000004012a0p-2}, //   68 * 0x1p-8 = 0x1.10p-2 (err =  0x1.0a1545f4d3cd3p-64)
    {  0x1.44fffffda0429p+0,   0x1.934c67fca4f64p-1,   0x1.608f1b37ca000p-2,   0x1.13fffff6810a4p-2}, //   69 * 0x1p-8 = 0x1.14p-2 (err =  0x1.50a76eedb6b41p-65)
    {  0x1.45fffffc3276ep+0,   0x1.920fb4a1bed02p-1,   0x1.6518fe353c000p-2,   0x1.17fffff0c9db8p-2}, //   70 * 0x1p-8 = 0x1.18p-2 (err =  0x1.135b7c52d1759p-65)
    {  0x1.47000000aa268p+0,   0x1.90d4f11f487bfp-1,   0x1.699f524bce000p-2,   0x1.1c000002a89a0p-2}, //   71 * 0x1p-8 = 0x1.1cp-2 (err =  0x1.bb33f7a107fc5p-64)
    {  0x1.47fffffd2479bp+0,   0x1.8f9c18fd3ccb4p-1,   0x1.6e221cccf2000p-2,   0x1.1ffffff491e6cp-2}, //   72 * 0x1p-8 = 0x1.20p-2 (err =  0x1.e48a6724a5ef2p-64)
    {  0x1.48fffffec5149p+0,   0x1.8e6527b090cc4p-1,   0x1.72a1637736000p-2,   0x1.23fffffb14524p-2}, //   73 * 0x1p-8 = 0x1.24p-2 (err =  0x1.8d52464695185p-64)
    {  0x1.4a000000e3770p+0,   0x1.8d3018d1efc66p-1,   0x1.771d2babea000p-2,   0x1.280000038ddc0p-2}, //   74 * 0x1p-8 = 0x1.28p-2 (err =  0x1.5070832b8a23ap-67)
    {  0x1.4b00000056559p+0,   0x1.8bfce805c8aabp-1,   0x1.7b957ac69c000p-2,   0x1.2c00000159564p-2}, //   75 * 0x1p-8 = 0x1.2cp-2 (err =  0x1.2c23c218964bdp-64)
    {  0x1.4bfffffeb70b8p+0,   0x1.8acb90f846674p-1,   0x1.800a562baa000p-2,   0x1.2ffffffadc2e0p-2}, //   76 * 0x1p-8 = 0x1.30p-2 (err =  0x1.8ca7aade0d286p-65)
    {  0x1.4cffffff0720ap+0,   0x1.899c0f613ec55p-1,   0x1.847bc33936000p-2,   0x1.33fffffc1c828p-2}, //   77 * 0x1p-8 = 0x1.34p-2 (err =  0x1.a23ec9166b7b1p-69)
    {  0x1.4dffffff5f392p+0,   0x1.886e5f0b77ebfp-1,   0x1.88e9c72b44000p-2,   0x1.37fffffd7ce48p-2}, //   78 * 0x1p-8 = 0x1.38p-2 (err =  0x1.ba90a83409d63p-64)
    {  0x1.4f000000bc9e0p+0,   0x1.87427bcb2ce07p-1,   0x1.8d54673e9c000p-2,   0x1.3c000002f2780p-2}, //   79 * 0x1p-8 = 0x1.3cp-2 (err =  0x1.606f4532cb43cp-64)
    {  0x1.50000000d748ep+0,   0x1.861861851e6fap-1,   0x1.91bba895a4000p-2,   0x1.400000035d238p-2}, //   80 * 0x1p-8 = 0x1.40p-2 (err =  0x1.a13980c5ac289p-65)
    {  0x1.50ffffff4232fp+0,   0x1.84f00c285b6ecp-1,   0x1.961f904f34000p-2,   0x1.43fffffd08cbcp-2}, //   81 * 0x1p-8 = 0x1.44p-2 (err =  0x1.c3da00cd022c1p-65)
    {  0x1.520000007e3f8p+0,   0x1.83c977aa9b15bp-1,   0x1.9a8023940a000p-2,   0x1.48000001f8fe0p-2}, //   82 * 0x1p-8 = 0x1.48p-2 (err =  0x1.31fccbf1c6c9bp-64)
    {  0x1.530000007988ep+0,   0x1.82a4a0179fac8p-1,   0x1.9edd675bc4000p-2,   0x1.4c000001e6238p-2}, //   83 * 0x1p-8 = 0x1.4cp-2 (err =  0x1.057f6fe59a7e3p-64)
    {  0x1.53ffffffbf8dfp+0,   0x1.81818181ca93bp-1,   0x1.a33760a6de000p-2,   0x1.4ffffffefe37cp-2}, //   84 * 0x1p-8 = 0x1.50p-2 (err =  0x1.89c89d1884106p-64)
    {  0x1.54ffffff7b0f9p+0,   0x1.806018069759cp-1,   0x1.a78e146d3c000p-2,   0x1.53fffffdec3e4p-2}, //   85 * 0x1p-8 = 0x1.54p-2 (err =  0x1.6700dd9518064p-65)
    {  0x1.55ffffffc1f7ep+0,   0x1.7f405fd05d779p-1,   0x1.abe18796e6000p-2,   0x1.57ffffff07df8p-2}, //   86 * 0x1p-8 = 0x1.58p-2 (err =  0x1.a132b821e03c7p-66)
    {  0x1.56ffffff28f16p+0,   0x1.7e2255169489dp-1,   0x1.b031befa68000p-2,   0x1.5bfffffca3c58p-2}, //   87 * 0x1p-8 = 0x1.5cp-2 (err =  0x1.29753b1d6a1cep-64)
    {  0x1.57ffffffa0f51p+0,   0x1.7d05f41839a4dp-1,   0x1.b47ebf71f0000p-2,   0x1.5ffffffe83d44p-2}, //   88 * 0x1p-8 = 0x1.60p-2 (err =  0x1.f846d1f9f5fb9p-64)
    {  0x1.58ffffff5317bp+0,   0x1.7beb39239e804p-1,   0x1.b8c88dbca4000p-2,   0x1.63fffffd4c5ecp-2}, //   89 * 0x1p-8 = 0x1.64p-2 (err =  0x1.ba20d80098953p-64)
    {  0x1.5a00000146b7ap+0,   0x1.7ad2208ca916bp-1,   0x1.bd0f2ea3ec000p-2,   0x1.680000051ade8p-2}, //   90 * 0x1p-8 = 0x1.68p-2 (err =  0x1.7cce2c88c052dp-67)
    {  0x1.5afffffd74bd4p+0,   0x1.79baa6be2887ep-1,   0x1.c152a6b778000p-2,   0x1.6bfffff5d2f50p-2}, //   91 * 0x1p-8 = 0x1.6cp-2 (err =  0x1.c93b96c27e562p-68)
    {  0x1.5c000000903b8p+0,   0x1.78a4c816ee31ep-1,   0x1.c592fad4fa000p-2,   0x1.7000000240ee0p-2}, //   92 * 0x1p-8 = 0x1.70p-2 (err =  0x1.de6cf2817c75ap-67)
    {  0x1.5cffffffae25dp+0,   0x1.7790811a0475fp-1,   0x1.c9d02f6b4a000p-2,   0x1.73fffffeb8974p-2}, //   93 * 0x1p-8 = 0x1.74p-2 (err =  0x1.7d924e88942b1p-66)
    {  0x1.5e0000005c822p+0,   0x1.767dce42e79fcp-1,   0x1.ce0a49252c000p-2,   0x1.7800000172088p-2}, //   94 * 0x1p-8 = 0x1.78p-2 (err =  0x1.1a733e6a6fee9p-64)
    {  0x1.5effffff738bcp+0,   0x1.756cac20acc43p-1,   0x1.d2414c7e70000p-2,   0x1.7bfffffdce2f0p-2}, //   95 * 0x1p-8 = 0x1.7cp-2 (err =  0x1.6a897e32fe944p-64)
    {  0x1.600000007c3a1p+0,   0x1.745d17454e0a8p-1,   0x1.d6753e0538000p-2,   0x1.80000001f0e84p-2}, //   96 * 0x1p-8 = 0x1.80p-2 (err =  0x1.2df4a6c6f5c71p-64)
    {  0x1.60ffffffcb327p+0,   0x1.734f0c5457737p-1,   0x1.daa6221f88000p-2,   0x1.83ffffff2cc9cp-2}, //   97 * 0x1p-8 = 0x1.84p-2 (err =  0x1.0c9d0ef143ca5p-65)
    {  0x1.61fffffebca77p+0,   0x1.724287f5c01e6p-1,   0x1.ded3fd3ede000p-2,   0x1.87fffffaf29dcp-2}, //   98 * 0x1p-8 = 0x1.88p-2 (err =  0x1.1b07436237708p-68)
    {  0x1.63000000a229ep+0,   0x1.713786d91f182p-1,   0x1.e2fed3d33a000p-2,   0x1.8c00000288a78p-2}, //   99 * 0x1p-8 = 0x1.8cp-2 (err =  0x1.30ff4095be42dp-64)
    {  0x1.64000000efb31p+0,   0x1.702e05bfc0309p-1,   0x1.e726aa2258000p-2,   0x1.90000003becc4p-2}, //  100 * 0x1p-8 = 0x1.90p-2 (err =  0x1.fdcb36bf30efbp-66)
    {  0x1.64ffffff09939p+0,   0x1.6f260170236f3p-1,   0x1.eb4b84791a000p-2,   0x1.93fffffc264e4p-2}, //  101 * 0x1p-8 = 0x1.94p-2 (err =  0x1.a7b4880780244p-65)
    {  0x1.6600000042d9ep+0,   0x1.6e1f76b3ef1e4p-1,   0x1.ef6d6733a2000p-2,   0x1.980000010b678p-2}, //  102 * 0x1p-8 = 0x1.98p-2 (err =  0x1.cf6e7f1dabcfdp-66)
    {  0x1.67000000777aep+0,   0x1.6d1a6267a3033p-1,   0x1.f38c567db8000p-2,   0x1.9c000001ddeb8p-2}, //  103 * 0x1p-8 = 0x1.9cp-2 (err =  0x1.7e5eefda88e99p-65)
    {  0x1.6800000006b83p+0,   0x1.6c16c16c0ff5bp-1,   0x1.f7a8568ccc000p-2,   0x1.a00000001ae0cp-2}, //  104 * 0x1p-8 = 0x1.a0p-2 (err =  0x1.828b9628f37e8p-64)
    {  0x1.68ffffff4f72cp+0,   0x1.6b1490aae3358p-1,   0x1.fbc16b8d54000p-2,   0x1.a3fffffd3dcb0p-2}, //  105 * 0x1p-8 = 0x1.a4p-2 (err =  0x1.999ff284bcc10p-64)
    {  0x1.69ffffff508f0p+0,   0x1.6a13cd15e6a3ap-1,   0x1.ffd799a574000p-2,   0x1.a7fffffd423c0p-2}, //  106 * 0x1p-8 = 0x1.a8p-2 (err =  0x1.0509e404ea4fcp-68)
    {  0x1.6b0000006515cp+0,   0x1.691473a8287f2p-1,   0x1.01f57277f4000p-1,   0x1.ac00000194570p-2}, //  107 * 0x1p-8 = 0x1.acp-2 (err =  0x1.58c5750438924p-65)
    {  0x1.6c000000c2d9ap+0,   0x1.6816816755bfep-1,   0x1.03fda8bb05000p-1,   0x1.b00000030b668p-2}, //  108 * 0x1p-8 = 0x1.b0p-2 (err =  0x1.8a0361f8315d9p-64)
    {  0x1.6d0000002c834p+0,   0x1.6719f35feaa68p-1,   0x1.0604719f7f000p-1,   0x1.b4000000b20d0p-2}, //  109 * 0x1p-8 = 0x1.b4p-2 (err =  0x1.7d68b63719da2p-64)
    {  0x1.6dffffffa9481p+0,   0x1.661ec6a567099p-1,   0x1.0809cf2748000p-1,   0x1.b7fffffea5204p-2}, //  110 * 0x1p-8 = 0x1.b8p-2 (err =  0x1.2972b780dba6ap-64)
    {  0x1.6f0000008a0fcp+0,   0x1.6524f8532e4f9p-1,   0x1.0a0dc350a4000p-1,   0x1.bc000002283f0p-2}, //  111 * 0x1p-8 = 0x1.bcp-2 (err =  0x1.dc88fdb6db9acp-64)
    {  0x1.6fffffff47d53p+0,   0x1.642c85916455fp-1,   0x1.0c10500bf2000p-1,   0x1.bffffffd1f54cp-2}, //  112 * 0x1p-8 = 0x1.c0p-2 (err =  0x1.51d8d68ad906ap-64)
    {  0x1.70fffffef85bdp+0,   0x1.63356b89a9d79p-1,   0x1.0e117752c8000p-1,   0x1.c3fffffbe16f4p-2}, //  113 * 0x1p-8 = 0x1.c4p-2 (err =  0x1.0af97d4746d4bp-67)
    {  0x1.71fffffe667adp+0,   0x1.623fa7719e3a2p-1,   0x1.10113b120b000p-1,   0x1.c7fffff999eb4p-2}, //  114 * 0x1p-8 = 0x1.c8p-2 (err =  0x1.e6a8c5aee5832p-64)
    {  0x1.72ffffff17338p+0,   0x1.614b3683f8997p-1,   0x1.120f9d3812000p-1,   0x1.cbfffffc5cce0p-2}, //  115 * 0x1p-8 = 0x1.ccp-2 (err =  0x1.abe2000efd33dp-65)
    {  0x1.740000006b1abp+0,   0x1.605816051bee7p-1,   0x1.140c9faaf3000p-1,   0x1.d0000001ac6acp-2}, //  116 * 0x1p-8 = 0x1.d0p-2 (err =  0x1.ae0bc565a241fp-67)
    {  0x1.7500000167897p+0,   0x1.5f66434140287p-1,   0x1.1608444c26000p-1,   0x1.d40000059e25cp-2}, //  117 * 0x1p-8 = 0x1.d4p-2 (err =  0x1.e9d94cf07336fp-64)
    {  0x1.760000014657ep+0,   0x1.5e75bb8bcf90fp-1,   0x1.18028cf9ae000p-1,   0x1.d8000005195f8p-2}, //  118 * 0x1p-8 = 0x1.d8p-2 (err =  0x1.dc5e53aef7b49p-64)
    {  0x1.77000000b7a5dp+0,   0x1.5d867c3e22fe4p-1,   0x1.19fb7b909c000p-1,   0x1.dc000002de974p-2}, //  119 * 0x1p-8 = 0x1.dcp-2 (err =  0x1.ae035c43d1d72p-64)
    {  0x1.7800000023a16p+0,   0x1.5c9882b90ffcep-1,   0x1.1bf311e9a3000p-1,   0x1.e00000008e858p-2}, //  120 * 0x1p-8 = 0x1.e0p-2 (err =  0x1.6e947f3bbfb48p-65)
    {  0x1.79000000e7da6p+0,   0x1.5babcc63a9d83p-1,   0x1.1de951db92000p-1,   0x1.e40000039f698p-2}, //  121 * 0x1p-8 = 0x1.e4p-2 (err =  0x1.6d5037ffe6253p-65)
    {  0x1.79ffffff92f69p+0,   0x1.5ac056b079b1ep-1,   0x1.1fde3d3013000p-1,   0x1.e7fffffe4bda4p-2}, //  122 * 0x1p-8 = 0x1.e8p-2 (err =  0x1.3132cbf3ad3a3p-67)
    {  0x1.7b0000005db75p+0,   0x1.59d61f11e746ap-1,   0x1.21d1d5bc24000p-1,   0x1.ec00000176dd4p-2}, //  123 * 0x1p-8 = 0x1.ecp-2 (err =  0x1.4823b76f3d991p-65)
    {  0x1.7bffffffe3749p+0,   0x1.58ed23082f77dp-1,   0x1.23c41d423b000p-1,   0x1.efffffff8dd24p-2}, //  124 * 0x1p-8 = 0x1.f0p-2 (err =  0x1.ba5e8fa707686p-64)
    {  0x1.7cffffff5c968p+0,   0x1.5805601613e34p-1,   0x1.25b5158a37000p-1,   0x1.f3fffffd725a0p-2}, //  125 * 0x1p-8 = 0x1.f4p-2 (err =  0x1.85e835162edebp-66)
    {  0x1.7e000000cb5f1p+0,   0x1.571ed3c450077p-1,   0x1.27a4c059e6000p-1,   0x1.f80000032d7c4p-2}, //  126 * 0x1p-8 = 0x1.f8p-2 (err =  0x1.3066926d849fcp-64)
    {  0x1.7f0000002844fp+0,   0x1.56397ba7a132bp-1,   0x1.29931f67df000p-1,   0x1.fc000000a113cp-2}, //  127 * 0x1p-8 = 0x1.fcp-2 (err =  0x1.0e9e2027efe3bp-66)
    {  0x1.80fffffcfaecep-1,   0x1.54725e6e63ccap+0,  -0x1.a527fda194000p-2,  -0x1.fc00000c144c8p-3}, // -127 * 0x1p-9 = -0x1.fcp-3 (err =  0x1.1457b753bdd4cp-66)
    {  0x1.8200000060b3fp-1,   0x1.5390948eebecdp+0,  -0x1.a152f14126000p-2,  -0x1.f7fffffe7d304p-3}, // -126 * 0x1p-9 = -0x1.f8p-3 (err =  0x1.839c1172870c4p-71)
    {  0x1.83000000eb899p-1,   0x1.52aff569b2329p+0,  -0x1.9d806eb916000p-2,  -0x1.f3fffffc51d9cp-3}, // -125 * 0x1p-9 = -0x1.f4p-3 (err =  0x1.e2f002b0ece16p-64)
    {  0x1.840000028178bp-1,   0x1.51d07eac01012p+0,  -0x1.99b0729fe2000p-2,  -0x1.effffff5fa1d4p-3}, // -124 * 0x1p-9 = -0x1.f0p-3 (err =  0x1.294bd163e25d0p-64)
    {  0x1.84fffffeff45fp-1,   0x1.50f22e11faabdp+0,  -0x1.95e2f9b8ee000p-2,  -0x1.ec00000402e84p-3}, // -123 * 0x1p-9 = -0x1.ecp-3 (err =  0x1.f48358eb621ffp-64)
    {  0x1.8600000124750p-1,   0x1.5015014f18fb1p+0,  -0x1.9218008dfa000p-2,  -0x1.e7fffffb6e2c0p-3}, // -122 * 0x1p-9 = -0x1.e8p-3 (err =  0x1.089484048b8b9p-65)
    {  0x1.86ffffff90687p-1,   0x1.4f38f62e3475fp+0,  -0x1.8e4f83fbba000p-2,  -0x1.e4000001be5e4p-3}, // -121 * 0x1p-9 = -0x1.e4p-3 (err =  0x1.d90aa854acf83p-64)
    {  0x1.8800000021eafp-1,   0x1.4e5e0a72d3654p+0,  -0x1.8a8980ab7c000p-2,  -0x1.dfffffff78544p-3}, // -120 * 0x1p-9 = -0x1.e0p-3 (err =  0x1.cedffce2199a5p-66)
    {  0x1.88ffffffaaefep-1,   0x1.4d843bee0af4ep+0,  -0x1.86c5f36f2a000p-2,  -0x1.dc00000154408p-3}, // -119 * 0x1p-9 = -0x1.dcp-3 (err =  0x1.387a4747c2b95p-68)
    {  0x1.8a00000102b20p-1,   0x1.4cab88718089ap+0,  -0x1.8304d90848000p-2,  -0x1.d7fffffbf5380p-3}, // -118 * 0x1p-9 = -0x1.d8p-3 (err =  0x1.d4449b83ef90bp-64)
    {  0x1.8b000000128eep-1,   0x1.4bd3edda59670p+0,  -0x1.7f462e589c000p-2,  -0x1.d3ffffffb5c48p-3}, // -117 * 0x1p-9 = -0x1.d4p-3 (err =  0x1.93705c61e2a3dp-64)
    {  0x1.8bffffff5d33ep-1,   0x1.4afd6a05b407fp+0,  -0x1.7b89f02f52000p-2,  -0x1.d00000028b308p-3}, // -116 * 0x1p-9 = -0x1.d0p-3 (err =  0x1.08edefbebaba4p-64)
    {  0x1.8cffffff9a5f9p-1,   0x1.4a27fad7b498ap+0,  -0x1.77d01b6876000p-2,  -0x1.cc0000019681cp-3}, // -115 * 0x1p-9 = -0x1.ccp-3 (err =  0x1.c12c9abc074b8p-64)
    {  0x1.8e000000511e2p-1,   0x1.49539e3ae9e77p+0,  -0x1.7418acea92000p-2,  -0x1.c7fffffebb878p-3}, // -114 * 0x1p-9 = -0x1.c8p-3 (err =  0x1.cc1c9d66dee10p-64)
    {  0x1.8f00000173fd2p-1,   0x1.4880521ee244dp+0,  -0x1.7063a1a09a000p-2,  -0x1.c3fffffa300b8p-3}, // -113 * 0x1p-9 = -0x1.c4p-3 (err =  0x1.3941054bba01ap-64)
    {  0x1.8ffffffe18c82p-1,   0x1.47ae147c7068cp+0,  -0x1.6cb0f68d64000p-2,  -0x1.c00000079cdf8p-3}, // -112 * 0x1p-9 = -0x1.c0p-3 (err =  0x1.6acbbdbd58cdep-64)
    {  0x1.90fffffeaf290p-1,   0x1.46dce346a896fp+0,  -0x1.6900a88846000p-2,  -0x1.bc000005435c0p-3}, // -111 * 0x1p-9 = -0x1.bcp-3 (err =  0x1.de0227f5f929cp-65)
    {  0x1.91ffffff657b2p-1,   0x1.460cbc7fda4d0p+0,  -0x1.6552b49bbe000p-2,  -0x1.b80000026a138p-3}, // -110 * 0x1p-9 = -0x1.b8p-3 (err =  0x1.62df4dcd03c25p-64)
    {  0x1.92ffffffe5b39p-1,   0x1.453d9e2c8ca61p+0,  -0x1.61a717cb22000p-2,  -0x1.b40000006931cp-3}, // -109 * 0x1p-9 = -0x1.b4p-3 (err =  0x1.5a826db193fd7p-64)
    {  0x1.93ffffffab771p-1,   0x1.446f86567182ap+0,  -0x1.5dfdcf2020000p-2,  -0x1.b00000015223cp-3}, // -108 * 0x1p-9 = -0x1.b0p-3 (err =  0x1.006c81cfd1934p-66)
    {  0x1.94fffffeee17fp-1,   0x1.43a2730b99c5ap+0,  -0x1.5a56d7a758000p-2,  -0x1.ac00000447a04p-3}, // -107 * 0x1p-9 = -0x1.acp-3 (err =  0x1.7c9ebf47d1019p-70)
    {  0x1.95fffffdf26b1p-1,   0x1.42d6625ef3e53p+0,  -0x1.56b22e72d0000p-2,  -0x1.a80000083653cp-3}, // -106 * 0x1p-9 = -0x1.a8p-3 (err =  0x1.a4e5f895aed1ep-65)
    {  0x1.96fffffcc3d82p-1,   0x1.420b526874df2p+0,  -0x1.530fd09ae8000p-2,  -0x1.a400000cf09f8p-3}, // -105 * 0x1p-9 = -0x1.a4p-3 (err =  0x1.a993f721101d1p-64)
    {  0x1.97ffffff92bb2p-1,   0x1.41414141974adp+0,  -0x1.4f6fbb2e78000p-2,  -0x1.a0000001b5138p-3}, // -104 * 0x1p-9 = -0x1.a0p-3 (err =  0x1.abb7d898bdbdap-64)
    {  0x1.98ffffffa283ep-1,   0x1.40782d112f964p+0,  -0x1.4bd1eb6960000p-2,  -0x1.9c00000175f08p-3}, // -103 * 0x1p-9 = -0x1.9cp-3 (err =  0x1.3157670e532d4p-66)
    {  0x1.9a0000001b0d5p-1,   0x1.3fb013faec27dp+0,  -0x1.48365e68fc000p-2,  -0x1.97ffffff93cacp-3}, // -102 * 0x1p-9 = -0x1.98p-3 (err =  0x1.dc2807ecf1c2bp-64)
    {  0x1.9b000000e5330p-1,   0x1.3ee8f429a9185p+0,  -0x1.449d115bc0000p-2,  -0x1.93fffffc6b340p-3}, // -101 * 0x1p-9 = -0x1.94p-3 (err =  0x1.cf0331bb6d56fp-65)
    {  0x1.9c00000162f5cp-1,   0x1.3e22cbcd3878ep+0,  -0x1.4106017746000p-2,  -0x1.8ffffffa74290p-3}, // -100 * 0x1p-9 = -0x1.90p-3 (err =  0x1.a9a5bb710b8f7p-64)
    {  0x1.9d0000004669cp-1,   0x1.3d5d991a7140ap+0,  -0x1.3d712bf8ce000p-2,  -0x1.8bfffffee6590p-3}, //  -99 * 0x1p-9 = -0x1.8cp-3 (err =  0x1.1e2dd7d6b225dp-66)
    {  0x1.9e000000e1dc8p-1,   0x1.3c995a470e054p+0,  -0x1.39de8e1234000p-2,  -0x1.87fffffc788e0p-3}, //  -98 * 0x1p-9 = -0x1.88p-3 (err =  0x1.c14cbb8e7a240p-67)
    {  0x1.9effffffa4909p-1,   0x1.3bd60d92782b9p+0,  -0x1.364e251312000p-2,  -0x1.840000016dbdcp-3}, //  -97 * 0x1p-9 = -0x1.84p-3 (err =  0x1.599d2777e28ecp-64)
    {  0x1.9fffffff94af9p-1,   0x1.3b13b13b64f8cp+0,  -0x1.32bfee388c000p-2,  -0x1.80000001ad41cp-3}, //  -96 * 0x1p-9 = -0x1.80p-3 (err =  0x1.f8449e265880cp-64)
    {  0x1.a10000000eb1dp-1,   0x1.3a524387a16eap+0,  -0x1.2f33e6d1de000p-2,  -0x1.7bffffffc538cp-3}, //  -95 * 0x1p-9 = -0x1.7cp-3 (err =  0x1.7cefaa427281bp-65)
    {  0x1.a2000001e9875p-1,   0x1.3991c2c018bbfp+0,  -0x1.2baa0c2dfc000p-2,  -0x1.77fffff859e2cp-3}, //  -94 * 0x1p-9 = -0x1.78p-3 (err =  0x1.afcc3852813b2p-68)
    {  0x1.a300000174c2ep-1,   0x1.38d22d354a3c2p+0,  -0x1.28225bb0c4000p-2,  -0x1.73fffffa2cf48p-3}, //  -93 * 0x1p-9 = -0x1.74p-3 (err =  0x1.32cf06a174379p-64)
    {  0x1.a40000002b73bp-1,   0x1.38138137f337ep+0,  -0x1.249cd2b0a4000p-2,  -0x1.6fffffff52314p-3}, //  -92 * 0x1p-9 = -0x1.70p-3 (err =  0x1.06ccc9b4303f4p-64)
    {  0x1.a4fffffe77163p-1,   0x1.3755bd1db6ef6p+0,  -0x1.21196e8caa000p-2,  -0x1.6c00000623a74p-3}, //  -91 * 0x1p-9 = -0x1.6cp-3 (err =  0x1.c4b5e0fdbfbf2p-64)
    {  0x1.a5fffffef5e5cp-1,   0x1.3698df3ea44f5p+0,  -0x1.1d982ca0f6000p-2,  -0x1.6800000428690p-3}, //  -90 * 0x1p-9 = -0x1.68p-3 (err =  0x1.de64da513a655p-65)
    {  0x1.a6ffffffbfa8ap-1,   0x1.35dce5fa21d16p+0,  -0x1.1a190a5e48000p-2,  -0x1.64000001015d8p-3}, //  -89 * 0x1p-9 = -0x1.64p-3 (err =  0x1.c53e2034f8451p-64)
    {  0x1.a800000108ee5p-1,   0x1.3521cfb1f663dp+0,  -0x1.169c0532a4000p-2,  -0x1.5ffffffbdc46cp-3}, //  -88 * 0x1p-9 = -0x1.60p-3 (err =  0x1.7ae46a1d8f288p-66)
    {  0x1.a9000001dc7a5p-1,   0x1.34679acca771ep+0,  -0x1.13211a94c0000p-2,  -0x1.5bfffff88e16cp-3}, //  -87 * 0x1p-9 = -0x1.5cp-3 (err =  0x1.cf478958d975dp-65)
    {  0x1.aa0000003b2c7p-1,   0x1.33ae45b5510e1p+0,  -0x1.0fa848037e000p-2,  -0x1.57ffffff134e4p-3}, //  -86 * 0x1p-9 = -0x1.58p-3 (err =  0x1.15e5f21cdd2c2p-64)
    {  0x1.ab00000039021p-1,   0x1.32f5ced678e45p+0,  -0x1.0c318aed3a000p-2,  -0x1.53ffffff1bf7cp-3}, //  -85 * 0x1p-9 = -0x1.54p-3 (err =  0x1.87d699bc0bba6p-66)
    {  0x1.ac00000067089p-1,   0x1.323e34a267530p+0,  -0x1.08bce0d7fc000p-2,  -0x1.4ffffffe63ddcp-3}, //  -84 * 0x1p-9 = -0x1.50p-3 (err =  0x1.d4272acc167b1p-64)
    {  0x1.ad000000f4c6bp-1,   0x1.3187758df067bp+0,  -0x1.054a4748a6000p-2,  -0x1.4bfffffc2ce54p-3}, //  -83 * 0x1p-9 = -0x1.4cp-3 (err =  0x1.ca6136d89a29fp-65)
    {  0x1.ae0000000a830p-1,   0x1.30d1901305a57p+0,  -0x1.01d9bbcf82000p-2,  -0x1.47ffffffd5f40p-3}, //  -82 * 0x1p-9 = -0x1.48p-3 (err =  0x1.975ddc2335c80p-64)
    {  0x1.af000000b74a2p-1,   0x1.301c82abbed20p+0,  -0x1.fcd677e0c4000p-3,  -0x1.43fffffd22d78p-3}, //  -81 * 0x1p-9 = -0x1.44p-3 (err =  0x1.4ba4b49a666d5p-64)
    {  0x1.affffffe60becp-1,   0x1.2f684bdb369c1p+0,  -0x1.f5fd8a9b7c000p-3,  -0x1.400000067d050p-3}, //  -80 * 0x1p-9 = -0x1.40p-3 (err =  0x1.5007ef6e22b7ap-67)
    {  0x1.b10000000c098p-1,   0x1.2eb4ea1fe4aa7p+0,  -0x1.ef28aacd20000p-3,  -0x1.3bffffffcfda0p-3}, //  -79 * 0x1p-9 = -0x1.3cp-3 (err =  0x1.89ac459b4e362p-64)
    {  0x1.b20000005c6c7p-1,   0x1.2e025c0477b8cp+0,  -0x1.e857d3d0ec000p-3,  -0x1.37fffffe8e4e4p-3}, //  -78 * 0x1p-9 = -0x1.38p-3 (err =  0x1.f35bb4844f2e8p-64)
    {  0x1.b3000001a085fp-1,   0x1.2d50a011b485dp+0,  -0x1.e18b00d624000p-3,  -0x1.33fffff97de84p-3}, //  -77 * 0x1p-9 = -0x1.34p-3 (err =  0x1.38b3053bbb0bdp-64)
    {  0x1.b4000003afd6fp-1,   0x1.2c9fb4d58801fp+0,  -0x1.dac22d2548000p-3,  -0x1.2ffffff140a44p-3}, //  -76 * 0x1p-9 = -0x1.30p-3 (err =  0x1.cd7138448ef1fp-64)
    {  0x1.b4ffffff4f870p-1,   0x1.2bef98e61c906p+0,  -0x1.d3fd543ef4000p-3,  -0x1.2c000002c1e40p-3}, //  -75 * 0x1p-9 = -0x1.2cp-3 (err =  0x1.b306dafb03859p-65)
    {  0x1.b6000000d049fp-1,   0x1.2b404acf84655p+0,  -0x1.cd3c7127b4000p-3,  -0x1.27fffffcbed84p-3}, //  -74 * 0x1p-9 = -0x1.28p-3 (err =  0x1.27742c79f982ep-66)
    {  0x1.b700000077d0dp-1,   0x1.2a91c92eea936p+0,  -0x1.c67f7f73e4000p-3,  -0x1.23fffffe20bccp-3}, //  -73 * 0x1p-9 = -0x1.24p-3 (err =  0x1.524d978092728p-64)
    {  0x1.b7ffffffb1445p-1,   0x1.29e4129e7677dp+0,  -0x1.bfc67a8210000p-3,  -0x1.200000013aeecp-3}, //  -72 * 0x1p-9 = -0x1.20p-3 (err =  0x1.da58f492de221p-65)
    {  0x1.b8ffffffe7db4p-1,   0x1.293725bb908fep+0,  -0x1.b9115db8dc000p-3,  -0x1.1c00000060930p-3}, //  -71 * 0x1p-9 = -0x1.1cp-3 (err =  0x1.73a4629d8193bp-64)
    {  0x1.b9ffffff1d64dp-1,   0x1.288b012923099p+0,  -0x1.b260249dc0000p-3,  -0x1.180000038a6ccp-3}, //  -70 * 0x1p-9 = -0x1.18p-3 (err =  0x1.e499249237c93p-64)
    {  0x1.bb00000055a76p-1,   0x1.27dfa389e3afep+0,  -0x1.abb2ca9c8c000p-3,  -0x1.13fffffea9628p-3}, //  -69 * 0x1p-9 = -0x1.14p-3 (err =  0x1.7c3e96a29ec96p-67)
    {  0x1.bc000000ba1d1p-1,   0x1.27350b8796b50p+0,  -0x1.a5094b4ffc000p-3,  -0x1.0ffffffd178bcp-3}, //  -68 * 0x1p-9 = -0x1.10p-3 (err =  0x1.2a0857d5942e4p-66)
    {  0x1.bd00000468ab4p-1,   0x1.268b37ca7502ap+0,  -0x1.9e63a22c2c000p-3,  -0x1.0bffffee5d530p-3}, //  -67 * 0x1p-9 = -0x1.0cp-3 (err =  0x1.533c9eb686e5dp-64)
    {  0x1.bdfffffe6c0c7p-1,   0x1.25e22709135c3p+0,  -0x1.97c1cb1e3c000p-3,  -0x1.080000064fce4p-3}, //  -66 * 0x1p-9 = -0x1.08p-3 (err =  0x1.9d42fc21ff462p-64)
    {  0x1.bf00000018e02p-1,   0x1.2539d7e90729cp+0,  -0x1.9123c151e8000p-3,  -0x1.03ffffff9c7f8p-3}, //  -65 * 0x1p-9 = -0x1.04p-3 (err =  0x1.61e051bda86f0p-68)
    {  0x1.c00000002800bp-1,   0x1.2492492478296p+0,  -0x1.8a8980aaf4000p-3,  -0x1.fffffffebffa8p-4}, //  -64 * 0x1p-9 = -0x1.00p-3 (err =  0x1.3341300a9282fp-66)
    {  0x1.c0ffffff919f8p-1,   0x1.23eb7971bdc8dp+0,  -0x1.83f304d09c000p-3,  -0x1.f800000373040p-4}, //  -63 * 0x1p-9 = -0x1.f8p-4 (err =  0x1.67cf749ecc3fep-64)
    {  0x1.c2000000ecf04p-1,   0x1.2345678912710p+0,  -0x1.7d604966e8000p-3,  -0x1.effffff8987e0p-4}, //  -62 * 0x1p-9 = -0x1.f0p-4 (err =  0x1.b12bf19f34e81p-64)
    {  0x1.c3000004ad7ddp-1,   0x1.22a01226fd783p+0,  -0x1.76d14a275c000p-3,  -0x1.e7ffffda94118p-4}, //  -61 * 0x1p-9 = -0x1.e8p-4 (err =  0x1.99edad9216df6p-64)
    {  0x1.c3ffffff5e249p-1,   0x1.21fb7812878e9p+0,  -0x1.704603209c000p-3,  -0x1.e00000050edb8p-4}, //  -60 * 0x1p-9 = -0x1.e0p-4 (err =  0x1.920e1dfd9fa84p-67)
    {  0x1.c50000001c010p-1,   0x1.21579804737b6p+0,  -0x1.69be6fba84000p-3,  -0x1.d7ffffff1ff80p-4}, //  -59 * 0x1p-9 = -0x1.d8p-4 (err =  0x1.1cbd6df8bdb80p-64)
    {  0x1.c5ffffff60c82p-1,   0x1.20b470c6e14d5p+0,  -0x1.633a8bf844000p-3,  -0x1.d0000004f9bf0p-4}, //  -58 * 0x1p-9 = -0x1.d0p-4 (err =  0x1.84c02cc6802c5p-65)
    {  0x1.c6ffffffd537bp-1,   0x1.201201202d174p+0,  -0x1.5cba53a18c000p-3,  -0x1.c800000156428p-4}, //  -57 * 0x1p-9 = -0x1.c8p-4 (err =  0x1.cf2e1dd2ff96ep-64)
    {  0x1.c7fffffe2d1bfp-1,   0x1.1f7047dd3844cp+0,  -0x1.563dc2abcc000p-3,  -0x1.c000000e97208p-4}, //  -56 * 0x1p-9 = -0x1.c0p-4 (err =  0x1.c452fc97ff1f4p-66)
    {  0x1.c9000000fceb2p-1,   0x1.1ecf43c75cca0p+0,  -0x1.4fc4d4d358000p-3,  -0x1.b7fffff818a70p-4}, //  -55 * 0x1p-9 = -0x1.b8p-4 (err =  0x1.9c13329428bb7p-64)
    {  0x1.ca0000006d6e0p-1,   0x1.1e2ef3b3b7269p+0,  -0x1.494f8638cc000p-3,  -0x1.affffffc94900p-4}, //  -54 * 0x1p-9 = -0x1.b0p-4 (err =  0x1.10efbfec31c60p-68)
    {  0x1.caffffff79ab2p-1,   0x1.1d8f5673383e5p+0,  -0x1.42ddd2bd7c000p-3,  -0x1.a800000432a70p-4}, //  -53 * 0x1p-9 = -0x1.a8p-4 (err =  0x1.cd1d4e607ecf8p-64)
    {  0x1.cbffffffb6cf8p-1,   0x1.1cf06ada5567dp+0,  -0x1.3c6fb652a4000p-3,  -0x1.a000000249840p-4}, //  -52 * 0x1p-9 = -0x1.a0p-4 (err =  0x1.2283bbe35879cp-64)
    {  0x1.ccffffff63f40p-1,   0x1.1c522fc22e437p+0,  -0x1.36052d05ac000p-3,  -0x1.98000004e0600p-4}, //  -51 * 0x1p-9 = -0x1.98p-4 (err =  0x1.fd5d1e559fcb3p-64)
    {  0x1.cdfffffe682e4p-1,   0x1.1bb4a4056941cp+0,  -0x1.2f9e32dff0000p-3,  -0x1.9000000cbe8e0p-4}, //  -50 * 0x1p-9 = -0x1.90p-4 (err =  0x1.697b86bdc92dep-65)
    {  0x1.cf00000059622p-1,   0x1.1b17c67ef5075p+0,  -0x1.293ac3d9e0000p-3,  -0x1.87fffffd34ef0p-4}, //  -49 * 0x1p-9 = -0x1.88p-4 (err =  0x1.18c6347c5d3a4p-64)
    {  0x1.d0000000fe4b7p-1,   0x1.1a7b96110ce8ep+0,  -0x1.22dadc2460000p-3,  -0x1.7ffffff80da48p-4}, //  -48 * 0x1p-9 = -0x1.80p-4 (err =  0x1.9db84bd4478b0p-64)
    {  0x1.d0fffffe6e3f7p-1,   0x1.19e0119ef4a2fp+0,  -0x1.1c7e77e7dc000p-3,  -0x1.7800000c8e048p-4}, //  -47 * 0x1p-9 = -0x1.78p-4 (err =  0x1.9ae35e3a471d6p-69)
    {  0x1.d2000000f6ee8p-1,   0x1.19453808351e7p+0,  -0x1.1625931250000p-3,  -0x1.6ffffff8488c0p-4}, //  -46 * 0x1p-9 = -0x1.70p-4 (err =  0x1.729a57f154a96p-64)
    {  0x1.d30000015d621p-1,   0x1.18ab083830c2cp+0,  -0x1.0fd029fad0000p-3,  -0x1.67fffff514ef8p-4}, //  -45 * 0x1p-9 = -0x1.68p-4 (err =  0x1.e0a288fcdccd7p-66)
    {  0x1.d3ffffffa38f4p-1,   0x1.1811811848d2fp+0,  -0x1.097e38d0a8000p-3,  -0x1.60000002e3860p-4}, //  -44 * 0x1p-9 = -0x1.60p-4 (err =  0x1.01e24b16b6a35p-66)
    {  0x1.d500000059575p-1,   0x1.1778a191882b8p+0,  -0x1.032fbbacb4000p-3,  -0x1.57fffffd35458p-4}, //  -43 * 0x1p-9 = -0x1.58p-4 (err =  0x1.9be2fdbaed9dbp-65)
    {  0x1.d5fffffffce7bp-1,   0x1.16e06894290dbp+0,  -0x1.f9c95dc1f8000p-4,  -0x1.5000000018c28p-4}, //  -42 * 0x1p-9 = -0x1.50p-4 (err =  0x1.daf88050bff64p-64)
    {  0x1.d700000022becp-1,   0x1.1648d50fae98bp+0,  -0x1.ed3a1d4b28000p-4,  -0x1.47fffffeea0a0p-4}, //  -41 * 0x1p-9 = -0x1.48p-4 (err =  0x1.3d2be5d87dd38p-64)
    {  0x1.d7ffffff5d0b0p-1,   0x1.15b1e5f7b2507p+0,  -0x1.e0b1ae9728000p-4,  -0x1.4000000517a80p-4}, //  -40 * 0x1p-9 = -0x1.40p-4 (err =  0x1.46729d69bbd2bp-66)
    {  0x1.d8fffffaed1e6p-1,   0x1.151b9a42d6503p+0,  -0x1.d4300a6488000p-4,  -0x1.38000028970d0p-4}, //  -39 * 0x1p-9 = -0x1.38p-4 (err =  0x1.8d91ca5e7fcfap-65)
    {  0x1.da0000005bb13p-1,   0x1.1485f0e07755ep+0,  -0x1.c7b528b298000p-4,  -0x1.2ffffffd22768p-4}, //  -38 * 0x1p-9 = -0x1.30p-4 (err =  0x1.20860dd845626p-64)
    {  0x1.daffffff01206p-1,   0x1.13f0e8d3d8827p+0,  -0x1.bb41030588000p-4,  -0x1.28000007f6fd0p-4}, //  -37 * 0x1p-9 = -0x1.28p-4 (err =  0x1.394ba8631017bp-66)
    {  0x1.dbfffffe263bfp-1,   0x1.135c81146e92cp+0,  -0x1.aed391c260000p-4,  -0x1.2000000ece208p-4}, //  -36 * 0x1p-9 = -0x1.20p-4 (err =  0x1.b1a1e91028173p-64)
    {  0x1.dcffffffb726cp-1,   0x1.12c8b89f0601fp+0,  -0x1.a26ccd9d08000p-4,  -0x1.1800000246ca0p-4}, //  -35 * 0x1p-9 = -0x1.18p-4 (err =  0x1.0321bd8803b95p-65)
    {  0x1.de000000b6c4fp-1,   0x1.12358e756a2a5p+0,  -0x1.960caf91e8000p-4,  -0x1.0ffffffa49d88p-4}, //  -34 * 0x1p-9 = -0x1.10p-4 (err =  0x1.ee6c88a0a3926p-65)
    {  0x1.df0000023032bp-1,   0x1.11a30199347cbp+0,  -0x1.89b33076d8000p-4,  -0x1.07ffffee7e6a8p-4}, //  -33 * 0x1p-9 = -0x1.08p-4 (err =  0x1.866ce65851827p-64)
    {  0x1.e00000007e5e0p-1,   0x1.11111110c92d8p+0,  -0x1.7d604966e8000p-4,  -0x1.fffffff81a200p-5}, //  -32 * 0x1p-9 = -0x1.00p-4 (err =  0x1.9792efd231f50p-65)
    {  0x1.e0ffffff34054p-1,   0x1.107fbbe084974p+0,  -0x1.7113f32f68000p-4,  -0x1.f000000cbfac0p-5}, //  -31 * 0x1p-9 = -0x1.f0p-5 (err =  0x1.cc78b783e6f0cp-66)
    {  0x1.e200000016c39p-1,   0x1.0fef010fe2295p+0,  -0x1.64ce26bf50000p-4,  -0x1.dffffffe93c70p-5}, //  -30 * 0x1p-9 = -0x1.e0p-5 (err =  0x1.7763ec7407254p-65)
    {  0x1.e2ffffff9a654p-1,   0x1.0f5edfab6b701p+0,  -0x1.588edd51f8000p-4,  -0x1.d000000659ac0p-5}, //  -29 * 0x1p-9 = -0x1.d0p-5 (err =  0x1.acdd0b21f77ffp-65)
    {  0x1.e3ffffffa679bp-1,   0x1.0ecf56be9be06p+0,  -0x1.4c560fead0000p-4,  -0x1.c000000598650p-5}, //  -28 * 0x1p-9 = -0x1.c0p-5 (err =  0x1.3a91294a28fd1p-64)
    {  0x1.e4ffffff6fc73p-1,   0x1.0e406558765e1p+0,  -0x1.4023b7b948000p-4,  -0x1.b0000009038d0p-5}, //  -27 * 0x1p-9 = -0x1.b0p-5 (err =  0x1.1ef12fe9be405p-66)
    {  0x1.e5ffffff8365dp-1,   0x1.0db20a89398e9p+0,  -0x1.33f7cde738000p-4,  -0x1.a0000007c9a30p-5}, //  -26 * 0x1p-9 = -0x1.a0p-5 (err =  0x1.0b15f97284399p-64)
    {  0x1.e6ffffffebc06p-1,   0x1.0d24456365146p+0,  -0x1.27d24baf78000p-4,  -0x1.9000000143fa0p-5}, //  -25 * 0x1p-9 = -0x1.90p-5 (err =  0x1.cdcdbc977c700p-64)
    {  0x1.e7ffffff4ac1ap-1,   0x1.0c9714fc3164ep+0,  -0x1.1bb32a6898000p-4,  -0x1.8000000b53e60p-5}, //  -24 * 0x1p-9 = -0x1.80p-5 (err =  0x1.632df441bf856p-64)
    {  0x1.e8ffffff4c3cep-1,   0x1.0c0a786916a00p+0,  -0x1.0f9a634ee0000p-4,  -0x1.7000000b3c320p-5}, //  -23 * 0x1p-9 = -0x1.70p-5 (err =  0x1.02891380d7d22p-64)
    {  0x1.e9ffffffe0db3p-1,   0x1.0b7e6ec26adcep+0,  -0x1.0387efbe20000p-4,  -0x1.60000001f24d0p-5}, //  -22 * 0x1p-9 = -0x1.60p-5 (err =  0x1.875232fc96991p-65)
    {  0x1.eafffffebcc1cp-1,   0x1.0af2f7239e896p+0,  -0x1.eef7926ef0000p-5,  -0x1.5000001433e40p-5}, //  -21 * 0x1p-9 = -0x1.50p-5 (err =  0x1.8805f217c1f29p-65)
    {  0x1.ebffffffd4a74p-1,   0x1.0a6810a698831p+0,  -0x1.d6ebd1f610000p-5,  -0x1.40000002b58c0p-5}, //  -20 * 0x1p-9 = -0x1.40p-5 (err =  0x1.7d0cb1b5cc34dp-69)
    {  0x1.ecffffff14d91p-1,   0x1.09ddba6b77064p+0,  -0x1.beec9167b0000p-5,  -0x1.3000000eb26f0p-5}, //  -19 * 0x1p-9 = -0x1.30p-5 (err =  0x1.5bbefef48adc7p-66)
    {  0x1.edffffff19277p-1,   0x1.0953f3908c921p+0,  -0x1.a6f9c38d70000p-5,  -0x1.2000000e6d890p-5}, //  -18 * 0x1p-9 = -0x1.20p-5 (err =  0x1.b8c8caf184b86p-66)
    {  0x1.ef000000146bcp-1,   0x1.08cabb374b71ap+0,  -0x1.8f135b7f20000p-5,  -0x1.0ffffffeb9440p-5}, //  -17 * 0x1p-9 = -0x1.10p-5 (err =  0x1.ee04516857ae4p-64)
    {  0x1.effffffe8eea6p-1,   0x1.08421084e5ac2p+0,  -0x1.77394cbff0000p-5,  -0x1.00000017115a0p-5}, //  -16 * 0x1p-9 = -0x1.00p-5 (err =  0x1.c9c44aabc32e9p-64)
    {  0x1.f1000002c09f5p-1,   0x1.07b9f29a18c82p+0,  -0x1.5f6b89d050000p-5,  -0x1.dfffffa7ec160p-6}, //  -15 * 0x1p-9 = -0x1.e0p-6 (err =  0x1.ce4a05ed8f561p-64)
    {  0x1.f1fffffffceccp-1,   0x1.073260a4811c8p+0,  -0x1.47aa0735c0000p-5,  -0x1.c000000062680p-6}, //  -14 * 0x1p-9 = -0x1.c0p-6 (err =  0x1.0ac00e30acc04p-64)
    {  0x1.f300000109a74p-1,   0x1.06ab59c705594p+0,  -0x1.2ff4b75b80000p-5,  -0x1.9fffffdecb180p-6}, //  -13 * 0x1p-9 = -0x1.a0p-6 (err =  0x1.bedfea8b87236p-66)
    {  0x1.f400000007ba9p-1,   0x1.0624dd2f16927p+0,  -0x1.184b8e4ba0000p-5,  -0x1.7fffffff08ae0p-6}, //  -12 * 0x1p-9 = -0x1.80p-6 (err =  0x1.9a2f219cf521ep-65)
    {  0x1.f500000043a92p-1,   0x1.059eea0704036p+0,  -0x1.00ae7f49f0000p-5,  -0x1.5ffffff78adc0p-6}, //  -11 * 0x1p-9 = -0x1.60p-6 (err =  0x1.7eb6a2be78098p-64)
    {  0x1.f6000000a385ep-1,   0x1.05197f7d1e332p+0,  -0x1.d23afc2b00000p-6,  -0x1.3fffffeb8f440p-6}, //  -10 * 0x1p-9 = -0x1.40p-6 (err =  0x1.8bc1dc00ea936p-65)
    {  0x1.f70000008728ep-1,   0x1.04949cc120474p+0,  -0x1.a330fce9c0000p-6,  -0x1.1fffffef1ae40p-6}, //   -9 * 0x1p-9 = -0x1.20p-6 (err =  0x1.89c609df98e03p-64)
    {  0x1.f80000002b65fp-1,   0x1.04104103f9dc5p+0,  -0x1.743ee85a00000p-6,  -0x1.fffffff526840p-7}, //   -8 * 0x1p-9 = -0x1.00p-6 (err =  0x1.8737a108c57b3p-64)
    {  0x1.f90000040cd71p-1,   0x1.038c6b760f9b8p+0,  -0x1.4564a56400000p-6,  -0x1.bffffefcca3c0p-7}, //   -7 * 0x1p-9 = -0x1.c0p-7 (err =  0x1.f261fa4ec8eccp-65)
    {  0x1.fa00000001624p-1,   0x1.03091b51f52c5p+0,  -0x1.16a21e2060000p-6,  -0x1.7fffffffa7700p-7}, //   -6 * 0x1p-9 = -0x1.80p-7 (err =  0x1.382dc21eee7e0p-64)
    {  0x1.fafffffe39c59p-1,   0x1.02864fc85a3c3p+0,  -0x1.cfee716b40000p-7,  -0x1.400000718e9c0p-7}, //   -5 * 0x1p-9 = -0x1.40p-7 (err =  0x1.7ba9b30208a96p-66)
    {  0x1.fc0000011b404p-1,   0x1.0204080f90633p+0,  -0x1.72c7b9ba00000p-7,  -0x1.ffffff725fe00p-8}, //   -4 * 0x1p-9 = -0x1.00p-7 (err =  0x1.bf511f790052bp-64)
    {  0x1.fcfffffedf103p-1,   0x1.01824365a9d0ap+0,  -0x1.15cfe953c0000p-7,  -0x1.8000009077e80p-8}, //   -3 * 0x1p-9 = -0x1.80p-8 (err =  0x1.56f66cd6ba2d5p-67)
    {  0x1.fdfffffe2cca1p-1,   0x1.01010101ec71fp+0,  -0x1.720d9d5900000p-8,  -0x1.000000e99af80p-8}, //   -2 * 0x1p-9 = -0x1.00p-8 (err =  0x1.2f5e63eeb436ep-65)
    {  0x1.ff000001cb47dp-1,   0x1.0080401f297ddp+0,  -0x1.71b0e7ab00000p-9, -0x1.fffffc6970600p-10}, //   -1 * 0x1p-9 = -0x1.00p-9 (err =  0x1.843350e306542p-64)
    {  0x1.0000000000000p+0,   0x1.0000000000000p+0,   0x0.0000000000000p+0,   0x0.0000000000000p+0}, //    0 * 0x1p-9 = 0x0.00p+0 (err =   0x0.0000000000000p+0)
};// Worst deviation:  ap1 =   0x1.45fffffc3276ep+0, log1pa =   0x1.6518fe353c000p-2, a =   0x1.17fffff0c9db8p-2,  err =  0x1.135b7c52d1759p-65, d = 0x1.e6c49p-31

static const logdata_t LOG10_LOOKUP_BIG[] = { //ap1, va, log1pa, a: va = 1/(a+1), log1pa = log10(1+a)+err
    {  0x1.0000000000000p+0,   0x1.0000000000000p+0,   0x0.0000000000000p+0,   0x0.0000000000000p+0}, //    0 * 0x1p-8 = 0x0.00p+0 (err =   0x0.0000000000000p+0)
    {  0x1.00ffffffa5c06p+0,   0x1.fe01fe02b11a6p-1,  0x1.bbd9e8ac00000p-10,   0x1.ffffff4b80c00p-9}, //    1 * 0x1p-8 = 0x1.00p-8 (err =  0x1.98f7b04c0c3e7p-65)
    {  0x1.01ffffff12006p+0,   0x1.fc07f02195246p-1,   0x1.bafd465500000p-9,   0x1.ffffff1200600p-8}, //    2 * 0x1p-8 = 0x1.00p-7 (err =  0x1.660b0455a9a04p-69)
    {  0x1.030000000c6dfp+0,   0x1.fa11caa00757dp-1,   0x1.4b99564280000p-8,   0x1.8000000636f80p-7}, //    3 * 0x1p-8 = 0x1.80p-7 (err =  0x1.12268332c991ap-64)
    {  0x1.040000007a121p+0,   0x1.f81f81f732d22p-1,   0x1.b9476a8400000p-8,   0x1.0000001e84840p-6}, //    4 * 0x1p-8 = 0x1.00p-6 (err =  0x1.f3a2ba234b703p-65)
    {  0x1.04ffffff9e260p+0,   0x1.f6310acaca023p-1,   0x1.1344da8e00000p-7,   0x1.3fffffe789800p-6}, //    5 * 0x1p-8 = 0x1.40p-6 (err =  0x1.422d264b88706p-66)
    {  0x1.05ffffffaea2ep+0,   0x1.f44659e53f833p-1,   0x1.49b0850300000p-7,   0x1.7fffffeba8b80p-6}, //    6 * 0x1p-8 = 0x1.80p-6 (err =  0x1.33de3b88bc9b8p-64)
    {  0x1.07000000b5e17p+0,   0x1.f25f6440d8039p-1,   0x1.7fe71cf2c0000p-7,   0x1.c000002d785c0p-6}, //    7 * 0x1p-8 = 0x1.c0p-6 (err =  0x1.6abf687d01f1bp-65)
    {  0x1.07fffffeba53ap+0,   0x1.f07c1f0a2668ap-1,   0x1.b5e908a680000p-7,   0x1.ffffffae94e80p-6}, //    8 * 0x1p-8 = 0x1.00p-5 (err =  0x1.b22652b519aa7p-64)
    {  0x1.0900000134b3ap+0,   0x1.ee9c7f8218b26p-1,   0x1.ebb6afa600000p-7,   0x1.2000002696740p-5}, //    9 * 0x1p-8 = 0x1.20p-5 (err =  0x1.386d542376313p-64)
    {  0x1.09ffffffc2bcbp+0,   0x1.ecc07b3090489p-1,   0x1.10a83a7de0000p-6,   0x1.3ffffff857960p-5}, //   10 * 0x1p-8 = 0x1.40p-5 (err =  0x1.c6fd2c3e1bff1p-70)
    {  0x1.0b00000040d17p+0,   0x1.eae807ab28f20p-1,   0x1.2b5b5ec6e0000p-6,   0x1.600000081a2e0p-5}, //   11 * 0x1p-8 = 0x1.60p-5 (err =  0x1.0b717da7c7374p-64)
    {  0x1.0bffffffcb3dfp+0,   0x1.e9131abf6bbdcp-1,   0x1.45f4f5a740000p-6,   0x1.7ffffff967be0p-5}, //   12 * 0x1p-8 = 0x1.80p-5 (err =  0x1.d22e3af341a56p-64)
    {  0x1.0cfffffbb18f5p+0,   0x1.e741aa6141f98p-1,   0x1.60752f91c0000p-6,   0x1.9fffff7631ea0p-5}, //   13 * 0x1p-8 = 0x1.a0p-5 (err =  0x1.bb37577f07df1p-64)
    {  0x1.0e00000012becp+0,   0x1.e573ac8ffca34p-1,   0x1.7adc3df5a0000p-6,   0x1.c000000257d80p-5}, //   14 * 0x1p-8 = 0x1.c0p-5 (err =  0x1.031d44a8e4f5ep-64)
    {  0x1.0efffffe44288p+0,   0x1.e3a917a0d9c9fp-1,   0x1.952a4ef540000p-6,   0x1.dfffffc885100p-5}, //   15 * 0x1p-8 = 0x1.e0p-5 (err =  0x1.edbfc4139f3a7p-64)
    {  0x1.0fffffff45844p+0,   0x1.e1e1e1e32c42ep-1,   0x1.af5f929d00000p-6,   0x1.ffffffe8b0880p-5}, //   16 * 0x1p-8 = 0x1.00p-4 (err =  0x1.dfc720d20642cp-67)
    {  0x1.10ffffffdd536p+0,   0x1.e01e01e05afcfp-1,   0x1.c97c373260000p-6,   0x1.0ffffffdd5360p-4}, //   17 * 0x1p-8 = 0x1.10p-4 (err =  0x1.fec56c1ddf32dp-66)
    {  0x1.11fffffef534cp+0,   0x1.de5d6e415a316p-1,   0x1.e3806ab0c0000p-6,   0x1.1fffffef534c0p-4}, //   18 * 0x1p-8 = 0x1.20p-4 (err =  0x1.049271f114ec3p-64)
    {  0x1.13000000571f3p+0,   0x1.dca01dc96add2p-1,   0x1.fd6c5b1120000p-6,   0x1.3000000571f30p-4}, //   19 * 0x1p-8 = 0x1.30p-4 (err =  0x1.1cd222605a94ep-64)
    {  0x1.1400000050ae3p+0,   0x1.dae6076b0d4b2p-1,   0x1.0ba01a8580000p-5,   0x1.400000050ae30p-4}, //   20 * 0x1p-8 = 0x1.40p-4 (err =  0x1.398aed4990c4dp-64)
    {  0x1.14fffffe22423p+0,   0x1.d92f223518123p-1,   0x1.187e1292e0000p-5,   0x1.4fffffe224230p-4}, //   21 * 0x1p-8 = 0x1.50p-4 (err =  0x1.c93932f2dfa99p-64)
    {  0x1.15ffffffcc7ffp+0,   0x1.d77b654bda1b2p-1,   0x1.25502c0d30000p-5,   0x1.5ffffffcc7ff0p-4}, //   22 * 0x1p-8 = 0x1.60p-4 (err =  0x1.390b7d127a918p-65)
    {  0x1.16ffffffe9965p+0,   0x1.d5cac8077ce88p-1,   0x1.32167c81a0000p-5,   0x1.6ffffffe99650p-4}, //   23 * 0x1p-8 = 0x1.70p-4 (err =  0x1.2ccd0d81d9a25p-65)
    {  0x1.18000000a3d62p+0,   0x1.d41d41d30b597p-1,   0x1.3ed119a280000p-5,   0x1.8000000a3d620p-4}, //   24 * 0x1p-8 = 0x1.80p-4 (err =  0x1.ca346505bfa6dp-65)
    {  0x1.18fffffe8dd24p+0,   0x1.d272ca422c2cep-1,   0x1.4b80189fd0000p-5,   0x1.8fffffe8dd240p-4}, //   25 * 0x1p-8 = 0x1.90p-4 (err =  0x1.74ce9d22e0b51p-67)
    {  0x1.19fffffd2a299p+0,   0x1.d0cb58fb985bep-1,   0x1.58238ec770000p-5,   0x1.9fffffd2a2990p-4}, //   26 * 0x1p-8 = 0x1.a0p-4 (err =  0x1.4bdfe660abb74p-64)
    {  0x1.1b0000018272ep+0,   0x1.cf26e5c1d3883p-1,   0x1.64bb914060000p-5,   0x1.b0000018272e0p-4}, //   27 * 0x1p-8 = 0x1.b0p-4 (err =  0x1.f9705ea91fb06p-64)
    {  0x1.1bffffffd087dp+0,   0x1.cd85689086d4ep-1,   0x1.7148342580000p-5,   0x1.bffffffd087d0p-4}, //   28 * 0x1p-8 = 0x1.c0p-4 (err =  0x1.14e0b47dadc8cp-65)
    {  0x1.1d00000106195p+0,   0x1.cbe6d95e75cc2p-1,   0x1.7dc98c5e90000p-5,   0x1.d000001061950p-4}, //   29 * 0x1p-8 = 0x1.d0p-4 (err =  0x1.752ef9c6f2b65p-64)
    {  0x1.1e00000058631p+0,   0x1.ca4b30556076ep-1,   0x1.8a3fadefd0000p-5,   0x1.e000000586310p-4}, //   30 * 0x1p-8 = 0x1.e0p-4 (err =  0x1.a0d8e44ec85f8p-64)
    {  0x1.1effffff6e656p+0,   0x1.c8b265b0a0567p-1,   0x1.96aaacf7f0000p-5,   0x1.effffff6e6560p-4}, //   31 * 0x1p-8 = 0x1.f0p-4 (err =  0x1.21f9a73475935p-64)
    {  0x1.20000000bbe2dp+0,   0x1.c71c71c5f389cp-1,   0x1.a30a9d69b0000p-5,   0x1.00000005df168p-3}, //   32 * 0x1p-8 = 0x1.00p-3 (err =  0x1.ad16db192ff96p-65)
    {  0x1.20ffffff7a433p+0,   0x1.c5894d11a6793p-1,   0x1.af5f92a9a0000p-5,   0x1.07fffffbd2198p-3}, //   33 * 0x1p-8 = 0x1.08p-3 (err =  0x1.0d5d6d6f3f51ep-64)
    {  0x1.21ffffffac8b4p+0,   0x1.c3f8f01cc1a08p-1,   0x1.bba9a054e0000p-5,   0x1.0ffffffd645a0p-3}, //   34 * 0x1p-8 = 0x1.10p-3 (err =  0x1.d62c64c846c58p-64)
    {  0x1.230000002b22dp+0,   0x1.c26b5392a73d5p-1,   0x1.c7e8d99560000p-5,   0x1.1800000159168p-3}, //   35 * 0x1p-8 = 0x1.18p-3 (err =  0x1.c40fa85f0206ap-65)
    {  0x1.240000004b478p+0,   0x1.c0e07037a854fp-1,   0x1.d41d516890000p-5,   0x1.200000025a3c0p-3}, //   36 * 0x1p-8 = 0x1.20p-3 (err =  0x1.6311e38d5219ep-64)
    {  0x1.24fffffc10be9p+0,   0x1.bf583eee6ab2ap-1,   0x1.e0471a71c0000p-5,   0x1.27ffffe085f48p-3}, //   37 * 0x1p-8 = 0x1.28p-3 (err =  0x1.02c0440d45842p-65)
    {  0x1.26000000299b8p+0,   0x1.bdd2b89901577p-1,   0x1.ec6647ed50000p-5,   0x1.300000014cdc0p-3}, //   38 * 0x1p-8 = 0x1.30p-3 (err =  0x1.27e41b1b4b2fap-67)
    {  0x1.27000000e1cbap+0,   0x1.bc4fd6572fd34p-1,   0x1.f87aebbee0000p-5,   0x1.380000070e5d0p-3}, //   39 * 0x1p-8 = 0x1.38p-3 (err =  0x1.5d7658d22807cp-64)
    {  0x1.27ffffff3055bp+0,   0x1.bacf914d5256ep-1,   0x1.02428c1a28000p-4,   0x1.3ffffff982ad8p-3}, //   40 * 0x1p-8 = 0x1.40p-3 (err =  0x1.0ecf33dc98a44p-64)
    {  0x1.2900000119818p+0,   0x1.b951e2afeda64p-1,   0x1.08426fd448000p-4,   0x1.48000008cc0c0p-3}, //   41 * 0x1p-8 = 0x1.48p-3 (err =  0x1.1b428a60769dbp-64)
    {  0x1.2a000000770aap+0,   0x1.b7d6c3dcf3851p-1,   0x1.0e3d29dad8000p-4,   0x1.50000003b8550p-3}, //   42 * 0x1p-8 = 0x1.50p-3 (err =  0x1.73a877cd99176p-64)
    {  0x1.2affffff9b2e4p+0,   0x1.b65e2e3c82b05p-1,   0x1.1432c316c0000p-4,   0x1.57fffffcd9720p-3}, //   43 * 0x1p-8 = 0x1.58p-3 (err =  0x1.2aef7c29f0f51p-64)
    {  0x1.2bffffffc9c78p+0,   0x1.b4e81b4ed0abep-1,   0x1.1a234453c0000p-4,   0x1.5ffffffe4e3c0p-3}, //   44 * 0x1p-8 = 0x1.60p-3 (err =  0x1.380b30596923cp-64)
    {  0x1.2cfffffcc1adbp+0,   0x1.b37484b231a5bp-1,   0x1.200eb62678000p-4,   0x1.67ffffe60d6d8p-3}, //   45 * 0x1p-8 = 0x1.68p-3 (err =  0x1.d1ace0476175bp-64)
    {  0x1.2e0000002e96ap+0,   0x1.b203640685197p-1,   0x1.25f5215fc8000p-4,   0x1.7000000174b50p-3}, //   46 * 0x1p-8 = 0x1.70p-3 (err =  0x1.63bdec8ef7ed5p-64)
    {  0x1.2effffffcaa63p+0,   0x1.b094b31dde54fp-1,   0x1.2bd68e44e8000p-4,   0x1.77fffffe55318p-3}, //   47 * 0x1p-8 = 0x1.78p-3 (err =  0x1.389fc6c5c159ap-68)
    {  0x1.30000001e5804p+0,   0x1.af286bc76a5ebp-1,   0x1.31b3056760000p-4,   0x1.8000000f2c020p-3}, //   48 * 0x1p-8 = 0x1.80p-3 (err =  0x1.2c58d01c5b4b3p-65)
    {  0x1.30ffffff6fba7p+0,   0x1.adbe87fa144d3p-1,   0x1.378a8ef500000p-4,   0x1.87fffffb7dd38p-3}, //   49 * 0x1p-8 = 0x1.88p-3 (err =  0x1.baca5ce8978ffp-65)
    {  0x1.3200000039444p+0,   0x1.ac5701ac06d83p-1,   0x1.3d5d335da0000p-4,   0x1.90000001ca220p-3}, //   50 * 0x1p-8 = 0x1.90p-3 (err =  0x1.02c2d38af266dp-66)
    {  0x1.32ffffff58090p+0,   0x1.aaf1d2f968567p-1,   0x1.432afab210000p-4,   0x1.97fffffac0480p-3}, //   51 * 0x1p-8 = 0x1.98p-3 (err =  0x1.609a9cfc44c47p-64)
    {  0x1.33ffffff683aap+0,   0x1.a98ef60777eeep-1,   0x1.48f3ed1a88000p-4,   0x1.9ffffffb41d50p-3}, //   52 * 0x1p-8 = 0x1.a0p-3 (err =  0x1.87effcf3adc18p-65)
    {  0x1.3500000033f60p+0,   0x1.a82e6512c6c13p-1,   0x1.4eb8129aa8000p-4,   0x1.a80000019fb00p-3}, //   53 * 0x1p-8 = 0x1.a8p-3 (err =  0x1.b3deec72f2d1fp-64)
    {  0x1.35ffffff711ddp+0,   0x1.a6d01a6dc4884p-1,   0x1.5477731640000p-4,   0x1.affffffb88ee8p-3}, //   54 * 0x1p-8 = 0x1.b0p-3 (err =  0x1.97b06522d08b2p-64)
    {  0x1.36ffffffa4932p+0,   0x1.a57410770489ep-1,   0x1.5a32167928000p-4,   0x1.b7fffffd24990p-3}, //   55 * 0x1p-8 = 0x1.b8p-3 (err =  0x1.b0e05b6d4cd9fp-67)
    {  0x1.3800000099101p+0,   0x1.a41a41a34c290p-1,   0x1.5fe8048c18000p-4,   0x1.c0000004c8808p-3}, //   56 * 0x1p-8 = 0x1.c0p-3 (err =  0x1.673a3b801ec16p-64)
    {  0x1.38fffff7596fbp+0,   0x1.a2c2a887e4a33p-1,   0x1.659944c790000p-4,   0x1.c7ffffbacb7d8p-3}, //   57 * 0x1p-8 = 0x1.c8p-3 (err =  0x1.dba0781e4b7a7p-64)
    {  0x1.3a00000a86abbp+0,   0x1.a16d3f89a66f3p-1,   0x1.6b45dfaae0000p-4,   0x1.d0000054355d8p-3}, //   58 * 0x1p-8 = 0x1.d0p-3 (err =  0x1.f7c90ae5e4cf8p-64)
    {  0x1.3b000000003e4p+0,   0x1.a01a01a019af6p-1,   0x1.70eddb7d80000p-4,   0x1.d800000001f20p-3}, //   59 * 0x1p-8 = 0x1.d8p-3 (err =  0x1.3a6b3dcfbd427p-64)
    {  0x1.3bffffffa8e0ep+0,   0x1.9ec8e9517598ep-1,   0x1.769140a068000p-4,   0x1.dffffffd47070p-3}, //   60 * 0x1p-8 = 0x1.e0p-3 (err =  0x1.6015dd29d8c99p-64)
    {  0x1.3d0000003a5bfp+0,   0x1.9d79f1766a641p-1,   0x1.7c30164ba8000p-4,   0x1.e8000001d2df8p-3}, //   61 * 0x1p-8 = 0x1.e8p-3 (err =  0x1.6c2b7d4205fd5p-64)
    {  0x1.3e000002fb478p+0,   0x1.9c2d14ea6cbcdp-1,   0x1.81ca63e108000p-4,   0x1.f0000017da3c0p-3}, //   62 * 0x1p-8 = 0x1.f0p-3 (err =  0x1.bcca7d76daea2p-64)
    {  0x1.3f00000128018p+0,   0x1.9ae24ea3d3c96p-1,   0x1.87603083a8000p-4,   0x1.f8000009400c0p-3}, //   63 * 0x1p-8 = 0x1.f8p-3 (err =  0x1.6444952edf563p-66)
    {  0x1.4000000243275p+0,   0x1.99999996b4489p-1,   0x1.8cf18394f8000p-4,   0x1.000000090c9d4p-2}, //   64 * 0x1p-8 = 0x1.00p-2 (err =  0x1.6b1a193f2c1fep-64)
    {  0x1.410000013395bp+0,   0x1.9852f0d764cd9p-1,   0x1.927e641eb8000p-4,   0x1.04000004ce56cp-2}, //   65 * 0x1p-8 = 0x1.04p-2 (err =  0x1.c0f109f1bd309p-64)
    {  0x1.41ffffffc158ap+0,   0x1.970e4f811abb5p-1,   0x1.9806d93ff0000p-4,   0x1.07ffffff05628p-2}, //   66 * 0x1p-8 = 0x1.08p-2 (err =  0x1.2a3570034cc77p-64)
    {  0x1.42ffffff42ad7p+0,   0x1.95cbb0bf25550p-1,   0x1.9d8aea0438000p-4,   0x1.0bfffffd0ab5cp-2}, //   67 * 0x1p-8 = 0x1.0cp-2 (err =  0x1.67c7419d8a79dp-69)
    {  0x1.4400000021b25p+0,   0x1.948b0fcd448b5p-1,   0x1.a30a9d6158000p-4,   0x1.1000000086c94p-2}, //   68 * 0x1p-8 = 0x1.10p-2 (err =  0x1.c71d9f69abdadp-64)
    {  0x1.44ffffffc837dp+0,   0x1.934c67f9f806fp-1,   0x1.a885fa2c30000p-4,   0x1.13ffffff20df4p-2}, //   69 * 0x1p-8 = 0x1.14p-2 (err =  0x1.66485fd2cc69ap-65)
    {  0x1.45fffffefd419p+0,   0x1.920fb49e4d3f9p-1,   0x1.adfd073be8000p-4,   0x1.17fffffbf5064p-2}, //   70 * 0x1p-8 = 0x1.18p-2 (err =  0x1.08c57441e5a26p-67)
    {  0x1.47000000dd577p+0,   0x1.90d4f11f09bc2p-1,   0x1.b36fcb6440000p-4,   0x1.1c000003755dcp-2}, //   71 * 0x1p-8 = 0x1.1cp-2 (err =  0x1.beccf5965d1b6p-64)
    {  0x1.47ffffffdbe35p+0,   0x1.8f9c18f9ed8eap-1,   0x1.b8de4d39f0000p-4,   0x1.1fffffff6f8d4p-2}, //   72 * 0x1p-8 = 0x1.20p-2 (err =  0x1.514c3c42611f7p-64)
    {  0x1.490000017e560p+0,   0x1.8e6527ad4478cp-1,   0x1.be48937e40000p-4,   0x1.24000005f9580p-2}, //   73 * 0x1p-8 = 0x1.24p-2 (err =  0x1.26d80b4634ddap-65)
    {  0x1.49ffffff9fbb4p+0,   0x1.8d3018d3756bap-1,   0x1.c3aea4a3c0000p-4,   0x1.27fffffe7eed0p-2}, //   74 * 0x1p-8 = 0x1.28p-2 (err =  0x1.344ec167b0404p-64)
    {  0x1.4afffffff3b95p+0,   0x1.8bfce8063ea35p-1,   0x1.c910874dc8000p-4,   0x1.2bffffffcee54p-2}, //   75 * 0x1p-8 = 0x1.2cp-2 (err =  0x1.534022c6c1236p-65)
    {  0x1.4c0000006dc4ep+0,   0x1.8acb90f63cb2bp-1,   0x1.ce6e41e6b0000p-4,   0x1.30000001b7138p-2}, //   76 * 0x1p-8 = 0x1.30p-2 (err =  0x1.5af1141764a9cp-64)
    {  0x1.4d0000003efddp+0,   0x1.899c0f5fce24ep-1,   0x1.d3c7dad0a8000p-4,   0x1.34000000fbf74p-2}, //   77 * 0x1p-8 = 0x1.34p-2 (err =  0x1.178b776d99c72p-65)
    {  0x1.4dfffffebf91cp+0,   0x1.886e5f0c33818p-1,   0x1.d91d586000000p-4,   0x1.37fffffafe470p-2}, //   78 * 0x1p-8 = 0x1.38p-2 (err =  0x1.1a5789293c0cap-68)
    {  0x1.4effffffebe88p+0,   0x1.87427bcc20a2ep-1,   0x1.de6ec0f328000p-4,   0x1.3bffffffafa20p-2}, //   79 * 0x1p-8 = 0x1.3cp-2 (err =  0x1.d67df7702714bp-67)
    {  0x1.5000000055d30p+0,   0x1.86186185b4bd4p-1,   0x1.e3bc1ab2a8000p-4,   0x1.40000001574c0p-2}, //   80 * 0x1p-8 = 0x1.40p-2 (err =  0x1.cd8109ccd43ccp-64)
    {  0x1.50ffffff0a37fp+0,   0x1.84f00c289c0a6p-1,   0x1.e9056bc620000p-4,   0x1.43fffffc28dfcp-2}, //   81 * 0x1p-8 = 0x1.44p-2 (err =  0x1.504a3d45d8182p-65)
    {  0x1.5200000186d2bp+0,   0x1.83c977a96b89dp-1,   0x1.ee4aba6918000p-4,   0x1.480000061b4acp-2}, //   82 * 0x1p-8 = 0x1.48p-2 (err =  0x1.cf138aea2b476p-65)
    {  0x1.52ffffffa6762p+0,   0x1.82a4a01890695p-1,   0x1.f38c0c8150000p-4,   0x1.4bfffffe99d88p-2}, //   83 * 0x1p-8 = 0x1.4cp-2 (err =  0x1.6fbdb906a9ac5p-66)
    {  0x1.53ffffffb9a50p+0,   0x1.81818181d1472p-1,   0x1.f8c96832f8000p-4,   0x1.4ffffffee6940p-2}, //   84 * 0x1p-8 = 0x1.50p-2 (err =  0x1.43c4be7c638b8p-64)
    {  0x1.550000000f143p+0,   0x1.80601805f0813p-1,   0x1.fe02d36a88000p-4,   0x1.540000003c50cp-2}, //   85 * 0x1p-8 = 0x1.54p-2 (err =  0x1.74f3d15308069p-64)
    {  0x1.55ffffffbe23bp+0,   0x1.7f405fd061c20p-1,   0x1.019c2a05a0000p-3,   0x1.57fffffef88ecp-2}, //   86 * 0x1p-8 = 0x1.58p-2 (err =  0x1.8d888051ba859p-67)
    {  0x1.57000000edddap+0,   0x1.7e2255149bf0ap-1,   0x1.0434f7fd88000p-3,   0x1.5c000003b7768p-2}, //   87 * 0x1p-8 = 0x1.5cp-2 (err =  0x1.c9e3114115de3p-64)
    {  0x1.580000010b8ccp+0,   0x1.7d05f416a806fp-1,   0x1.06cbd67d20000p-3,   0x1.600000042e330p-2}, //   88 * 0x1p-8 = 0x1.60p-2 (err =  0x1.2a4bd851017ffp-64)
    {  0x1.58ffffffd3aefp+0,   0x1.7beb392310e51p-1,   0x1.0960c8641c000p-3,   0x1.63ffffff4ebbcp-2}, //   89 * 0x1p-8 = 0x1.64p-2 (err =  0x1.19660f6bbe2cep-64)
    {  0x1.5a0000004f422p+0,   0x1.7ad2208db8056p-1,   0x1.0bf3d09448000p-3,   0x1.680000013d088p-2}, //   90 * 0x1p-8 = 0x1.68p-2 (err =  0x1.e307dffae1290p-64)
    {  0x1.5b0000002bf5bp+0,   0x1.79baa6bb33be7p-1,   0x1.0e84f1db4c000p-3,   0x1.6c000000afd6cp-2}, //   91 * 0x1p-8 = 0x1.6cp-2 (err =  0x1.3264ea7dccd0fp-64)
    {  0x1.5bffffff0b619p+0,   0x1.78a4c818930d6p-1,   0x1.11142f05a0000p-3,   0x1.6ffffffc2d864p-2}, //   92 * 0x1p-8 = 0x1.70p-2 (err =  0x1.12875e17607f5p-66)
    {  0x1.5d0000004a44cp+0,   0x1.779081195c74ep-1,   0x1.13a18ae314000p-3,   0x1.7400000129130p-2}, //   93 * 0x1p-8 = 0x1.74p-2 (err =  0x1.601258e932514p-64)
    {  0x1.5e0000001d312p+0,   0x1.767dce432b5f0p-1,   0x1.162d082b14000p-3,   0x1.7800000074c48p-2}, //   94 * 0x1p-8 = 0x1.78p-2 (err =  0x1.62a92b228f2e8p-65)
    {  0x1.5effffff6a465p+0,   0x1.756cac20b6a14p-1,   0x1.18b6a99b04000p-3,   0x1.7bfffffda9194p-2}, //   95 * 0x1p-8 = 0x1.7cp-2 (err =  0x1.2eb72ef4fa65ep-68)
    {  0x1.5ffffffeb32e7p+0,   0x1.745d17473186fp-1,   0x1.1b3e71e94c000p-3,   0x1.7ffffffaccb9cp-2}, //   96 * 0x1p-8 = 0x1.80p-2 (err =  0x1.1839aa6a16844p-64)
    {  0x1.60ffffffa65afp+0,   0x1.734f0c547e342p-1,   0x1.1dc463c960000p-3,   0x1.83fffffe996bcp-2}, //   97 * 0x1p-8 = 0x1.84p-2 (err =  0x1.b178b0300aa1ap-64)
    {  0x1.61fffffeff7fbp+0,   0x1.724287f57a342p-1,   0x1.204881dc64000p-3,   0x1.87fffffbfdfecp-2}, //   98 * 0x1p-8 = 0x1.88p-2 (err =  0x1.3fc7e39ccb4c2p-64)
    {  0x1.62fffffd0081bp+0,   0x1.713786dce5fb5p-1,   0x1.22cacec6a4000p-3,   0x1.8bfffff40206cp-2}, //   99 * 0x1p-8 = 0x1.8cp-2 (err =  0x1.ce7dba85504bcp-64)
    {  0x1.63ffffff5328dp+0,   0x1.702e05c16ad80p-1,   0x1.254b4d3438000p-3,   0x1.8ffffffd4ca34p-2}, //  100 * 0x1p-8 = 0x1.90p-2 (err =  0x1.7c94b9b74f329p-65)
    {  0x1.64ffffff0c852p+0,   0x1.6f26017020683p-1,   0x1.27c9ffac14000p-3,   0x1.93fffffc32148p-2}, //  101 * 0x1p-8 = 0x1.94p-2 (err =  0x1.b89ca10283494p-65)
    {  0x1.65ffffffd174ep+0,   0x1.6e1f76b46315ep-1,   0x1.2a46e8ca08000p-3,   0x1.97ffffff45d38p-2}, //  102 * 0x1p-8 = 0x1.98p-2 (err =  0x1.0f00f8f68f9a8p-64)
    {  0x1.66ffffffe7b85p+0,   0x1.6d1a626835377p-1,   0x1.2cc20b16f8000p-3,   0x1.9bffffff9ee14p-2}, //  103 * 0x1p-8 = 0x1.9cp-2 (err =  0x1.9bed31f4ae761p-64)
    {  0x1.6800000039798p+0,   0x1.6c16c16bdca0dp-1,   0x1.2f3b691ce8000p-3,   0x1.a0000000e5e60p-2}, //  104 * 0x1p-8 = 0x1.a0p-2 (err =  0x1.07ee3eb4d8dcdp-64)
    {  0x1.690000002278cp+0,   0x1.6b1490aa0ef83p-1,   0x1.31b3055c9c000p-3,   0x1.a400000089e30p-2}, //  105 * 0x1p-8 = 0x1.a4p-2 (err =  0x1.c9ecaf5cdfa68p-65)
    {  0x1.6a0000006170bp+0,   0x1.6a13cd14d5b30p-1,   0x1.3428e254f0000p-3,   0x1.a800000185c2cp-2}, //  106 * 0x1p-8 = 0x1.a8p-2 (err =  0x1.790ba2bc618e3p-65)
    {  0x1.6affffffce223p+0,   0x1.691473a8bea64p-1,   0x1.369d027acc000p-3,   0x1.abffffff3888cp-2}, //  107 * 0x1p-8 = 0x1.acp-2 (err =  0x1.d9ebf95adcfc8p-64)
    {  0x1.6bffffffda0e0p+0,   0x1.681681683c0b0p-1,   0x1.390f684444000p-3,   0x1.afffffff68380p-2}, //  108 * 0x1p-8 = 0x1.b0p-2 (err =  0x1.8c98cbf7c3929p-64)
    {  0x1.6d000000a3c26p+0,   0x1.6719f35f7554cp-1,   0x1.3b80161f00000p-3,   0x1.b40000028f098p-2}, //  109 * 0x1p-8 = 0x1.b4p-2 (err =  0x1.6efd95070482fp-65)
    {  0x1.6e0000005812ap+0,   0x1.661ec6a4bc026p-1,   0x1.3def0e6ed4000p-3,   0x1.b8000001604a8p-2}, //  110 * 0x1p-8 = 0x1.b8p-2 (err =  0x1.28d6ffc86e65ap-64)
    {  0x1.6f00000024887p+0,   0x1.6524f853911cfp-1,   0x1.405c5399dc000p-3,   0x1.bc0000009221cp-2}, //  111 * 0x1p-8 = 0x1.bcp-2 (err =  0x1.d2a94ba574dc6p-64)
    {  0x1.6ffffffe30b54p+0,   0x1.642c8592727d9p-1,   0x1.42c7e7f9e0000p-3,   0x1.bffffff8c2d50p-2}, //  112 * 0x1p-8 = 0x1.c0p-2 (err =  0x1.8795aedc65876p-64)
    {  0x1.710000000448cp+0,   0x1.63356b88a7ee2p-1,   0x1.4531cdf58c000p-3,   0x1.c400000011230p-2}, //  113 * 0x1p-8 = 0x1.c4p-2 (err =  0x1.aa68101395385p-67)
    {  0x1.71ffffff6af7ep+0,   0x1.623fa770a4d3fp-1,   0x1.479a07d250000p-3,   0x1.c7fffffdabdf8p-2}, //  114 * 0x1p-8 = 0x1.c8p-2 (err =  0x1.d44483759c922p-64)
    {  0x1.73000000f4596p+0,   0x1.614b368232394p-1,   0x1.4a0097eac0000p-3,   0x1.cc000003d1658p-2}, //  115 * 0x1p-8 = 0x1.ccp-2 (err =  0x1.e6f6e123ea9ebp-65)
    {  0x1.73fffffe0213ep+0,   0x1.60581607645afp-1,   0x1.4c658079d0000p-3,   0x1.cffffff8084f8p-2}, //  116 * 0x1p-8 = 0x1.d0p-2 (err =  0x1.a21ae547af626p-65)
    {  0x1.750000002117dp+0,   0x1.5f66434273b29p-1,   0x1.4ec8c3dc74000p-3,   0x1.d4000000845f4p-2}, //  117 * 0x1p-8 = 0x1.d4p-2 (err =  0x1.31c0714142537p-64)
    {  0x1.75ffffffa90eap+0,   0x1.5e75bb8d52d6fp-1,   0x1.512a6441c8000p-3,   0x1.d7fffffea43a8p-2}, //  118 * 0x1p-8 = 0x1.d8p-2 (err =  0x1.efab9c6633eebp-64)
    {  0x1.77000000b17fap+0,   0x1.5d867c3e28b98p-1,   0x1.538a63f058000p-3,   0x1.dc000002c5fe8p-2}, //  119 * 0x1p-8 = 0x1.dcp-2 (err =  0x1.cfe3b18f4a8b9p-69)
    {  0x1.78000000bb3f9p+0,   0x1.5c9882b8836bap-1,   0x1.55e8c51a6c000p-3,   0x1.e0000002ecfe4p-2}, //  120 * 0x1p-8 = 0x1.e0p-2 (err =  0x1.e0b6e4a5d18cap-67)
    {  0x1.7900000654629p+0,   0x1.5babcc5ea9559p-1,   0x1.58458a0330000p-3,   0x1.e4000019518a4p-2}, //  121 * 0x1p-8 = 0x1.e4p-2 (err =  0x1.624bc150462e0p-64)
    {  0x1.7a0000008cdbap+0,   0x1.5ac056af94756p-1,   0x1.5aa0b4b1e4000p-3,   0x1.e8000002336e8p-2}, //  122 * 0x1p-8 = 0x1.e8p-2 (err =  0x1.fb75caf4af4ebp-66)
    {  0x1.7b000000a46e4p+0,   0x1.59d61f11a6bfcp-1,   0x1.5cfa477a00000p-3,   0x1.ec00000291b90p-2}, //  123 * 0x1p-8 = 0x1.ecp-2 (err =  0x1.2bae7b3be75edp-65)
    {  0x1.7bffffffc36c8p+0,   0x1.58ed23084c8b0p-1,   0x1.5f524471c8000p-3,   0x1.efffffff0db20p-2}, //  124 * 0x1p-8 = 0x1.f0p-2 (err =  0x1.c8d3552d32cc7p-66)
    {  0x1.7cffffffcc4f2p+0,   0x1.58056015af028p-1,   0x1.61a8adbfb4000p-3,   0x1.f3ffffff313c8p-2}, //  125 * 0x1p-8 = 0x1.f4p-2 (err =  0x1.f718cf9e6b7ffp-66)
    {  0x1.7e00000012a2ep+0,   0x1.571ed3c4f5f65p-1,   0x1.63fd857ff0000p-3,   0x1.f80000004a8b8p-2}, //  126 * 0x1p-8 = 0x1.f8p-2 (err =  0x1.8e62144a7a1f5p-64)
    {  0x1.7effffffd5151p+0,   0x1.56397ba7eb876p-1,   0x1.6650cdca3c000p-3,   0x1.fbffffff54544p-2}, //  127 * 0x1p-8 = 0x1.fcp-2 (err =  0x1.528de7b730fcep-65)
    {  0x1.8100000020579p-1,   0x1.54725e6b9b967p+0,  -0x1.fb1f6380f0000p-4,  -0x1.fbffffff7ea1cp-3}, // -127 * 0x1p-9 = -0x1.fcp-3 (err =  0x1.f5f81c032ad42p-66)
    {  0x1.8200000015cbdp-1,   0x1.5390948f2dd22p+0,  -0x1.f68216c968000p-4,  -0x1.f7ffffffa8d0cp-3}, // -126 * 0x1p-9 = -0x1.f8p-3 (err =  0x1.2f17a6177e274p-65)
    {  0x1.83000000f6d2bp-1,   0x1.52aff569a8522p+0,  -0x1.f1e7d87e48000p-4,  -0x1.f3fffffc24b54p-3}, // -125 * 0x1p-9 = -0x1.f4p-3 (err =  0x1.8a499f9340ac8p-64)
    {  0x1.83fffffd4e779p-1,   0x1.51d07eb087d9ep+0,  -0x1.ed50a4aec8000p-4,  -0x1.f000000ac621cp-3}, // -124 * 0x1p-9 = -0x1.f0p-3 (err =  0x1.78793cf3a9569p-64)
    {  0x1.850000001247ap-1,   0x1.50f22e110c76fp+0,  -0x1.e8bc7726c8000p-4,  -0x1.ebffffffb6e18p-3}, // -123 * 0x1p-9 = -0x1.ecp-3 (err =  0x1.373118a2248c3p-64)
    {  0x1.85fffffeb357ep-1,   0x1.5015015133abfp+0,  -0x1.e42b4c1cb8000p-4,  -0x1.e800000532a08p-3}, // -122 * 0x1p-9 = -0x1.e8p-3 (err =  0x1.4786730f5a50bp-64)
    {  0x1.870000004fe08p-1,   0x1.4f38f62d904e3p+0,  -0x1.df9d1f7df0000p-4,  -0x1.e3fffffec07e0p-3}, // -121 * 0x1p-9 = -0x1.e4p-3 (err =  0x1.d66ed595d875fp-64)
    {  0x1.87ffffff02bedp-1,   0x1.4e5e0a73c858ep+0,  -0x1.db11ed7ae8000p-4,  -0x1.e0000003f504cp-3}, // -120 * 0x1p-9 = -0x1.e0p-3 (err =  0x1.f4bb406d244c0p-66)
    {  0x1.8900000113832p-1,   0x1.4d843becd8f50p+0,  -0x1.d689b21460000p-4,  -0x1.dbfffffbb1f38p-3}, // -119 * 0x1p-9 = -0x1.dcp-3 (err =  0x1.9b05437deff54p-69)
    {  0x1.89ffffff43545p-1,   0x1.4cab8872fa444p+0,  -0x1.d204699008000p-4,  -0x1.d8000002f2aecp-3}, // -118 * 0x1p-9 = -0x1.d8p-3 (err =  0x1.c307e2c7eb2efp-64)
    {  0x1.8b000000b18b0p-1,   0x1.4bd3edd9d3d80p+0,  -0x1.cd820ffa08000p-4,  -0x1.d3fffffd39d40p-3}, // -117 * 0x1p-9 = -0x1.d4p-3 (err =  0x1.a04b840f7796fp-65)
    {  0x1.8bffffffbbac3p-1,   0x1.4afd6a056511ep+0,  -0x1.c902a19f98000p-4,  -0x1.d0000001114f4p-3}, // -116 * 0x1p-9 = -0x1.d0p-3 (err =  0x1.2ca02b531bedbp-64)
    {  0x1.8cfffffdac7c8p-1,   0x1.4a27fad94f538p+0,  -0x1.c4861ab600000p-4,  -0x1.cc0000094e0e0p-3}, // -115 * 0x1p-9 = -0x1.ccp-3 (err =  0x1.cb90b0fee4e94p-64)
    {  0x1.8e0000022272bp-1,   0x1.49539e3968dd6p+0,  -0x1.c00c775d98000p-4,  -0x1.c7fffff776354p-3}, // -114 * 0x1p-9 = -0x1.c8p-3 (err =  0x1.c1c55c33073bbp-64)
    {  0x1.8f000000401b3p-1,   0x1.4880521fdfc08p+0,  -0x1.bb95b41998000p-4,  -0x1.c3fffffeff934p-3}, // -113 * 0x1p-9 = -0x1.c4p-3 (err =  0x1.de89092b54d7cp-65)
    {  0x1.8ffffffe3f82fp-1,   0x1.47ae147c50ae9p+0,  -0x1.b721cd1ee0000p-4,  -0x1.c000000701f44p-3}, // -112 * 0x1p-9 = -0x1.c0p-3 (err =  0x1.3bd630edabde4p-64)
    {  0x1.90fffffdf1c99p-1,   0x1.46dce34742f37p+0,  -0x1.b2b0bebd38000p-4,  -0x1.bc00000838d9cp-3}, // -111 * 0x1p-9 = -0x1.bcp-3 (err =  0x1.a2695aed9d91ep-64)
    {  0x1.91fffffd2d244p-1,   0x1.460cbc81a7436p+0,  -0x1.ae42855d18000p-4,  -0x1.b800000b4b6f0p-3}, // -110 * 0x1p-9 = -0x1.b8p-3 (err =  0x1.e7d79353e2afcp-64)
    {  0x1.930000012b1b9p-1,   0x1.453d9e2b8607ap+0,  -0x1.a9d71d4d30000p-4,  -0x1.b3fffffb5391cp-3}, // -109 * 0x1p-9 = -0x1.b4p-3 (err =  0x1.f75d1ec8134aap-66)
    {  0x1.93ffffffef250p-1,   0x1.446f86563b28ep+0,  -0x1.a56e832640000p-4,  -0x1.b0000000436c0p-3}, // -108 * 0x1p-9 = -0x1.b0p-3 (err =  0x1.3c23f33545c02p-66)
    {  0x1.94fffffff7939p-1,   0x1.43a2730ac5a00p+0,  -0x1.a108b33f00000p-4,  -0x1.ac00000021b1cp-3}, // -107 * 0x1p-9 = -0x1.acp-3 (err =  0x1.fac93023a08b1p-65)
    {  0x1.9600000047a7cp-1,   0x1.42d6625d18fe2p+0,  -0x1.9ca5aa15f0000p-4,  -0x1.a7fffffee1610p-3}, // -106 * 0x1p-9 = -0x1.a8p-3 (err =  0x1.1a6691f7557adp-64)
    {  0x1.9700000159275p-1,   0x1.420b5264d4799p+0,  -0x1.98456429c8000p-4,  -0x1.a3fffffa9b62cp-3}, // -105 * 0x1p-9 = -0x1.a4p-3 (err =  0x1.3dd5e25e8cccfp-64)
    {  0x1.97ffffff80747p-1,   0x1.41414141a5aecp+0,  -0x1.93e7de11f0000p-4,  -0x1.a0000001fe2e4p-3}, // -104 * 0x1p-9 = -0x1.a0p-3 (err =  0x1.47a7866eaf563p-64)
    {  0x1.98ffffff82dd1p-1,   0x1.40782d1148632p+0,  -0x1.8f8d144778000p-4,  -0x1.9c000001f48bcp-3}, // -103 * 0x1p-9 = -0x1.9cp-3 (err =  0x1.0df5f320a6769p-64)
    {  0x1.99ffffff4e8fcp-1,   0x1.3fb013fb8b9a3p+0,  -0x1.8b350367c8000p-4,  -0x1.98000002c5c10p-3}, // -102 * 0x1p-9 = -0x1.98p-3 (err =  0x1.083be905a8af3p-64)
    {  0x1.9affffff0ea73p-1,   0x1.3ee8f42b16358p+0,  -0x1.86dfa80ce8000p-4,  -0x1.94000003c5634p-3}, // -101 * 0x1p-9 = -0x1.94p-3 (err =  0x1.aa355ac8cfe29p-64)
    {  0x1.9c0000003639fp-1,   0x1.3e22cbce20b0ep+0,  -0x1.828cfed1b0000p-4,  -0x1.8fffffff27184p-3}, // -100 * 0x1p-9 = -0x1.90p-3 (err =  0x1.c8c1f402dec10p-64)
    {  0x1.9d0000007986dp-1,   0x1.3d5d991a49f99p+0,  -0x1.7e3d046770000p-4,  -0x1.8bfffffe19e4cp-3}, //  -99 * 0x1p-9 = -0x1.8cp-3 (err =  0x1.fe27330bc0203p-64)
    {  0x1.9dffffffb9222p-1,   0x1.3c995a47f0f02p+0,  -0x1.79efb57c40000p-4,  -0x1.880000011b778p-3}, //  -98 * 0x1p-9 = -0x1.88p-3 (err =  0x1.1d22717714906p-65)
    {  0x1.9effffff970d1p-1,   0x1.3bd60d9282746p+0,  -0x1.75a50ebcd8000p-4,  -0x1.84000001a3cbcp-3}, //  -97 * 0x1p-9 = -0x1.84p-3 (err =  0x1.238309114dc79p-65)
    {  0x1.a0000000cbd97p-1,   0x1.3b13b13a794c3p+0,  -0x1.715d0ce000000p-4,  -0x1.7ffffffcd09a4p-3}, //  -96 * 0x1p-9 = -0x1.80p-3 (err =  0x1.0d449f4cebe58p-64)
    {  0x1.a10000007d2b9p-1,   0x1.3a5243874e28cp+0,  -0x1.6d17acb1d0000p-4,  -0x1.7bfffffe0b51cp-3}, //  -95 * 0x1p-9 = -0x1.7cp-3 (err =  0x1.83ed5b1649431p-65)
    {  0x1.a20000018a2ddp-1,   0x1.3991c2c060431p+0,  -0x1.68d4eaebe0000p-4,  -0x1.77fffff9d748cp-3}, //  -94 * 0x1p-9 = -0x1.78p-3 (err =  0x1.324bf40c39a19p-64)
    {  0x1.a2ffffff96376p-1,   0x1.38d22d36af830p+0,  -0x1.6494c46c88000p-4,  -0x1.74000001a7228p-3}, //  -93 * 0x1p-9 = -0x1.74p-3 (err =  0x1.9c355ffc2c56ap-64)
    {  0x1.a3ffffff52508p-1,   0x1.38138138948f5p+0,  -0x1.605735f178000p-4,  -0x1.70000002b6be0p-3}, //  -92 * 0x1p-9 = -0x1.70p-3 (err =  0x1.bc96b35af404cp-66)
    {  0x1.a500000023d96p-1,   0x1.3755bd1c79dc1p+0,  -0x1.5c1c3c54c0000p-4,  -0x1.6bffffff709a8p-3}, //  -91 * 0x1p-9 = -0x1.6cp-3 (err =  0x1.13e1fc8967545p-64)
    {  0x1.a600000a98b56p-1,   0x1.3698df3613de5p+0,  -0x1.57e3d44f90000p-4,  -0x1.67ffffd59d2a8p-3}, //  -90 * 0x1p-9 = -0x1.68p-3 (err =  0x1.c6ecd3a78c265p-64)
    {  0x1.a6ffffffa5d94p-1,   0x1.35dce5fa34b99p+0,  -0x1.53adfb47a8000p-4,  -0x1.64000001689b0p-3}, //  -89 * 0x1p-9 = -0x1.64p-3 (err =  0x1.276501a837f13p-65)
    {  0x1.a7ffffffc7ec3p-1,   0x1.3521cfb2e06ebp+0,  -0x1.4f7aad9ca8000p-4,  -0x1.60000000e04f4p-3}, //  -88 * 0x1p-9 = -0x1.60p-3 (err =  0x1.0f3a99ed0924bp-70)
    {  0x1.a9000000c368ap-1,   0x1.34679acd7367ap+0,  -0x1.4b49e867e0000p-4,  -0x1.5bfffffcf25d8p-3}, //  -87 * 0x1p-9 = -0x1.5cp-3 (err =  0x1.bae812af44d60p-64)
    {  0x1.a9ffffff83088p-1,   0x1.33ae45b5d60d2p+0,  -0x1.471ba8a9e8000p-4,  -0x1.58000001f3de0p-3}, //  -86 * 0x1p-9 = -0x1.58p-3 (err =  0x1.c5c4a7eb5fbdap-64)
    {  0x1.aafffffd78f4bp-1,   0x1.32f5ced87304bp+0,  -0x1.42efeb55d8000p-4,  -0x1.5400000a1c2d4p-3}, //  -85 * 0x1p-9 = -0x1.54p-3 (err =  0x1.cea2ebb55fbfcp-65)
    {  0x1.ac000000f82ffp-1,   0x1.323e34a1ff76bp+0,  -0x1.3ec6ad5000000p-4,  -0x1.4ffffffc1f404p-3}, //  -84 * 0x1p-9 = -0x1.50p-3 (err =  0x1.2cb663b47aee1p-64)
    {  0x1.acffffffd2f69p-1,   0x1.3187758ebece8p+0,  -0x1.3a9febc6c0000p-4,  -0x1.4c000000b425cp-3}, //  -83 * 0x1p-9 = -0x1.4cp-3 (err =  0x1.25b620ef3d7b6p-65)
    {  0x1.ae00000055754p-1,   0x1.30d19012d084ap+0,  -0x1.367ba3a940000p-4,  -0x1.47fffffeaa2b0p-3}, //  -82 * 0x1p-9 = -0x1.48p-3 (err =  0x1.65d549f554757p-66)
    {  0x1.aeffffff6e125p-1,   0x1.301c82aca71d6p+0,  -0x1.3259d212d8000p-4,  -0x1.4400000247b6cp-3}, //  -81 * 0x1p-9 = -0x1.44p-3 (err =  0x1.170c3023b0acap-67)
    {  0x1.b00000009d5e4p-1,   0x1.2f684bd9a4703p+0,  -0x1.2e3a7408f0000p-4,  -0x1.3ffffffd8a870p-3}, //  -80 * 0x1p-9 = -0x1.40p-3 (err =  0x1.3d496a2956019p-64)
    {  0x1.b100000823666p-1,   0x1.2eb4ea1a3c97ep+0,  -0x1.2a1d869330000p-4,  -0x1.3bffffdf72668p-3}, //  -79 * 0x1p-9 = -0x1.3cp-3 (err =  0x1.82a9382d970c1p-64)
    {  0x1.b1ffffffd490ap-1,   0x1.2e025c04d6432p+0,  -0x1.2603072ad8000p-4,  -0x1.38000000adbd8p-3}, //  -78 * 0x1p-9 = -0x1.38p-3 (err =  0x1.a13c499f1c8c2p-64)
    {  0x1.b2ffffff92582p-1,   0x1.2d50a01320fecp+0,  -0x1.21eaf29118000p-4,  -0x1.34000001b69f8p-3}, //  -77 * 0x1p-9 = -0x1.34p-3 (err =  0x1.c04677a71964bp-65)
    {  0x1.b3ffffffb2535p-1,   0x1.2c9fb4d848589p+0,  -0x1.1dd5460dc8000p-4,  -0x1.3000000136b2cp-3}, //  -76 * 0x1p-9 = -0x1.30p-3 (err =  0x1.3a5a15e4c0c45p-65)
    {  0x1.b4ffffff1df96p-1,   0x1.2bef98e63e933p+0,  -0x1.19c1fed2b0000p-4,  -0x1.2c000003881a8p-3}, //  -75 * 0x1p-9 = -0x1.2cp-3 (err =  0x1.6e7f44b633b7dp-65)
    {  0x1.b60000002008ep-1,   0x1.2b404acffcd10p+0,  -0x1.15b11a08c8000p-4,  -0x1.27ffffff7fdc8p-3}, //  -74 * 0x1p-9 = -0x1.28p-3 (err =  0x1.376d66fdbe1f8p-65)
    {  0x1.b70000004ca84p-1,   0x1.2a91c92f07edap+0,  -0x1.11a294f120000p-4,  -0x1.23fffffecd5f0p-3}, //  -73 * 0x1p-9 = -0x1.24p-3 (err =  0x1.67b2f3a689ec8p-64)
    {  0x1.b7ffffff79753p-1,   0x1.29e4129e9c408p+0,  -0x1.0d966cc870000p-4,  -0x1.200000021a2b4p-3}, //  -72 * 0x1p-9 = -0x1.20p-3 (err =  0x1.3d9df5dad23d4p-64)
    {  0x1.b8ffffff9da44p-1,   0x1.293725bbc2946p+0,  -0x1.098c9ec7a8000p-4,  -0x1.1c000001896f0p-3}, //  -71 * 0x1p-9 = -0x1.1cp-3 (err =  0x1.abb922a537b2ap-65)
    {  0x1.ba0000013b15fp-1,   0x1.288b0127b79c2p+0,  -0x1.0585283270000p-4,  -0x1.17fffffb13a84p-3}, //  -70 * 0x1p-9 = -0x1.18p-3 (err =  0x1.cde72a6cdae3ep-64)
    {  0x1.bafffffeedb0ap-1,   0x1.27dfa38ad41a1p+0,  -0x1.01800668e0000p-4,  -0x1.14000004493d8p-3}, //  -69 * 0x1p-9 = -0x1.14p-3 (err =  0x1.57fcbaa448a6bp-64)
    {  0x1.bc00000001defp-1,   0x1.27350b881134ep+0,  -0x1.fafa6d3970000p-5,  -0x1.0ffffffff8844p-3}, //  -68 * 0x1p-9 = -0x1.10p-3 (err =  0x1.ade6e3953f591p-64)
    {  0x1.bcffffffd68f2p-1,   0x1.268b37cd7b806p+0,  -0x1.f2f96c68a0000p-5,  -0x1.0c000000a5c38p-3}, //  -67 * 0x1p-9 = -0x1.0cp-3 (err =  0x1.c11f54c7b5617p-64)
    {  0x1.bdffffff30e20p-1,   0x1.25e2270891a8fp+0,  -0x1.eafd0509d0000p-5,  -0x1.080000033c780p-3}, //  -66 * 0x1p-9 = -0x1.08p-3 (err =  0x1.a03c80aa96503p-72)
    {  0x1.bf0000007a264p-1,   0x1.2539d7e8c75a4p+0,  -0x1.e30531c3a0000p-5,  -0x1.03fffffe17670p-3}, //  -65 * 0x1p-9 = -0x1.04p-3 (err =  0x1.abe41083c1a0ap-67)
    {  0x1.c000000009699p-1,   0x1.249249248c239p+0,  -0x1.db11ed7620000p-5,  -0x1.ffffffffb4b38p-4}, //  -64 * 0x1p-9 = -0x1.00p-3 (err =  0x1.471003ee27e44p-64)
    {  0x1.c1000000bc678p-1,   0x1.23eb7970fb87cp+0,  -0x1.d32332d660000p-5,  -0x1.f7fffffa1cc40p-4}, //  -63 * 0x1p-9 = -0x1.f8p-4 (err =  0x1.79ef2e3241e98p-68)
    {  0x1.c1ffffffd0f25p-1,   0x1.23456789ca42cp+0,  -0x1.cb38fccf00000p-5,  -0x1.f0000001786d8p-4}, //  -62 * 0x1p-9 = -0x1.f0p-4 (err =  0x1.12eea6bc3f3e8p-67)
    {  0x1.c3000000943e6p-1,   0x1.22a01229a19b5p+0,  -0x1.c353462370000p-5,  -0x1.e7fffffb5e0d0p-4}, //  -61 * 0x1p-9 = -0x1.e8p-4 (err =  0x1.98ce078c07e6bp-70)
    {  0x1.c4000000004b0p-1,   0x1.21fb78121f876p+0,  -0x1.bb7209d1e0000p-5,  -0x1.dffffffffda80p-4}, //  -60 * 0x1p-9 = -0x1.e0p-4 (err =  0x1.5a317837c6c44p-65)
    {  0x1.c5000001eb774p-1,   0x1.215798034b752p+0,  -0x1.b39542ab10000p-5,  -0x1.d7fffff0a4460p-4}, //  -59 * 0x1p-9 = -0x1.d8p-4 (err =  0x1.c297f04ef2610p-64)
    {  0x1.c600000086ba7p-1,   0x1.20b470c626609p+0,  -0x1.abbcebd430000p-5,  -0x1.cffffffbca2c8p-4}, //  -58 * 0x1p-9 = -0x1.d0p-4 (err =  0x1.d60dbfb20bfb5p-64)
    {  0x1.c700000043ab2p-1,   0x1.2012011fe7298p+0,  -0x1.a3e9002a60000p-5,  -0x1.c7fffffde2a70p-4}, //  -57 * 0x1p-9 = -0x1.c8p-4 (err =  0x1.3cbe150f04f65p-67)
    {  0x1.c7ffffffd0f1ap-1,   0x1.1f7047dc2fa07p+0,  -0x1.9c197ac070000p-5,  -0x1.c000000178730p-4}, //  -56 * 0x1p-9 = -0x1.c0p-4 (err =  0x1.23cdc274933d9p-64)
    {  0x1.c900000079addp-1,   0x1.1ecf43c7af276p+0,  -0x1.944e569d20000p-5,  -0x1.b7fffffc32918p-4}, //  -55 * 0x1p-9 = -0x1.b8p-4 (err =  0x1.534f263d2542dp-64)
    {  0x1.ca000000e5472p-1,   0x1.1e2ef3b36c436p+0,  -0x1.8c878ee410000p-5,  -0x1.affffff8d5c70p-4}, //  -54 * 0x1p-9 = -0x1.b0p-4 (err =  0x1.269df39788bc1p-69)
    {  0x1.caffffff6c835p-1,   0x1.1d8f5673406d9p+0,  -0x1.84c51ec360000p-5,  -0x1.a80000049be58p-4}, //  -53 * 0x1p-9 = -0x1.a8p-4 (err =  0x1.5d7df5549a39fp-64)
    {  0x1.cc0000002e284p-1,   0x1.1cf06ada0b7a7p+0,  -0x1.7d07014490000p-5,  -0x1.9ffffffe8ebe0p-4}, //  -52 * 0x1p-9 = -0x1.a0p-4 (err =  0x1.130b10b0910fcp-64)
    {  0x1.cd00000025826p-1,   0x1.1c522fc1b6e35p+0,  -0x1.754d31b090000p-5,  -0x1.97fffffed3ed0p-4}, //  -51 * 0x1p-9 = -0x1.98p-4 (err =  0x1.f2af006ce11c7p-65)
    {  0x1.cdffffff1c36cp-1,   0x1.1bb4a404fab3ap+0,  -0x1.6d97ab4280000p-5,  -0x1.900000071e4a0p-4}, //  -50 * 0x1p-9 = -0x1.90p-4 (err =  0x1.f50723d3e4082p-64)
    {  0x1.cefffffffac3bp-1,   0x1.1b17c67f2ee1bp+0,  -0x1.65e6692570000p-5,  -0x1.8800000029e28p-4}, //  -49 * 0x1p-9 = -0x1.88p-4 (err =  0x1.e8ea6d784411ep-67)
    {  0x1.d00000000ba0ep-1,   0x1.1a7b9611a0a51p+0,  -0x1.5e3966b790000p-5,  -0x1.7fffffffa2f90p-4}, //  -48 * 0x1p-9 = -0x1.80p-4 (err =  0x1.52b53bff11a05p-64)
    {  0x1.d10000006b976p-1,   0x1.19e0119dbfe19p+0,  -0x1.56909f4170000p-5,  -0x1.77fffffca3450p-4}, //  -47 * 0x1p-9 = -0x1.78p-4 (err =  0x1.a75ed9b4aadd5p-66)
    {  0x1.d20000010b53ep-1,   0x1.1945380828cefp+0,  -0x1.4eec0e1c60000p-5,  -0x1.6ffffff7a5610p-4}, //  -46 * 0x1p-9 = -0x1.70p-4 (err =  0x1.416bae6086ffdp-64)
    {  0x1.d3000001193d5p-1,   0x1.18ab083859b72p+0,  -0x1.474baeaf20000p-5,  -0x1.67fffff736158p-4}, //  -45 * 0x1p-9 = -0x1.68p-4 (err =  0x1.56049ebaf6ce6p-64)
    {  0x1.d4000000effc8p-1,   0x1.1811811781e34p+0,  -0x1.3faf7c5f10000p-5,  -0x1.5ffffff8801c0p-4}, //  -44 * 0x1p-9 = -0x1.60p-4 (err =  0x1.e9a79d012c6d4p-66)
    {  0x1.d4fffffff1058p-1,   0x1.1778a191c6553p+0,  -0x1.381772a080000p-5,  -0x1.5800000077d40p-4}, //  -43 * 0x1p-9 = -0x1.58p-4 (err =  0x1.e18c35a0734e1p-65)
    {  0x1.d600000091cffp-1,   0x1.16e06893d0b2ep+0,  -0x1.30838cd7e0000p-5,  -0x1.4ffffffb71808p-4}, //  -42 * 0x1p-9 = -0x1.50p-4 (err =  0x1.c2576d5e6b027p-65)
    {  0x1.d6fffffdb7ba8p-1,   0x1.1648d5111c55bp+0,  -0x1.28f3c6aa50000p-5,  -0x1.48000012422c0p-4}, //  -41 * 0x1p-9 = -0x1.48p-4 (err =  0x1.fb496467f27cdp-64)
    {  0x1.d7ffffff68c67p-1,   0x1.15b1e5f7ab697p+0,  -0x1.21681b6100000p-5,  -0x1.40000004b9cc8p-4}, //  -40 * 0x1p-9 = -0x1.40p-4 (err =  0x1.836857a3dd890p-66)
    {  0x1.d8ffffffbd052p-1,   0x1.151b9a40049a1p+0,  -0x1.19e086b5b0000p-5,  -0x1.3800000217d70p-4}, //  -39 * 0x1p-9 = -0x1.38p-4 (err =  0x1.2dca9e936de19p-64)
    {  0x1.d9ffffff76ca2p-1,   0x1.1485f0e0fcdf8p+0,  -0x1.125d0436f0000p-5,  -0x1.3000000449af0p-4}, //  -38 * 0x1p-9 = -0x1.30p-4 (err =  0x1.9878b07dcb7cdp-66)
    {  0x1.db000000eed51p-1,   0x1.13f0e8d2b9b3bp+0,  -0x1.0add8f6ea0000p-5,  -0x1.27fffff889578p-4}, //  -37 * 0x1p-9 = -0x1.28p-4 (err =  0x1.17d18596a2f05p-64)
    {  0x1.dc00000040b25p-1,   0x1.135c81133713fp+0,  -0x1.0362241c80000p-5,  -0x1.1ffffffdfa6d8p-4}, //  -36 * 0x1p-9 = -0x1.20p-4 (err =  0x1.2f7845fc1a3a7p-65)
    {  0x1.dd00000091216p-1,   0x1.12c8b89e886fep+0,  -0x1.f7d57ba540000p-6,  -0x1.17fffffb76f50p-4}, //  -35 * 0x1p-9 = -0x1.18p-4 (err =  0x1.6f89f51faf024p-65)
    {  0x1.de000000f83fdp-1,   0x1.12358e75449a2p+0,  -0x1.e8eeb090c0000p-6,  -0x1.0ffffff83e018p-4}, //  -34 * 0x1p-9 = -0x1.10p-4 (err =  0x1.51499979a0123p-66)
    {  0x1.deffffff8d594p-1,   0x1.11a3019ab6018p+0,  -0x1.da0fde86e0000p-6,  -0x1.0800000395360p-4}, //  -33 * 0x1p-9 = -0x1.08p-4 (err =  0x1.9a612b6674ca8p-65)
    {  0x1.e00000012f05fp-1,   0x1.1111111064ae2p+0,  -0x1.cb38fcbc00000p-6,  -0x1.ffffffed0fa10p-5}, //  -32 * 0x1p-9 = -0x1.00p-4 (err =  0x1.a305dfc8744e3p-66)
    {  0x1.e1000000521d4p-1,   0x1.107fbbdfe282ep+0,  -0x1.bc6a030cc0000p-6,  -0x1.effffffade2c0p-5}, //  -31 * 0x1p-9 = -0x1.f0p-5 (err =  0x1.833c63be0c8a9p-65)
    {  0x1.e2000006e92cbp-1,   0x1.0fef010c08e02p+0,  -0x1.ada2e87de0000p-6,  -0x1.dfffff916d350p-5}, //  -30 * 0x1p-9 = -0x1.e0p-5 (err =  0x1.6a10d0c80584dp-64)
    {  0x1.e2ffffffaeeabp-1,   0x1.0f5edfab5fe88p+0,  -0x1.9ee3a5eea0000p-6,  -0x1.d000000511550p-5}, //  -29 * 0x1p-9 = -0x1.d0p-5 (err =  0x1.02665a5e9f3fap-64)
    {  0x1.e4000000b333ap-1,   0x1.0ecf56be05847p+0,  -0x1.902c31cbe0000p-6,  -0x1.bffffff4ccc60p-5}, //  -28 * 0x1p-9 = -0x1.c0p-5 (err =  0x1.4c45a25a0d467p-66)
    {  0x1.e4ffffffe66bep-1,   0x1.0e4065583441dp+0,  -0x1.817c8469a0000p-6,  -0x1.b000000199420p-5}, //  -27 * 0x1p-9 = -0x1.b0p-5 (err =  0x1.e5ed0c71568e6p-66)
    {  0x1.e60000000252fp-1,   0x1.0db20a88f31f3p+0,  -0x1.72d4956c80000p-6,  -0x1.9fffffffdad10p-5}, //  -26 * 0x1p-9 = -0x1.a0p-5 (err =  0x1.c5ea60d10aca9p-67)
    {  0x1.e6ffffffdc910p-1,   0x1.0d2445636d78cp+0,  -0x1.64345cbf40000p-6,  -0x1.9000000236f00p-5}, //  -25 * 0x1p-9 = -0x1.90p-5 (err =  0x1.a9057be67f589p-66)
    {  0x1.e8000000053a7p-1,   0x1.0c9714fbcac30p+0,  -0x1.559bd24020000p-6,  -0x1.7fffffffac590p-5}, //  -24 * 0x1p-9 = -0x1.80p-5 (err =  0x1.0ef67d2f55647p-67)
    {  0x1.e90000015ea93p-1,   0x1.0c0a7867f3e0ep+0,  -0x1.470aedd580000p-6,  -0x1.6fffffea156d0p-5}, //  -23 * 0x1p-9 = -0x1.70p-5 (err =  0x1.f9da595e4b211p-64)
    {  0x1.e9ffffffb02f6p-1,   0x1.0b7e6ec2856ecp+0,  -0x1.3881a7bca0000p-6,  -0x1.60000004fd0a0p-5}, //  -22 * 0x1p-9 = -0x1.60p-5 (err =  0x1.ace56d7614280p-64)
    {  0x1.eb00000040c25p-1,   0x1.0af2f722cb960p+0,  -0x1.29fff7b520000p-6,  -0x1.4ffffffbf3db0p-5}, //  -21 * 0x1p-9 = -0x1.50p-5 (err =  0x1.e2f7b9f224074p-64)
    {  0x1.ebffffff1ae53p-1,   0x1.0a6810a6fd187p+0,  -0x1.1b85d61140000p-6,  -0x1.4000000e51ad0p-5}, //  -20 * 0x1p-9 = -0x1.40p-5 (err =  0x1.c048e303b90d7p-67)
    {  0x1.ed000007a8391p-1,   0x1.09ddba66d7190p+0,  -0x1.0d133a5140000p-6,  -0x1.2fffff857c6f0p-5}, //  -19 * 0x1p-9 = -0x1.30p-5 (err =  0x1.d7a6726f938b4p-64)
    {  0x1.edffffff02e7cp-1,   0x1.0953f39098854p+0,  -0x1.fd503c5580000p-7,  -0x1.2000000fd1840p-5}, //  -18 * 0x1p-9 = -0x1.20p-5 (err =  0x1.d8587e5f4bda3p-66)
    {  0x1.eefffffffbb46p-1,   0x1.08cabb3758aa6p+0,  -0x1.e088f0b080000p-7,  -0x1.1000000044ba0p-5}, //  -17 * 0x1p-9 = -0x1.10p-5 (err =  0x1.73ec8f0604f7bp-66)
    {  0x1.f0000001a3864p-1,   0x1.084210834184dp+0,  -0x1.c3d0834880000p-7,  -0x1.ffffffcb8f380p-6}, //  -16 * 0x1p-9 = -0x1.00p-5 (err =  0x1.57726af49b585p-64)
    {  0x1.f0ffffff1ae95p-1,   0x1.07b9f29c083e3p+0,  -0x1.a726e55400000p-7,  -0x1.e000001ca2d60p-6}, //  -15 * 0x1p-9 = -0x1.e0p-6 (err =  0x1.f386919eb8327p-66)
    {  0x1.f2000001c0f23p-1,   0x1.073260a39236fp+0,  -0x1.8a8c068900000p-7,  -0x1.bfffffc7e1ba0p-6}, //  -14 * 0x1p-9 = -0x1.c0p-6 (err =  0x1.355f247665d73p-65)
    {  0x1.f2fffffe69ac3p-1,   0x1.06ab59c86712ep+0,  -0x1.6dffd90100000p-7,  -0x1.a0000032ca7a0p-6}, //  -13 * 0x1p-9 = -0x1.a0p-6 (err =  0x1.1b4423b88e8e0p-65)
    {  0x1.f3fffffefb704p-1,   0x1.0624dd2fa33bcp+0,  -0x1.51824c9280000p-7,  -0x1.8000002091f80p-6}, //  -12 * 0x1p-9 = -0x1.80p-6 (err =  0x1.5b466ed8cf8d1p-64)
    {  0x1.f4fffffef9c40p-1,   0x1.059eea07b0489p+0,  -0x1.351352c600000p-7,  -0x1.60000020c7800p-6}, //  -11 * 0x1p-9 = -0x1.60p-6 (err =  0x1.5893fba0e8a3ep-64)
    {  0x1.f6000003495fdp-1,   0x1.05197f7bbda2fp+0,  -0x1.18b2dc3000000p-7,  -0x1.3fffff96d4060p-6}, //  -10 * 0x1p-9 = -0x1.40p-6 (err =  0x1.9df140f79b9e2p-64)
    {  0x1.f70000011e502p-1,   0x1.04949cc0d1f91p+0,  -0x1.f8c1b67180000p-8,  -0x1.1fffffdc35fc0p-6}, //   -9 * 0x1p-9 = -0x1.20p-6 (err =  0x1.e3ec48bbdf8bfp-64)
    {  0x1.f7ffffff7522ap-1,   0x1.0410410457e87p+0,  -0x1.c03a80cd00000p-8,  -0x1.000000115bac0p-6}, //   -8 * 0x1p-9 = -0x1.00p-6 (err =  0x1.42926465cd1e3p-64)
    {  0x1.f8fffffeb282dp-1,   0x1.038c6b78cfe5fp+0,  -0x1.87cffa2280000p-8,  -0x1.c00000535f4c0p-7}, //   -7 * 0x1p-9 = -0x1.c0p-7 (err =  0x1.888d8a93b5612p-65)
    {  0x1.f9ffffffd1a71p-1,   0x1.03091b520d9bap+0,  -0x1.4f82052d80000p-8,  -0x1.8000000b963c0p-7}, //   -6 * 0x1p-9 = -0x1.80p-7 (err =  0x1.440fdc5622a9fp-64)
    {  0x1.fb00000113ffbp-1,   0x1.02864fc6e5e27p+0,  -0x1.1750856f00000p-8,  -0x1.3fffffbb00140p-7}, //   -5 * 0x1p-9 = -0x1.40p-7 (err =  0x1.22956b8d53559p-65)
    {  0x1.fbffffffef56ap-1,   0x1.0204081028b6fp+0,  -0x1.be76bd7f00000p-9,  -0x1.000000042a580p-7}, //   -4 * 0x1p-9 = -0x1.00p-7 (err =  0x1.2655d6e6878fbp-71)
    {  0x1.fd0000009f6b0p-1,   0x1.01824364c6fcbp+0,  -0x1.4e84e74e00000p-9,  -0x1.7fffffb04a800p-8}, //   -3 * 0x1p-9 = -0x1.80p-8 (err =  0x1.d41a098c40802p-65)
    {  0x1.fdfffffda0070p-1,   0x1.0101010233611p+0, -0x1.bd96a3ea00000p-10,  -0x1.0000012ffc800p-8}, //   -2 * 0x1p-9 = -0x1.00p-8 (err =  0x1.c0adc2ebc99f6p-64)
    {  0x1.ff00000336b57p-1,   0x1.0080401e7310cp+0, -0x1.bd26fec400000p-11, -0x1.fffff99295200p-10}, //   -1 * 0x1p-9 = -0x1.00p-9 (err =  0x1.03e81dc11191fp-67)
    {  0x1.0000000000000p+0,   0x1.0000000000000p+0,   0x0.0000000000000p+0,   0x0.0000000000000p+0}, //    0 * 0x1p-9 = 0x0.00p+0 (err =   0x0.0000000000000p+0)
};// Worst deviation:  ap1 =   0x1.3a00000a86abbp+0, log1pa =   0x1.6b45dfaae0000p-4, a =   0x1.d0000054355d8p-3,  err =  0x1.f7c90ae5e4cf8p-64, d = 0x1.50d576p-29

static const logdata_t LOG_LOOKUP[] = { //ap1, va, log1pa, a: va = 1/(a+1), log1pa = log(1+a)+err
    {  0x1.0000000000000p+0,   0x1.0000000000000p+0,   0x0.0000000000000p+0,   0x0.0000000000000p+0}, //    0 * 0x1p-8 = 0x0.00p+0 (err =   0x0.0000000000000p+0)
    {  0x1.00fffffffffdbp+0,   0x1.fe01fe01fe069p-1,   0x1.ff00aa2b0c20ap-9,   0x1.fffffffffb600p-9}, //    1 * 0x1p-8 = 0x1.00p-8 (err =  0x1.48867ff2c4899p-71)
    {  0x1.0200000000001p+0,   0x1.fc07f01fc07eep-1,   0x1.fe02a6b106887p-8,   0x1.0000000000080p-7}, //    2 * 0x1p-8 = 0x1.00p-7 (err =  0x1.7bc71e8aaa2e9p-71)
    {  0x1.0300000000008p+0,   0x1.fa11caa01fa02p-1,   0x1.7dc475f810e6bp-7,   0x1.8000000000400p-7}, //    3 * 0x1p-8 = 0x1.80p-7 (err =  0x1.74a65f3301efbp-68)
    {  0x1.03ffffff503a2p+0,   0x1.f81f81f97451fp-1,   0x1.fc0a8ab937717p-7,   0x1.ffffffa81d100p-7}, //    4 * 0x1p-8 = 0x1.00p-6 (err =  0x1.fc14f990d4bbap-68)
    {  0x1.0500000000080p+0,   0x1.f6310aca0dabfp-1,   0x1.3cea44346c4d8p-6,   0x1.4000000002000p-6}, //    5 * 0x1p-8 = 0x1.40p-6 (err =  0x1.eb9b4f0e9ef5ep-70)
    {  0x1.05ffffff91d2bp+0,   0x1.f44659e57687ap-1,   0x1.7b91b0627138cp-6,   0x1.7fffffe474ac0p-6}, //    6 * 0x1p-8 = 0x1.80p-6 (err =  0x1.7b90c83722efdp-67)
    {  0x1.0700000000036p+0,   0x1.f25f644230a4fp-1,   0x1.b9fc027af9ebcp-6,   0x1.c000000000d80p-6}, //    7 * 0x1p-8 = 0x1.c0p-6 (err =  0x1.1b6e00e299c84p-68)
    {  0x1.07ffffffffffbp+0,   0x1.f07c1f07c1f11p-1,   0x1.f829b0e7831cap-6,   0x1.ffffffffffec0p-6}, //    8 * 0x1p-8 = 0x1.00p-5 (err =  0x1.34d18add8019dp-67)
    {  0x1.090000000007cp+0,   0x1.ee9c7f8458d1ap-1,   0x1.1b0d98923e879p-5,   0x1.2000000000f80p-5}, //    9 * 0x1p-8 = 0x1.20p-5 (err =  0x1.663d8b929b0c4p-67)
    {  0x1.09fffffffffd2p+0,   0x1.ecc07b301ed16p-1,   0x1.39e87b9feb7d7p-5,   0x1.3fffffffffa40p-5}, //   10 * 0x1p-8 = 0x1.40p-5 (err =  0x1.833ce19e13c94p-67)
    {  0x1.0b00000000008p+0,   0x1.eae807aba01dcp-1,   0x1.58a5bafc8e5cap-5,   0x1.6000000000100p-5}, //   11 * 0x1p-8 = 0x1.60p-5 (err =  0x1.b9948b59ba875p-67)
    {  0x1.0c000000dfddep+0,   0x1.e9131abd72ed6p-1,   0x1.77458f7de8c82p-5,   0x1.8000001bfbbc0p-5}, //   12 * 0x1p-8 = 0x1.80p-5 (err =  0x1.77443de6cbd02p-66)
    {  0x1.0d00000000020p+0,   0x1.e741aa59750aap-1,   0x1.95c830ec8e7bap-5,   0x1.a000000000400p-5}, //   13 * 0x1p-8 = 0x1.a0p-5 (err =  0x1.79d9d6c23a68fp-66)
    {  0x1.0dfffffffffc6p+0,   0x1.e573ac901e5dcp-1,   0x1.b42dd71196adfp-5,   0x1.bfffffffff8c0p-5}, //   14 * 0x1p-8 = 0x1.c0p-5 (err =  0x1.88eb4c5c1cf48p-66)
    {  0x1.0efffffffffd4p+0,   0x1.e3a9179dc1ac2p-1,   0x1.d276b8adb0620p-5,   0x1.dfffffffffa80p-5}, //   15 * 0x1p-8 = 0x1.e0p-5 (err =  0x1.c5a6bb8add924p-66)
    {  0x1.10000002a3559p+0,   0x1.e1e1e1dd35714p-1,   0x1.f0a30c5089a4cp-5,   0x1.0000002a35590p-4}, //   16 * 0x1p-8 = 0x1.00p-4 (err =  0x1.f0a160c7d4670p-66)
    {  0x1.10fffffffffcdp+0,   0x1.e01e01e01e078p-1,   0x1.075983598e174p-4,   0x1.0fffffffffcd0p-4}, //   17 * 0x1p-8 = 0x1.10p-4 (err =  0x1.2134da845ab2ap-66)
    {  0x1.11ffffff72e8dp+0,   0x1.de5d6e407ebbap-1,   0x1.16536ee1fa87cp-4,   0x1.1ffffff72e8d0p-4}, //   18 * 0x1p-8 = 0x1.20p-4 (err =  0x1.165460c092331p-65)
    {  0x1.130000000007bp+0,   0x1.dca01dca01cf5p-1,   0x1.253f62f0a1b3fp-4,   0x1.30000000007b0p-4}, //   19 * 0x1p-8 = 0x1.30p-4 (err =  0x1.b7a8783d63776p-66)
    {  0x1.14000001bd6a4p+0,   0x1.dae6076899b6cp-1,   0x1.341d797b8f54bp-4,   0x1.4000001bd6a40p-4}, //   20 * 0x1p-8 = 0x1.40p-4 (err =  0x1.341ee89cdf022p-65)
    {  0x1.14ffffffffff3p+0,   0x1.d92f2231e7fa0p-1,   0x1.42edcbea64630p-4,   0x1.4ffffffffff30p-4}, //   21 * 0x1p-8 = 0x1.50p-4 (err =  0x1.2975c0ebcca52p-65)
    {  0x1.15fffffffffebp+0,   0x1.d77b654b82c57p-1,   0x1.51b073f06170ap-4,   0x1.5fffffffffeb0p-4}, //   22 * 0x1p-8 = 0x1.60p-4 (err =  0x1.68a6d4cab8f2bp-67)
    {  0x1.16fffffffffa1p+0,   0x1.d5cac80757352p-1,   0x1.60658a9374b51p-4,   0x1.6fffffffffa10p-4}, //   23 * 0x1p-8 = 0x1.70p-4 (err =  0x1.17602efa61478p-67)
    {  0x1.18000001f11eap+0,   0x1.d41d41d0de278p-1,   0x1.6f0d28cabedb2p-4,   0x1.8000001f11ea0p-4}, //   24 * 0x1p-8 = 0x1.80p-4 (err =  0x1.6f0d7b0b5f2ecp-65)
    {  0x1.1900000000027p+0,   0x1.d272ca3fc5adap-1,   0x1.7da766d7b1505p-4,   0x1.9000000000270p-4}, //   25 * 0x1p-8 = 0x1.90p-4 (err =  0x1.57f40c4839d72p-67)
    {  0x1.1a0000000002ap+0,   0x1.d0cb58f6ec02fp-1,   0x1.8c345d6319d83p-4,   0x1.a0000000002a0p-4}, //   26 * 0x1p-8 = 0x1.a0p-4 (err =  0x1.22f080593df5cp-65)
    {  0x1.1b00000000059p+0,   0x1.cf26e5c44bf34p-1,   0x1.9ab42462038b5p-4,   0x1.b000000000590p-4}, //   27 * 0x1p-8 = 0x1.b0p-4 (err =  0x1.bcdbb18a9c55bp-68)
    {  0x1.1c00000000012p+0,   0x1.cd85689039aeep-1,   0x1.a926d3a4ad667p-4,   0x1.c000000000120p-4}, //   28 * 0x1p-8 = 0x1.c0p-4 (err =  0x1.6a34b0f185d61p-68)
    {  0x1.1d00000000033p+0,   0x1.cbe6d9601cb95p-1,   0x1.b78c82bb0f07ep-4,   0x1.d000000000330p-4}, //   29 * 0x1p-8 = 0x1.d0p-4 (err =  0x1.71bd2f2523501p-67)
    {  0x1.1dfffffffffcep+0,   0x1.ca4b3055ee1e1p-1,   0x1.c5e548f5bc477p-4,   0x1.dfffffffffce0p-4}, //   30 * 0x1p-8 = 0x1.e0p-4 (err =  0x1.6a46add93c028p-66)
    {  0x1.1f00000000006p+0,   0x1.c8b265afb8a38p-1,   0x1.d4313d66cb3b3p-4,   0x1.f000000000060p-4}, //   31 * 0x1p-8 = 0x1.f0p-4 (err =  0x1.25e2e3ac11919p-66)
    {  0x1.1ffffffc4bee8p+0,   0x1.c71c71ccf6a06p-1,   0x1.e27076ae038adp-4,   0x1.ffffffc4bee80p-4}, //   32 * 0x1p-8 = 0x1.00p-3 (err =  0x1.e270b2e028600p-65)
    {  0x1.20ffffffffff8p+0,   0x1.c5894d10d4992p-1,   0x1.f0a30c0116235p-4,   0x1.07fffffffffc0p-3}, //   33 * 0x1p-8 = 0x1.08p-3 (err =  0x1.acf5876e89e20p-65)
    {  0x1.220000000002bp+0,   0x1.c3f8f01c3f8adp-1,   0x1.fec9131dbed1ap-4,   0x1.1000000000158p-3}, //   34 * 0x1p-8 = 0x1.10p-3 (err =  0x1.61ef1ebd187fdp-65)
    {  0x1.230000000004bp+0,   0x1.c26b5392e9fa8p-1,   0x1.0671512ca5b7ep-3,   0x1.1800000000258p-3}, //   35 * 0x1p-8 = 0x1.18p-3 (err =  0x1.0a36c2cb3a546p-66)
    {  0x1.23fffffe9e61ap+0,   0x1.c0e0703a3ba7ap-1,   0x1.0d77e7c358b8ep-3,   0x1.1ffffff4f30d0p-3}, //   36 * 0x1p-8 = 0x1.20p-3 (err =  0x1.0d77566a4bea5p-64)
    {  0x1.2500000000028p+0,   0x1.bf583ee868d4ep-1,   0x1.14785846743c4p-3,   0x1.2800000000140p-3}, //   37 * 0x1p-8 = 0x1.28p-3 (err =  0x1.b54e2c7e40373p-66)
    {  0x1.25fffffe84fcfp+0,   0x1.bdd2b89b7f2bdp-1,   0x1.1b72ad48a648dp-3,   0x1.2ffffff427e78p-3}, //   38 * 0x1p-8 = 0x1.30p-3 (err =  0x1.1b71b8cabe4aap-64)
    {  0x1.26fffffffff8fp+0,   0x1.bc4fd65883f25p-1,   0x1.2266f190a57bbp-3,   0x1.37ffffffffc78p-3}, //   39 * 0x1p-8 = 0x1.38p-3 (err =  0x1.6dedf44ff3e72p-65)
    {  0x1.2800000000010p+0,   0x1.bacf914c1bab8p-1,   0x1.29552f81ff592p-3,   0x1.4000000000080p-3}, //   40 * 0x1p-8 = 0x1.40p-3 (err =  0x1.5d08812fa9f7ep-67)
    {  0x1.2900000000043p+0,   0x1.b951e2b18fec0p-1,   0x1.303d718e481a1p-3,   0x1.4800000000218p-3}, //   41 * 0x1p-8 = 0x1.48p-3 (err =  0x1.36d5946114216p-65)
    {  0x1.29ffffffffffap+0,   0x1.b7d6c3dda3394p-1,   0x1.371fc201e8f4bp-3,   0x1.4ffffffffffd0p-3}, //   42 * 0x1p-8 = 0x1.50p-3 (err =  0x1.531807bc9894ap-65)
    {  0x1.2b00000000055p+0,   0x1.b65e2e3beed89p-1,   0x1.3dfc2b0ecc870p-3,   0x1.58000000002a8p-3}, //   43 * 0x1p-8 = 0x1.58p-3 (err =  0x1.47a8d9cd7c694p-65)
    {  0x1.2bfffffde9a87p+0,   0x1.b4e81b518be61p-1,   0x1.44d2b6be780b8p-3,   0x1.5fffffef4d438p-3}, //   44 * 0x1p-8 = 0x1.60p-3 (err =  0x1.44d2a83b94ee4p-64)
    {  0x1.2d0000000000cp+0,   0x1.b37484ad806bcp-1,   0x1.4ba36f39a5637p-3,   0x1.6800000000060p-3}, //   45 * 0x1p-8 = 0x1.68p-3 (err =  0x1.1884de133c2ebp-71)
    {  0x1.2dfffffffffccp+0,   0x1.b2036406c8124p-1,   0x1.526e5e3a1b2d7p-3,   0x1.6fffffffffe60p-3}, //   46 * 0x1p-8 = 0x1.70p-3 (err =  0x1.15e2a2b7021d1p-66)
    {  0x1.2efffffffff6ep+0,   0x1.b094b31d92374p-1,   0x1.59338d9981cabp-3,   0x1.77ffffffffb70p-3}, //   47 * 0x1p-8 = 0x1.78p-3 (err =  0x1.a3e6ad62b84d6p-68)
    {  0x1.30000007ecba4p+0,   0x1.af286bbedda37p-1,   0x1.5ff3073fdc74dp-3,   0x1.8000003f65d20p-3}, //   48 * 0x1p-8 = 0x1.80p-3 (err =  0x1.5feed4cbb8237p-64)
    {  0x1.30fffffffffe7p+0,   0x1.adbe87f949081p-1,   0x1.66acd4272aca9p-3,   0x1.87fffffffff38p-3}, //   49 * 0x1p-8 = 0x1.88p-3 (err =  0x1.cb180cefd3f34p-65)
    {  0x1.320000009ebf5p+0,   0x1.ac5701ab78caap-1,   0x1.6d60fe75c3990p-3,   0x1.90000004f5fa8p-3}, //   50 * 0x1p-8 = 0x1.90p-3 (err =  0x1.6d609cd52209dp-64)
    {  0x1.32fffffffffe4p+0,   0x1.aaf1d2f87ec24p-1,   0x1.740f8f54036eap-3,   0x1.97fffffffff20p-3}, //   51 * 0x1p-8 = 0x1.98p-3 (err =  0x1.8cd7cc50aa728p-67)
    {  0x1.33fffffe36a04p+0,   0x1.a98ef6091e2e0p-1,   0x1.7ab890152c531p-3,   0x1.9ffffff1b5020p-3}, //   52 * 0x1p-8 = 0x1.a0p-3 (err =  0x1.7ab4ad5ba25fdp-64)
    {  0x1.350000000000dp+0,   0x1.a82e65130e147p-1,   0x1.815c0a1435841p-3,   0x1.a800000000068p-3}, //   53 * 0x1p-8 = 0x1.a8p-3 (err =  0x1.0082f97c7acb7p-67)
    {  0x1.3600000000025p+0,   0x1.a6d01a6d01a3bp-1,   0x1.87fa06520ca05p-3,   0x1.b000000000128p-3}, //   54 * 0x1p-8 = 0x1.b0p-3 (err =  0x1.bd4019c54201ep-65)
    {  0x1.370000000002dp+0,   0x1.a574107688a0dp-1,   0x1.8e928de886e69p-3,   0x1.b800000000168p-3}, //   55 * 0x1p-8 = 0x1.b8p-3 (err =  0x1.79bdd0666aa2dp-68)
    {  0x1.37ffffffffff1p+0,   0x1.a41a41a41a42ep-1,   0x1.9525a9cf45652p-3,   0x1.bffffffffff88p-3}, //   56 * 0x1p-8 = 0x1.c0p-3 (err =  0x1.9cdff113041a2p-67)
    {  0x1.390000000001bp+0,   0x1.a2c2a87c51c7cp-1,   0x1.9bb362e7dfc34p-3,   0x1.c8000000000d8p-3}, //   57 * 0x1p-8 = 0x1.c8p-3 (err =  0x1.2be22fc8f4f92p-68)
    {  0x1.3a0000000003fp+0,   0x1.a16d3f97a4aaep-1,   0x1.a23bc1fe2b6fep-3,   0x1.d0000000001f8p-3}, //   58 * 0x1p-8 = 0x1.d0p-3 (err =  0x1.8355ed2abf605p-64)
    {  0x1.3b00000000096p+0,   0x1.a01a01a019f54p-1,   0x1.a8becfc8832e8p-3,   0x1.d8000000004b0p-3}, //   59 * 0x1p-8 = 0x1.d8p-3 (err =  0x1.24ff31993e51cp-66)
    {  0x1.3bffffffffff8p+0,   0x1.9ec8e951033e4p-1,   0x1.af3c94e80bfbfp-3,   0x1.dffffffffffc0p-3}, //   60 * 0x1p-8 = 0x1.e0p-3 (err =  0x1.3b281cbfbf131p-65)
    {  0x1.3d00000000031p+0,   0x1.9d79f176b67edp-1,   0x1.b5b519e8fb6e1p-3,   0x1.e800000000188p-3}, //   61 * 0x1p-8 = 0x1.e8p-3 (err =  0x1.921abddd6f78fp-67)
    {  0x1.3dffffffffff2p+0,   0x1.9c2d14ee4a114p-1,   0x1.bc286742d8c7cp-3,   0x1.effffffffff90p-3}, //   62 * 0x1p-8 = 0x1.f0p-3 (err =  0x1.d3a68da7945cep-67)
    {  0x1.3f00000000008p+0,   0x1.9ae24ea5510d0p-1,   0x1.c2968558c18f4p-3,   0x1.f800000000040p-3}, //   63 * 0x1p-8 = 0x1.f8p-3 (err =  0x1.5bc870dc86d2ep-64)
    {  0x1.4000000a81296p+0,   0x1.9999998c27792p-1,   0x1.c8ff7cbce4448p-3,   0x1.0000002a04a58p-2}, //   64 * 0x1p-8 = 0x1.00p-2 (err =  0x1.c9002c4a93bcep-64)
    {  0x1.40fffffffffc8p+0,   0x1.9852f0d8ec146p-1,   0x1.cf6354e09c477p-3,   0x1.03fffffffff20p-2}, //   65 * 0x1p-8 = 0x1.04p-2 (err =  0x1.4f109b1bcb71ap-65)
    {  0x1.41fffffffffffp+0,   0x1.970e4f80cb874p-1,   0x1.d5c216b4fbb8bp-3,   0x1.07ffffffffffcp-2}, //   66 * 0x1p-8 = 0x1.08p-2 (err =  0x1.50613a69cf8e1p-64)
    {  0x1.4300000000028p+0,   0x1.95cbb0be3777bp-1,   0x1.dc1bca0abed7bp-3,   0x1.0c000000000a0p-2}, //   67 * 0x1p-8 = 0x1.0cp-2 (err =  0x1.0c5a48754333ap-66)
    {  0x1.43fffffffffdep+0,   0x1.948b0fcd6ea0bp-1,   0x1.e27076e2af20fp-3,   0x1.0ffffffffff78p-2}, //   68 * 0x1p-8 = 0x1.10p-2 (err =  0x1.445ba3aadad2ap-68)
    {  0x1.450000000003cp+0,   0x1.934c67f9b2c9cp-1,   0x1.e8c0252aa5bdap-3,   0x1.14000000000f0p-2}, //   69 * 0x1p-8 = 0x1.14p-2 (err =  0x1.827fbbbf0626ap-64)
    {  0x1.45fffffffffb8p+0,   0x1.920fb49d0e282p-1,   0x1.ef0adcbdc5772p-3,   0x1.17ffffffffee0p-2}, //   70 * 0x1p-8 = 0x1.18p-2 (err =  0x1.b6b6913427f38p-65)
    {  0x1.46fffffffff77p+0,   0x1.90d4f1201917dp-1,   0x1.f550a564b77ddp-3,   0x1.1bffffffffddcp-2}, //   71 * 0x1p-8 = 0x1.1cp-2 (err =  0x1.789a6cd6ac1a9p-64)
    {  0x1.47fffffb9458fp+0,   0x1.8f9c18ff24476p-1,   0x1.fb9186ba49f50p-3,   0x1.1fffffee5163cp-2}, //   72 * 0x1p-8 = 0x1.20p-2 (err =  0x1.fb924dace4bb4p-64)
    {  0x1.4900000000002p+0,   0x1.8e6527af1373dp-1,   0x1.00e6c45ad5023p-2,   0x1.2400000000008p-2}, //   73 * 0x1p-8 = 0x1.24p-2 (err =  0x1.0f8ce2fe90d36p-65)
    {  0x1.49fffffffffeep+0,   0x1.8d3018d3018e9p-1,   0x1.0402594b4d009p-2,   0x1.27fffffffffb8p-2}, //   74 * 0x1p-8 = 0x1.28p-2 (err =  0x1.efe247e088286p-66)
    {  0x1.4afffffffff94p+0,   0x1.8bfce8062ffbbp-1,   0x1.071b85fcd57bfp-2,   0x1.2bffffffffe50p-2}, //   75 * 0x1p-8 = 0x1.2cp-2 (err =  0x1.32f4fbf462d8ap-64)
    {  0x1.4bfffffffffd8p+0,   0x1.8acb90f6bf3d9p-1,   0x1.0a324e2739068p-2,   0x1.2ffffffffff60p-2}, //   76 * 0x1p-8 = 0x1.30p-2 (err =  0x1.4aabdd52b8795p-65)
    {  0x1.4cffffffffff1p+0,   0x1.899c0f60189aep-1,   0x1.0d46b579ab71dp-2,   0x1.33fffffffffc4p-2}, //   77 * 0x1p-8 = 0x1.34p-2 (err =  0x1.9e17acbb2f454p-65)
    {  0x1.4dfffffffffd1p+0,   0x1.886e5f0abb081p-1,   0x1.1058bf9ae4a45p-2,   0x1.37fffffffff44p-2}, //   78 * 0x1p-8 = 0x1.38p-2 (err =  0x1.8babaa0609c2fp-66)
    {  0x1.4f00000000040p+0,   0x1.87427bcc0926ep-1,   0x1.136870293a974p-2,   0x1.3c00000000100p-2}, //   79 * 0x1p-8 = 0x1.3cp-2 (err =  0x1.770684be59d27p-66)
    {  0x1.5000000680c34p+0,   0x1.8618617e8b986p-1,   0x1.1675cace8bf0ep-2,   0x1.4000001a030d0p-2}, //   80 * 0x1p-8 = 0x1.40p-2 (err =  0x1.1673f932534d0p-63)
    {  0x1.50fffffffff5ap+0,   0x1.84f00c27806d3p-1,   0x1.1980d2dd42177p-2,   0x1.43ffffffffd68p-2}, //   81 * 0x1p-8 = 0x1.44p-2 (err =  0x1.f1dbca619d9c1p-72)
    {  0x1.51fffffffffbdp+0,   0x1.83c977ab2bf2ap-1,   0x1.1c898c1699930p-2,   0x1.47ffffffffef4p-2}, //   82 * 0x1p-8 = 0x1.48p-2 (err =  0x1.47ab7101eb535p-64)
    {  0x1.5300000000046p+0,   0x1.82a4a0182a450p-1,   0x1.1f8ff9e48a3c6p-2,   0x1.4c00000000118p-2}, //   83 * 0x1p-8 = 0x1.4cp-2 (err =  0x1.dfc4c44842456p-64)
    {  0x1.540000000001fp+0,   0x1.81818181817f5p-1,   0x1.22941fbcf79c3p-2,   0x1.500000000007cp-2}, //   84 * 0x1p-8 = 0x1.50p-2 (err =  0x1.807593ee25e59p-64)
    {  0x1.5500000000016p+0,   0x1.80601806017edp-1,   0x1.2596010df767cp-2,   0x1.5400000000058p-2}, //   85 * 0x1p-8 = 0x1.54p-2 (err =  0x1.cd2d3b15c0caep-64)
    {  0x1.560000000003fp+0,   0x1.7f405fd017efap-1,   0x1.2895a13de8760p-2,   0x1.58000000000fcp-2}, //   86 * 0x1p-8 = 0x1.58p-2 (err =  0x1.8eedc31454c0bp-64)
    {  0x1.56fffffffffa1p+0,   0x1.7e225515a4f87p-1,   0x1.2b9303ab89c09p-2,   0x1.5bffffffffe84p-2}, //   87 * 0x1p-8 = 0x1.5cp-2 (err =  0x1.500fcbeb34df7p-65)
    {  0x1.57fffffc9d61dp+0,   0x1.7d05f41b9042bp-1,   0x1.2e8e2ba3fe1fep-2,   0x1.5ffffff275874p-2}, //   88 * 0x1p-8 = 0x1.60p-2 (err =  0x1.2e8dd451d8d09p-63)
    {  0x1.58fffffffffe0p+0,   0x1.7beb3922e019fp-1,   0x1.31871c9544126p-2,   0x1.63fffffffff80p-2}, //   89 * 0x1p-8 = 0x1.64p-2 (err =  0x1.4fbcb08a30581p-66)
    {  0x1.59ffffffffffcp+0,   0x1.7ad2208e0ecc8p-1,   0x1.347dd9a987d49p-2,   0x1.67ffffffffff0p-2}, //   90 * 0x1p-8 = 0x1.68p-2 (err =  0x1.2e7482225ac5fp-64)
    {  0x1.5b00000000065p+0,   0x1.79baa6bb6391dp-1,   0x1.3772662bfd985p-2,   0x1.6c00000000194p-2}, //   91 * 0x1p-8 = 0x1.6cp-2 (err =  0x1.8e143dd3d26aep-64)
    {  0x1.5c000004954f4p+0,   0x1.78a4c812946a3p-1,   0x1.3a64c56410ddep-2,   0x1.70000012553d0p-2}, //   92 * 0x1p-8 = 0x1.70p-2 (err =  0x1.3a6440a7dc469p-63)
    {  0x1.5d00000000008p+0,   0x1.77908119ac605p-1,   0x1.3d54fa5c1f727p-2,   0x1.7400000000020p-2}, //   93 * 0x1p-8 = 0x1.74p-2 (err =  0x1.f3d401f71595ap-65)
    {  0x1.5dfffffffffefp+0,   0x1.767dce434a9c3p-1,   0x1.404308686a7b2p-2,   0x1.77fffffffffbcp-2}, //   94 * 0x1p-8 = 0x1.78p-2 (err =  0x1.5ab638306b4edp-64)
    {  0x1.5f00000000039p+0,   0x1.756cac2017530p-1,   0x1.432ef2a04e8bap-2,   0x1.7c000000000e4p-2}, //   95 * 0x1p-8 = 0x1.7cp-2 (err =  0x1.f7b0755cbfb3bp-74)
    {  0x1.6000000b07c96p+0,   0x1.745d173a264efp-1,   0x1.4618bc41dc931p-2,   0x1.8000002c1f258p-2}, //   96 * 0x1p-8 = 0x1.80p-2 (err =  0x1.4618f6ec2219ap-63)
    {  0x1.60fffffffff8fp+0,   0x1.734f0c541ff04p-1,   0x1.4900680400889p-2,   0x1.83ffffffffe3cp-2}, //   97 * 0x1p-8 = 0x1.84p-2 (err =  0x1.bcb9bf722c81bp-65)
    {  0x1.6200000000008p+0,   0x1.724287f46deb4p-1,   0x1.4be5f957778b8p-2,   0x1.8800000000020p-2}, //   98 * 0x1p-8 = 0x1.88p-2 (err =  0x1.15cede549545fp-63)
    {  0x1.62fffffffffd2p+0,   0x1.713786d9c7c38p-1,   0x1.4ec97326001e5p-2,   0x1.8bfffffffff48p-2}, //   99 * 0x1p-8 = 0x1.8cp-2 (err =  0x1.3d2b5352598fap-63)
    {  0x1.64000002c6202p+0,   0x1.702e05bdd9ab4p-1,   0x1.51aad87ada1e8p-2,   0x1.9000000b18808p-2}, //  100 * 0x1p-8 = 0x1.90p-2 (err =  0x1.51ab1c70ba1cfp-63)
    {  0x1.64fffffffffd1p+0,   0x1.6f26016f26047p-1,   0x1.548a2c3add1dcp-2,   0x1.93fffffffff44p-2}, //  101 * 0x1p-8 = 0x1.94p-2 (err =  0x1.412e62b1841b8p-65)
    {  0x1.660000000001fp+0,   0x1.6e1f76b4337a7p-1,   0x1.5767717455ac5p-2,   0x1.980000000007cp-2}, //  102 * 0x1p-8 = 0x1.98p-2 (err =  0x1.cbb3742be8aa9p-65)
    {  0x1.6700000000026p+0,   0x1.6d1a62681c83ap-1,   0x1.5a42ab0f4d04ep-2,   0x1.9c00000000098p-2}, //  103 * 0x1p-8 = 0x1.9cp-2 (err =  0x1.304785e83f67ap-65)
    {  0x1.67fffffe1b3d6p+0,   0x1.6c16c16e01059p-1,   0x1.5d1bdbf01dbd0p-2,   0x1.9ffffff86cf58p-2}, //  104 * 0x1p-8 = 0x1.a0p-2 (err =  0x1.5d1bcddbc3dd6p-63)
    {  0x1.68fffffffffa9p+0,   0x1.6b1490aa31a94p-1,   0x1.5ff3070a792ddp-2,   0x1.a3ffffffffea4p-2}, //  105 * 0x1p-8 = 0x1.a4p-2 (err =  0x1.e639711fb881ap-64)
    {  0x1.6a0000000004dp+0,   0x1.6a13cd1537243p-1,   0x1.62c82f2b9c86fp-2,   0x1.a800000000134p-2}, //  106 * 0x1p-8 = 0x1.a8p-2 (err =  0x1.4e84345988995p-63)
    {  0x1.6b0000000000ep+0,   0x1.691473a88d0b2p-1,   0x1.659b57303e21ap-2,   0x1.ac00000000038p-2}, //  107 * 0x1p-8 = 0x1.acp-2 (err =  0x1.7b1685cfbbd86p-66)
    {  0x1.6bfffffffffe4p+0,   0x1.6816816816832p-1,   0x1.686c81e9b1460p-2,   0x1.affffffffff90p-2}, //  108 * 0x1p-8 = 0x1.b0p-2 (err =  0x1.532169af376f1p-63)
    {  0x1.6d00000000055p+0,   0x1.6719f360166c6p-1,   0x1.6b3bb2235952cp-2,   0x1.b400000000154p-2}, //  109 * 0x1p-8 = 0x1.b4p-2 (err =  0x1.348536ff31544p-63)
    {  0x1.6e00000000043p+0,   0x1.661ec6a5122b7p-1,   0x1.6e08eaa2ba29fp-2,   0x1.b80000000010cp-2}, //  110 * 0x1p-8 = 0x1.b8p-2 (err =  0x1.7c6c02d75547ep-65)
    {  0x1.6f00000000055p+0,   0x1.6524f853b4a51p-1,   0x1.70d42e2789323p-2,   0x1.bc00000000154p-2}, //  111 * 0x1p-8 = 0x1.bcp-2 (err =  0x1.9abb9241a12b5p-65)
    {  0x1.6ffffffda8307p+0,   0x1.642c8592f69f4p-1,   0x1.739d7f6537f68p-2,   0x1.bffffff6a0c1cp-2}, //  112 * 0x1p-8 = 0x1.c0p-2 (err =  0x1.739d60509db2ap-63)
    {  0x1.70fffffffffcep+0,   0x1.63356b88ac10ep-1,   0x1.7664e1239db44p-2,   0x1.c3fffffffff38p-2}, //  113 * 0x1p-8 = 0x1.c4p-2 (err =  0x1.1d0f4b79207cfp-64)
    {  0x1.720000000000fp+0,   0x1.623fa77016231p-1,   0x1.792a55fdd47ccp-2,   0x1.c80000000003cp-2}, //  114 * 0x1p-8 = 0x1.c8p-2 (err =  0x1.d21a55b56b0b2p-64)
    {  0x1.72fffffffff60p+0,   0x1.614b36831af2cp-1,   0x1.7bede0a37ae06p-2,   0x1.cbffffffffd80p-2}, //  115 * 0x1p-8 = 0x1.ccp-2 (err =  0x1.ae93856199bf9p-66)
    {  0x1.74000003ce5a7p+0,   0x1.60581601e681ap-1,   0x1.7eaf83c2a5137p-2,   0x1.d000000f3969cp-2}, //  116 * 0x1p-8 = 0x1.d0p-2 (err =  0x1.7eaf41ae2e020p-63)
    {  0x1.74ffffffffff7p+0,   0x1.5f66434292e04p-1,   0x1.816f41da0d47dp-2,   0x1.d3fffffffffdcp-2}, //  117 * 0x1p-8 = 0x1.d4p-2 (err =  0x1.0ca5a0fcfed91p-63)
    {  0x1.75fffffffffeep+0,   0x1.5e75bb8d015f8p-1,   0x1.842d1da1e8ae6p-2,   0x1.d7fffffffffb8p-2}, //  118 * 0x1p-8 = 0x1.d8p-2 (err =  0x1.5969235f7ca32p-63)
    {  0x1.76ffffffffff5p+0,   0x1.5d867c3ece2afp-1,   0x1.86e919a330b82p-2,   0x1.dbfffffffffd4p-2}, //  119 * 0x1p-8 = 0x1.dcp-2 (err =  0x1.b8ad48f852dc5p-64)
    {  0x1.780000046f1b6p+0,   0x1.5c9882b514a52p-1,   0x1.89a3387827809p-2,   0x1.e0000011bc6d8p-2}, //  120 * 0x1p-8 = 0x1.e0p-2 (err =  0x1.89a242e80d44ap-63)
    {  0x1.7900000000017p+0,   0x1.5babcc647fa7cp-1,   0x1.8c5b7c858b4c9p-2,   0x1.e40000000005cp-2}, //  121 * 0x1p-8 = 0x1.e4p-2 (err =  0x1.67cc2e1059eecp-63)
    {  0x1.7a0000000000cp+0,   0x1.5ac056b015ab5p-1,   0x1.8f11e873662e8p-2,   0x1.e800000000030p-2}, //  122 * 0x1p-8 = 0x1.e8p-2 (err =  0x1.f8a57719ecae6p-66)
    {  0x1.7affffffffff1p+0,   0x1.59d61f123ccb8p-1,   0x1.91c67eb45a815p-2,   0x1.ebfffffffffc4p-2}, //  123 * 0x1p-8 = 0x1.ecp-2 (err =  0x1.6060a81af7809p-63)
    {  0x1.7bffffffffff2p+0,   0x1.58ed2308158fap-1,   0x1.947941c2116d5p-2,   0x1.effffffffffc8p-2}, //  124 * 0x1p-8 = 0x1.f0p-2 (err =  0x1.7420c885994b5p-64)
    {  0x1.7d0000000008bp+0,   0x1.58056015804e3p-1,   0x1.972a3411352cep-2,   0x1.f40000000022cp-2}, //  125 * 0x1p-8 = 0x1.f4p-2 (err =  0x1.7302baf233028p-63)
    {  0x1.7dfffffffffc7p+0,   0x1.571ed3c506b6dp-1,   0x1.99d958117dff2p-2,   0x1.f7fffffffff1cp-2}, //  126 * 0x1p-8 = 0x1.f8p-2 (err =  0x1.11ed0b52decacp-66)
    {  0x1.7f00000000008p+0,   0x1.56397ba7c52dbp-1,   0x1.9c86b02dc0878p-2,   0x1.fc00000000020p-2}, //  127 * 0x1p-8 = 0x1.fcp-2 (err =  0x1.9000bc48e4549p-63)
    {  0x1.810000000002cp-1,   0x1.54725e6bb82d7p+0,  -0x1.23ec5991eb9d4p-2,  -0x1.fbfffffffff50p-3}, // -127 * 0x1p-9 = -0x1.fcp-3 (err =  0x1.8a7702622fb02p-64)
    {  0x1.820000019d22ep-1,   0x1.5390948dd58efp+0,  -0x1.214456cca3904p-2,  -0x1.f7fffff98b748p-3}, // -126 * 0x1p-9 = -0x1.f8p-3 (err =  0x1.2142b0418dc13p-63)
    {  0x1.82fffffffffe0p-1,   0x1.52aff56a80567p+0,  -0x1.1e9e167889a49p-2,  -0x1.f400000000080p-3}, // -125 * 0x1p-9 = -0x1.f4p-3 (err =  0x1.21a1c9f770efdp-64)
    {  0x1.8400000000011p-1,   0x1.51d07eae2f806p+0,  -0x1.1bf99635a6b68p-2,  -0x1.effffffffffbcp-3}, // -124 * 0x1p-9 = -0x1.f0p-3 (err =  0x1.aa8aa6105a9a2p-68)
    {  0x1.8500000000043p-1,   0x1.50f22e111c48bp+0,  -0x1.1956d3b9bc24ap-2,  -0x1.ebffffffffef4p-3}, // -123 * 0x1p-9 = -0x1.ecp-3 (err =  0x1.19ea039c17f89p-65)
    {  0x1.85fffffffffd7p-1,   0x1.5015015015038p+0,  -0x1.16b5ccbacfbdfp-2,  -0x1.e8000000000a4p-3}, // -122 * 0x1p-9 = -0x1.e8p-3 (err =  0x1.e581262bac141p-64)
    {  0x1.86fffffffffc1p-1,   0x1.4f38f62dd4cd1p+0,  -0x1.14167ef367828p-2,  -0x1.e4000000000fcp-3}, // -121 * 0x1p-9 = -0x1.e4p-3 (err =  0x1.64206869adaa7p-66)
    {  0x1.87fffffa00a4fp-1,   0x1.4e5e0a780df38p+0,  -0x1.1178e83229016p-2,  -0x1.e0000017fd6c4p-3}, // -120 * 0x1p-9 = -0x1.e0p-3 (err =  0x1.1179163e1667dp-63)
    {  0x1.88fffffffffddp-1,   0x1.4d843bedc2c69p+0,  -0x1.0edd060b780dcp-2,  -0x1.dc0000000008cp-3}, // -119 * 0x1p-9 = -0x1.dcp-3 (err =  0x1.7186d87254117p-65)
    {  0x1.89fffffffffbfp-1,   0x1.4cab88725afa5p+0,  -0x1.0c42d6761638cp-2,  -0x1.d800000000104p-3}, // -118 * 0x1p-9 = -0x1.d8p-3 (err =  0x1.f846be3d4967ap-64)
    {  0x1.8afffffffffd1p-1,   0x1.4bd3edda69008p+0,  -0x1.09aa572e6c74ep-2,  -0x1.d4000000000bcp-3}, // -117 * 0x1p-9 = -0x1.d4p-3 (err =  0x1.26cc0d795f730p-64)
    {  0x1.8c000002db85cp-1,   0x1.4afd6a02c8876p+0,  -0x1.071385fd71e91p-2,  -0x1.cffffff491e90p-3}, // -116 * 0x1p-9 = -0x1.d0p-3 (err =  0x1.0711b8968fe35p-63)
    {  0x1.8d00000000094p-1,   0x1.4a27fad7600cfp+0,  -0x1.047e60cde823ap-2,  -0x1.cbffffffffdb0p-3}, // -115 * 0x1p-9 = -0x1.ccp-3 (err =  0x1.861d72bdc537dp-66)
    {  0x1.8e0000000004bp-1,   0x1.49539e3b2d029p+0,  -0x1.01eae5626c5d0p-2,  -0x1.c7ffffffffed4p-3}, // -114 * 0x1p-9 = -0x1.c8p-3 (err =  0x1.0003d42b128c6p-64)
    {  0x1.8efffffffffd3p-1,   0x1.48805220148a5p+0,  -0x1.feb2233ea08b4p-3,  -0x1.c4000000000b4p-3}, // -113 * 0x1p-9 = -0x1.c4p-3 (err =  0x1.c4facd4d1c49fp-65)
    {  0x1.90000007fcebcp-1,   0x1.47ae14745614bp+0,  -0x1.f991c6a25538ep-3,  -0x1.bfffffe00c510p-3}, // -112 * 0x1p-9 = -0x1.c0p-3 (err =  0x1.f990f59df1fadp-64)
    {  0x1.9100000000008p-1,   0x1.46dce34596060p+0,  -0x1.f474b134df200p-3,  -0x1.bbfffffffffe0p-3}, // -111 * 0x1p-9 = -0x1.bcp-3 (err =  0x1.2ab0e0df363efp-64)
    {  0x1.91fffffffffaap-1,   0x1.460cbc7f5cfe0p+0,  -0x1.ef5ade4dd019cp-3,  -0x1.b800000000158p-3}, // -110 * 0x1p-9 = -0x1.b8p-3 (err =  0x1.f95a59137bbbdp-69)
    {  0x1.92fffffffffe2p-1,   0x1.453d9e2c776e2p+0,  -0x1.ea4449f04ab8dp-3,  -0x1.b400000000078p-3}, // -109 * 0x1p-9 = -0x1.b4p-3 (err =  0x1.3de6d1644047ep-67)
    {  0x1.9400000000001p-1,   0x1.446f86562d9fap+0,  -0x1.e530effe7100dp-3,  -0x1.afffffffffffcp-3}, // -108 * 0x1p-9 = -0x1.b0p-3 (err =  0x1.a51ba4f6c67edp-65)
    {  0x1.9500000000016p-1,   0x1.43a2730abee3cp+0,  -0x1.e020cc6235a46p-3,  -0x1.abfffffffffa8p-3}, // -107 * 0x1p-9 = -0x1.acp-3 (err =  0x1.7cfb53ba149aap-70)
    {  0x1.95fffffffffc1p-1,   0x1.42d6625d51fb9p+0,  -0x1.db13db0d48a7ep-3,  -0x1.a8000000000fcp-3}, // -106 * 0x1p-9 = -0x1.a8p-3 (err =  0x1.2c39c293c4bd2p-65)
    {  0x1.96fffffffffd3p-1,   0x1.420b5265e5975p+0,  -0x1.d60a17f9035f7p-3,  -0x1.a4000000000b4p-3}, // -105 * 0x1p-9 = -0x1.a4p-3 (err =  0x1.07c584bb53290p-65)
    {  0x1.98000003b3904p-1,   0x1.4141413e57274p+0,  -0x1.d1037f13c1821p-3,  -0x1.9ffffff131bf0p-3}, // -104 * 0x1p-9 = -0x1.a0p-3 (err =  0x1.d102f19ef6b52p-64)
    {  0x1.990000000002dp-1,   0x1.40782d10e6543p+0,  -0x1.cc000c9db3b71p-3,  -0x1.9bfffffffff4cp-3}, // -103 * 0x1p-9 = -0x1.9cp-3 (err =  0x1.d294a033bf2b7p-65)
    {  0x1.9a00000000050p-1,   0x1.3fb013fb013bdp+0,  -0x1.c6ffbc6f00de1p-3,  -0x1.97ffffffffec0p-3}, // -102 * 0x1p-9 = -0x1.98p-3 (err =  0x1.4366345104703p-65)
    {  0x1.9b00000000059p-1,   0x1.3ee8f42a5aec2p+0,  -0x1.c2028ab17f7f9p-3,  -0x1.93ffffffffe9cp-3}, // -101 * 0x1p-9 = -0x1.94p-3 (err =  0x1.4c77eb6cfe579p-65)
    {  0x1.9bffffffffffep-1,   0x1.3e22cbce4a904p+0,  -0x1.bd087383bd8b7p-3,  -0x1.9000000000008p-3}, // -100 * 0x1p-9 = -0x1.90p-3 (err =  0x1.2db4e1a19357ep-76)
    {  0x1.9d00000000017p-1,   0x1.3d5d991aa75b4p+0,  -0x1.b811730b82360p-3,  -0x1.8bfffffffffa4p-3}, //  -99 * 0x1p-9 = -0x1.8cp-3 (err =  0x1.3723f2700b633p-64)
    {  0x1.9e0000000003fp-1,   0x1.3c995a47babb7p+0,  -0x1.b31d8575bcd05p-3,  -0x1.87fffffffff04p-3}, //  -98 * 0x1p-9 = -0x1.88p-3 (err =  0x1.b1b4b09ade1a2p-66)
    {  0x1.9f00000000009p-1,   0x1.3bd60d923294ep+0,  -0x1.ae2ca6f672ba8p-3,  -0x1.83fffffffffdcp-3}, //  -97 * 0x1p-9 = -0x1.84p-3 (err =  0x1.7a83d716576bap-64)
    {  0x1.9ffffff7f069ep-1,   0x1.3b13b1412ea53p+0,  -0x1.a93ed3f05cd01p-3,  -0x1.800000203e588p-3}, //  -96 * 0x1p-9 = -0x1.80p-3 (err =  0x1.a93dd27acca16p-64)
    {  0x1.a0fffffffffc5p-1,   0x1.3a524387ac84fp+0,  -0x1.a454082e6ac27p-3,  -0x1.7c000000000ecp-3}, //  -95 * 0x1p-9 = -0x1.7cp-3 (err =  0x1.16bcd0f94d83fp-65)
    {  0x1.a200000000031p-1,   0x1.3991c2c187f3ep+0,  -0x1.9f6c407089574p-3,  -0x1.77fffffffff3cp-3}, //  -94 * 0x1p-9 = -0x1.78p-3 (err =  0x1.f8165dc9b279fp-66)
    {  0x1.a2fffffffffd8p-1,   0x1.38d22d36608acp+0,  -0x1.9a8778debaafcp-3,  -0x1.74000000000a0p-3}, //  -93 * 0x1p-9 = -0x1.74p-3 (err =  0x1.ef05787803101p-65)
    {  0x1.a400000000007p-1,   0x1.381381381380ep+0,  -0x1.95a5adcf7015dp-3,  -0x1.6ffffffffffe4p-3}, //  -92 * 0x1p-9 = -0x1.70p-3 (err =  0x1.8d9fb7535b943p-65)
    {  0x1.a4fffffffff63p-1,   0x1.3755bd1c94662p+0,  -0x1.90c6db9fcbfd5p-3,  -0x1.6c00000000274p-3}, //  -91 * 0x1p-9 = -0x1.6cp-3 (err =  0x1.6be3fbdebc54dp-65)
    {  0x1.a5fffffffffb4p-1,   0x1.3698df3de0780p+0,  -0x1.8beafeb38fffdp-3,  -0x1.6800000000130p-3}, //  -90 * 0x1p-9 = -0x1.68p-3 (err =  0x1.f3573dfabfd2fp-66)
    {  0x1.a700000000088p-1,   0x1.35dce5f9f2a95p+0,  -0x1.871213750e702p-3,  -0x1.63ffffffffde0p-3}, //  -89 * 0x1p-9 = -0x1.64p-3 (err =  0x1.c64da0619cf87p-66)
    {  0x1.a7fffffe9fb13p-1,   0x1.3521cfb3b868ep+0,  -0x1.823c165bbff2cp-3,  -0x1.60000005813b4p-3}, //  -88 * 0x1p-9 = -0x1.60p-3 (err =  0x1.823cb0e54d6fdp-64)
    {  0x1.a90000000007ap-1,   0x1.34679ace012eep+0,  -0x1.7d6903caf5884p-3,  -0x1.5bffffffffe18p-3}, //  -87 * 0x1p-9 = -0x1.5cp-3 (err =  0x1.1425caf15ebd7p-65)
    {  0x1.a9fffffffffd0p-1,   0x1.33ae45b57bcd5p+0,  -0x1.7898d85444d5ap-3,  -0x1.58000000000c0p-3}, //  -86 * 0x1p-9 = -0x1.58p-3 (err =  0x1.4c6be8f1151c9p-64)
    {  0x1.aafffffffffeep-1,   0x1.32f5ced6a1e07p+0,  -0x1.73cb9074fd1a3p-3,  -0x1.5400000000048p-3}, //  -85 * 0x1p-9 = -0x1.54p-3 (err =  0x1.3758b5678c3f3p-64)
    {  0x1.abfffffd467f8p-1,   0x1.323e34a4a41fcp+0,  -0x1.6f0128c460401p-3,  -0x1.5000000ae6020p-3}, //  -84 * 0x1p-9 = -0x1.50p-3 (err =  0x1.6f017faf04836p-64)
    {  0x1.ad00000000045p-1,   0x1.3187758e9eb85p+0,  -0x1.6a399dabbd23ap-3,  -0x1.4bffffffffeecp-3}, //  -83 * 0x1p-9 = -0x1.4cp-3 (err =  0x1.f7cb37ce2d02ap-65)
    {  0x1.adfffffffffc1p-1,   0x1.30d190130d1bdp+0,  -0x1.6574ebe8c1466p-3,  -0x1.48000000000fcp-3}, //  -82 * 0x1p-9 = -0x1.48p-3 (err =  0x1.4296e52c44645p-65)
    {  0x1.af0000000002dp-1,   0x1.301c82ac40240p+0,  -0x1.60b3100b093a0p-3,  -0x1.43fffffffff4cp-3}, //  -81 * 0x1p-9 = -0x1.44p-3 (err =  0x1.1eb4cdb7ea863p-64)
    {  0x1.b00000069689ap-1,   0x1.2f684bd572742p+0,  -0x1.5bf40696086b2p-3,  -0x1.3fffffe5a5d98p-3}, //  -80 * 0x1p-9 = -0x1.40p-3 (err =  0x1.5bf44c78d1847p-64)
    {  0x1.b100000000015p-1,   0x1.2eb4ea1fed13cp+0,  -0x1.5737cc9018c7ap-3,  -0x1.3bfffffffffacp-3}, //  -79 * 0x1p-9 = -0x1.3cp-3 (err =  0x1.da2728c1aa376p-65)
    {  0x1.b200000000039p-1,   0x1.2e025c04b806fp+0,  -0x1.527e5e4a1b480p-3,  -0x1.37fffffffff1cp-3}, //  -78 * 0x1p-9 = -0x1.38p-3 (err =  0x1.f662b8a5bcc74p-67)
    {  0x1.b30000000003fp-1,   0x1.2d50a012d5074p+0,  -0x1.4dc7b897bc09fp-3,  -0x1.33fffffffff04p-3}, //  -77 * 0x1p-9 = -0x1.34p-3 (err =  0x1.8dcb1e335692dp-70)
    {  0x1.b40000014a5f9p-1,   0x1.2c9fb4d72efecp+0,  -0x1.4913d82d2b7dbp-3,  -0x1.2ffffffad681cp-3}, //  -76 * 0x1p-9 = -0x1.30p-3 (err =  0x1.4913568d5deffp-64)
    {  0x1.b4fffffffffb4p-1,   0x1.2bef98e5a3745p+0,  -0x1.4462b9dc9b540p-3,  -0x1.2c00000000130p-3}, //  -75 * 0x1p-9 = -0x1.2cp-3 (err =  0x1.903ecf3b5a581p-66)
    {  0x1.b5ffffffffff2p-1,   0x1.2b404ad012b4ap+0,  -0x1.3fb45a599290dp-3,  -0x1.2800000000038p-3}, //  -74 * 0x1p-9 = -0x1.28p-3 (err =  0x1.e7a12779f2ca6p-68)
    {  0x1.b6fffffffffe2p-1,   0x1.2a91c92f3c11ap+0,  -0x1.3b08b6757f335p-3,  -0x1.2400000000078p-3}, //  -73 * 0x1p-9 = -0x1.24p-3 (err =  0x1.17db62abc750fp-66)
    {  0x1.b7fffffa1be15p-1,   0x1.29e412a23e32cp+0,  -0x1.365fcb1cc49e3p-3,  -0x1.20000017907acp-3}, //  -72 * 0x1p-9 = -0x1.20p-3 (err =  0x1.365f3027064d8p-64)
    {  0x1.b8ffffffffffbp-1,   0x1.293725bb804a8p+0,  -0x1.31b994d3a4f9cp-3,  -0x1.1c00000000014p-3}, //  -71 * 0x1p-9 = -0x1.1cp-3 (err =  0x1.f9d6af924b190p-66)
    {  0x1.ba00000000019p-1,   0x1.288b01288b002p+0,  -0x1.2d1610c8680c6p-3,  -0x1.17fffffffff9cp-3}, //  -70 * 0x1p-9 = -0x1.18p-3 (err =  0x1.008889fb9baa0p-64)
    {  0x1.baffffffffff1p-1,   0x1.27dfa38a1ce57p+0,  -0x1.28753bc11abeap-3,  -0x1.140000000003cp-3}, //  -69 * 0x1p-9 = -0x1.14p-3 (err =  0x1.eb887f1c5c65bp-65)
    {  0x1.bbfffffc76ecap-1,   0x1.27350b8a6c37ep+0,  -0x1.23d712b4eae39p-3,  -0x1.1000000e244d8p-3}, //  -68 * 0x1p-9 = -0x1.10p-3 (err =  0x1.23d6ac713ed08p-64)
    {  0x1.bd00000000080p-1,   0x1.268b37cd600d2p+0,  -0x1.1f3b925f25af4p-3,  -0x1.0bffffffffe00p-3}, //  -67 * 0x1p-9 = -0x1.0cp-3 (err =  0x1.0bef1b9765e7dp-65)
    {  0x1.bdfffffffff92p-1,   0x1.25e227080933ap+0,  -0x1.1aa2b7e23f923p-3,  -0x1.08000000001b8p-3}, //  -66 * 0x1p-9 = -0x1.08p-3 (err =  0x1.108fd8676af21p-65)
    {  0x1.bf0000000001bp-1,   0x1.2539d7e9177a0p+0,  -0x1.160c8024b2735p-3,  -0x1.03fffffffff94p-3}, //  -65 * 0x1p-9 = -0x1.04p-3 (err =  0x1.0ca7469e666ccp-65)
    {  0x1.c00000142fb7bp-1,   0x1.2492491763732p+0,  -0x1.1178e7c6366dcp-3,  -0x1.ffffff5e82428p-4}, //  -64 * 0x1p-9 = -0x1.00p-3 (err =  0x1.1177c8bb6f04bp-64)
    {  0x1.c100000000085p-1,   0x1.23eb797176005p+0,  -0x1.0ce7ecdccc02ep-3,  -0x1.f7ffffffffbd8p-4}, //  -63 * 0x1p-9 = -0x1.f8p-4 (err =  0x1.7480852a7bcb7p-65)
    {  0x1.c20000000001ep-1,   0x1.23456789abcccp+0,  -0x1.08598b59e397ep-3,  -0x1.effffffffff10p-4}, //  -62 * 0x1p-9 = -0x1.f0p-4 (err =  0x1.b75136cc37ffdp-67)
    {  0x1.c2fffffffffa6p-1,   0x1.22a0122a01264p+0,  -0x1.03cdc0a51eda6p-3,  -0x1.e8000000002d0p-4}, //  -61 * 0x1p-9 = -0x1.e8p-4 (err =  0x1.b6ffc0db38c54p-65)
    {  0x1.c3fffffffffd9p-1,   0x1.21fb78121fb91p+0,  -0x1.fe89139dbd6c7p-4,  -0x1.e000000000138p-4}, //  -60 * 0x1p-9 = -0x1.e0p-4 (err =  0x1.5af4ddab69f1bp-65)
    {  0x1.c4fffffffffc7p-1,   0x1.215798048560ap+0,  -0x1.f57bc7d9007dep-4,  -0x1.d8000000001c8p-4}, //  -59 * 0x1p-9 = -0x1.d8p-4 (err =  0x1.a31b8cb4e335bp-65)
    {  0x1.c600000000024p-1,   0x1.20b470c67c0c2p+0,  -0x1.ec739830a0fdbp-4,  -0x1.cfffffffffee0p-4}, //  -58 * 0x1p-9 = -0x1.d0p-4 (err =  0x1.525b2302f40e7p-65)
    {  0x1.c700000000075p-1,   0x1.2012012011fc8p+0,  -0x1.e3707ee30445ep-4,  -0x1.c7ffffffffc58p-4}, //  -57 * 0x1p-9 = -0x1.c8p-4 (err =  0x1.3e3be7259fadap-65)
    {  0x1.c7fffffffffeep-1,   0x1.1f7047dc11f7cp+0,  -0x1.da72763844744p-4,  -0x1.c000000000090p-4}, //  -56 * 0x1p-9 = -0x1.c0p-4 (err =  0x1.9f54e030340edp-65)
    {  0x1.c8fffffffffaap-1,   0x1.1ecf43c7fb882p+0,  -0x1.d179788219667p-4,  -0x1.b8000000002b0p-4}, //  -55 * 0x1p-9 = -0x1.b8p-4 (err =  0x1.783265cd5d4c5p-65)
    {  0x1.ca0000000001cp-1,   0x1.1e2ef3b3fb863p+0,  -0x1.c885801bc4a29p-4,  -0x1.affffffffff20p-4}, //  -54 * 0x1p-9 = -0x1.b0p-4 (err =  0x1.90812d6228d5ep-67)
    {  0x1.cb00000000003p-1,   0x1.1d8f5672e4abbp+0,  -0x1.bf968769fc9f6p-4,  -0x1.a7fffffffffe8p-4}, //  -53 * 0x1p-9 = -0x1.a8p-4 (err =  0x1.ad4cb7a2ea27ap-65)
    {  0x1.cbffffffa1e78p-1,   0x1.1cf06ada625b0p+0,  -0x1.b6ac88de1b8e2p-4,  -0x1.a0000002f0c40p-4}, //  -52 * 0x1p-9 = -0x1.a0p-4 (err =  0x1.b6ae19e39bf97p-65)
    {  0x1.ccfffffffff95p-1,   0x1.1c522fc1ce09bp+0,  -0x1.adc77ee5aee43p-4,  -0x1.9800000000358p-4}, //  -51 * 0x1p-9 = -0x1.98p-4 (err =  0x1.8bd9dfca20541p-65)
    {  0x1.ce0000000000ap-1,   0x1.1bb4a4046ed23p+0,  -0x1.a4e7640b1bbdfp-4,  -0x1.8ffffffffffb0p-4}, //  -50 * 0x1p-9 = -0x1.90p-4 (err =  0x1.6b2c38ca71ddfp-65)
    {  0x1.cf0000000001fp-1,   0x1.1b17c67f2bad0p+0,  -0x1.9c0c32d4d2436p-4,  -0x1.87fffffffff08p-4}, //  -49 * 0x1p-9 = -0x1.88p-4 (err =  0x1.64c51d200e0adp-66)
    {  0x1.d00000034ac31p-1,   0x1.1a7b960fa6a6cp+0,  -0x1.9335e5b8850acp-4,  -0x1.7fffffe5a9e78p-4}, //  -48 * 0x1p-9 = -0x1.80p-4 (err =  0x1.93373c4fb7b9ap-65)
    {  0x1.d100000000040p-1,   0x1.19e0119e01177p+0,  -0x1.8a6477a91d9f5p-4,  -0x1.77ffffffffe00p-4}, //  -47 * 0x1p-9 = -0x1.78p-4 (err =  0x1.18bfa95c38273p-65)
    {  0x1.d200000000009p-1,   0x1.19453808ca297p+0,  -0x1.8197e2f40e3a1p-4,  -0x1.6ffffffffffb8p-4}, //  -46 * 0x1p-9 = -0x1.70p-4 (err =  0x1.3b5c3719db3e2p-67)
    {  0x1.d300000000014p-1,   0x1.18ab083902bcfp+0,  -0x1.78d02263d8224p-4,  -0x1.67fffffffff60p-4}, //  -45 * 0x1p-9 = -0x1.68p-4 (err =  0x1.ae6648125b7e9p-69)
    {  0x1.d3fffffffffecp-1,   0x1.181181181181dp+0,  -0x1.700d30aeac190p-4,  -0x1.60000000000a0p-4}, //  -44 * 0x1p-9 = -0x1.60p-4 (err =  0x1.44080b4b073f2p-65)
    {  0x1.d4ffffffffffap-1,   0x1.1778a191bd688p+0,  -0x1.674f089365aaep-4,  -0x1.5800000000030p-4}, //  -43 * 0x1p-9 = -0x1.58p-4 (err =  0x1.517797aadca19p-68)
    {  0x1.d600000000064p-1,   0x1.16e068942733ep+0,  -0x1.5e95a4d978e64p-4,  -0x1.4fffffffffce0p-4}, //  -42 * 0x1p-9 = -0x1.50p-4 (err =  0x1.93f6245af9a11p-66)
    {  0x1.d6fffffffffeap-1,   0x1.1648d50fc320ep+0,  -0x1.55e10050e0443p-4,  -0x1.48000000000b0p-4}, //  -41 * 0x1p-9 = -0x1.48p-4 (err =  0x1.280a9e18460a1p-65)
    {  0x1.d7fffffd33c12p-1,   0x1.15b1e5f8f7d59p+0,  -0x1.4d3115ea4f78ep-4,  -0x1.4000001661f70p-4}, //  -40 * 0x1p-9 = -0x1.40p-4 (err =  0x1.4d2e11415094dp-65)
    {  0x1.d900000000008p-1,   0x1.151b9a3fdd5c4p+0,  -0x1.4485e03dbdf68p-4,  -0x1.37fffffffffc0p-4}, //  -39 * 0x1p-9 = -0x1.38p-4 (err =  0x1.2139afebf54bep-71)
    {  0x1.da00000000013p-1,   0x1.1485f0e0acd30p+0,  -0x1.3bdf5a7d1edc0p-4,  -0x1.2ffffffffff68p-4}, //  -38 * 0x1p-9 = -0x1.30p-4 (err =  0x1.a0bdf990b7f08p-67)
    {  0x1.db00000000071p-1,   0x1.13f0e8d3446e2p+0,  -0x1.333d7f8183b7dp-4,  -0x1.27ffffffffc78p-4}, //  -37 * 0x1p-9 = -0x1.28p-4 (err =  0x1.adab97b9dbc98p-66)
    {  0x1.dc0000000001dp-1,   0x1.135c81135c800p+0,  -0x1.2aa04a44716abp-4,  -0x1.1ffffffffff18p-4}, //  -36 * 0x1p-9 = -0x1.20p-4 (err =  0x1.623e04987e3d1p-67)
    {  0x1.dd0000000007fp-1,   0x1.12c8b89edc063p+0,  -0x1.2207b5c78505bp-4,  -0x1.17ffffffffc08p-4}, //  -35 * 0x1p-9 = -0x1.18p-4 (err =  0x1.7d6939f4fac5ap-66)
    {  0x1.de0000000004dp-1,   0x1.12358e75d3007p+0,  -0x1.1973bd14652d3p-4,  -0x1.0fffffffffd98p-4}, //  -34 * 0x1p-9 = -0x1.10p-4 (err =  0x1.d8b6b3d8f3c1cp-66)
    {  0x1.deffffffffff8p-1,   0x1.11a3019a7482bp+0,  -0x1.10e45b3cae875p-4,  -0x1.0800000000040p-4}, //  -33 * 0x1p-9 = -0x1.08p-4 (err =  0x1.9f8e5d0fdc1fep-66)
    {  0x1.e000000000001p-1,   0x1.1111111111110p+0,  -0x1.08598b59e39fep-4,  -0x1.ffffffffffff0p-5}, //  -32 * 0x1p-9 = -0x1.00p-4 (err =  0x1.b75136c7ad30cp-68)
    {  0x1.e100000000092p-1,   0x1.107fbbe01102dp+0,  -0x1.ffa6911ab894ap-5,  -0x1.efffffffff6e0p-5}, //  -31 * 0x1p-9 = -0x1.f0p-5 (err =  0x1.f4490183fd9f8p-66)
    {  0x1.e20000000004bp-1,   0x1.0fef010feefe7p+0,  -0x1.eea31c006b381p-5,  -0x1.dfffffffffb50p-5}, //  -30 * 0x1p-9 = -0x1.e0p-5 (err =  0x1.71300d25e5ea2p-69)
    {  0x1.e300000000044p-1,   0x1.0f5edfab3257bp+0,  -0x1.dda8adc67e9cdp-5,  -0x1.cfffffffffbc0p-5}, //  -29 * 0x1p-9 = -0x1.d0p-5 (err =  0x1.923a175d4eb6ap-67)
    {  0x1.e3fffffffffdep-1,   0x1.0ecf56be69ca3p+0,  -0x1.ccb73cdddb50bp-5,  -0x1.c000000000220p-5}, //  -28 * 0x1p-9 = -0x1.c0p-5 (err =  0x1.172ee66ec0cb1p-66)
    {  0x1.e500000000007p-1,   0x1.0e4065582600dp+0,  -0x1.bbcebfc68f3aap-5,  -0x1.affffffffff90p-5}, //  -27 * 0x1p-9 = -0x1.b0p-5 (err =  0x1.1b216294ec9e4p-66)
    {  0x1.e600000000022p-1,   0x1.0db20a88f4683p+0,  -0x1.aaef2d0fb0ebfp-5,  -0x1.9fffffffffde0p-5}, //  -26 * 0x1p-9 = -0x1.a0p-5 (err =  0x1.093a9984956bcp-68)
    {  0x1.e70000000001ep-1,   0x1.0d24456359e29p+0,  -0x1.9a187b573dc83p-5,  -0x1.8fffffffffe20p-5}, //  -25 * 0x1p-9 = -0x1.90p-5 (err =  0x1.3f47f549395bap-66)
    {  0x1.e80000019729bp-1,   0x1.0c9714faed8a8p+0,  -0x1.894aa12f48354p-5,  -0x1.7fffffe68d650p-5}, //  -24 * 0x1p-9 = -0x1.80p-5 (err =  0x1.894e344ced046p-66)
    {  0x1.e8ffffffffff6p-1,   0x1.0c0a7868b4176p+0,  -0x1.788595a357862p-5,  -0x1.70000000000a0p-5}, //  -23 * 0x1p-9 = -0x1.70p-5 (err =  0x1.c8f4da19612e4p-71)
    {  0x1.e9ffffffffff8p-1,   0x1.0b7e6ec259dccp+0,  -0x1.67c94f2d4bbdep-5,  -0x1.6000000000080p-5}, //  -22 * 0x1p-9 = -0x1.60p-5 (err =  0x1.e2d6082f3101dp-68)
    {  0x1.eaffffffffffap-1,   0x1.0af2f722eecb9p+0,  -0x1.5715c4c03cf53p-5,  -0x1.5000000000060p-5}, //  -21 * 0x1p-9 = -0x1.50p-5 (err =  0x1.45b7a587a1cbdp-66)
    {  0x1.ebfffffedc914p-1,   0x1.0a6810a71ed83p+0,  -0x1.466aed55d2b75p-5,  -0x1.4000001236ec0p-5}, //  -20 * 0x1p-9 = -0x1.40p-5 (err =  0x1.4668732da06f3p-66)
    {  0x1.ed0000000002fp-1,   0x1.09ddba6af8347p+0,  -0x1.35c8bfaa12d5ep-5,  -0x1.2fffffffffd10p-5}, //  -19 * 0x1p-9 = -0x1.30p-5 (err =  0x1.2b4cc56340a58p-66)
    {  0x1.edfffffffffc5p-1,   0x1.0953f39010974p+0,  -0x1.252f32f8d1c11p-5,  -0x1.20000000003b0p-5}, //  -18 * 0x1p-9 = -0x1.20p-5 (err =  0x1.c913ce926e762p-67)
    {  0x1.ef00000000045p-1,   0x1.08cabb37565bdp+0,  -0x1.149e3e4005617p-5,  -0x1.0fffffffffbb0p-5}, //  -17 * 0x1p-9 = -0x1.10p-5 (err =  0x1.083d132a6e949p-66)
    {  0x1.f00000035d1eap-1,   0x1.08421082563f1p+0,  -0x1.0415d866e5e76p-5,  -0x1.ffffff945c2c0p-6}, //  -16 * 0x1p-9 = -0x1.00p-5 (err =  0x1.0417fcb3a768fp-66)
    {  0x1.f0fffffffffd8p-1,   0x1.07b9f29b8eaf7p+0,  -0x1.e72bf2813d378p-6,  -0x1.e000000000500p-6}, //  -15 * 0x1p-9 = -0x1.e0p-6 (err =  0x1.abc9a24703a91p-67)
    {  0x1.f1fffffffff8dp-1,   0x1.073260a47f803p+0,  -0x1.c63d2ec14b9b9p-6,  -0x1.c000000000e60p-6}, //  -14 * 0x1p-9 = -0x1.c0p-6 (err =  0x1.58f41d393aa5ep-67)
    {  0x1.f2fffffffff7bp-1,   0x1.06ab59c791341p+0,  -0x1.a55f548c5d54ep-6,  -0x1.a0000000010a0p-6}, //  -13 * 0x1p-9 = -0x1.a0p-6 (err =  0x1.1388736a55bf9p-67)
    {  0x1.f40000000003ap-1,   0x1.0624dd2f1a9ddp+0,  -0x1.8492528c8c352p-6,  -0x1.7fffffffff8c0p-6}, //  -12 * 0x1p-9 = -0x1.80p-6 (err =  0x1.5ed99a6354ef9p-68)
    {  0x1.f4ffffffffff6p-1,   0x1.059eea072758cp+0,  -0x1.63d6178690d1dp-6,  -0x1.6000000000140p-6}, //  -11 * 0x1p-9 = -0x1.60p-6 (err =  0x1.a4d68f3e33445p-68)
    {  0x1.f5fffffffffebp-1,   0x1.05197f7d7340fp+0,  -0x1.432a925980f6ep-6,  -0x1.40000000002a0p-6}, //  -10 * 0x1p-9 = -0x1.40p-6 (err =  0x1.204e55ddd6457p-68)
    {  0x1.f700000000034p-1,   0x1.04949cc1664abp+0,  -0x1.228fb1fea278ap-6,  -0x1.1fffffffff980p-6}, //   -9 * 0x1p-9 = -0x1.20p-6 (err =  0x1.56a82c46ebd6dp-68)
    {  0x1.f7fffffdb056ap-1,   0x1.04104105418d0p+0,  -0x1.020565d457374p-6,  -0x1.00000049f52c0p-6}, //   -8 * 0x1p-9 = -0x1.00p-6 (err =  0x1.0201ad05c0d85p-67)
    {  0x1.f900000000029p-1,   0x1.038c6b78247e7p+0,  -0x1.c317384c754a2p-7,  -0x1.bfffffffff5c0p-7}, //   -7 * 0x1p-9 = -0x1.c0p-7 (err =  0x1.95e573f8c3074p-68)
    {  0x1.f9fffffffffd6p-1,   0x1.03091b51f5e30p+0,  -0x1.82448a388ad4ap-7,  -0x1.8000000000a80p-7}, //   -6 * 0x1p-9 = -0x1.80p-7 (err =  0x1.a845e63329033p-70)
    {  0x1.fb00000000032p-1,   0x1.02864fc7729cfp+0,  -0x1.41929f9682650p-7,  -0x1.3fffffffff380p-7}, //   -5 * 0x1p-9 = -0x1.40p-7 (err =  0x1.cce78f176cfd3p-70)
    {  0x1.fc00000000028p-1,   0x1.02040810203f4p+0,  -0x1.010157588d45dp-7,  -0x1.fffffffffec00p-8}, //   -4 * 0x1p-9 = -0x1.00p-7 (err =  0x1.f09196c3b35ecp-69)
    {  0x1.fd0000000000cp-1,   0x1.0182436517a31p+0,  -0x1.812121458654bp-8,  -0x1.7fffffffffa00p-8}, //   -3 * 0x1p-9 = -0x1.80p-8 (err =  0x1.d7f9bb20855e5p-70)
    {  0x1.fdffffffffffdp-1,   0x1.0101010101012p+0,  -0x1.0080559588cb7p-8,  -0x1.0000000000180p-8}, //   -2 * 0x1p-9 = -0x1.00p-8 (err =  0x1.2782552c93f88p-71)
    {  0x1.fefffffffff9ep-1,   0x1.00804020100b1p+0,  -0x1.0040155d5eacfp-9,  -0x1.0000000006200p-9}, //   -1 * 0x1p-9 = -0x1.00p-9 (err =  0x1.b51fb5af12f76p-71)
    {  0x1.0000000000000p+0,   0x1.0000000000000p+0,   0x0.0000000000000p+0,   0x0.0000000000000p+0}, //    0 * 0x1p-9 = 0x0.00p+0 (err =   0x0.0000000000000p+0)
};// Worst deviation:  ap1 =   0x1.6000000b07c96p+0, log1pa =   0x1.4618bc41dc931p-2, a =   0x1.8000002c1f258p-2,  err =  0x1.4618f6ec2219ap-63, d = 0x1.60f92cp-29

static const logdata_t LOG2_LOOKUP[] = { //ap1, va, log1pa, a: va = 1/(a+1), log1pa = log2(1+a)+err
    {  0x1.0000000000000p+0,   0x1.0000000000000p+0,   0x0.0000000000000p+0,   0x0.0000000000000p+0}, //    0 * 0x1p-8 = 0x0.00p+0 (err =   0x0.0000000000000p+0)
    {  0x1.01000000001e8p+0,   0x1.fe01fe01fdc57p-1,   0x1.709c46d7d69c3p-8,   0x1.000000001e800p-8}, //    1 * 0x1p-8 = 0x1.00p-8 (err =  0x1.63287def39a0ep-69)
    {  0x1.020000000000fp+0,   0x1.fc07f01fc07d3p-1,   0x1.6fe50b6ef130ep-7,   0x1.0000000000780p-7}, //    2 * 0x1p-8 = 0x1.00p-7 (err =  0x1.2dc5a59888648p-73)
    {  0x1.02fffffffff89p+0,   0x1.fa11caa01fafap-1,   0x1.1363117a950a0p-6,   0x1.7ffffffffc480p-7}, //    3 * 0x1p-8 = 0x1.80p-7 (err =  0x1.43672d33f32cap-69)
    {  0x1.03fffffffffc2p+0,   0x1.f81f81f81f898p-1,   0x1.6e79685c2bc25p-6,   0x1.fffffffffe100p-7}, //    4 * 0x1p-8 = 0x1.00p-6 (err =  0x1.1fc43a05f9103p-67)
    {  0x1.04fffffffffeap+0,   0x1.f6310aca0dbe0p-1,   0x1.c9363ba8507bep-6,   0x1.3fffffffffa80p-6}, //    5 * 0x1p-8 = 0x1.40p-6 (err =  0x1.b93972e09e8f0p-70)
    {  0x1.0600000000013p+0,   0x1.f44659e4a424dp-1,   0x1.11cd1d513376cp-5,   0x1.80000000004c0p-6}, //    6 * 0x1p-8 = 0x1.80p-6 (err =  0x1.4af9c34b858ebp-67)
    {  0x1.0700000000002p+0,   0x1.f25f644230ab1p-1,   0x1.3ed3094685a80p-5,   0x1.c000000000080p-6}, //    7 * 0x1p-8 = 0x1.c0p-6 (err =  0x1.2784d1316b583p-68)
    {  0x1.07fffffffff9ep+0,   0x1.f07c1f07c1fc0p-1,   0x1.6bad3758eec64p-5,   0x1.fffffffffe780p-6}, //    8 * 0x1p-8 = 0x1.00p-5 (err =  0x1.5952e2b0f46adp-69)
    {  0x1.08ffffffffff2p+0,   0x1.ee9c7f8458e1cp-1,   0x1.985bfc3494f24p-5,   0x1.1fffffffffe40p-5}, //    9 * 0x1p-8 = 0x1.20p-5 (err =  0x1.acc4b65833331p-68)
    {  0x1.0a00000000015p+0,   0x1.ecc07b301ec9ap-1,   0x1.c4dfab90aaf04p-5,   0x1.40000000002a0p-5}, //   10 * 0x1p-8 = 0x1.40p-5 (err =  0x1.f6b6e71d08d59p-68)
    {  0x1.0b00000000060p+0,   0x1.eae807aba013ap-1,   0x1.f138983326439p-5,   0x1.6000000000c00p-5}, //   11 * 0x1p-8 = 0x1.60p-5 (err =  0x1.26364a255f5edp-67)
    {  0x1.0c00000000006p+0,   0x1.e9131abf0b75cp-1,   0x1.0eb389fa2a01fp-4,   0x1.80000000000c0p-5}, //   12 * 0x1p-8 = 0x1.80p-5 (err =  0x1.57175ecbefe86p-68)
    {  0x1.0cfffffffffb4p+0,   0x1.e741aa597516ep-1,   0x1.24b5b7e1353b7p-4,   0x1.9fffffffff680p-5}, //   13 * 0x1p-8 = 0x1.a0p-5 (err =  0x1.a2e596adcba52p-66)
    {  0x1.0e00000000019p+0,   0x1.e573ac901e547p-1,   0x1.3aa2fdd27f3e6p-4,   0x1.c000000000320p-5}, //   14 * 0x1p-8 = 0x1.c0p-5 (err =  0x1.7b9017a5884ffp-67)
    {  0x1.0effffffffffap+0,   0x1.e3a9179dc1a7ep-1,   0x1.507b836033b34p-4,   0x1.dffffffffff40p-5}, //   15 * 0x1p-8 = 0x1.e0p-5 (err =  0x1.de7fb018600fcp-67)
    {  0x1.0ffffffffff86p+0,   0x1.e1e1e1e1e1ef6p-1,   0x1.663f6fac908bcp-4,   0x1.ffffffffff0c0p-5}, //   16 * 0x1p-8 = 0x1.00p-4 (err =  0x1.147c385f9770ep-65)
    {  0x1.110000000002bp+0,   0x1.e01e01e01dfd2p-1,   0x1.7beee96b8a624p-4,   0x1.10000000002b0p-4}, //   17 * 0x1p-8 = 0x1.10p-4 (err =  0x1.e06211128a5d4p-67)
    {  0x1.11ffffffffee8p+0,   0x1.de5d6e3f88873p-1,   0x1.918a16e461bc4p-4,   0x1.1ffffffffee80p-4}, //   18 * 0x1p-8 = 0x1.20p-4 (err =  0x1.2a2d45d87ba05p-65)
    {  0x1.1300000000079p+0,   0x1.dca01dca01cf8p-1,   0x1.a7111df348ebcp-4,   0x1.3000000000790p-4}, //   19 * 0x1p-8 = 0x1.30p-4 (err =  0x1.c36f9c5cae53bp-66)
    {  0x1.13fffffffffeep+0,   0x1.dae6076b981fap-1,   0x1.bc84240adaa39p-4,   0x1.3fffffffffee0p-4}, //   20 * 0x1p-8 = 0x1.40p-4 (err =  0x1.bbd39ac96807ap-66)
    {  0x1.14fffffffff6cp+0,   0x1.d92f2231e8087p-1,   0x1.d1e34e35b7685p-4,   0x1.4fffffffff6c0p-4}, //   21 * 0x1p-8 = 0x1.50p-4 (err =  0x1.af4f8b4eef528p-65)
    {  0x1.1600000000212p+0,   0x1.d77b654b828b1p-1,   0x1.e72ec117fd1b4p-4,   0x1.6000000002120p-4}, //   22 * 0x1p-8 = 0x1.60p-4 (err =  0x1.e18cfc305d93fp-65)
    {  0x1.1700000000045p+0,   0x1.d5cac8075723ep-1,   0x1.fc66a0f0b065ap-4,   0x1.7000000000450p-4}, //   23 * 0x1p-8 = 0x1.70p-4 (err =  0x1.ccf2e28016db0p-66)
    {  0x1.17fffffffffc5p+0,   0x1.d41d41d41d480p-1,   0x1.08c588cda7775p-3,   0x1.7fffffffffc50p-4}, //   24 * 0x1p-8 = 0x1.80p-4 (err =  0x1.d8e13d0546506p-65)
    {  0x1.1900000000044p+0,   0x1.d272ca3fc5aaap-1,   0x1.134e1b48908f9p-3,   0x1.9000000000440p-4}, //   25 * 0x1p-8 = 0x1.90p-4 (err =  0x1.679e1b516b46fp-65)
    {  0x1.19fffffffffe4p+0,   0x1.d0cb58f6ec0a2p-1,   0x1.1dcd197552a56p-3,   0x1.9fffffffffe40p-4}, //   26 * 0x1p-8 = 0x1.a0p-4 (err =  0x1.44f0f7bcf41d9p-65)
    {  0x1.1b0000000006ep+0,   0x1.cf26e5c44bf12p-1,   0x1.284294b07aabcp-3,   0x1.b0000000006e0p-4}, //   27 * 0x1p-8 = 0x1.b0p-4 (err =  0x1.01b37a5daad83p-65)
    {  0x1.1c00000000083p+0,   0x1.cd85689039a36p-1,   0x1.32ae9e278b36dp-3,   0x1.c000000000830p-4}, //   28 * 0x1p-8 = 0x1.c0p-4 (err =  0x1.d9685484781abp-65)
    {  0x1.1cffffffffff8p+0,   0x1.cbe6d9601cbf4p-1,   0x1.3d1146d9a8a11p-3,   0x1.cffffffffff80p-4}, //   29 * 0x1p-8 = 0x1.d0p-4 (err =  0x1.bc293ab17d02dp-67)
    {  0x1.1dfffffffffc6p+0,   0x1.ca4b3055ee1eep-1,   0x1.476a9f983f4f6p-3,   0x1.dfffffffffc60p-4}, //   30 * 0x1p-8 = 0x1.e0p-4 (err =  0x1.6453a00ec2a40p-67)
    {  0x1.1f00000000010p+0,   0x1.c8b265afb8a29p-1,   0x1.51bab907a5d2fp-3,   0x1.f000000000100p-4}, //   31 * 0x1p-8 = 0x1.f0p-4 (err =  0x1.4929ea6a8bd43p-64)
    {  0x1.1ffffffffffd7p+0,   0x1.c71c71c71c75dp-1,   0x1.5c01a39fbd4e3p-3,   0x1.ffffffffffd70p-4}, //   32 * 0x1p-8 = 0x1.00p-3 (err =  0x1.22c2b85cc0c94p-64)
    {  0x1.20fffffffffe3p+0,   0x1.c5894d10d49b3p-1,   0x1.663f6fac911eep-3,   0x1.07fffffffff18p-3}, //   33 * 0x1p-8 = 0x1.08p-3 (err =  0x1.35cdb84bb2832p-65)
    {  0x1.21fffffffff50p+0,   0x1.c3f8f01c3fa02p-1,   0x1.70742d4eefb7ep-3,   0x1.0fffffffffa80p-3}, //   34 * 0x1p-8 = 0x1.10p-3 (err =  0x1.547b5bc467dddp-64)
    {  0x1.230000000002ep+0,   0x1.c26b5392e9fd5p-1,   0x1.7a9fec7d05fb2p-3,   0x1.1800000000170p-3}, //   35 * 0x1p-8 = 0x1.18p-3 (err =  0x1.10362cd278004p-67)
    {  0x1.23ffffffffe9ep+0,   0x1.c0e070381c301p-1,   0x1.84c2bd02ef5b5p-3,   0x1.1fffffffff4f0p-3}, //   36 * 0x1p-8 = 0x1.20p-3 (err =  0x1.7cf0e600f29bcp-64)
    {  0x1.2500000000033p+0,   0x1.bf583ee868d3dp-1,   0x1.8edcae8352d6ep-3,   0x1.2800000000198p-3}, //   37 * 0x1p-8 = 0x1.28p-3 (err =  0x1.15d61add46617p-65)
    {  0x1.260000000003cp+0,   0x1.bdd2b8994069cp-1,   0x1.98edd077e733ap-3,   0x1.30000000001e0p-3}, //   38 * 0x1p-8 = 0x1.30p-3 (err =  0x1.78eab28401b06p-66)
    {  0x1.270000000004dp+0,   0x1.bc4fd65883e07p-1,   0x1.a2f632320bb6ep-3,   0x1.3800000000268p-3}, //   39 * 0x1p-8 = 0x1.38p-3 (err =  0x1.d70279b66a1d5p-66)
    {  0x1.280000000006bp+0,   0x1.bacf914c1ba2fp-1,   0x1.acf5e2db4f0c0p-3,   0x1.4000000000358p-3}, //   40 * 0x1p-8 = 0x1.40p-3 (err =  0x1.83e2ef582598ap-66)
    {  0x1.28ffffffffc68p+0,   0x1.b951e2b19047ap-1,   0x1.b6ecf175f7229p-3,   0x1.47fffffffe340p-3}, //   41 * 0x1p-8 = 0x1.48p-3 (err =  0x1.983ae183bb5edp-64)
    {  0x1.2a00000000003p+0,   0x1.b7d6c3dda3387p-1,   0x1.c0db6cdd94e0cp-3,   0x1.5000000000018p-3}, //   42 * 0x1p-8 = 0x1.50p-3 (err =  0x1.e5a5849fb3092p-65)
    {  0x1.2b00000000005p+0,   0x1.b65e2e3beedfep-1,   0x1.cac163c770dfap-3,   0x1.5800000000028p-3}, //   43 * 0x1p-8 = 0x1.58p-3 (err =  0x1.3714f5f377565p-64)
    {  0x1.2bfffffffffa2p+0,   0x1.b4e81b4e81bd7p-1,   0x1.d49ee4c3255d2p-3,   0x1.5fffffffffd10p-3}, //   44 * 0x1p-8 = 0x1.60p-3 (err =  0x1.0b9db98d142a2p-64)
    {  0x1.2cfffffffffc9p+0,   0x1.b37484ad8071dp-1,   0x1.de73fe3b145f3p-3,   0x1.67ffffffffe48p-3}, //   45 * 0x1p-8 = 0x1.68p-3 (err =  0x1.4451b7961aff0p-64)
    {  0x1.2dfffffffffffp+0,   0x1.b2036406c80dap-1,   0x1.e840be74e6a43p-3,   0x1.6fffffffffff8p-3}, //   46 * 0x1p-8 = 0x1.70p-3 (err =  0x1.9cbf78f470c03p-64)
    {  0x1.2efffffffffe9p+0,   0x1.b094b31d922c5p-1,   0x1.f205339208e47p-3,   0x1.77fffffffff48p-3}, //   47 * 0x1p-8 = 0x1.78p-3 (err =  0x1.c313a819a2a3dp-67)
    {  0x1.2ffffffffffe4p+0,   0x1.af286bca1af50p-1,   0x1.fbc16b90266fap-3,   0x1.7ffffffffff20p-3}, //   48 * 0x1p-8 = 0x1.80p-3 (err =  0x1.7c8d871b4ba3dp-65)
    {  0x1.310000000008bp+0,   0x1.adbe87f948f9ap-1,   0x1.02baba24d0905p-2,   0x1.8800000000458p-3}, //   49 * 0x1p-8 = 0x1.88p-3 (err =  0x1.27e86ce965e88p-64)
    {  0x1.31fffffffffb7p+0,   0x1.ac5701ac57081p-1,   0x1.0790adbb02ea9p-2,   0x1.8fffffffffdb8p-3}, //   50 * 0x1p-8 = 0x1.90p-3 (err =  0x1.2fad12e139793p-64)
    {  0x1.330000000002ep+0,   0x1.aaf1d2f87ebbdp-1,   0x1.0c62975542b6cp-2,   0x1.9800000000170p-3}, //   51 * 0x1p-8 = 0x1.98p-3 (err =  0x1.da4c614770a53p-64)
    {  0x1.3400000000070p+0,   0x1.a98ef606a6323p-1,   0x1.11307dad30d8fp-2,   0x1.a000000000380p-3}, //   52 * 0x1p-8 = 0x1.a0p-3 (err =  0x1.2a2dab8e490afp-66)
    {  0x1.3500000000086p+0,   0x1.a82e65130e0a1p-1,   0x1.15fa676bb0b80p-2,   0x1.a800000000430p-3}, //   53 * 0x1p-8 = 0x1.a8p-3 (err =  0x1.cec05a8bd35a3p-66)
    {  0x1.360000000006cp+0,   0x1.a6d01a6d019dap-1,   0x1.1ac05b291f273p-2,   0x1.b000000000360p-3}, //   54 * 0x1p-8 = 0x1.b0p-3 (err =  0x1.9f5fbe4e584aap-64)
    {  0x1.3700000000155p+0,   0x1.a57410768887cp-1,   0x1.1f825f6d89467p-2,   0x1.b800000000aa8p-3}, //   55 * 0x1p-8 = 0x1.b8p-3 (err =  0x1.0b0f7e92f6ceap-63)
    {  0x1.37fffffffffbep+0,   0x1.a41a41a41a473p-1,   0x1.24407ab0e0601p-2,   0x1.bfffffffffdf0p-3}, //   56 * 0x1p-8 = 0x1.c0p-3 (err =  0x1.171f197b985b9p-64)
    {  0x1.38fffffffffa1p+0,   0x1.a2c2a87c51d1fp-1,   0x1.28fab35b324c3p-2,   0x1.c7ffffffffd08p-3}, //   57 * 0x1p-8 = 0x1.c8p-3 (err =  0x1.065ea6ddf9f00p-64)
    {  0x1.3a0000000004cp+0,   0x1.a16d3f97a4a9dp-1,   0x1.2db10fc4d9c15p-2,   0x1.d000000000260p-3}, //   58 * 0x1p-8 = 0x1.d0p-3 (err =  0x1.c356f53eeaaa9p-64)
    {  0x1.3b0000000003cp+0,   0x1.a01a01a019fcbp-1,   0x1.32639636b294fp-2,   0x1.d8000000001e0p-3}, //   59 * 0x1p-8 = 0x1.d8p-3 (err =  0x1.7d63ae5a4c64cp-65)
    {  0x1.3c00000000012p+0,   0x1.9ec8e951033c1p-1,   0x1.37124cea4ce41p-2,   0x1.e000000000090p-3}, //   60 * 0x1p-8 = 0x1.e0p-3 (err =  0x1.04e53d0b439ebp-64)
    {  0x1.3cffffffffff3p+0,   0x1.9d79f176b683ep-1,   0x1.3bbd3a0a1dcbep-2,   0x1.e7fffffffff98p-3}, //   61 * 0x1p-8 = 0x1.e8p-3 (err =  0x1.eb489931da48fp-69)
    {  0x1.3e00000000075p+0,   0x1.9c2d14ee4a06ap-1,   0x1.406463b1b0669p-2,   0x1.f0000000003a8p-3}, //   62 * 0x1p-8 = 0x1.f0p-3 (err =  0x1.f844ec2652671p-64)
    {  0x1.3f00000000007p+0,   0x1.9ae24ea5510d1p-1,   0x1.4507cfedd4fe4p-2,   0x1.f800000000038p-3}, //   63 * 0x1p-8 = 0x1.f8p-3 (err =  0x1.ed6fdca8e48c8p-65)
    {  0x1.3ffffffffff88p+0,   0x1.9999999999a33p-1,   0x1.49a784bcd1961p-2,   0x1.ffffffffffc40p-3}, //   64 * 0x1p-8 = 0x1.00p-2 (err =  0x1.a140753c09fddp-64)
    {  0x1.40fffffffff55p+0,   0x1.9852f0d8ec1d9p-1,   0x1.4e43880e8f857p-2,   0x1.03ffffffffd54p-2}, //   65 * 0x1p-8 = 0x1.04p-2 (err =  0x1.6f6676ff8a771p-69)
    {  0x1.420000000004ap+0,   0x1.970e4f80cb815p-1,   0x1.52dbdfc4c9807p-2,   0x1.0800000000128p-2}, //   66 * 0x1p-8 = 0x1.08p-2 (err =  0x1.35b02db3508a0p-66)
    {  0x1.4300000000048p+0,   0x1.95cbb0be37753p-1,   0x1.577091b337a14p-2,   0x1.0c00000000120p-2}, //   67 * 0x1p-8 = 0x1.0cp-2 (err =  0x1.355edd88962fap-64)
    {  0x1.43fffffffffd4p+0,   0x1.948b0fcd6ea17p-1,   0x1.5c01a39fbd5bfp-2,   0x1.0ffffffffff50p-2}, //   68 * 0x1p-8 = 0x1.10p-2 (err =  0x1.10dffafe47984p-67)
    {  0x1.44ffffffffee1p+0,   0x1.934c67f9b2e4ap-1,   0x1.608f1b4294395p-2,   0x1.13ffffffffb84p-2}, //   69 * 0x1p-8 = 0x1.14p-2 (err =  0x1.5ad3beb4b6179p-63)
    {  0x1.4600000000062p+0,   0x1.920fb49d0e1b0p-1,   0x1.6518fe4677d63p-2,   0x1.1800000000188p-2}, //   70 * 0x1p-8 = 0x1.18p-2 (err =  0x1.613f5b609e660p-63)
    {  0x1.4700000000005p+0,   0x1.90d4f120190cfp-1,   0x1.699f5248cd4cfp-2,   0x1.1c00000000014p-2}, //   71 * 0x1p-8 = 0x1.1cp-2 (err =  0x1.cb92dcedcce7bp-64)
    {  0x1.4800000000027p+0,   0x1.8f9c18f9c18cap-1,   0x1.6e221cd9d0d8ep-2,   0x1.200000000009cp-2}, //   72 * 0x1p-8 = 0x1.20p-2 (err =  0x1.1b9116de967acp-64)
    {  0x1.490000000003cp+0,   0x1.8e6527af136f6p-1,   0x1.72a1637cbc290p-2,   0x1.24000000000f0p-2}, //   73 * 0x1p-8 = 0x1.24p-2 (err =  0x1.157b2d538701fp-64)
    {  0x1.49fffffffffbbp+0,   0x1.8d3018d301926p-1,   0x1.771d2ba7efa07p-2,   0x1.27ffffffffeecp-2}, //   74 * 0x1p-8 = 0x1.28p-2 (err =  0x1.d57405f9488e4p-64)
    {  0x1.4afffffffff8fp+0,   0x1.8bfce8062ffc1p-1,   0x1.7b957ac51a8ccp-2,   0x1.2bffffffffe3cp-2}, //   75 * 0x1p-8 = 0x1.2cp-2 (err =  0x1.b5634c2267d8cp-66)
    {  0x1.4bffffffffff0p+0,   0x1.8acb90f6bf3bdp-1,   0x1.800a563161c0dp-2,   0x1.2ffffffffffc0p-2}, //   76 * 0x1p-8 = 0x1.30p-2 (err =  0x1.e43bd77ccb22cp-64)
    {  0x1.4cfffffffffe5p+0,   0x1.899c0f60189bcp-1,   0x1.847bc33d86116p-2,   0x1.33fffffffff94p-2}, //   77 * 0x1p-8 = 0x1.34p-2 (err =  0x1.3a4a0f4ea02ebp-63)
    {  0x1.4e000000000dfp+0,   0x1.886e5f0abaf44p-1,   0x1.88e9c72e0b600p-2,   0x1.380000000037cp-2}, //   78 * 0x1p-8 = 0x1.38p-2 (err =  0x1.3b676bd1f5023p-63)
    {  0x1.4f0000000000dp+0,   0x1.87427bcc092aap-1,   0x1.8d54673b5c3abp-2,   0x1.3c00000000034p-2}, //   79 * 0x1p-8 = 0x1.3cp-2 (err =  0x1.2a2cab9b37c31p-63)
    {  0x1.4ffffffffff67p+0,   0x1.86186186186cap-1,   0x1.91bba891f1468p-2,   0x1.3fffffffffd9cp-2}, //   80 * 0x1p-8 = 0x1.40p-2 (err =  0x1.250c6fa15f910p-63)
    {  0x1.50fffffffffb3p+0,   0x1.84f00c278066dp-1,   0x1.961f905273f4ap-2,   0x1.43ffffffffeccp-2}, //   81 * 0x1p-8 = 0x1.44p-2 (err =  0x1.6e1b0580e8dc2p-63)
    {  0x1.5200000000030p+0,   0x1.83c977ab2bea6p-1,   0x1.9a802391e2401p-2,   0x1.48000000000c0p-2}, //   82 * 0x1p-8 = 0x1.48p-2 (err =  0x1.77f9fb68622c6p-63)
    {  0x1.530000000002ep+0,   0x1.82a4a0182a46cp-1,   0x1.9edd6759b26a8p-2,   0x1.4c000000000b8p-2}, //   83 * 0x1p-8 = 0x1.4cp-2 (err =  0x1.9c04cbaf5ddb3p-63)
    {  0x1.5400000000021p+0,   0x1.81818181817f3p-1,   0x1.a33760a7f60e0p-2,   0x1.5000000000084p-2}, //   84 * 0x1p-8 = 0x1.50p-2 (err =  0x1.121f473b74576p-63)
    {  0x1.54ffffffffff6p+0,   0x1.8060180601811p-1,   0x1.a78e146f7bec9p-2,   0x1.53fffffffffd8p-2}, //   85 * 0x1p-8 = 0x1.54p-2 (err =  0x1.c43559d1e805ep-65)
    {  0x1.55fffffffff96p+0,   0x1.7f405fd017fb7p-1,   0x1.abe18797f1d7fp-2,   0x1.57ffffffffe58p-2}, //   86 * 0x1p-8 = 0x1.58p-2 (err =  0x1.562db258b9a14p-64)
    {  0x1.570000000001cp+0,   0x1.7e225515a4efep-1,   0x1.b031befe064adp-2,   0x1.5c00000000070p-2}, //   87 * 0x1p-8 = 0x1.5cp-2 (err =  0x1.50973bb998d58p-63)
    {  0x1.5800000000008p+0,   0x1.7d05f417d05ebp-1,   0x1.b47ebf73882c3p-2,   0x1.6000000000020p-2}, //   88 * 0x1p-8 = 0x1.60p-2 (err =  0x1.6dc5450dec4b8p-63)
    {  0x1.5900000000005p+0,   0x1.7beb3922e0176p-1,   0x1.b8c88dbf8868fp-2,   0x1.6400000000014p-2}, //   89 * 0x1p-8 = 0x1.64p-2 (err =  0x1.1f6b02ed609bep-64)
    {  0x1.5a0000000002ap+0,   0x1.7ad2208e0ec95p-1,   0x1.bd0f2e9e790e4p-2,   0x1.68000000000a8p-2}, //   90 * 0x1p-8 = 0x1.68p-2 (err =  0x1.9ea4531962ef1p-63)
    {  0x1.5afffffffffdep+0,   0x1.79baa6bb639b0p-1,   0x1.c152a6c24ca55p-2,   0x1.6bfffffffff78p-2}, //   91 * 0x1p-8 = 0x1.6cp-2 (err =  0x1.3da247b0ce87ap-64)
    {  0x1.5c00000000033p+0,   0x1.78a4c8178a491p-1,   0x1.c592fad295c2fp-2,   0x1.70000000000ccp-2}, //   92 * 0x1p-8 = 0x1.70p-2 (err =  0x1.1661acae276cep-63)
    {  0x1.5cfffffffffe7p+0,   0x1.77908119ac628p-1,   0x1.c9d02f6ca474ap-2,   0x1.73fffffffff9cp-2}, //   93 * 0x1p-8 = 0x1.74p-2 (err =  0x1.fe6a3a2b849ddp-65)
    {  0x1.5dfffffffffb4p+0,   0x1.767dce434aa02p-1,   0x1.ce0a4923a573cp-2,   0x1.77ffffffffed0p-2}, //   94 * 0x1p-8 = 0x1.78p-2 (err =  0x1.7579ddc72deb9p-63)
    {  0x1.5f00000000063p+0,   0x1.756cac2017503p-1,   0x1.d2414c80bf41ep-2,   0x1.7c0000000018cp-2}, //   95 * 0x1p-8 = 0x1.7cp-2 (err =  0x1.28acba8e7b43cp-65)
    {  0x1.600000000007fp+0,   0x1.745d1745d16bfp-1,   0x1.d6753e032ec24p-2,   0x1.80000000001fcp-2}, //   96 * 0x1p-8 = 0x1.80p-2 (err =  0x1.bbbc699ed93fdp-63)
    {  0x1.610000000001bp+0,   0x1.734f0c541fe70p-1,   0x1.daa622206502ap-2,   0x1.840000000006cp-2}, //   97 * 0x1p-8 = 0x1.84p-2 (err =  0x1.4b3ffebce34f7p-65)
    {  0x1.6200000000004p+0,   0x1.724287f46deb8p-1,   0x1.ded3fd442365dp-2,   0x1.8800000000010p-2}, //   98 * 0x1p-8 = 0x1.88p-2 (err =  0x1.b90d2fe10d5f3p-66)
    {  0x1.630000000002bp+0,   0x1.713786d9c7bdcp-1,   0x1.e2fed3d09734bp-2,   0x1.8c000000000acp-2}, //   99 * 0x1p-8 = 0x1.8cp-2 (err =  0x1.5112f42a0e0d6p-63)
    {  0x1.63ffffffffff9p+0,   0x1.702e05c0b8177p-1,   0x1.e726aa1e754b5p-2,   0x1.8ffffffffffe4p-2}, //  100 * 0x1p-8 = 0x1.90p-2 (err =  0x1.3bbd903d6206bp-66)
    {  0x1.6500000000046p+0,   0x1.6f26016f25fcfp-1,   0x1.eb4b847d15cf0p-2,   0x1.9400000000118p-2}, //  101 * 0x1p-8 = 0x1.94p-2 (err =  0x1.c875857fa213ap-64)
    {  0x1.66000000000a2p+0,   0x1.6e1f76b433721p-1,   0x1.ef6d67328e4bdp-2,   0x1.9800000000288p-2}, //  102 * 0x1p-8 = 0x1.98p-2 (err =  0x1.c4b1c4c2ea6cep-63)
    {  0x1.6700000000013p+0,   0x1.6d1a62681c84ep-1,   0x1.f38c567bcc58fp-2,   0x1.9c0000000004cp-2}, //  103 * 0x1p-8 = 0x1.9cp-2 (err =  0x1.a59ecaf7d28b8p-63)
    {  0x1.67fffffffffc8p+0,   0x1.6c16c16c16c4fp-1,   0x1.f7a8568cb05e9p-2,   0x1.9ffffffffff20p-2}, //  104 * 0x1p-8 = 0x1.a0p-2 (err =  0x1.c97d9cec96072p-67)
    {  0x1.690000000001fp+0,   0x1.6b1490aa31a1ep-1,   0x1.fbc16b9026889p-2,   0x1.a40000000007cp-2}, //  105 * 0x1p-8 = 0x1.a4p-2 (err =  0x1.1e0697a4e477cp-65)
    {  0x1.69ffffffffff7p+0,   0x1.6a13cd1537299p-1,   0x1.ffd799a83ff76p-2,   0x1.a7fffffffffdcp-2}, //  106 * 0x1p-8 = 0x1.a8p-2 (err =  0x1.9f120b7792ba2p-63)
    {  0x1.6b0000000013dp+0,   0x1.691473a88cf85p-1,   0x1.01f5727726765p-1,   0x1.ac000000004f4p-2}, //  107 * 0x1p-8 = 0x1.acp-2 (err =  0x1.b53a304e70051p-63)
    {  0x1.6bfffffffffd7p+0,   0x1.681681681683fp-1,   0x1.03fda8b97992cp-1,   0x1.affffffffff5cp-2}, //  108 * 0x1p-8 = 0x1.b0p-2 (err =  0x1.99c217bc19d8fp-64)
    {  0x1.6d0000000004ap+0,   0x1.6719f360166d1p-1,   0x1.0604719f24f48p-1,   0x1.b400000000128p-2}, //  109 * 0x1p-8 = 0x1.b4p-2 (err =  0x1.d11cee67bb69dp-66)
    {  0x1.6e00000000101p+0,   0x1.661ec6a5121fep-1,   0x1.0809cf27f7244p-1,   0x1.b800000000404p-2}, //  110 * 0x1p-8 = 0x1.b8p-2 (err =  0x1.e133adf04410dp-63)
    {  0x1.6efffffffffc5p+0,   0x1.6524f853b4addp-1,   0x1.0a0dc34f8e185p-1,   0x1.bbfffffffff14p-2}, //  111 * 0x1p-8 = 0x1.bcp-2 (err =  0x1.197ffa6008266p-63)
    {  0x1.6ffffffffffd0p+0,   0x1.642c8590b2193p-1,   0x1.0c10500d63a46p-1,   0x1.bffffffffff40p-2}, //  112 * 0x1p-8 = 0x1.c0p-2 (err =  0x1.3a7375c5ff13bp-63)
    {  0x1.70fffffffffd6p+0,   0x1.63356b88ac106p-1,   0x1.0e117754d7bbdp-1,   0x1.c3fffffffff58p-2}, //  113 * 0x1p-8 = 0x1.c4p-2 (err =  0x1.d21095abd7e8bp-63)
    {  0x1.7200000000085p+0,   0x1.623fa770161c0p-1,   0x1.10113b153c9f4p-1,   0x1.c800000000214p-2}, //  114 * 0x1p-8 = 0x1.c8p-2 (err =  0x1.ac0eabb35c24ap-64)
    {  0x1.73000000000f2p+0,   0x1.614b36831adadp-1,   0x1.120f9d39e19e9p-1,   0x1.cc000000003c8p-2}, //  115 * 0x1p-8 = 0x1.ccp-2 (err =  0x1.dd224cbfdfc2ep-63)
    {  0x1.740000000002bp+0,   0x1.60581605815ddp-1,   0x1.140c9faa1e599p-1,   0x1.d0000000000acp-2}, //  116 * 0x1p-8 = 0x1.d0p-2 (err =  0x1.fef332df384d7p-66)
    {  0x1.74fffffffff69p+0,   0x1.5f66434292e8ap-1,   0x1.160844495dedbp-1,   0x1.d3ffffffffda4p-2}, //  117 * 0x1p-8 = 0x1.d4p-2 (err =  0x1.ec46dc5bcd967p-64)
    {  0x1.75fffffffff1cp+0,   0x1.5e75bb8d016bdp-1,   0x1.18028cf7295a8p-1,   0x1.d7ffffffffc70p-2}, //  118 * 0x1p-8 = 0x1.d8p-2 (err =  0x1.889337093c4a7p-63)
    {  0x1.77000000000b4p+0,   0x1.5d867c3ece1fdp-1,   0x1.19fb7b8f32584p-1,   0x1.dc000000002d0p-2}, //  119 * 0x1p-8 = 0x1.dcp-2 (err =  0x1.82ab2c539760fp-65)
    {  0x1.77fffffffff70p+0,   0x1.5c9882b9310ddp-1,   0x1.1bf311e95cef3p-1,   0x1.dfffffffffdc0p-2}, //  120 * 0x1p-8 = 0x1.e0p-2 (err =  0x1.90c4925e0d0d0p-63)
    {  0x1.78fffffffffa0p+0,   0x1.5babcc647faeap-1,   0x1.1de951d9cbaeap-1,   0x1.e3ffffffffe80p-2}, //  121 * 0x1p-8 = 0x1.e4p-2 (err =  0x1.e72dec881fe94p-63)
    {  0x1.79fffffffffdap+0,   0x1.5ac056b015ae3p-1,   0x1.1fde3d30e80dcp-1,   0x1.e7fffffffff68p-2}, //  122 * 0x1p-8 = 0x1.e8p-2 (err =  0x1.05f155fb4b6bap-62)
    {  0x1.7b0000000002bp+0,   0x1.59d61f123cc83p-1,   0x1.21d1d5bb6d5eep-1,   0x1.ec000000000acp-2}, //  123 * 0x1p-8 = 0x1.ecp-2 (err =  0x1.a946286038e70p-63)
    {  0x1.7c00000000036p+0,   0x1.58ed2308158bcp-1,   0x1.23c41d4272831p-1,   0x1.f0000000000d8p-2}, //  124 * 0x1p-8 = 0x1.f0p-2 (err =  0x1.1890c59bdfba9p-63)
    {  0x1.7cfffffffff21p+0,   0x1.5805601580629p-1,   0x1.25b5158b73b54p-1,   0x1.f3ffffffffc84p-2}, //  125 * 0x1p-8 = 0x1.f4p-2 (err =  0x1.01b9791688ce4p-63)
    {  0x1.7dfffffffffc4p+0,   0x1.571ed3c506b70p-1,   0x1.27a4c0585cb84p-1,   0x1.f7fffffffff10p-2}, //  126 * 0x1p-8 = 0x1.f8p-2 (err =  0x1.87872cdd04f16p-63)
    {  0x1.7effffffffd2cp+0,   0x1.56397ba7c5569p-1,   0x1.29931f6790fecp-1,   0x1.fbffffffff4b0p-2}, //  127 * 0x1p-8 = 0x1.fcp-2 (err =  0x1.2985180aaf751p-62)
    {  0x1.80fffffffffb5p-1,   0x1.54725e6bb8340p+0,  -0x1.a527fd95fda1fp-2,  -0x1.fc0000000012cp-3}, // -127 * 0x1p-9 = -0x1.fcp-3 (err =  0x1.9b7d78ca866bcp-63)
    {  0x1.8200000000035p-1,   0x1.5390948f40fbcp+0,  -0x1.a152f142980e9p-2,  -0x1.f7fffffffff2cp-3}, // -126 * 0x1p-9 = -0x1.f8p-3 (err =  0x1.eb73d056c7a56p-64)
    {  0x1.82ffffffffffdp-1,   0x1.52aff56a8054dp+0,  -0x1.9d806ebc99227p-2,  -0x1.f40000000000cp-3}, // -125 * 0x1p-9 = -0x1.f4p-3 (err =  0x1.41d30420557f4p-65)
    {  0x1.83fffffffffcfp-1,   0x1.51d07eae2f840p+0,  -0x1.99b072a96c76dp-2,  -0x1.f0000000000c4p-3}, // -124 * 0x1p-9 = -0x1.f0p-3 (err =  0x1.8200ec1407bccp-63)
    {  0x1.84fffffffff57p-1,   0x1.50f22e111c558p+0,  -0x1.95e2f9b51f2d0p-2,  -0x1.ec000000002a4p-3}, // -123 * 0x1p-9 = -0x1.ecp-3 (err =  0x1.c99a30dcdd6f5p-64)
    {  0x1.860000000005cp-1,   0x1.5015015014fc6p+0,  -0x1.921800924dbdfp-2,  -0x1.e7ffffffffe90p-3}, // -122 * 0x1p-9 = -0x1.e8p-3 (err =  0x1.ee316edec949ep-64)
    {  0x1.8700000000021p-1,   0x1.4f38f62dd4c7ep+0,  -0x1.8e4f83fa14571p-2,  -0x1.e3fffffffff7cp-3}, // -121 * 0x1p-9 = -0x1.e4p-3 (err =  0x1.376a59e7ce3d7p-63)
    {  0x1.87fffffffffe0p-1,   0x1.4e5e0a72f0555p+0,  -0x1.8a8980abfbdabp-2,  -0x1.e000000000080p-3}, // -120 * 0x1p-9 = -0x1.e0p-3 (err =  0x1.3b8dc4d28f8bfp-63)
    {  0x1.8900000000050p-1,   0x1.4d843bedc2c08p+0,  -0x1.86c5f36dea2afp-2,  -0x1.dbffffffffec0p-3}, // -119 * 0x1p-9 = -0x1.dcp-3 (err =  0x1.8584304425acbp-64)
    {  0x1.8a000000000b7p-1,   0x1.4cab88725aed4p+0,  -0x1.8304d90c11d25p-2,  -0x1.d7ffffffffd24p-3}, // -118 * 0x1p-9 = -0x1.d8p-3 (err =  0x1.57129c3531529p-64)
    {  0x1.8affffffffff0p-1,   0x1.4bd3edda68feep+0,  -0x1.7f462e58e16c4p-2,  -0x1.d400000000040p-3}, // -117 * 0x1p-9 = -0x1.d4p-3 (err =  0x1.378e95d98b1b3p-63)
    {  0x1.8bfffffffffe9p-1,   0x1.4afd6a052bf6ep+0,  -0x1.7b89f02cf2b03p-2,  -0x1.d00000000005cp-3}, // -116 * 0x1p-9 = -0x1.d0p-3 (err =  0x1.03672a28442bep-64)
    {  0x1.8cfffffffffe1p-1,   0x1.4a27fad760164p+0,  -0x1.77d01b66fbdaap-2,  -0x1.cc0000000007cp-3}, // -115 * 0x1p-9 = -0x1.ccp-3 (err =  0x1.f19d21b534ce8p-66)
    {  0x1.8e00000000001p-1,   0x1.49539e3b2d066p+0,  -0x1.7418acebbf18bp-2,  -0x1.c7ffffffffffcp-3}, // -114 * 0x1p-9 = -0x1.c8p-3 (err =  0x1.3a2cb5b24d6f5p-66)
    {  0x1.8efffffffffbcp-1,   0x1.48805220148b8p+0,  -0x1.7063a1a5fb5eep-2,  -0x1.c400000000110p-3}, // -113 * 0x1p-9 = -0x1.c4p-3 (err =  0x1.c0dc40aef5eaap-64)
    {  0x1.900000000000dp-1,   0x1.47ae147ae1470p+0,  -0x1.6cb0f6865c8bap-2,  -0x1.bffffffffffccp-3}, // -112 * 0x1p-9 = -0x1.c0p-3 (err =  0x1.06ad182ba8a16p-65)
    {  0x1.91000000000c1p-1,   0x1.46dce34595fc9p+0,  -0x1.6900a8836ce0ep-2,  -0x1.bbffffffffcfcp-3}, // -111 * 0x1p-9 = -0x1.bcp-3 (err =  0x1.4fc2dbdf48a1cp-65)
    {  0x1.9200000000033p-1,   0x1.460cbc7f5cf71p+0,  -0x1.6552b499861bcp-2,  -0x1.b7fffffffff34p-3}, // -110 * 0x1p-9 = -0x1.b8p-3 (err =  0x1.59de5c8947844p-63)
    {  0x1.92fffffffffb2p-1,   0x1.453d9e2c77709p+0,  -0x1.61a717cac1aa1p-2,  -0x1.b400000000138p-3}, // -109 * 0x1p-9 = -0x1.b4p-3 (err =  0x1.71cd8b369ca3ap-65)
    {  0x1.93fffffffffe5p-1,   0x1.446f86562da11p+0,  -0x1.5dfdcf1eeae71p-2,  -0x1.b00000000006cp-3}, // -108 * 0x1p-9 = -0x1.b0p-3 (err =  0x1.f6da78a8da72dp-64)
    {  0x1.9500000000032p-1,   0x1.43a2730abee25p+0,  -0x1.5a56d7a370d37p-2,  -0x1.abfffffffff38p-3}, // -107 * 0x1p-9 = -0x1.acp-3 (err =  0x1.04ec0407a6c86p-63)
    {  0x1.95ffffffffff6p-1,   0x1.42d6625d51f8fp+0,  -0x1.56b22e6b57909p-2,  -0x1.a800000000028p-3}, // -106 * 0x1p-9 = -0x1.a8p-3 (err =  0x1.0c07a3910ed39p-64)
    {  0x1.9700000000008p-1,   0x1.420b5265e594bp+0,  -0x1.530fd08f29f8ap-2,  -0x1.a3fffffffffe0p-3}, // -105 * 0x1p-9 = -0x1.a4p-3 (err =  0x1.e34b613d09bdep-68)
    {  0x1.97fffffffffcap-1,   0x1.414141414143fp+0,  -0x1.4f6fbb2cec65cp-2,  -0x1.a0000000000d8p-3}, // -104 * 0x1p-9 = -0x1.a0p-3 (err =  0x1.3411dc518a7ccp-67)
    {  0x1.98ffffffffff4p-1,   0x1.40782d10e656fp+0,  -0x1.4bd1eb680e573p-2,  -0x1.9c00000000030p-3}, // -103 * 0x1p-9 = -0x1.9cp-3 (err =  0x1.33ccca6e81b54p-63)
    {  0x1.99fffffffffd6p-1,   0x1.3fb013fb0141cp+0,  -0x1.48365e695d82ep-2,  -0x1.98000000000a8p-3}, // -102 * 0x1p-9 = -0x1.98p-3 (err =  0x1.da76709165404p-68)
    {  0x1.9b00000000044p-1,   0x1.3ee8f42a5aed2p+0,  -0x1.449d115ef7c93p-2,  -0x1.93ffffffffef0p-3}, // -101 * 0x1p-9 = -0x1.94p-3 (err =  0x1.0e3a828c02257p-64)
    {  0x1.9bffffffffff5p-1,   0x1.3e22cbce4a90bp+0,  -0x1.4106017c3eccap-2,  -0x1.900000000002cp-3}, // -100 * 0x1p-9 = -0x1.90p-3 (err =  0x1.6ce41b630b2c6p-65)
    {  0x1.9cfffffffffc3p-1,   0x1.3d5d991aa75f5p+0,  -0x1.3d712bf9c9ec9p-2,  -0x1.8c000000000f4p-3}, //  -99 * 0x1p-9 = -0x1.8cp-3 (err =  0x1.e7e2c484b22f1p-64)
    {  0x1.9dfffffffffeap-1,   0x1.3c995a47babf8p+0,  -0x1.39de8e1559fbep-2,  -0x1.8800000000058p-3}, //  -98 * 0x1p-9 = -0x1.88p-3 (err =  0x1.b508c73a1b476p-64)
    {  0x1.9efffffffffd3p-1,   0x1.3bd60d9232977p+0,  -0x1.364e2511cc8c1p-2,  -0x1.84000000000b4p-3}, //  -97 * 0x1p-9 = -0x1.84p-3 (err =  0x1.abd879c932e6dp-71)
    {  0x1.a000000000025p-1,   0x1.3b13b13b13af8p+0,  -0x1.32bfee370ede5p-2,  -0x1.7ffffffffff6cp-3}, //  -96 * 0x1p-9 = -0x1.80p-3 (err =  0x1.9a254493b4bf5p-65)
    {  0x1.a100000000019p-1,   0x1.3a524387ac810p+0,  -0x1.2f33e6d212099p-2,  -0x1.7bfffffffff9cp-3}, //  -95 * 0x1p-9 = -0x1.7cp-3 (err =  0x1.2ad97a2e8fcf2p-63)
    {  0x1.a20000000016bp-1,   0x1.3991c2c187e53p+0,  -0x1.2baa0c34bdce9p-2,  -0x1.77ffffffffa54p-3}, //  -94 * 0x1p-9 = -0x1.78p-3 (err =  0x1.b30cf07c8d27ap-64)
    {  0x1.a30000000004ap-1,   0x1.38d22d3660856p+0,  -0x1.28225bb5e639fp-2,  -0x1.73ffffffffed8p-3}, //  -93 * 0x1p-9 = -0x1.74p-3 (err =  0x1.1366e853248efp-64)
    {  0x1.a3fffffffffd8p-1,   0x1.3813813813831p+0,  -0x1.249cd2b13cdf9p-2,  -0x1.70000000000a0p-3}, //  -92 * 0x1p-9 = -0x1.70p-3 (err =  0x1.0b522f8bc4e44p-63)
    {  0x1.a4ffffffffe94p-1,   0x1.3755bd1c946fbp+0,  -0x1.21196e87478cep-2,  -0x1.6c000000005b0p-3}, //  -91 * 0x1p-9 = -0x1.6cp-3 (err =  0x1.9361d33640aa2p-64)
    {  0x1.a600000000114p-1,   0x1.3698df3de067cp+0,  -0x1.1d982c9d52342p-2,  -0x1.67ffffffffbb0p-3}, //  -90 * 0x1p-9 = -0x1.68p-3 (err =  0x1.2bcfc616e9397p-64)
    {  0x1.a6fffffffff87p-1,   0x1.35dce5f9f2b51p+0,  -0x1.1a190a5d67647p-2,  -0x1.64000000001e4p-3}, //  -89 * 0x1p-9 = -0x1.64p-3 (err =  0x1.de8d7bff8628bp-66)
    {  0x1.a800000000043p-1,   0x1.3521cfb2b7890p+0,  -0x1.169c05363f06fp-2,  -0x1.5fffffffffef4p-3}, //  -88 * 0x1p-9 = -0x1.60p-3 (err =  0x1.c61117a541897p-64)
    {  0x1.a8fffffffffb1p-1,   0x1.34679ace01380p+0,  -0x1.13211a9b38537p-2,  -0x1.5c0000000013cp-3}, //  -87 * 0x1p-9 = -0x1.5cp-3 (err =  0x1.ab6f9414bfd7ap-65)
    {  0x1.aa0000000023fp-1,   0x1.33ae45b57bb13p+0,  -0x1.0fa848044ab87p-2,  -0x1.57ffffffff704p-3}, //  -86 * 0x1p-9 = -0x1.58p-3 (err =  0x1.eff36983ac8c4p-64)
    {  0x1.aafffffffff53p-1,   0x1.32f5ced6a1e76p+0,  -0x1.0c318aedff617p-2,  -0x1.54000000002b4p-3}, //  -85 * 0x1p-9 = -0x1.54p-3 (err =  0x1.ec898f135164ep-64)
    {  0x1.abfffffffff5cp-1,   0x1.323e34a2b1135p+0,  -0x1.08bce0d95fc6ep-2,  -0x1.5000000000290p-3}, //  -84 * 0x1p-9 = -0x1.50p-3 (err =  0x1.bc20888ba1380p-64)
    {  0x1.ad00000000047p-1,   0x1.3187758e9eb83p+0,  -0x1.054a474bf0dc3p-2,  -0x1.4bffffffffee4p-3}, //  -83 * 0x1p-9 = -0x1.4cp-3 (err =  0x1.49632089669dap-67)
    {  0x1.adfffffffffe3p-1,   0x1.30d190130d1a5p+0,  -0x1.01d9bbcfa6238p-2,  -0x1.4800000000074p-3}, //  -82 * 0x1p-9 = -0x1.48p-3 (err =  0x1.135198c00e801p-64)
    {  0x1.af00000000001p-1,   0x1.301c82ac40260p+0,  -0x1.fcd677e5ac816p-3,  -0x1.43ffffffffffcp-3}, //  -81 * 0x1p-9 = -0x1.44p-3 (err =  0x1.877b7c05197d8p-64)
    {  0x1.b000000000041p-1,   0x1.2f684bda12f3bp+0,  -0x1.f5fd8a9063c78p-3,  -0x1.3fffffffffefcp-3}, //  -80 * 0x1p-9 = -0x1.40p-3 (err =  0x1.d32f3076023d3p-66)
    {  0x1.b100000000038p-1,   0x1.2eb4ea1fed124p+0,  -0x1.ef28aacd720b3p-3,  -0x1.3bfffffffff20p-3}, //  -79 * 0x1p-9 = -0x1.3cp-3 (err =  0x1.ec030b7428946p-64)
    {  0x1.b20000000002bp-1,   0x1.2e025c04b8079p+0,  -0x1.e857d3d361243p-3,  -0x1.37fffffffff54p-3}, //  -78 * 0x1p-9 = -0x1.38p-3 (err =  0x1.02a25d687af50p-65)
    {  0x1.b2fffffffffe3p-1,   0x1.2d50a012d50b4p+0,  -0x1.e18b00e131302p-3,  -0x1.3400000000074p-3}, //  -77 * 0x1p-9 = -0x1.34p-3 (err =  0x1.22488a8f5c0eep-65)
    {  0x1.b400000000009p-1,   0x1.2c9fb4d812c9ap+0,  -0x1.dac22d3e44196p-3,  -0x1.2ffffffffffdcp-3}, //  -76 * 0x1p-9 = -0x1.30p-3 (err =  0x1.8710322e7a2a5p-64)
    {  0x1.b4ffffffffffep-1,   0x1.2bef98e5a3712p+0,  -0x1.d3fd543a4ad6ap-3,  -0x1.2c00000000008p-3}, //  -75 * 0x1p-9 = -0x1.2cp-3 (err =  0x1.09c62452fdd64p-68)
    {  0x1.b600000000132p-1,   0x1.2b404ad012a6fp+0,  -0x1.cd3c712d308f9p-3,  -0x1.27ffffffffb38p-3}, //  -74 * 0x1p-9 = -0x1.28p-3 (err =  0x1.bf9caa9261497p-64)
    {  0x1.b700000000013p-1,   0x1.2a91c92f3c0f8p+0,  -0x1.c67f7f770a5fep-3,  -0x1.23fffffffffb4p-3}, //  -73 * 0x1p-9 = -0x1.24p-3 (err =  0x1.0fbd4b90ce9d8p-65)
    {  0x1.b800000000023p-1,   0x1.29e4129e41287p+0,  -0x1.bfc67a7fff3e1p-3,  -0x1.1ffffffffff74p-3}, //  -72 * 0x1p-9 = -0x1.20p-3 (err =  0x1.0f68e4f4a2babp-65)
    {  0x1.b900000000092p-1,   0x1.293725bb80442p+0,  -0x1.b9115db83a00bp-3,  -0x1.1bffffffffdb8p-3}, //  -71 * 0x1p-9 = -0x1.1cp-3 (err =  0x1.b84fd3749d149p-64)
    {  0x1.ba00000000031p-1,   0x1.288b01288aff2p+0,  -0x1.b2602497d51fep-3,  -0x1.17fffffffff3cp-3}, //  -70 * 0x1p-9 = -0x1.18p-3 (err =  0x1.0e5f6daf01564p-67)
    {  0x1.bb00000000011p-1,   0x1.27dfa38a1ce42p+0,  -0x1.abb2ca9ec7401p-3,  -0x1.13fffffffffbcp-3}, //  -69 * 0x1p-9 = -0x1.14p-3 (err =  0x1.47fd0b4cb80c6p-64)
    {  0x1.bbfffffffffebp-1,   0x1.27350b8812743p+0,  -0x1.a5094b54d28b4p-3,  -0x1.1000000000054p-3}, //  -68 * 0x1p-9 = -0x1.10p-3 (err =  0x1.32416a42f9140p-64)
    {  0x1.bd00000000003p-1,   0x1.268b37cd60125p+0,  -0x1.9e63a24971f32p-3,  -0x1.0bffffffffff4p-3}, //  -67 * 0x1p-9 = -0x1.0cp-3 (err =  0x1.cf0c7072c5d17p-65)
    {  0x1.bdfffffffffacp-1,   0x1.25e2270809328p+0,  -0x1.97c1cb13c80edp-3,  -0x1.0800000000150p-3}, //  -66 * 0x1p-9 = -0x1.08p-3 (err =  0x1.565d70e7e3590p-65)
    {  0x1.bf00000000036p-1,   0x1.2539d7e91778fp+0,  -0x1.9123c1528c569p-3,  -0x1.03fffffffff28p-3}, //  -65 * 0x1p-9 = -0x1.04p-3 (err =  0x1.917db222fab14p-65)
    {  0x1.c000000000029p-1,   0x1.2492492492478p+0,  -0x1.8a8980abfbc24p-3,  -0x1.ffffffffffeb8p-4}, //  -64 * 0x1p-9 = -0x1.00p-3 (err =  0x1.212fda7aed870p-64)
    {  0x1.c0fffffffffb2p-1,   0x1.23eb79717608ep+0,  -0x1.83f304cdc5ca8p-3,  -0x1.f800000000270p-4}, //  -63 * 0x1p-9 = -0x1.f8p-4 (err =  0x1.2367c60bdc02bp-73)
    {  0x1.c1fffffffffd1p-1,   0x1.23456789abcfdp+0,  -0x1.7d60496cfbc81p-3,  -0x1.f000000000178p-4}, //  -62 * 0x1p-9 = -0x1.f0p-4 (err =  0x1.1a3b606ea9291p-65)
    {  0x1.c2fffffffffccp-1,   0x1.22a0122a0124cp+0,  -0x1.76d14a460137ap-3,  -0x1.e8000000001a0p-4}, //  -61 * 0x1p-9 = -0x1.e8p-4 (err =  0x1.311341ceb3699p-64)
    {  0x1.c40000000009cp-1,   0x1.21fb78121fb14p+0,  -0x1.7046031c79b89p-3,  -0x1.dfffffffffb20p-4}, //  -60 * 0x1p-9 = -0x1.e0p-4 (err =  0x1.ee6dbad0615a1p-68)
    {  0x1.c4ffffffffe3dp-1,   0x1.2157980485706p+0,  -0x1.69be6fbb3b5edp-3,  -0x1.d800000000e18p-4}, //  -59 * 0x1p-9 = -0x1.d8p-4 (err =  0x1.45af432ed0c4cp-64)
    {  0x1.c5fffffffffc7p-1,   0x1.20b470c67c0fdp+0,  -0x1.633a8bf437e54p-3,  -0x1.d0000000001c8p-4}, //  -58 * 0x1p-9 = -0x1.d0p-4 (err =  0x1.79fc4fdd73047p-65)
    {  0x1.c70000000007ep-1,   0x1.2012012011fc2p+0,  -0x1.5cba53a075fbbp-3,  -0x1.c7ffffffffc10p-4}, //  -57 * 0x1p-9 = -0x1.c8p-4 (err =  0x1.88e0a47f57bb7p-67)
    {  0x1.c7fffffffff53p-1,   0x1.1f7047dc11fddp+0,  -0x1.563dc29ffb113p-3,  -0x1.c000000000568p-4}, //  -56 * 0x1p-9 = -0x1.c0p-4 (err =  0x1.30f98b5f3566bp-64)
    {  0x1.c900000000077p-1,   0x1.1ecf43c7fb802p+0,  -0x1.4fc4d4d9bb012p-3,  -0x1.b7ffffffffc48p-4}, //  -55 * 0x1p-9 = -0x1.b8p-4 (err =  0x1.48baff8b4fd57p-65)
    {  0x1.ca0000000001ap-1,   0x1.1e2ef3b3fb864p+0,  -0x1.494f863b8de8dp-3,  -0x1.affffffffff30p-4}, //  -54 * 0x1p-9 = -0x1.b0p-4 (err =  0x1.8b636e069c156p-65)
    {  0x1.cb0000000008ap-1,   0x1.1d8f5672e4a67p+0,  -0x1.42ddd2ba1b131p-3,  -0x1.a7ffffffffbb0p-4}, //  -53 * 0x1p-9 = -0x1.a8p-4 (err =  0x1.08c813af551d4p-64)
    {  0x1.cc000000000fcp-1,   0x1.1cf06ada28081p+0,  -0x1.3c6fb650cd7fep-3,  -0x1.9fffffffff820p-4}, //  -52 * 0x1p-9 = -0x1.a0p-4 (err =  0x1.eaab4eaa1860cp-65)
    {  0x1.ccfffffffff72p-1,   0x1.1c522fc1ce0b0p+0,  -0x1.36052d01c4165p-3,  -0x1.9800000000470p-4}, //  -51 * 0x1p-9 = -0x1.98p-4 (err =  0x1.04d46461a289cp-65)
    {  0x1.ce0000000002ap-1,   0x1.1bb4a4046ed0fp+0,  -0x1.2f9e32d5bfcc4p-3,  -0x1.8fffffffffeb0p-4}, //  -50 * 0x1p-9 = -0x1.90p-4 (err =  0x1.9dfdf8671c3dcp-65)
    {  0x1.cefffffffffcap-1,   0x1.1b17c67f2bb04p+0,  -0x1.293ac3dc1a7c1p-3,  -0x1.88000000001b0p-4}, //  -49 * 0x1p-9 = -0x1.88p-4 (err =  0x1.98d14bb876153p-67)
    {  0x1.cffffffffff7dp-1,   0x1.1a7b9611a7be6p+0,  -0x1.22dadc2ab37d9p-3,  -0x1.8000000000418p-4}, //  -48 * 0x1p-9 = -0x1.80p-4 (err =  0x1.0de1488b18035p-67)
    {  0x1.d1000000000abp-1,   0x1.19e0119e01136p+0,  -0x1.1c7e77dde2f9dp-3,  -0x1.77ffffffffaa8p-4}, //  -47 * 0x1p-9 = -0x1.78p-4 (err =  0x1.8b0240fe20e9ep-65)
    {  0x1.d20000000000ep-1,   0x1.19453808ca294p+0,  -0x1.162593186da17p-3,  -0x1.6ffffffffff90p-4}, //  -46 * 0x1p-9 = -0x1.70p-4 (err =  0x1.7e5c12368f0c4p-69)
    {  0x1.d2fffffffff91p-1,   0x1.18ab083902c1dp+0,  -0x1.0fd02a0372aa8p-3,  -0x1.6800000000378p-4}, //  -45 * 0x1p-9 = -0x1.68p-4 (err =  0x1.df6a31f6481c5p-65)
    {  0x1.d400000000328p-1,   0x1.181181181162ep+0,  -0x1.097e38ce5f25cp-3,  -0x1.5ffffffffe6c0p-4}, //  -44 * 0x1p-9 = -0x1.60p-4 (err =  0x1.f0699af012423p-65)
    {  0x1.d4ffffffffe8ep-1,   0x1.1778a191bd761p+0,  -0x1.032fbbaee7680p-3,  -0x1.5800000000b90p-4}, //  -43 * 0x1p-9 = -0x1.58p-4 (err =  0x1.1ff4d665ba58dp-65)
    {  0x1.d600000000081p-1,   0x1.16e068942732cp+0,  -0x1.f9c95dc1d0b0fp-4,  -0x1.4fffffffffbf8p-4}, //  -42 * 0x1p-9 = -0x1.50p-4 (err =  0x1.43a604bd161e3p-65)
    {  0x1.d700000000003p-1,   0x1.1648d50fc31ffp+0,  -0x1.ed3a1d4cdbe95p-4,  -0x1.47fffffffffe8p-4}, //  -41 * 0x1p-9 = -0x1.48p-4 (err =  0x1.1de9b9afa54eep-65)
    {  0x1.d800000000014p-1,   0x1.15b1e5f752701p+0,  -0x1.e0b1ae8f2fc5cp-4,  -0x1.3ffffffffff60p-4}, //  -40 * 0x1p-9 = -0x1.40p-4 (err =  0x1.94bc23146ca9ap-69)
    {  0x1.d900000000005p-1,   0x1.151b9a3fdd5c6p+0,  -0x1.d4300a2524d03p-4,  -0x1.37fffffffffd8p-4}, //  -39 * 0x1p-9 = -0x1.38p-4 (err =  0x1.2a6e5eea4d4c2p-65)
    {  0x1.d9fffffffff88p-1,   0x1.1485f0e0acd81p+0,  -0x1.c7b528b70f79dp-4,  -0x1.30000000003c0p-4}, //  -38 * 0x1p-9 = -0x1.30p-4 (err =  0x1.8ffcb7c4470e1p-65)
    {  0x1.db0000000001dp-1,   0x1.13f0e8d344713p+0,  -0x1.bb4102f92522bp-4,  -0x1.27fffffffff18p-4}, //  -37 * 0x1p-9 = -0x1.28p-4 (err =  0x1.0c9b3c9152a78p-65)
    {  0x1.dc00000000008p-1,   0x1.135c81135c80dp+0,  -0x1.aed391ab666ebp-4,  -0x1.1ffffffffffc0p-4}, //  -36 * 0x1p-9 = -0x1.20p-4 (err =  0x1.860f7c01ea754p-65)
    {  0x1.dd00000000028p-1,   0x1.12c8b89edc095p+0,  -0x1.a26ccd9981663p-4,  -0x1.17ffffffffec0p-4}, //  -35 * 0x1p-9 = -0x1.18p-4 (err =  0x1.a6e47661a6100p-68)
    {  0x1.de00000000092p-1,   0x1.12358e75d2fe0p+0,  -0x1.960caf9abb0bdp-4,  -0x1.0fffffffffb70p-4}, //  -34 * 0x1p-9 = -0x1.10p-4 (err =  0x1.937b25c743172p-65)
    {  0x1.df00000000018p-1,   0x1.11a3019a74819p+0,  -0x1.89b33091d6ec0p-4,  -0x1.07fffffffff40p-4}, //  -33 * 0x1p-9 = -0x1.08p-4 (err =  0x1.e281ca372c86bp-67)
    {  0x1.dffffffffffeep-1,   0x1.111111111111bp+0,  -0x1.7d60496cfbc2ap-4,  -0x1.0000000000090p-4}, //  -32 * 0x1p-9 = -0x1.00p-4 (err =  0x1.3c413523cb76dp-66)
    {  0x1.e0fffffffffffp-1,   0x1.107fbbe011080p+0,  -0x1.7113f3259e086p-4,  -0x1.f000000000010p-5}, //  -31 * 0x1p-9 = -0x1.f0p-5 (err =  0x1.4752fd51b8b5bp-67)
    {  0x1.e1fffffffffc9p-1,   0x1.0fef010fef030p+0,  -0x1.64ce26c0673f9p-4,  -0x1.e000000000370p-5}, //  -30 * 0x1p-9 = -0x1.e0p-5 (err =  0x1.43179f797f3f1p-67)
    {  0x1.e2fffffffffb5p-1,   0x1.0f5edfab325ccp+0,  -0x1.588edd4d1d240p-4,  -0x1.d0000000004b0p-5}, //  -29 * 0x1p-9 = -0x1.d0p-5 (err =  0x1.2b74666d4c46dp-67)
    {  0x1.e3fffffffff84p-1,   0x1.0ecf56be69cd5p+0,  -0x1.4c560fe68b572p-4,  -0x1.c0000000007c0p-5}, //  -28 * 0x1p-9 = -0x1.c0p-5 (err =  0x1.30af689b73933p-65)
    {  0x1.e5000000000d8p-1,   0x1.0e40655825f99p+0,  -0x1.4023b7b26a256p-4,  -0x1.afffffffff280p-5}, //  -27 * 0x1p-9 = -0x1.b0p-5 (err =  0x1.31e69ca1d6761p-65)
    {  0x1.e600000000025p-1,   0x1.0db20a88f4681p+0,  -0x1.33f7cde14cd98p-4,  -0x1.9fffffffffdb0p-5}, //  -26 * 0x1p-9 = -0x1.a0p-5 (err =  0x1.138f0161ac734p-66)
    {  0x1.e6fffffffff69p-1,   0x1.0d24456359e8dp+0,  -0x1.27d24bae82c03p-4,  -0x1.9000000000970p-5}, //  -25 * 0x1p-9 = -0x1.90p-5 (err =  0x1.be05a74f54f3bp-66)
    {  0x1.e800000000093p-1,   0x1.0c9714fbcd9eap+0,  -0x1.1bb32a6004da9p-4,  -0x1.7fffffffff6d0p-5}, //  -24 * 0x1p-9 = -0x1.80p-5 (err =  0x1.7c7720930e440p-67)
    {  0x1.e900000000295p-1,   0x1.0c0a7868b4006p+0,  -0x1.0f9a634661ba9p-4,  -0x1.6ffffffffd6b0p-5}, //  -23 * 0x1p-9 = -0x1.70p-5 (err =  0x1.0830a45dc5461p-65)
    {  0x1.e9fffffffffe9p-1,   0x1.0b7e6ec259dd4p+0,  -0x1.0387efbca87b3p-4,  -0x1.6000000000170p-5}, //  -22 * 0x1p-9 = -0x1.60p-5 (err =  0x1.636f60ad191a5p-66)
    {  0x1.eafffffffffe9p-1,   0x1.0af2f722eecc2p+0,  -0x1.eef792508b8c7p-5,  -0x1.5000000000170p-5}, //  -21 * 0x1p-9 = -0x1.50p-5 (err =  0x1.7d8f952278f5fp-67)
    {  0x1.ec0000000005ep-1,   0x1.0a6810a681074p+0,  -0x1.d6ebd1f1fe32cp-5,  -0x1.3fffffffffa20p-5}, //  -20 * 0x1p-9 = -0x1.40p-5 (err =  0x1.b554354e012a3p-70)
    {  0x1.ed00000000019p-1,   0x1.09ddba6af8353p+0,  -0x1.beec9151aa9d7p-5,  -0x1.2fffffffffe70p-5}, //  -19 * 0x1p-9 = -0x1.30p-5 (err =  0x1.8960ee0078590p-68)
    {  0x1.ee00000000005p-1,   0x1.0953f39010951p+0,  -0x1.a6f9c377dd2a3p-5,  -0x1.1ffffffffffb0p-5}, //  -18 * 0x1p-9 = -0x1.20p-5 (err =  0x1.19ddd81d92af3p-66)
    {  0x1.eeffffffffff3p-1,   0x1.08cabb37565e9p+0,  -0x1.8f135b8107a48p-5,  -0x1.10000000000d0p-5}, //  -17 * 0x1p-9 = -0x1.10p-5 (err =  0x1.4b0e68e72d91fp-68)
    {  0x1.effffffffffbep-1,   0x1.08421084210a7p+0,  -0x1.77394c9d95efap-5,  -0x1.0000000000420p-5}, //  -16 * 0x1p-9 = -0x1.00p-5 (err =  0x1.4d13618863155p-66)
    {  0x1.f100000000036p-1,   0x1.07b9f29b8eac5p+0,  -0x1.5f6b8a11c375dp-5,  -0x1.dfffffffff940p-6}, //  -15 * 0x1p-9 = -0x1.e0p-6 (err =  0x1.04715825a2e50p-66)
    {  0x1.f20000000004ap-1,   0x1.073260a47f79fp+0,  -0x1.47aa073576973p-5,  -0x1.bfffffffff6c0p-6}, //  -14 * 0x1p-9 = -0x1.c0p-6 (err =  0x1.17bf70e3155fcp-66)
    {  0x1.f2fffffffffafp-1,   0x1.06ab59c791326p+0,  -0x1.2ff4b77414549p-5,  -0x1.a000000000a20p-6}, //  -13 * 0x1p-9 = -0x1.a0p-6 (err =  0x1.f07431c877d79p-67)
    {  0x1.f3fffffffffe4p-1,   0x1.0624dd2f1aa0bp+0,  -0x1.184b8e4c56d8ep-5,  -0x1.8000000000380p-6}, //  -12 * 0x1p-9 = -0x1.80p-6 (err =  0x1.fece2a5e8c90ap-72)
    {  0x1.f4fffffffffc4p-1,   0x1.059eea07275a6p+0,  -0x1.00ae7f502c74bp-5,  -0x1.6000000000780p-6}, //  -11 * 0x1p-9 = -0x1.60p-6 (err =  0x1.d8fc24a4c3884p-67)
    {  0x1.f60000000002ap-1,   0x1.05197f7d733eep+0,  -0x1.d23afc491323fp-6,  -0x1.3fffffffffac0p-6}, //  -10 * 0x1p-9 = -0x1.40p-6 (err =  0x1.5cae9ad093f16p-68)
    {  0x1.f70000000002dp-1,   0x1.04949cc1664aep+0,  -0x1.a330fd028ef1cp-6,  -0x1.1fffffffffa60p-6}, //   -9 * 0x1p-9 = -0x1.20p-6 (err =  0x1.61d3e38db5d55p-67)
    {  0x1.f7ffffffffff4p-1,   0x1.0410410410416p+0,  -0x1.743ee861f3789p-6,  -0x1.0000000000180p-6}, //   -8 * 0x1p-9 = -0x1.00p-6 (err =  0x1.6b179d900ee23p-69)
    {  0x1.f90000000003cp-1,   0x1.038c6b78247ddp+0,  -0x1.4564a62191d3cp-6,  -0x1.bfffffffff100p-7}, //   -7 * 0x1p-9 = -0x1.c0p-7 (err =  0x1.fbf65fd4707b1p-68)
    {  0x1.f9fffffffffb3p-1,   0x1.03091b51f5e42p+0,  -0x1.16a21e20a1852p-6,  -0x1.8000000001340p-7}, //   -6 * 0x1p-9 = -0x1.80p-7 (err =  0x1.a213aa38d460ep-68)
    {  0x1.fb00000000054p-1,   0x1.02864fc7729bep+0,  -0x1.cfee70c5cc744p-7,  -0x1.3ffffffffeb00p-7}, //   -5 * 0x1p-9 = -0x1.40p-7 (err =  0x1.a4bacdadb9a7bp-68)
    {  0x1.fc00000000006p-1,   0x1.0204081020405p+0,  -0x1.72c7ba20f70f9p-7,  -0x1.ffffffffffd00p-8}, //   -4 * 0x1p-9 = -0x1.00p-7 (err =  0x1.668c6ed048cfap-72)
    {  0x1.fd00000000030p-1,   0x1.0182436517a1fp+0,  -0x1.15cfe8eaeb6c6p-7,  -0x1.7ffffffffe800p-8}, //   -3 * 0x1p-9 = -0x1.80p-8 (err =  0x1.53822ca4cbf5ap-69)
    {  0x1.fdfffffffffc4p-1,   0x1.010101010102ep+0,  -0x1.720d9c06aaed2p-8,  -0x1.0000000001e00p-8}, //   -2 * 0x1p-9 = -0x1.00p-8 (err =  0x1.300bbdda8b43fp-71)
    {  0x1.fefffffffff91p-1,   0x1.00804020100b8p+0,  -0x1.71b0ea42f004ep-9,  -0x1.0000000006f00p-9}, //   -1 * 0x1p-9 = -0x1.00p-9 (err =  0x1.4f9225e94fde8p-70)
    {  0x1.0000000000000p+0,   0x1.0000000000000p+0,   0x0.0000000000000p+0,   0x0.0000000000000p+0}, //    0 * 0x1p-9 = 0x0.00p+0 (err =   0x0.0000000000000p+0)
};// Worst deviation:  ap1 =   0x1.28ffffffffc68p+0, log1pa =   0x1.b6ecf175f7229p-3, a =   0x1.47fffffffe340p-3,  err =  0x1.983ae183bb5edp-64, d = 0x1.ccp-43

static const logdata_t LOG10_LOOKUP[] = { //ap1, va, log1pa, a: va = 1/(a+1), log1pa = log10(1+a)+err
    {  0x1.0000000000000p+0,   0x1.0000000000000p+0,   0x0.0000000000000p+0,   0x0.0000000000000p+0}, //    0 * 0x1p-8 = 0x0.00p+0 (err =   0x0.0000000000000p+0)
    {  0x1.00fffffffff99p+0,   0x1.fe01fe01fe0ecp-1,  0x1.bbd9e9481fccdp-10,   0x1.fffffffff3200p-9}, //    1 * 0x1p-8 = 0x1.00p-8 (err =  0x1.1ad8beb52b7a8p-73)
    {  0x1.0200000000061p+0,   0x1.fc07f01fc0731p-1,   0x1.bafd4722240c0p-9,   0x1.0000000003080p-7}, //    2 * 0x1p-8 = 0x1.00p-7 (err =  0x1.67c0c24174fe1p-71)
    {  0x1.0300000000013p+0,   0x1.fa11caa01f9edp-1,   0x1.4b99563d2a9e5p-8,   0x1.8000000000980p-7}, //    3 * 0x1p-8 = 0x1.80p-7 (err =  0x1.24d433242d5ebp-69)
    {  0x1.03fffffffffebp+0,   0x1.f81f81f81f848p-1,   0x1.b9476a4fcc814p-8,   0x1.ffffffffff580p-7}, //    4 * 0x1p-8 = 0x1.00p-6 (err =  0x1.8e6ab7fefbfe2p-70)
    {  0x1.0500000000010p+0,   0x1.f6310aca0db97p-1,   0x1.1344daa2d78bbp-7,   0x1.4000000000400p-6}, //    5 * 0x1p-8 = 0x1.40p-6 (err =  0x1.0d9573d90df77p-68)
    {  0x1.06000000000a8p+0,   0x1.f44659e4a4131p-1,   0x1.49b0851445a29p-7,   0x1.8000000002a00p-6}, //    6 * 0x1p-8 = 0x1.80p-6 (err =  0x1.e60951e4f2731p-69)
    {  0x1.06fffffffffc6p+0,   0x1.f25f644230b23p-1,   0x1.7fe71ccc4da6ep-7,   0x1.bfffffffff180p-6}, //    7 * 0x1p-8 = 0x1.c0p-6 (err =  0x1.098fbc4f50a3fp-69)
    {  0x1.07fffffffffe1p+0,   0x1.f07c1f07c1f42p-1,   0x1.b5e908eb13109p-7,   0x1.ffffffffff840p-6}, //    8 * 0x1p-8 = 0x1.00p-5 (err =  0x1.0b518ab12e917p-69)
    {  0x1.08ffffffffffap+0,   0x1.ee9c7f8458e0dp-1,   0x1.ebb6af653e1acp-7,   0x1.1ffffffffff40p-5}, //    9 * 0x1p-8 = 0x1.20p-5 (err =  0x1.9ead1c2905d47p-68)
    {  0x1.09ffffffff993p+0,   0x1.ecc07b301f8a8p-1,   0x1.10a83a843c094p-6,   0x1.3ffffffff3260p-5}, //   10 * 0x1p-8 = 0x1.40p-5 (err =  0x1.fbe8c11089595p-68)
    {  0x1.0afffffffffcap+0,   0x1.eae807aba024ep-1,   0x1.2b5b5ec020434p-6,   0x1.5fffffffff940p-5}, //   11 * 0x1p-8 = 0x1.60p-5 (err =  0x1.1dc149d7af7cdp-68)
    {  0x1.0bfffffffffc5p+0,   0x1.e9131abf0b7d3p-1,   0x1.45f4f5acb85c2p-6,   0x1.7fffffffff8a0p-5}, //   12 * 0x1p-8 = 0x1.80p-5 (err =  0x1.5bce44135cb65p-68)
    {  0x1.0d0000000003bp+0,   0x1.e741aa597507ap-1,   0x1.60753003a9b08p-6,   0x1.a000000000760p-5}, //   13 * 0x1p-8 = 0x1.a0p-5 (err =  0x1.4b314aedce1ebp-67)
    {  0x1.0e0000000000bp+0,   0x1.e573ac901e560p-1,   0x1.7adc3df3b211ap-6,   0x1.c000000000160p-5}, //   14 * 0x1p-8 = 0x1.c0p-5 (err =  0x1.2758dc728214dp-67)
    {  0x1.0f000000000b0p+0,   0x1.e3a9179dc1939p-1,   0x1.952a4f22c6cf6p-6,   0x1.e000000001600p-5}, //   15 * 0x1p-8 = 0x1.e0p-5 (err =  0x1.42054d0ae39d9p-67)
    {  0x1.1000000000047p+0,   0x1.e1e1e1e1e1da0p-1,   0x1.af5f92b00ed51p-6,   0x1.0000000000470p-4}, //   16 * 0x1p-8 = 0x1.00p-4 (err =  0x1.a11252b83cfd4p-68)
    {  0x1.110000000002bp+0,   0x1.e01e01e01dfd2p-1,   0x1.c97c3735e806bp-6,   0x1.10000000002b0p-4}, //   17 * 0x1p-8 = 0x1.10p-4 (err =  0x1.15d5e3122142ap-68)
    {  0x1.1200000000042p+0,   0x1.de5d6e3f88617p-1,   0x1.e3806acbd0c41p-6,   0x1.2000000000420p-4}, //   18 * 0x1p-8 = 0x1.20p-4 (err =  0x1.2ae351f766e10p-69)
    {  0x1.130000000002cp+0,   0x1.dca01dca01d7ep-1,   0x1.fd6c5b08520bep-6,   0x1.30000000002c0p-4}, //   19 * 0x1p-8 = 0x1.30p-4 (err =  0x1.7f380f8198361p-70)
    {  0x1.140000000006fp+0,   0x1.dae6076b9811cp-1,   0x1.0ba01a8170597p-5,   0x1.40000000006f0p-4}, //   20 * 0x1p-8 = 0x1.40p-4 (err =  0x1.dcddae49bae3bp-67)
    {  0x1.1500000000029p+0,   0x1.d92f2231e7f44p-1,   0x1.187e12aad8286p-5,   0x1.5000000000290p-4}, //   21 * 0x1p-8 = 0x1.50p-4 (err =  0x1.dc02801a6853ep-67)
    {  0x1.160000000002ap+0,   0x1.d77b654b82becp-1,   0x1.25502c0fc3365p-5,   0x1.60000000002a0p-4}, //   22 * 0x1p-8 = 0x1.60p-4 (err =  0x1.a3a49739a4af9p-68)
    {  0x1.16fffffffffc2p+0,   0x1.d5cac8075731ap-1,   0x1.32167c82bd9c3p-5,   0x1.6fffffffffc20p-4}, //   23 * 0x1p-8 = 0x1.70p-4 (err =  0x1.2862509d56158p-66)
    {  0x1.17fffffffffdep+0,   0x1.d41d41d41d456p-1,   0x1.3ed1199a5e275p-5,   0x1.7fffffffffde0p-4}, //   24 * 0x1p-8 = 0x1.80p-4 (err =  0x1.1cc5792252ec4p-66)
    {  0x1.18fffffffff12p+0,   0x1.d272ca3fc5ca5p-1,   0x1.4b8018b21e18ap-5,   0x1.8fffffffff120p-4}, //   25 * 0x1p-8 = 0x1.90p-4 (err =  0x1.a245cd1a9bce9p-67)
    {  0x1.19fffffffffe2p+0,   0x1.d0cb58f6ec0a6p-1,   0x1.58238eeb35260p-5,   0x1.9fffffffffe20p-4}, //   26 * 0x1p-8 = 0x1.a0p-4 (err =  0x1.375223c1ad876p-67)
    {  0x1.1b000000000d0p+0,   0x1.cf26e5c44be72p-1,   0x1.64bb912d6663ep-5,   0x1.b000000000d00p-4}, //   27 * 0x1p-8 = 0x1.b0p-4 (err =  0x1.5202edd4be640p-66)
    {  0x1.1c0000000009ap+0,   0x1.cd85689039a11p-1,   0x1.71483427d3222p-5,   0x1.c0000000009a0p-4}, //   28 * 0x1p-8 = 0x1.c0p-4 (err =  0x1.3afb998bf2418p-66)
    {  0x1.1d0000000008ap+0,   0x1.cbe6d9601cb08p-1,   0x1.7dc98c51c88fdp-5,   0x1.d0000000008a0p-4}, //   29 * 0x1p-8 = 0x1.d0p-4 (err =  0x1.6a33dec573444p-66)
    {  0x1.1e00000000033p+0,   0x1.ca4b3055ee13fp-1,   0x1.8a3fadeb84a6ep-5,   0x1.e000000000330p-4}, //   30 * 0x1p-8 = 0x1.e0p-4 (err =  0x1.cad99a8ea0705p-68)
    {  0x1.1efffffffffb5p+0,   0x1.c8b265afb8ab9p-1,   0x1.96aaacfefcb9ap-5,   0x1.efffffffffb50p-4}, //   31 * 0x1p-8 = 0x1.f0p-4 (err =  0x1.007fd00173e90p-67)
    {  0x1.200000000000cp+0,   0x1.c71c71c71c709p-1,   0x1.a30a9d609f07ep-5,   0x1.0000000000060p-3}, //   32 * 0x1p-8 = 0x1.00p-3 (err =  0x1.0d31619d0d5f7p-67)
    {  0x1.210000000000ep+0,   0x1.c5894d10d4970p-1,   0x1.af5f92b00e6bcp-5,   0x1.0800000000070p-3}, //   33 * 0x1p-8 = 0x1.08p-3 (err =  0x1.d1ff63ba638d0p-68)
    {  0x1.2200000000016p+0,   0x1.c3f8f01c3f8cep-1,   0x1.bba9a058dfe92p-5,   0x1.10000000000b0p-3}, //   34 * 0x1p-8 = 0x1.10p-3 (err =  0x1.74fc515be53dbp-67)
    {  0x1.230000000000ap+0,   0x1.c26b5392ea00dp-1,   0x1.c7e8d99350a73p-5,   0x1.1800000000050p-3}, //   35 * 0x1p-8 = 0x1.18p-3 (err =  0x1.0db091ec28decp-66)
    {  0x1.23fffffffffdcp+0,   0x1.c0e070381c118p-1,   0x1.d41d5164faafdp-5,   0x1.1fffffffffee0p-3}, //   36 * 0x1p-8 = 0x1.20p-3 (err =  0x1.41f838651daa7p-70)
    {  0x1.24fffffffffbdp+0,   0x1.bf583ee868df1p-1,   0x1.e0471aa1865c7p-5,   0x1.27ffffffffde8p-3}, //   37 * 0x1p-8 = 0x1.28p-3 (err =  0x1.5242e762beabbp-66)
    {  0x1.2600000000011p+0,   0x1.bdd2b899406dep-1,   0x1.ec6647eb588d6p-5,   0x1.3000000000088p-3}, //   38 * 0x1p-8 = 0x1.30p-3 (err =  0x1.a72e8a3c0943ep-70)
    {  0x1.26fffffffffe1p+0,   0x1.bc4fd65883eaap-1,   0x1.f87aebb43cc90p-5,   0x1.37fffffffff08p-3}, //   39 * 0x1p-8 = 0x1.38p-3 (err =  0x1.cfc181130dcc4p-68)
    {  0x1.2800000000070p+0,   0x1.bacf914c1ba28p-1,   0x1.02428c1f082b7p-4,   0x1.4000000000380p-3}, //   40 * 0x1p-8 = 0x1.40p-3 (err =  0x1.c4e24fa7846cbp-66)
    {  0x1.29000000000b3p+0,   0x1.b951e2b18fe19p-1,   0x1.08426fcdb2317p-4,   0x1.4800000000598p-3}, //   41 * 0x1p-8 = 0x1.48p-3 (err =  0x1.398bc80571f26p-66)
    {  0x1.29fffffffff8ap+0,   0x1.b7d6c3dda3439p-1,   0x1.0e3d29d81139ep-4,   0x1.4fffffffffc50p-3}, //   42 * 0x1p-8 = 0x1.50p-3 (err =  0x1.7c1d45ba33251p-67)
    {  0x1.2afffffffff73p+0,   0x1.b65e2e3beeed4p-1,   0x1.1432c319179c1p-4,   0x1.57ffffffffb98p-3}, //   43 * 0x1p-8 = 0x1.58p-3 (err =  0x1.5f9c1b0a1b192p-66)
    {  0x1.2c000000001d1p+0,   0x1.b4e81b4e818a9p-1,   0x1.1a234455022dbp-4,   0x1.6000000000e88p-3}, //   44 * 0x1p-8 = 0x1.60p-3 (err =  0x1.fece1a4df952fp-66)
    {  0x1.2cffffffffff0p+0,   0x1.b37484ad806e5p-1,   0x1.200eb639a3114p-4,   0x1.67fffffffff80p-3}, //   45 * 0x1p-8 = 0x1.68p-3 (err =  0x1.c7ac829b610e9p-66)
    {  0x1.2e0000000005ap+0,   0x1.b2036406c8058p-1,   0x1.25f5215eb5b5cp-4,   0x1.70000000002d0p-3}, //   46 * 0x1p-8 = 0x1.70p-3 (err =  0x1.69a1ab681e82dp-66)
    {  0x1.2efffffffff8fp+0,   0x1.b094b31d92345p-1,   0x1.2bd68e46210dap-4,   0x1.77ffffffffc78p-3}, //   47 * 0x1p-8 = 0x1.78p-3 (err =  0x1.6ec632e6984c2p-66)
    {  0x1.300000000001bp+0,   0x1.af286bca1af02p-1,   0x1.31b3055c471b6p-4,   0x1.80000000000d8p-3}, //   48 * 0x1p-8 = 0x1.80p-3 (err =  0x1.9f4b540a21a82p-66)
    {  0x1.30fffffffffb9p+0,   0x1.adbe87f9490c2p-1,   0x1.378a8ef849580p-4,   0x1.87ffffffffdc8p-3}, //   49 * 0x1p-8 = 0x1.88p-3 (err =  0x1.ed1b33455a401p-66)
    {  0x1.32000000000b5p+0,   0x1.ac5701ac56f1dp-1,   0x1.3d5d335c53595p-4,   0x1.90000000005a8p-3}, //   50 * 0x1p-8 = 0x1.90p-3 (err =  0x1.95800b6b3df9ep-68)
    {  0x1.32fffffffffacp+0,   0x1.aaf1d2f87ec71p-1,   0x1.432afab5dd218p-4,   0x1.97ffffffffd60p-3}, //   51 * 0x1p-8 = 0x1.98p-3 (err =  0x1.849a7a421b087p-66)
    {  0x1.340000000000fp+0,   0x1.a98ef606a63a9p-1,   0x1.48f3ed1df4952p-4,   0x1.a000000000078p-3}, //   52 * 0x1p-8 = 0x1.a0p-3 (err =  0x1.3c017d30fe341p-67)
    {  0x1.34fffffffffeap+0,   0x1.a82e65130e177p-1,   0x1.4eb812997cd65p-4,   0x1.a7fffffffff50p-3}, //   53 * 0x1p-8 = 0x1.a8p-3 (err =  0x1.f09b529cf6f77p-68)
    {  0x1.35fffffffffd8p+0,   0x1.a6d01a6d01aa4p-1,   0x1.5477731973d9fp-4,   0x1.afffffffffec0p-3}, //   54 * 0x1p-8 = 0x1.b0p-3 (err =  0x1.11673e6fcbab6p-65)
    {  0x1.36fffffffffddp+0,   0x1.a574107688a7ap-1,   0x1.5a32167b32e3ap-4,   0x1.b7ffffffffee8p-3}, //   55 * 0x1p-8 = 0x1.b8p-3 (err =  0x1.6e3d91a8051ccp-67)
    {  0x1.37fffffffff97p+0,   0x1.a41a41a41a4a8p-1,   0x1.5fe80488af2a6p-4,   0x1.bfffffffffcb8p-3}, //   56 * 0x1p-8 = 0x1.c0p-3 (err =  0x1.45cddc084a48fp-65)
    {  0x1.38fffffffffdcp+0,   0x1.a2c2a87c51cd0p-1,   0x1.659944f8b9f60p-4,   0x1.c7ffffffffee0p-3}, //   57 * 0x1p-8 = 0x1.c8p-3 (err =  0x1.3b74ed8848a48p-65)
    {  0x1.3a0000000000ap+0,   0x1.a16d3f97a4af4p-1,   0x1.6b45df6f3e302p-4,   0x1.d000000000050p-3}, //   58 * 0x1p-8 = 0x1.d0p-3 (err =  0x1.bd0560e753ec8p-69)
    {  0x1.3b0000000009dp+0,   0x1.a01a01a019f4bp-1,   0x1.70eddb7d7ed7ep-4,   0x1.d8000000004e8p-3}, //   59 * 0x1p-8 = 0x1.d8p-3 (err =  0x1.e6a30759e708bp-66)
    {  0x1.3c00000000060p+0,   0x1.9ec8e9510335bp-1,   0x1.769140a252919p-4,   0x1.e000000000300p-3}, //   60 * 0x1p-8 = 0x1.e0p-3 (err =  0x1.05bae11249697p-65)
    {  0x1.3d00000000031p+0,   0x1.9d79f176b67edp-1,   0x1.7c30164a60949p-4,   0x1.e800000000188p-3}, //   61 * 0x1p-8 = 0x1.e8p-3 (err =  0x1.2a3217aa50a28p-67)
    {  0x1.3dfffffffffd0p+0,   0x1.9c2d14ee4a140p-1,   0x1.81ca63d05a33dp-4,   0x1.efffffffffe80p-3}, //   62 * 0x1p-8 = 0x1.f0p-3 (err =  0x1.b61138eb0b3d3p-67)
    {  0x1.3efffffffff9fp+0,   0x1.9ae24ea551157p-1,   0x1.8760307d3538ep-4,   0x1.f7ffffffffcf8p-3}, //   63 * 0x1p-8 = 0x1.f8p-3 (err =  0x1.1680b1e3f074bp-65)
    {  0x1.4000000000053p+0,   0x1.999999999992fp-1,   0x1.8cf18388649dap-4,   0x1.000000000014cp-2}, //   64 * 0x1p-8 = 0x1.00p-2 (err =  0x1.1eef2c3db549cp-67)
    {  0x1.40ffffffffb9ap+0,   0x1.9852f0d8ec698p-1,   0x1.927e64180df30p-4,   0x1.03fffffffee68p-2}, //   65 * 0x1p-8 = 0x1.04p-2 (err =  0x1.908ba78a4fc49p-65)
    {  0x1.41fffffffff9cp+0,   0x1.970e4f80cb8f1p-1,   0x1.9806d94149fe1p-4,   0x1.07ffffffffe70p-2}, //   66 * 0x1p-8 = 0x1.08p-2 (err =  0x1.4ab17110f4a66p-66)
    {  0x1.42fffffffffc7p+0,   0x1.95cbb0be377f5p-1,   0x1.9d8aea084a962p-4,   0x1.0bfffffffff1cp-2}, //   67 * 0x1p-8 = 0x1.0cp-2 (err =  0x1.5f8677481eac0p-66)
    {  0x1.440000000001bp+0,   0x1.948b0fcd6e9bfp-1,   0x1.a30a9d609f07ep-4,   0x1.100000000006cp-2}, //   68 * 0x1p-8 = 0x1.10p-2 (err =  0x1.0d31619dd3067p-66)
    {  0x1.4500000000014p+0,   0x1.934c67f9b2ccdp-1,   0x1.a885fa2d6158bp-4,   0x1.1400000000050p-2}, //   69 * 0x1p-8 = 0x1.14p-2 (err =  0x1.7d1751f6cfc6bp-65)
    {  0x1.45fffffffffafp+0,   0x1.920fb49d0e28dp-1,   0x1.adfd07416bc4dp-4,   0x1.17ffffffffebcp-2}, //   70 * 0x1p-8 = 0x1.18p-2 (err =  0x1.caa7a368c0837p-67)
    {  0x1.46ffffffffe48p+0,   0x1.90d4f120192f0p-1,   0x1.b36fcb5f8b530p-4,   0x1.1bffffffff920p-2}, //   71 * 0x1p-8 = 0x1.1cp-2 (err =  0x1.87e59d56dd922p-65)
    {  0x1.47ffffffffff9p+0,   0x1.8f9c18f9c1902p-1,   0x1.b8de4d3ab3d72p-4,   0x1.1ffffffffffe4p-2}, //   72 * 0x1p-8 = 0x1.20p-2 (err =  0x1.b00394fa9ca03p-65)
    {  0x1.48fffffffffc6p+0,   0x1.8e6527af13785p-1,   0x1.be4893762cabdp-4,   0x1.23fffffffff18p-2}, //   73 * 0x1p-8 = 0x1.24p-2 (err =  0x1.a049533787c07p-66)
    {  0x1.4a00000000041p+0,   0x1.8d3018d301885p-1,   0x1.c3aea4a5c705dp-4,   0x1.2800000000104p-2}, //   74 * 0x1p-8 = 0x1.28p-2 (err =  0x1.6b8bfded7cf4cp-65)
    {  0x1.4b00000000037p+0,   0x1.8bfce8062fef8p-1,   0x1.c910874e0a0c0p-4,   0x1.2c000000000dcp-2}, //   75 * 0x1p-8 = 0x1.2cp-2 (err =  0x1.5b967527add39p-65)
    {  0x1.4bfffffffff96p+0,   0x1.8acb90f6bf428p-1,   0x1.ce6e41e463b6dp-4,   0x1.2fffffffffe58p-2}, //   76 * 0x1p-8 = 0x1.30p-2 (err =  0x1.5d36191958f79p-65)
    {  0x1.4d00000000018p+0,   0x1.899c0f6018980p-1,   0x1.d3c7dacf5788bp-4,   0x1.3400000000060p-2}, //   77 * 0x1p-8 = 0x1.34p-2 (err =  0x1.42d0a34ebf643p-65)
    {  0x1.4e0000000001dp+0,   0x1.886e5f0abb028p-1,   0x1.d91d5866aaa36p-4,   0x1.3800000000074p-2}, //   78 * 0x1p-8 = 0x1.38p-2 (err =  0x1.266fe0e72ef77p-66)
    {  0x1.4effffffffff4p+0,   0x1.87427bcc092c7p-1,   0x1.de6ec0f392ac5p-4,   0x1.3bfffffffffd0p-2}, //   79 * 0x1p-8 = 0x1.3cp-2 (err =  0x1.03a942da75869p-65)
    {  0x1.4fffffffffff5p+0,   0x1.8618618618625p-1,   0x1.e3bc1ab0e19c4p-4,   0x1.3ffffffffffd4p-2}, //   80 * 0x1p-8 = 0x1.40p-2 (err =  0x1.6b9a7ecb9d4bap-65)
    {  0x1.5100000000025p+0,   0x1.84f00c27805e9p-1,   0x1.e9056bcb316abp-4,   0x1.4400000000094p-2}, //   81 * 0x1p-8 = 0x1.44p-2 (err =  0x1.94596b735b2e2p-65)
    {  0x1.5200000000052p+0,   0x1.83c977ab2be7fp-1,   0x1.ee4aba610f3b4p-4,   0x1.4800000000148p-2}, //   82 * 0x1p-8 = 0x1.48p-2 (err =  0x1.d2f245b50ba45p-66)
    {  0x1.52fffffffffe5p+0,   0x1.82a4a0182a4bfp-1,   0x1.f38c0c8325cf8p-4,   0x1.4bfffffffff94p-2}, //   83 * 0x1p-8 = 0x1.4cp-2 (err =  0x1.5436e5ee141a2p-65)
    {  0x1.540000000000fp+0,   0x1.8181818181807p-1,   0x1.f8c96834681dfp-4,   0x1.500000000003cp-2}, //   84 * 0x1p-8 = 0x1.50p-2 (err =  0x1.6b6bf518eca29p-66)
    {  0x1.550000000006dp+0,   0x1.806018060178bp-1,   0x1.fe02d36a397a6p-4,   0x1.54000000001b4p-2}, //   85 * 0x1p-8 = 0x1.54p-2 (err =  0x1.52510c8fc1f06p-65)
    {  0x1.55fffffffffc0p+0,   0x1.7f405fd017f88p-1,   0x1.019c2a064b3e0p-3,   0x1.57fffffffff00p-2}, //   86 * 0x1p-8 = 0x1.58p-2 (err =  0x1.799d60e5272e0p-66)
    {  0x1.5700000000033p+0,   0x1.7e225515a4ee4p-1,   0x1.0434f7fb1f38bp-3,   0x1.5c000000000ccp-2}, //   87 * 0x1p-8 = 0x1.5cp-2 (err =  0x1.b11f7fede3a37p-65)
    {  0x1.57fffffffff47p+0,   0x1.7d05f417d06c1p-1,   0x1.06cbd67a6c1d8p-3,   0x1.5fffffffffd1cp-2}, //   88 * 0x1p-8 = 0x1.60p-2 (err =  0x1.d06343c9d7e9fp-67)
    {  0x1.5900000000022p+0,   0x1.7beb3922e0156p-1,   0x1.0960c8648e45ep-3,   0x1.6400000000088p-2}, //   89 * 0x1p-8 = 0x1.64p-2 (err =  0x1.8d6bbfe7910f3p-67)
    {  0x1.5a00000000339p+0,   0x1.7ad2208e0e93cp-1,   0x1.0bf3d0937cc65p-3,   0x1.6800000000ce4p-2}, //   90 * 0x1p-8 = 0x1.68p-2 (err =  0x1.093b5a6d6d1b4p-64)
    {  0x1.5afffffffffe3p+0,   0x1.79baa6bb639abp-1,   0x1.0e84f1dadb4dcp-3,   0x1.6bfffffffff8cp-2}, //   91 * 0x1p-8 = 0x1.6cp-2 (err =  0x1.0e732226bdd75p-64)
    {  0x1.5bffffffffff9p+0,   0x1.78a4c8178a4d0p-1,   0x1.11142f0811345p-3,   0x1.6ffffffffffe4p-2}, //   92 * 0x1p-8 = 0x1.70p-2 (err =  0x1.7d438de245991p-65)
    {  0x1.5d00000000008p+0,   0x1.77908119ac605p-1,   0x1.13a18ae256badp-3,   0x1.7400000000020p-2}, //   93 * 0x1p-8 = 0x1.74p-2 (err =  0x1.b966590f0303ep-65)
    {  0x1.5dfffffffffc2p+0,   0x1.767dce434a9f3p-1,   0x1.162d082ac9c72p-3,   0x1.77fffffffff08p-2}, //   94 * 0x1p-8 = 0x1.78p-2 (err =  0x1.5678d183ab630p-66)
    {  0x1.5efffffffff65p+0,   0x1.756cac2017612p-1,   0x1.18b6a99c7f4f0p-3,   0x1.7bffffffffd94p-2}, //   95 * 0x1p-8 = 0x1.7cp-2 (err =  0x1.6f3ff4fb6dfa9p-68)
    {  0x1.600000000000ap+0,   0x1.745d1745d173bp-1,   0x1.1b3e71ec94f94p-3,   0x1.8000000000028p-2}, //   96 * 0x1p-8 = 0x1.80p-2 (err =  0x1.668dda21eca5ep-65)
    {  0x1.60ffffffffff2p+0,   0x1.734f0c541fe9bp-1,   0x1.1dc463ca41dd5p-3,   0x1.83fffffffffc8p-2}, //   97 * 0x1p-8 = 0x1.84p-2 (err =  0x1.9bac1da4b46c3p-65)
    {  0x1.620000000005dp+0,   0x1.724287f46de5bp-1,   0x1.204881dee8861p-3,   0x1.8800000000174p-2}, //   98 * 0x1p-8 = 0x1.88p-2 (err =  0x1.18e657b5d6603p-64)
    {  0x1.62fffffffffffp+0,   0x1.713786d9c7c0ap-1,   0x1.22cacece26eaap-3,   0x1.8bffffffffffcp-2}, //   99 * 0x1p-8 = 0x1.8cp-2 (err =  0x1.257c523106732p-65)
    {  0x1.6400000000048p+0,   0x1.702e05c0b8126p-1,   0x1.254b4d35e7df0p-3,   0x1.9000000000120p-2}, //  100 * 0x1p-8 = 0x1.90p-2 (err =  0x1.cf0f251cd1feap-66)
    {  0x1.6500000000043p+0,   0x1.6f26016f25fd2p-1,   0x1.27c9ffae72a68p-3,   0x1.940000000010cp-2}, //  101 * 0x1p-8 = 0x1.94p-2 (err =  0x1.18acc1fd14cb5p-65)
    {  0x1.65fffffffffc5p+0,   0x1.6e1f76b433803p-1,   0x1.2a46e8ca7b997p-3,   0x1.97fffffffff14p-2}, //  102 * 0x1p-8 = 0x1.98p-2 (err =  0x1.2a3a7119b05adp-66)
    {  0x1.670000000003cp+0,   0x1.6d1a62681c824p-1,   0x1.2cc20b173430ep-3,   0x1.9c000000000f0p-2}, //  103 * 0x1p-8 = 0x1.9cp-2 (err =  0x1.0e69f6307bf8bp-64)
    {  0x1.680000000013bp+0,   0x1.6c16c16c16ad8p-1,   0x1.2f3b691c5a30bp-3,   0x1.a0000000004ecp-2}, //  104 * 0x1p-8 = 0x1.a0p-2 (err =  0x1.53c2dba513c75p-65)
    {  0x1.68fffffffff76p+0,   0x1.6b1490aa31ac8p-1,   0x1.31b3055c46fc4p-3,   0x1.a3ffffffffdd8p-2}, //  105 * 0x1p-8 = 0x1.a4p-2 (err =  0x1.047d983079cf7p-65)
    {  0x1.6a00000000011p+0,   0x1.6a13cd153727fp-1,   0x1.3428e25400997p-3,   0x1.a800000000044p-2}, //  106 * 0x1p-8 = 0x1.a8p-2 (err =  0x1.0f3bcf7932be9p-65)
    {  0x1.6b0000000002dp+0,   0x1.691473a88d093p-1,   0x1.369d027b46362p-3,   0x1.ac000000000b4p-2}, //  107 * 0x1p-8 = 0x1.acp-2 (err =  0x1.2ff2bf2f9bd78p-64)
    {  0x1.6c00000000009p+0,   0x1.681681681680ep-1,   0x1.390f6844a0b99p-3,   0x1.b000000000024p-2}, //  108 * 0x1p-8 = 0x1.b0p-2 (err =  0x1.c83aae9c9d87cp-65)
    {  0x1.6cfffffffffc4p+0,   0x1.6719f36016755p-1,   0x1.3b80161d70ea1p-3,   0x1.b3fffffffff10p-2}, //  109 * 0x1p-8 = 0x1.b4p-2 (err =  0x1.cce96a8dc2adap-66)
    {  0x1.6dfffffffffdbp+0,   0x1.661ec6a51231dp-1,   0x1.3def0e6dfdf2bp-3,   0x1.b7fffffffff6cp-2}, //  110 * 0x1p-8 = 0x1.b8p-2 (err =  0x1.4b10a79c1f7a9p-65)
    {  0x1.6f0000000001bp+0,   0x1.6524f853b4a89p-1,   0x1.405c5399837a3p-3,   0x1.bc0000000006cp-2}, //  111 * 0x1p-8 = 0x1.bcp-2 (err =  0x1.986d2bdbfc8d2p-66)
    {  0x1.6ffffffffffafp+0,   0x1.642c8590b21b3p-1,   0x1.42c7e7fe3fb3ep-3,   0x1.bfffffffffebcp-2}, //  112 * 0x1p-8 = 0x1.c0p-2 (err =  0x1.50d990323c999p-71)
    {  0x1.70fffffffffb1p+0,   0x1.63356b88ac12ap-1,   0x1.4531cdf581a08p-3,   0x1.c3ffffffffec4p-2}, //  113 * 0x1p-8 = 0x1.c4p-2 (err =  0x1.e185efe3dc75ap-66)
    {  0x1.72000000000bep+0,   0x1.623fa7701618ap-1,   0x1.479a07d3b65dap-3,   0x1.c8000000002f8p-2}, //  114 * 0x1p-8 = 0x1.c8p-2 (err =  0x1.1afda5b16f734p-64)
    {  0x1.730000000000ep+0,   0x1.614b36831ae86p-1,   0x1.4a0097e87634bp-3,   0x1.cc00000000038p-2}, //  115 * 0x1p-8 = 0x1.ccp-2 (err =  0x1.d4d887331a2cep-65)
    {  0x1.73fffffffff8ap+0,   0x1.6058160581675p-1,   0x1.4c65807e9321ep-3,   0x1.cfffffffffe28p-2}, //  116 * 0x1p-8 = 0x1.d0p-2 (err =  0x1.79cf4afeeaeb7p-65)
    {  0x1.74fffffffff97p+0,   0x1.5f66434292e5fp-1,   0x1.4ec8c3dc2506fp-3,   0x1.d3ffffffffe5cp-2}, //  117 * 0x1p-8 = 0x1.d4p-2 (err =  0x1.12269a92234a1p-64)
    {  0x1.760000000003bp+0,   0x1.5e75bb8d015b0p-1,   0x1.512a644296cc9p-3,   0x1.d8000000000ecp-2}, //  118 * 0x1p-8 = 0x1.d8p-2 (err =  0x1.ccb7c37132186p-65)
    {  0x1.76fffffffffeap+0,   0x1.5d867c3ece2bap-1,   0x1.538a63eeb2fddp-3,   0x1.dbfffffffffa8p-2}, //  119 * 0x1p-8 = 0x1.dcp-2 (err =  0x1.2e598a72cf006p-66)
    {  0x1.77fffffffffd6p+0,   0x1.5c9882b93107ep-1,   0x1.55e8c518b1095p-3,   0x1.dffffffffff58p-2}, //  120 * 0x1p-8 = 0x1.e0p-2 (err =  0x1.405c839f84608p-65)
    {  0x1.7900000000089p+0,   0x1.5babcc647fa13p-1,   0x1.584589f44131ep-3,   0x1.e400000000224p-2}, //  121 * 0x1p-8 = 0x1.e4p-2 (err =  0x1.7cbbade4b942fp-66)
    {  0x1.7a000000001adp+0,   0x1.5ac056b015937p-1,   0x1.5aa0b4b098cebp-3,   0x1.e8000000006b4p-2}, //  122 * 0x1p-8 = 0x1.e8p-2 (err =  0x1.07197828f4c3cp-64)
    {  0x1.7b00000000070p+0,   0x1.59d61f123cc44p-1,   0x1.5cfa47787e2dep-3,   0x1.ec000000001c0p-2}, //  123 * 0x1p-8 = 0x1.ecp-2 (err =  0x1.8d8987cc16b26p-66)
    {  0x1.7bfffffffffd6p+0,   0x1.58ed230815913p-1,   0x1.5f52447255c30p-3,   0x1.effffffffff58p-2}, //  124 * 0x1p-8 = 0x1.f0p-2 (err =  0x1.9674bfc521195p-67)
    {  0x1.7cfffffffff97p+0,   0x1.58056015805bfp-1,   0x1.61a8adc02c9c8p-3,   0x1.f3ffffffffe5cp-2}, //  125 * 0x1p-8 = 0x1.f4p-2 (err =  0x1.e84e25495e13ep-65)
    {  0x1.7dffffffffffep+0,   0x1.571ed3c506b3bp-1,   0x1.63fd857fc49b6p-3,   0x1.f7ffffffffff8p-2}, //  126 * 0x1p-8 = 0x1.f8p-2 (err =  0x1.7a2bad919dd86p-65)
    {  0x1.7f0000000000ap+0,   0x1.56397ba7c52d9p-1,   0x1.6650cdca9fac4p-3,   0x1.fc00000000028p-2}, //  127 * 0x1p-8 = 0x1.fcp-2 (err =  0x1.bcf6907fc21fap-66)
    {  0x1.810000000000bp-1,   0x1.54725e6bb82f4p+0,  -0x1.fb1f6381856c1p-4,  -0x1.fbfffffffffd4p-3}, // -127 * 0x1p-9 = -0x1.fcp-3 (err =  0x1.36ad67f765f2bp-68)
    {  0x1.8200000000013p-1,   0x1.5390948f40fdap+0,  -0x1.f68216c9cc6cfp-4,  -0x1.f7fffffffffb4p-3}, // -126 * 0x1p-9 = -0x1.f8p-3 (err =  0x1.4ef7c2ae7c5bep-65)
    {  0x1.82fffffffffeep-1,   0x1.52aff56a8055bp+0,  -0x1.f1e7d882b68edp-4,  -0x1.f400000000048p-3}, // -125 * 0x1p-9 = -0x1.f4p-3 (err =  0x1.66816d1f83e13p-65)
    {  0x1.83fffffffffe8p-1,   0x1.51d07eae2f82ap+0,  -0x1.ed50a4a26eb6ap-4,  -0x1.f000000000060p-3}, // -124 * 0x1p-9 = -0x1.f0p-3 (err =  0x1.60494a789385ep-67)
    {  0x1.850000000002bp-1,   0x1.50f22e111c4a0p+0,  -0x1.e8bc77271b8b5p-4,  -0x1.ebfffffffff54p-3}, // -123 * 0x1p-9 = -0x1.ecp-3 (err =  0x1.3926730d95ed4p-65)
    {  0x1.85fffffffffe8p-1,   0x1.501501501502ap+0,  -0x1.e42b4c16cab60p-4,  -0x1.e800000000060p-3}, // -122 * 0x1p-9 = -0x1.e8p-3 (err =  0x1.7aac96d21e000p-70)
    {  0x1.8700000000011p-1,   0x1.4f38f62dd4c8cp+0,  -0x1.df9d1f7f5b627p-4,  -0x1.e3fffffffffbcp-3}, // -121 * 0x1p-9 = -0x1.e4p-3 (err =  0x1.f9522343b5908p-66)
    {  0x1.880000000003ap-1,   0x1.4e5e0a72f0508p+0,  -0x1.db11ed766aaedp-4,  -0x1.dffffffffff18p-3}, // -120 * 0x1p-9 = -0x1.e0p-3 (err =  0x1.8ba8e9ea44f5cp-66)
    {  0x1.88fffffffff42p-1,   0x1.4d843bedc2cedp+0,  -0x1.d689b2193f48fp-4,  -0x1.dc000000002f8p-3}, // -119 * 0x1p-9 = -0x1.dcp-3 (err =  0x1.8bd65bc625e1ap-65)
    {  0x1.89fffffffffc7p-1,   0x1.4cab88725af9fp+0,  -0x1.d204698cb43bep-4,  -0x1.d8000000000e4p-3}, // -118 * 0x1p-9 = -0x1.d8p-3 (err =  0x1.b8a4f649ede6fp-67)
    {  0x1.8b0000000001fp-1,   0x1.4bd3edda68fc7p+0,  -0x1.cd820ffd27867p-4,  -0x1.d3fffffffff84p-3}, // -117 * 0x1p-9 = -0x1.d4p-3 (err =  0x1.557d4a32106c0p-65)
    {  0x1.8c00000000085p-1,   0x1.4afd6a052beebp+0,  -0x1.c902a19e64ebcp-4,  -0x1.cfffffffffdecp-3}, // -116 * 0x1p-9 = -0x1.d0p-3 (err =  0x1.54379aaebebb2p-65)
    {  0x1.8cfffffffff46p-1,   0x1.4a27fad7601e5p+0,  -0x1.c4861aab93d64p-4,  -0x1.cc000000002e8p-3}, // -115 * 0x1p-9 = -0x1.ccp-3 (err =  0x1.95722622dc55cp-65)
    {  0x1.8e0000000000ep-1,   0x1.49539e3b2d05bp+0,  -0x1.c00c77672258cp-4,  -0x1.c7fffffffffc8p-3}, // -114 * 0x1p-9 = -0x1.c8p-3 (err =  0x1.83e2e8b4f16bdp-65)
    {  0x1.8effffffffdbfp-1,   0x1.4880522014a5bp+0,  -0x1.bb95b41ab66f2p-4,  -0x1.c400000000904p-3}, // -113 * 0x1p-9 = -0x1.c4p-3 (err =  0x1.9765aa8b98fd4p-65)
    {  0x1.8ffffffffffe2p-1,   0x1.47ae147ae1493p+0,  -0x1.b721cd1715868p-4,  -0x1.c000000000078p-3}, // -112 * 0x1p-9 = -0x1.c0p-3 (err =  0x1.14b29efe24cedp-66)
    {  0x1.910000000003cp-1,   0x1.46dce34596035p+0,  -0x1.b2b0beb4199c6p-4,  -0x1.bbfffffffff10p-3}, // -111 * 0x1p-9 = -0x1.bcp-3 (err =  0x1.18441bfb190a8p-65)
    {  0x1.91fffffffff9ep-1,   0x1.460cbc7f5cfeap+0,  -0x1.ae428550996bdp-4,  -0x1.b800000000188p-3}, // -110 * 0x1p-9 = -0x1.b8p-3 (err =  0x1.5bca76e367334p-65)
    {  0x1.92ffffffffffdp-1,   0x1.453d9e2c776ccp+0,  -0x1.a9d71d5258491p-4,  -0x1.b40000000000cp-3}, // -109 * 0x1p-9 = -0x1.b4p-3 (err =  0x1.8f35edbceccc6p-65)
    {  0x1.940000000002ap-1,   0x1.446f86562d9d9p+0,  -0x1.a56e8325f5bcep-4,  -0x1.affffffffff58p-3}, // -108 * 0x1p-9 = -0x1.b0p-3 (err =  0x1.4614f76254751p-66)
    {  0x1.94ffffffffffcp-1,   0x1.43a2730abee50p+0,  -0x1.a108b33edb017p-4,  -0x1.ac00000000010p-3}, // -107 * 0x1p-9 = -0x1.acp-3 (err =  0x1.3504644217c38p-66)
    {  0x1.960000000009fp-1,   0x1.42d6625d51f09p+0,  -0x1.9ca5aa1729c8cp-4,  -0x1.a7ffffffffd84p-3}, // -106 * 0x1p-9 = -0x1.a8p-3 (err =  0x1.34424f2f994c0p-65)
    {  0x1.9700000000014p-1,   0x1.420b5265e5941p+0,  -0x1.9845642fac899p-4,  -0x1.a3fffffffffb0p-3}, // -105 * 0x1p-9 = -0x1.a4p-3 (err =  0x1.e3634380c0564p-66)
    {  0x1.98000000001d7p-1,   0x1.41414141412a1p+0,  -0x1.93e7de0fc367ap-4,  -0x1.9fffffffff8a4p-3}, // -104 * 0x1p-9 = -0x1.a0p-3 (err =  0x1.5533136dfa8ffp-65)
    {  0x1.9900000000000p-1,   0x1.40782d10e6566p+0,  -0x1.8f8d144557bdfp-4,  -0x1.9c00000000000p-3}, // -103 * 0x1p-9 = -0x1.9cp-3 (err =  0x1.2034e6320e8c0p-65)
    {  0x1.99ffffffffa5ap-1,   0x1.3fb013fb01862p+0,  -0x1.8b350364c7ad9p-4,  -0x1.9800000001698p-3}, // -102 * 0x1p-9 = -0x1.98p-3 (err =  0x1.842bb19ba0484p-65)
    {  0x1.9afffffffff39p-1,   0x1.3ee8f42a5afa1p+0,  -0x1.86dfa808d39fdp-4,  -0x1.940000000031cp-3}, // -101 * 0x1p-9 = -0x1.94p-3 (err =  0x1.0727d4f2c70ddp-65)
    {  0x1.9c00000000b33p-1,   0x1.3e22cbce4a05dp+0,  -0x1.828cfed2971bap-4,  -0x1.8ffffffffd334p-3}, // -100 * 0x1p-9 = -0x1.90p-3 (err =  0x1.825c456566c46p-65)
    {  0x1.9cffffffffffcp-1,   0x1.3d5d991aa75c9p+0,  -0x1.7e3d04697b715p-4,  -0x1.8c00000000010p-3}, //  -99 * 0x1p-9 = -0x1.8cp-3 (err =  0x1.aae73d5fa0fffp-66)
    {  0x1.9e00000000008p-1,   0x1.3c995a47babe1p+0,  -0x1.79efb57b0f7e1p-4,  -0x1.87fffffffffe0p-3}, //  -98 * 0x1p-9 = -0x1.88p-3 (err =  0x1.02c0176df7eb7p-66)
    {  0x1.9efffffffffbbp-1,   0x1.3bd60d9232989p+0,  -0x1.75a50ebb16372p-4,  -0x1.8400000000114p-3}, //  -97 * 0x1p-9 = -0x1.84p-3 (err =  0x1.5177f0bf812ddp-67)
    {  0x1.9ffffffffffd7p-1,   0x1.3b13b13b13b33p+0,  -0x1.715d0ce367babp-4,  -0x1.80000000000a4p-3}, //  -96 * 0x1p-9 = -0x1.80p-3 (err =  0x1.d91f0563e21fbp-68)
    {  0x1.a10000000001dp-1,   0x1.3a524387ac80dp+0,  -0x1.6d17acb3e5ee2p-4,  -0x1.7bfffffffff8cp-3}, //  -95 * 0x1p-9 = -0x1.7cp-3 (err =  0x1.9fd7f33e7ea3bp-66)
    {  0x1.a20000000000dp-1,   0x1.3991c2c187f59p+0,  -0x1.68d4eaf26d7b7p-4,  -0x1.77fffffffffccp-3}, //  -94 * 0x1p-9 = -0x1.78p-3 (err =  0x1.f2690d5f466b7p-67)
    {  0x1.a300000000012p-1,   0x1.38d22d3660880p+0,  -0x1.6494c46ac6e01p-4,  -0x1.73fffffffffb8p-3}, //  -93 * 0x1p-9 = -0x1.74p-3 (err =  0x1.ba3f3db6ea753p-69)
    {  0x1.a400000000059p-1,   0x1.38138138137d1p+0,  -0x1.605735ee98478p-4,  -0x1.6fffffffffe9cp-3}, //  -92 * 0x1p-9 = -0x1.70p-3 (err =  0x1.3f7411630f6a9p-66)
    {  0x1.a4ffffffffe6dp-1,   0x1.3755bd1c94718p+0,  -0x1.5c1c3c5557e40p-4,  -0x1.6c0000000064cp-3}, //  -91 * 0x1p-9 = -0x1.6cp-3 (err =  0x1.40da692c32252p-65)
    {  0x1.a5fffffffffa3p-1,   0x1.3698df3de078cp+0,  -0x1.57e3d47c3b103p-4,  -0x1.6800000000174p-3}, //  -90 * 0x1p-9 = -0x1.68p-3 (err =  0x1.035e3b32c9f56p-65)
    {  0x1.a700000000028p-1,   0x1.35dce5f9f2adbp+0,  -0x1.53adfb462cd6ep-4,  -0x1.63fffffffff60p-3}, //  -89 * 0x1p-9 = -0x1.64p-3 (err =  0x1.27e2b731f0c24p-65)
    {  0x1.a7ffffffffffap-1,   0x1.3521cfb2b78c6p+0,  -0x1.4f7aad9bbcbc8p-4,  -0x1.6000000000018p-3}, //  -88 * 0x1p-9 = -0x1.60p-3 (err =  0x1.108b9c040bdb3p-65)
    {  0x1.a90000000006ap-1,   0x1.34679ace012fap+0,  -0x1.4b49e86b11ca3p-4,  -0x1.5bffffffffe58p-3}, //  -87 * 0x1p-9 = -0x1.5cp-3 (err =  0x1.b8271852396f6p-66)
    {  0x1.aa00000000006p-1,   0x1.33ae45b57bcaep+0,  -0x1.471ba8a7de29ep-4,  -0x1.57fffffffffe8p-3}, //  -86 * 0x1p-9 = -0x1.58p-3 (err =  0x1.003139f5f2e28p-65)
    {  0x1.ab00000000052p-1,   0x1.32f5ced6a1dbfp+0,  -0x1.42efeb4b50593p-4,  -0x1.53ffffffffeb8p-3}, //  -85 * 0x1p-9 = -0x1.54p-3 (err =  0x1.4aef92d1af904p-70)
    {  0x1.abfffffffff36p-1,   0x1.323e34a2b1150p+0,  -0x1.3ec6ad5407bb0p-4,  -0x1.5000000000328p-3}, //  -84 * 0x1p-9 = -0x1.50p-3 (err =  0x1.3a9962504c5a2p-65)
    {  0x1.acfffffffffcep-1,   0x1.3187758e9ebdap+0,  -0x1.3a9febc6054d9p-4,  -0x1.4c000000000c8p-3}, //  -83 * 0x1p-9 = -0x1.4cp-3 (err =  0x1.ebfae80376833p-66)
    {  0x1.adfffffffff46p-1,   0x1.30d190130d214p+0,  -0x1.367ba3aaa1b84p-4,  -0x1.48000000002e8p-3}, //  -82 * 0x1p-9 = -0x1.48p-3 (err =  0x1.2cdc5025119c5p-65)
    {  0x1.aefffffffffecp-1,   0x1.301c82ac4026ep+0,  -0x1.3259d2107dba7p-4,  -0x1.4400000000050p-3}, //  -81 * 0x1p-9 = -0x1.44p-3 (err =  0x1.01d2089c927f7p-65)
    {  0x1.affffffffffc1p-1,   0x1.2f684bda12f95p+0,  -0x1.2e3a740b78112p-4,  -0x1.40000000000fcp-3}, //  -80 * 0x1p-9 = -0x1.40p-3 (err =  0x1.2bbf6c06321bdp-65)
    {  0x1.b100000000016p-1,   0x1.2eb4ea1fed13cp+0,  -0x1.2a1d86b49f188p-4,  -0x1.3bfffffffffa8p-3}, //  -79 * 0x1p-9 = -0x1.3cp-3 (err =  0x1.d6898f55b785dp-67)
    {  0x1.b200000000067p-1,   0x1.2e025c04b804fp+0,  -0x1.2603072a25ddcp-4,  -0x1.37ffffffffe64p-3}, //  -78 * 0x1p-9 = -0x1.38p-3 (err =  0x1.a6162d9777e47p-66)
    {  0x1.b2fffffffffe3p-1,   0x1.2d50a012d50b4p+0,  -0x1.21eaf28f579b8p-4,  -0x1.3400000000074p-3}, //  -77 * 0x1p-9 = -0x1.34p-3 (err =  0x1.974ad39d54395p-66)
    {  0x1.b40000000007ap-1,   0x1.2c9fb4d812c4cp+0,  -0x1.1dd5460c8af7dp-4,  -0x1.2fffffffffe18p-3}, //  -76 * 0x1p-9 = -0x1.30p-3 (err =  0x1.1931421ac6f24p-65)
    {  0x1.b4fffffffffd9p-1,   0x1.2bef98e5a372cp+0,  -0x1.19c1fecf17f7fp-4,  -0x1.2c0000000009cp-3}, //  -75 * 0x1p-9 = -0x1.2cp-3 (err =  0x1.99a6ed37ffbabp-66)
    {  0x1.b5fffffffffb9p-1,   0x1.2b404ad012b71p+0,  -0x1.15b11a094a2cap-4,  -0x1.280000000011cp-3}, //  -74 * 0x1p-9 = -0x1.28p-3 (err =  0x1.08f7db222282dp-65)
    {  0x1.b6fffffffffd1p-1,   0x1.2a91c92f3c125p+0,  -0x1.11a294f256ab4p-4,  -0x1.24000000000bcp-3}, //  -73 * 0x1p-9 = -0x1.24p-3 (err =  0x1.93d1e13776178p-68)
    {  0x1.b7fffffffffcbp-1,   0x1.29e4129e412c2p+0,  -0x1.0d966cc6501d0p-4,  -0x1.20000000000d4p-3}, //  -72 * 0x1p-9 = -0x1.20p-3 (err =  0x1.cfb42149acca1p-66)
    {  0x1.b9000000000d9p-1,   0x1.293725bb80413p+0,  -0x1.098c9ec61b094p-4,  -0x1.1bffffffffc9cp-3}, //  -71 * 0x1p-9 = -0x1.1cp-3 (err =  0x1.bcbd318d6d7a1p-66)
    {  0x1.ba0000000018dp-1,   0x1.288b01288af08p+0,  -0x1.0585283763b3ap-4,  -0x1.17ffffffff9ccp-3}, //  -70 * 0x1p-9 = -0x1.18p-3 (err =  0x1.ca08e5a203154p-66)
    {  0x1.bb0000000000ep-1,   0x1.27dfa38a1ce44p+0,  -0x1.01800664927dfp-4,  -0x1.13fffffffffc8p-3}, //  -69 * 0x1p-9 = -0x1.14p-3 (err =  0x1.7aa9f6214aeedp-66)
    {  0x1.bc00000000007p-1,   0x1.27350b8812730p+0,  -0x1.fafa6d397efa3p-5,  -0x1.0ffffffffffe4p-3}, //  -68 * 0x1p-9 = -0x1.10p-3 (err =  0x1.f75814937eb1ep-66)
    {  0x1.bd0000000000fp-1,   0x1.268b37cd6011dp+0,  -0x1.f2f96c6754a76p-5,  -0x1.0bfffffffffc4p-3}, //  -67 * 0x1p-9 = -0x1.0cp-3 (err =  0x1.3ccee16d713a8p-68)
    {  0x1.be00000000025p-1,   0x1.25e22708092d9p+0,  -0x1.eafd05035bc14p-5,  -0x1.07fffffffff6cp-3}, //  -66 * 0x1p-9 = -0x1.08p-3 (err =  0x1.d4233b902bc5ap-67)
    {  0x1.befffffffff82p-1,   0x1.2539d7e917805p+0,  -0x1.e30531c76c735p-5,  -0x1.04000000001f8p-3}, //  -65 * 0x1p-9 = -0x1.04p-3 (err =  0x1.73ae0e2f571b4p-66)
    {  0x1.c000000000063p-1,   0x1.2492492492452p+0,  -0x1.db11ed766a8e2p-5,  -0x1.ffffffffffce8p-4}, //  -64 * 0x1p-9 = -0x1.00p-3 (err =  0x1.9d4e19aa1434ep-67)
    {  0x1.c100000000049p-1,   0x1.23eb79717602cp+0,  -0x1.d32332dc34b7bp-5,  -0x1.f7ffffffffdb8p-4}, //  -63 * 0x1p-9 = -0x1.f8p-4 (err =  0x1.4f9736621a6f8p-69)
    {  0x1.c1ffffffffff1p-1,   0x1.23456789abce9p+0,  -0x1.cb38fccd8c052p-5,  -0x1.f000000000078p-4}, //  -62 * 0x1p-9 = -0x1.f0p-4 (err =  0x1.9b4b4fcb47b86p-66)
    {  0x1.c300000000065p-1,   0x1.22a0122a011e9p+0,  -0x1.c3534628013c0p-5,  -0x1.e7ffffffffcd8p-4}, //  -61 * 0x1p-9 = -0x1.e8p-4 (err =  0x1.5f0c036b16ea7p-66)
    {  0x1.c40000000000dp-1,   0x1.21fb78121fb70p+0,  -0x1.bb7209d1e247fp-5,  -0x1.dffffffffff98p-4}, //  -60 * 0x1p-9 = -0x1.e0p-4 (err =  0x1.37fac686c91acp-67)
    {  0x1.c500000000021p-1,   0x1.21579804855d1p+0,  -0x1.b39542ba23c70p-5,  -0x1.d7ffffffffef8p-4}, //  -59 * 0x1p-9 = -0x1.d8p-4 (err =  0x1.7ce936641d3d9p-67)
    {  0x1.c5ffffffffff5p-1,   0x1.20b470c67c0e0p+0,  -0x1.abbcebd84fcf6p-5,  -0x1.d000000000058p-4}, //  -58 * 0x1p-9 = -0x1.d0p-4 (err =  0x1.aa3c3a801aa67p-67)
    {  0x1.c700000000046p-1,   0x1.2012012011fe6p+0,  -0x1.a3e9002c70fafp-5,  -0x1.c7ffffffffdd0p-4}, //  -57 * 0x1p-9 = -0x1.c8p-4 (err =  0x1.cf8d95e2035cep-67)
    {  0x1.c7fffffffffd3p-1,   0x1.1f7047dc11f8dp+0,  -0x1.9c197abf00f36p-5,  -0x1.c000000000168p-4}, //  -56 * 0x1p-9 = -0x1.c0p-4 (err =  0x1.3ecc34d4b7b69p-66)
    {  0x1.c90000000000bp-1,   0x1.1ecf43c7fb845p+0,  -0x1.944e56a0d33fap-5,  -0x1.b7fffffffffa8p-4}, //  -55 * 0x1p-9 = -0x1.b8p-4 (err =  0x1.ff6fd5e8fff90p-68)
    {  0x1.ca00000000003p-1,   0x1.1e2ef3b3fb872p+0,  -0x1.8c878eeb0505dp-5,  -0x1.affffffffffe8p-4}, //  -54 * 0x1p-9 = -0x1.b0p-4 (err =  0x1.74fd759bd3f10p-67)
    {  0x1.cb0000000004bp-1,   0x1.1d8f5672e4a8ep+0,  -0x1.84c51ebee8ad0p-5,  -0x1.a7ffffffffda8p-4}, //  -53 * 0x1p-9 = -0x1.a8p-4 (err =  0x1.25e8ec266617fp-66)
    {  0x1.cc0000000001dp-1,   0x1.1cf06ada2810bp+0,  -0x1.7d070145f4ef7p-5,  -0x1.9ffffffffff18p-4}, //  -52 * 0x1p-9 = -0x1.a0p-4 (err =  0x1.f5b1a0b874eadp-67)
    {  0x1.ccffffffffff1p-1,   0x1.1c522fc1ce062p+0,  -0x1.754d31b1b1810p-5,  -0x1.9800000000078p-4}, //  -51 * 0x1p-9 = -0x1.98p-4 (err =  0x1.2e02d636e9207p-67)
    {  0x1.cdfffffffffe8p-1,   0x1.1bb4a4046ed38p+0,  -0x1.6d97ab3ba5ec9p-5,  -0x1.90000000000c0p-4}, //  -50 * 0x1p-9 = -0x1.90p-4 (err =  0x1.284ac375a5464p-67)
    {  0x1.cefffffffffa3p-1,   0x1.1b17c67f2bb1cp+0,  -0x1.65e6692547f19p-5,  -0x1.88000000002e8p-4}, //  -49 * 0x1p-9 = -0x1.88p-4 (err =  0x1.0838d1b593c6fp-66)
    {  0x1.d00000000007bp-1,   0x1.1a7b9611a7b4bp+0,  -0x1.5e3966b7e8ee6p-5,  -0x1.7fffffffffc28p-4}, //  -48 * 0x1p-9 = -0x1.80p-4 (err =  0x1.a084ea0d85de1p-69)
    {  0x1.d100000000009p-1,   0x1.19e0119e01199p+0,  -0x1.56909f44a72b9p-5,  -0x1.77fffffffffb8p-4}, //  -47 * 0x1p-9 = -0x1.78p-4 (err =  0x1.1167b8db4600cp-68)
    {  0x1.d1fffffffff3bp-1,   0x1.19453808ca313p+0,  -0x1.4eec0e2459510p-5,  -0x1.7000000000628p-4}, //  -46 * 0x1p-9 = -0x1.70p-4 (err =  0x1.9cb9901a1b071p-67)
    {  0x1.d30000000001dp-1,   0x1.18ab083902bc9p+0,  -0x1.474baeb77e827p-5,  -0x1.67fffffffff18p-4}, //  -45 * 0x1p-9 = -0x1.68p-4 (err =  0x1.e8ea571a867e0p-68)
    {  0x1.d400000000063p-1,   0x1.18118118117d6p+0,  -0x1.3faf7c663031dp-5,  -0x1.5fffffffffce8p-4}, //  -44 * 0x1p-9 = -0x1.60p-4 (err =  0x1.2e2ea21984a9ep-66)
    {  0x1.d4fffffffffaep-1,   0x1.1778a191bd6b5p+0,  -0x1.381772a00e872p-5,  -0x1.5800000000290p-4}, //  -43 * 0x1p-9 = -0x1.58p-4 (err =  0x1.1a78111432c27p-67)
    {  0x1.d5fffffffff69p-1,   0x1.16e06894273d3p+0,  -0x1.30838cdc30074p-5,  -0x1.50000000004b8p-4}, //  -42 * 0x1p-9 = -0x1.50p-4 (err =  0x1.a3b18dbdb3e4dp-70)
    {  0x1.d700000000154p-1,   0x1.1648d50fc3138p+0,  -0x1.28f3c6991206cp-5,  -0x1.47ffffffff560p-4}, //  -41 * 0x1p-9 = -0x1.48p-4 (err =  0x1.1203c80205722p-66)
    {  0x1.d7fffffffffd4p-1,   0x1.15b1e5f752727p+0,  -0x1.21681b5c8c35fp-5,  -0x1.4000000000160p-4}, //  -40 * 0x1p-9 = -0x1.40p-4 (err =  0x1.ab9c75c9f8790p-68)
    {  0x1.d900000000010p-1,   0x1.151b9a3fdd5bfp+0,  -0x1.19e086b3b82bbp-5,  -0x1.37fffffffff80p-4}, //  -39 * 0x1p-9 = -0x1.38p-4 (err =  0x1.b791385a1eb51p-67)
    {  0x1.d9fffffffff7fp-1,   0x1.1485f0e0acd87p+0,  -0x1.125d0432ea5d6p-5,  -0x1.3000000000408p-4}, //  -38 * 0x1p-9 = -0x1.30p-4 (err =  0x1.a1559854fce19p-69)
    {  0x1.db00000000054p-1,   0x1.13f0e8d3446f3p+0,  -0x1.0add8f759cb20p-5,  -0x1.27ffffffffd60p-4}, //  -37 * 0x1p-9 = -0x1.28p-4 (err =  0x1.f389f4a0d8f77p-73)
    {  0x1.dc00000000005p-1,   0x1.135c81135c80ep+0,  -0x1.0362241e638c7p-5,  -0x1.1ffffffffffd8p-4}, //  -36 * 0x1p-9 = -0x1.20p-4 (err =  0x1.93119dc0520a8p-67)
    {  0x1.dcfffffffffbap-1,   0x1.12c8b89edc0d4p+0,  -0x1.f7d57badb52fcp-6,  -0x1.1800000000230p-4}, //  -35 * 0x1p-9 = -0x1.18p-4 (err =  0x1.875ef181e08eap-68)
    {  0x1.de00000000015p-1,   0x1.12358e75d3027p+0,  -0x1.e8eeb09f2f592p-6,  -0x1.0ffffffffff58p-4}, //  -34 * 0x1p-9 = -0x1.10p-4 (err =  0x1.df935374c535ep-67)
    {  0x1.df00000000010p-1,   0x1.11a3019a7481dp+0,  -0x1.da0fde8038cfbp-6,  -0x1.07fffffffff80p-4}, //  -33 * 0x1p-9 = -0x1.08p-4 (err =  0x1.0afb35a3eb32ap-69)
    {  0x1.dfffffffffff8p-1,   0x1.1111111111116p+0,  -0x1.cb38fccd8c052p-6,  -0x1.0000000000040p-4}, //  -32 * 0x1p-9 = -0x1.00p-4 (err =  0x1.9b4b4fcb28188p-67)
    {  0x1.e0ffffffff450p-1,   0x1.107fbbe01171fp+0,  -0x1.bc6a03118987cp-6,  -0x1.f00000000bb00p-5}, //  -31 * 0x1p-9 = -0x1.f0p-5 (err =  0x1.ae409ea202d3bp-67)
    {  0x1.e1fffffffffeep-1,   0x1.0fef010fef01bp+0,  -0x1.ada2e8e3e5579p-6,  -0x1.e000000000120p-5}, //  -30 * 0x1p-9 = -0x1.e0p-5 (err =  0x1.637b488761798p-68)
    {  0x1.e300000000034p-1,   0x1.0f5edfab32584p+0,  -0x1.9ee3a5e9f54eap-6,  -0x1.cfffffffffcc0p-5}, //  -29 * 0x1p-9 = -0x1.d0p-5 (err =  0x1.7a085f145eb5cp-68)
    {  0x1.e40000000002ap-1,   0x1.0ecf56be69c78p+0,  -0x1.902c31d62a5dap-6,  -0x1.bfffffffffd60p-5}, //  -28 * 0x1p-9 = -0x1.c0p-5 (err =  0x1.a30b7485ee229p-70)
    {  0x1.e4fffffffffcep-1,   0x1.0e4065582602dp+0,  -0x1.817c846828e9bp-6,  -0x1.b000000000320p-5}, //  -27 * 0x1p-9 = -0x1.b0p-5 (err =  0x1.c781565b915fbp-71)
    {  0x1.e5ffffffffffbp-1,   0x1.0db20a88f4698p+0,  -0x1.72d4956ca20b0p-6,  -0x1.a000000000050p-5}, //  -26 * 0x1p-9 = -0x1.a0p-5 (err =  0x1.8fda190e2ec4cp-68)
    {  0x1.e6fffffffffddp-1,   0x1.0d24456359e4dp+0,  -0x1.64345cbd3a690p-6,  -0x1.9000000000230p-5}, //  -25 * 0x1p-9 = -0x1.90p-5 (err =  0x1.ba544db64857fp-68)
    {  0x1.e7fffffffffaap-1,   0x1.0c9714fbcda6ap+0,  -0x1.559bd2406c8a0p-6,  -0x1.8000000000560p-5}, //  -24 * 0x1p-9 = -0x1.80p-5 (err =  0x1.1274d9db45315p-68)
    {  0x1.e8fffffffffeap-1,   0x1.0c0a7868b417dp+0,  -0x1.470aede96e932p-6,  -0x1.7000000000160p-5}, //  -23 * 0x1p-9 = -0x1.70p-5 (err =  0x1.71bc5867f6578p-70)
    {  0x1.ea00000000040p-1,   0x1.0b7e6ec259da5p+0,  -0x1.3881a7b818bfdp-6,  -0x1.5fffffffffc00p-5}, //  -22 * 0x1p-9 = -0x1.60p-5 (err =  0x1.728bba671a4fdp-68)
    {  0x1.eaffffffffe02p-1,   0x1.0af2f722eedcbp+0,  -0x1.29fff7b8cc47cp-6,  -0x1.5000000001fe0p-5}, //  -21 * 0x1p-9 = -0x1.50p-5 (err =  0x1.e61d8a940061bp-68)
    {  0x1.ec0000000022ep-1,   0x1.0a6810a680f78p+0,  -0x1.1b85d6044ca28p-6,  -0x1.3ffffffffdd20p-5}, //  -20 * 0x1p-9 = -0x1.40p-5 (err =  0x1.fe2cf15f183fbp-68)
    {  0x1.ecfffffffff9ep-1,   0x1.09ddba6af8395p+0,  -0x1.0d133abfc44a1p-6,  -0x1.3000000000620p-5}, //  -19 * 0x1p-9 = -0x1.30p-5 (err =  0x1.2732911bd69edp-68)
    {  0x1.ee00000000080p-1,   0x1.0953f3901090fp+0,  -0x1.fd503c39040b6p-7,  -0x1.1fffffffff800p-5}, //  -18 * 0x1p-9 = -0x1.20p-5 (err =  0x1.25dd7a6a5b41bp-68)
    {  0x1.eefffffffffedp-1,   0x1.08cabb37565ecp+0,  -0x1.e088f0b004a49p-7,  -0x1.1000000000130p-5}, //  -17 * 0x1p-9 = -0x1.10p-5 (err =  0x1.a5f64bde91973p-69)
    {  0x1.effffffffffadp-1,   0x1.08421084210b0p+0,  -0x1.c3d083778558ep-7,  -0x1.0000000000530p-5}, //  -16 * 0x1p-9 = -0x1.00p-5 (err =  0x1.8f44aa5f3d06dp-68)
    {  0x1.f100000000017p-1,   0x1.07b9f29b8ead5p+0,  -0x1.a726e53a602dbp-7,  -0x1.dfffffffffd20p-6}, //  -15 * 0x1p-9 = -0x1.e0p-6 (err =  0x1.9e2ed93a5798cp-69)
    {  0x1.f200000000007p-1,   0x1.073260a47f7c3p+0,  -0x1.8a8c06bb1d22cp-7,  -0x1.bffffffffff20p-6}, //  -14 * 0x1p-9 = -0x1.c0p-6 (err =  0x1.78ca9f2a8f2dbp-68)
    {  0x1.f2ffffffffff1p-1,   0x1.06ab59c791303p+0,  -0x1.6dffd8d3bc11cp-7,  -0x1.a0000000001e0p-6}, //  -13 * 0x1p-9 = -0x1.a0p-6 (err =  0x1.3f528f67e40fbp-69)
    {  0x1.f3fffffffffeap-1,   0x1.0624dd2f1aa07p+0,  -0x1.51824c7588122p-7,  -0x1.80000000002c0p-6}, //  -12 * 0x1p-9 = -0x1.80p-6 (err =  0x1.5a793726d662fp-73)
    {  0x1.f500000000086p-1,   0x1.059eea0727540p+0,  -0x1.351352a8e6461p-7,  -0x1.5ffffffffef40p-6}, //  -11 * 0x1p-9 = -0x1.60p-6 (err =  0x1.375ecd08417f1p-70)
    {  0x1.f5fffffffffe3p-1,   0x1.05197f7d73413p+0,  -0x1.18b2dc8d2bec7p-7,  -0x1.40000000003a0p-6}, //  -10 * 0x1p-9 = -0x1.40p-6 (err =  0x1.406cb75b5e36cp-69)
    {  0x1.f6fffffffffc4p-1,   0x1.04949cc1664e5p+0,  -0x1.f8c1b6b0c9a91p-8,  -0x1.2000000000780p-6}, //   -9 * 0x1p-9 = -0x1.20p-6 (err =  0x1.3f84c4e3398f8p-70)
    {  0x1.f7fffffffffcap-1,   0x1.041041041042cp+0,  -0x1.c03a80ae5ec3dp-8,  -0x1.00000000006c0p-6}, //   -8 * 0x1p-9 = -0x1.00p-6 (err =  0x1.733ba7cba8703p-69)
    {  0x1.f8ffffffffff0p-1,   0x1.038c6b7824804p+0,  -0x1.87cff9d914b2bp-8,  -0x1.c000000000400p-7}, //   -7 * 0x1p-9 = -0x1.c0p-7 (err =  0x1.b12d23e92ca3dp-71)
    {  0x1.f9fffffffffb6p-1,   0x1.03091b51f5e40p+0,  -0x1.4f8205235206fp-8,  -0x1.8000000001280p-7}, //   -6 * 0x1p-9 = -0x1.80p-7 (err =  0x1.42699924d2ea5p-71)
    {  0x1.fafffffffff8ep-1,   0x1.02864fc772a23p+0,  -0x1.175085ab878f0p-8,  -0x1.4000000001c80p-7}, //   -5 * 0x1p-9 = -0x1.40p-7 (err =  0x1.10582e64be26bp-69)
    {  0x1.fc00000000001p-1,   0x1.0204081020408p+0,  -0x1.be76bd77b4f53p-9,  -0x1.fffffffffff80p-8}, //   -4 * 0x1p-9 = -0x1.00p-7 (err =  0x1.375efe6df9e11p-70)
    {  0x1.fcfffffffffe5p-1,   0x1.0182436517a45p+0,  -0x1.4e84e793a5316p-9,  -0x1.8000000000d80p-8}, //   -3 * 0x1p-9 = -0x1.80p-8 (err =  0x1.a8d7d221fe319p-71)
    {  0x1.fe00000000c5cp-1,   0x1.01010101009d6p+0, -0x1.bd96a1d72d5bep-10,  -0x1.ffffffff3a400p-9}, //   -2 * 0x1p-9 = -0x1.00p-8 (err =  0x1.bd2d1dc6ea08ep-71)
    {  0x1.ff0000000002ap-1,   0x1.008040201006bp+0, -0x1.bd27045bf870ap-11, -0x1.fffffffffac00p-10}, //   -1 * 0x1p-9 = -0x1.00p-9 (err =  0x1.ee71c3c117703p-73)
    {  0x1.0000000000000p+0,   0x1.0000000000000p+0,   0x0.0000000000000p+0,   0x0.0000000000000p+0}, //    0 * 0x1p-9 = 0x0.00p+0 (err =   0x0.0000000000000p+0)
};// Worst deviation:  ap1 =   0x1.09ffffffff993p+0, log1pa =   0x1.10a83a843c094p-6, a =   0x1.3ffffffff3260p-5,  err =  0x1.fbe8c11089595p-68, d = 0x1.9b4p-42

//gcc makeispowerof10.c -o p10 && ./p10
/* This table is a collection of pairs of {10.^n, n} in a "clever" order
   The heart of the table generation algorithm is:
        double x = 1.0;
        for(n = 0; n <= 22; n++) {
                union{ double d; uint64_t u; } u = { x };
                uint64_t key = (u.u & 0x07e0000000000000ULL)>>53;
                k = (int)key; //the top 5 of bottom 6 bits of exponent of x
                isPowerOf10[k][0] = x; 
                isPowerOf10[k][1] = (double)n; 
                x *= 10.0;
        }
    This scheme is chosen so that in the range of interest there are no collisions.
    Thus if get_key(x) denotes the key: the top 5 of bottom 6 bits of exponent of x,
    we just check to see if x == isPowerOf10[k][0]. 
    If it is, then we can return the exact result without raising inexact. 
    Otherwise, we continue with the algorithm. 
    If the key is associated with no power of 10 in the double precision range,
    then the value of 0 with not match x (zero is treated as a special operand). 
*/

static const double isPowerOf10[64][2] = { // {10^n, log10(10^n)}
    {0x0p+0, 0x0p+0}, // 0
    {0x1.4p+3, 0x1p+0}, // 1
    {0x1.9p+6, 0x1p+1}, // 2
    {0x0p+0, 0x0p+0}, // 3
    {0x1.f4p+9, 0x1.8p+1}, // 4
    {0x0p+0, 0x0p+0}, // 5
    {0x1.388p+13, 0x1p+2}, // 6
    {0x1.86ap+16, 0x1.4p+2}, // 7
    {0x0p+0, 0x0p+0}, // 8
    {0x1.e848p+19, 0x1.8p+2}, // 9
    {0x0p+0, 0x0p+0}, // 10
    {0x1.312dp+23, 0x1.cp+2}, // 11
    {0x1.7d784p+26, 0x1p+3}, // 12
    {0x0p+0, 0x0p+0}, // 13
    {0x1.dcd65p+29, 0x1.2p+3}, // 14
    {0x0p+0, 0x0p+0}, // 15
    {0x1.2a05f2p+33, 0x1.4p+3}, // 16
    {0x1.74876e8p+36, 0x1.6p+3}, // 17
    {0x0p+0, 0x0p+0}, // 18
    {0x1.d1a94a2p+39, 0x1.8p+3}, // 19
    {0x0p+0, 0x0p+0}, // 20
    {0x1.2309ce54p+43, 0x1.ap+3}, // 21
    {0x1.6bcc41e9p+46, 0x1.cp+3}, // 22
    {0x0p+0, 0x0p+0}, // 23
    {0x1.c6bf52634p+49, 0x1.ep+3}, // 24
    {0x0p+0, 0x0p+0}, // 25
    {0x1.1c37937e08p+53, 0x1p+4}, // 26
    {0x1.6345785d8ap+56, 0x1.1p+4}, // 27
    {0x0p+0, 0x0p+0}, // 28
    {0x1.bc16d674ec8p+59, 0x1.2p+4}, // 29
    {0x0p+0, 0x0p+0}, // 30
    {0x1.158e460913dp+63, 0x1.3p+4}, // 31
    {0x1.5af1d78b58c4p+66, 0x1.4p+4}, // 32
    {0x0p+0, 0x0p+0}, // 33
    {0x1.b1ae4d6e2ef5p+69, 0x1.5p+4}, // 34
    {0x0p+0, 0x0p+0}, // 35
    {0x1.0f0cf064dd592p+73, 0x1.6p+4}, // 36
    {0x0p+0, 0x0p+0}, // 37
    {0x0p+0, 0x0p+0}, // 38
    {0x0p+0, 0x0p+0}, // 39
    {0x0p+0, 0x0p+0}, // 40
    {0x0p+0, 0x0p+0}, // 41
    {0x0p+0, 0x0p+0}, // 42
    {0x0p+0, 0x0p+0}, // 43
    {0x0p+0, 0x0p+0}, // 44
    {0x0p+0, 0x0p+0}, // 45
    {0x0p+0, 0x0p+0}, // 46
    {0x0p+0, 0x0p+0}, // 47
    {0x0p+0, 0x0p+0}, // 48
    {0x0p+0, 0x0p+0}, // 49
    {0x0p+0, 0x0p+0}, // 50
    {0x0p+0, 0x0p+0}, // 51
    {0x0p+0, 0x0p+0}, // 52
    {0x0p+0, 0x0p+0}, // 53
    {0x0p+0, 0x0p+0}, // 54
    {0x0p+0, 0x0p+0}, // 55
    {0x0p+0, 0x0p+0}, // 56
    {0x0p+0, 0x0p+0}, // 57
    {0x0p+0, 0x0p+0}, // 58
    {0x0p+0, 0x0p+0}, // 59
    {0x0p+0, 0x0p+0}, // 60
    {0x0p+0, 0x0p+0}, // 61
    {0x0p+0, 0x0p+0}, // 62
    {0x1p+0, 0x0p+0}, // 63
};

#define LOG2_OVERFLOW_GUARANTEED 1025.
#define LOG2_UNDERFLOW_GUARANTEED -1100.

static const double ln2hi = 0x1.62E42FEFA3p-1;
static const double ln2lo = 0x0.9EF35793C76730p-41;

static const double lgehi = 0x1.7154768p0, lgelo = -0x2.D47D01E8882F0p-28;
static const double lge = 0x1.71547652B82FE1777D0p0;

static const double log10ehi = 0x0.6F2DEC4p0, log10elo = 0x0.149B9438CA9AAep-24;
static const double log10e =   0x0.6F2DEC549B9438CA8p0; //log10(e)

static const double log102hi = 0x0.4D104D426p0, log102lo = 0x1.DE7FBCC47C4ADp-36;
static const double log102 =   0x0.4D104D427DE7FBCC8p0; //log10(2) 

static const double one = 1.0;
static const double mone = -1.0;


//Infs, zeros, subnormals, and NaNs -- not supported. you get what you get.
//Assumes x>0.
//For normal inpust this returns frexp(x) in [0.75,1.5)
//And the coresponding exponent. 
//This form accepxt x encoded in hi 32-bits and lo 32-bits.
static inline double frexp_near1_normal_32( uint32_t xhi, uint32_t xlo, int *exp, int *k) ALWAYS_INLINE;
static inline double frexp_near1_normal_32( uint32_t xhi, uint32_t xlo, int *exp, int *k)
{
        union{ double d; uint64_t u; } u ;
        

        const uint32_t        exponent = xhi >> (52-32);
        uint32_t        mantissahi = xhi & 0x000fffff;
        const uint32_t        mantissalo = xlo;

        // or in -1 + bias to the new exponent for the mantissa
        const uint32_t        isupperhalf = (xhi>> (51-32)) & 0x1;
        const uint32_t        upperhalf = xhi & 0x00080000;
        const uint32_t        head = mantissahi>>(52-32-8); // grab before adjusting for which half we are in
        *k = head;
        mantissahi = (mantissahi | 0x3ff00000) ^ (upperhalf << 1); // Divide by two if significand >= 3/2
        *exp = exponent-1023 + isupperhalf; //right shift exponent and remove the bias, and store
        u.u = ((uint64_t)mantissahi << 32) | (uint64_t)mantissalo;

        return u.d;
}

// This only works for non-zero subnormal inputs
static inline double frexp_near1_subnorm_32( uint32_t xhi, uint32_t xlo, int *exp, int *k) ALWAYS_INLINE;
static inline double frexp_near1_subnorm_32( uint32_t xhi, uint32_t xlo, int *exp, int *k)
{
        union{ double d; uint64_t u; } u ;
        int exponent; 
        uint32_t        mantissahi = xhi; // == xhi & 0x000fffff;
        uint32_t        mantissalo = xlo;

        if(xhi) {
                const int lz = __builtin_clz(xhi) - 12;
		const int shift = lz + 1;
                exponent = -1022 - shift;
                mantissahi = (mantissahi << shift) | (mantissalo >> (32 - shift));
                mantissalo = mantissalo << shift;
        } else {
            int lzlo = __builtin_clz(xlo);
	    const int lz = (52 - 32) + lzlo; // hi zeroes + lo zeroes
	    // Need to shift the lo bits to the right place.
	    // 6666555555555544444444443333333333222222222211111111110000000000
	    // 3210987654321098765432109876543210987654321098765432109876543210
	    //                                 RRRRRRRRRRnLLLLLLLLLLLLLLLLLLLLL
	    // If lzlo == 11:
	    //    Then we have everything lined up so 
	    //        mantissahi = mantissalo; mantissalo = 0
	    // If lzlo < 11:
	    //     We have bits too far to the left (into the exponent field)
	    //        mantissahi = mantissalo >> -(lzlo - 12); mantissalo = mantissalo << (32 + (lzlo - 12))
	    // If lzlo > 11:
	    //        mantissahi = mantissalo << (lzlo - 12); mantissalo = 0
	    const int shift = 1 + lzlo - 12;
	    exponent = -1022 - (1 + lz) ;
	    if(shift < 0) {
	      mantissahi = mantissalo >> -shift;
	      mantissalo = mantissalo << (32 + shift);
	    } else if (shift > 0) {
	      mantissahi = (mantissalo << shift);
	      mantissalo = 0;
	    } else {
	      mantissahi = mantissalo;
	      mantissalo = 0;
	    }
        }

        // or in -1 + bias to the new exponent for the mantissa
        const uint32_t        isupperhalf = (mantissahi>> (51-32)) & 0x1;
        const uint32_t        upperhalf = mantissahi & 0x00080000;
        const uint32_t        head = (mantissahi>>(52-32-8)) & 0xff; // grab before adjusting for which half we are in
        *k = head;
        mantissahi = (mantissahi | 0x3ff00000) ^ (upperhalf << 1); // Divide by two if significand >= 3/2
        *exp = exponent + isupperhalf;
        u.u = ((uint64_t)mantissahi << 32) | (uint64_t)mantissalo;

        return u.d;
}

// Here we assume -0.25 <= absx <= 1.fffffffffffffp-2
// Care needs to be taken at the top endpoint since 1.fffffffffffffp-2 + 1.0 rounds to 1.5
static inline int get_k( double f) ALWAYS_INLINE;
static inline int get_k( double f)
#if AVOID_64_BIT_INTS
{
        double absx = f + one;
        union{ double d; uint64_t u; }u = { absx };

        const uint32_t fp1hi = (uint32_t) (u.u >> 32);
        const int      is_three_halves = (fp1hi == 0x3ff80000);
        const uint32_t mhi = fp1hi & 0x000fffff;;
        const uint32_t head = mhi>>(52-32-8); // grab before adjusting for which half we are in

        int k = (int)head - is_three_halves; 

        return k;
}
#else
{
        double absx = f + one;
        union{ double d; uint64_t u; } u = { absx };
        uint64_t        head = u.u & 0x000ff00000000000ULL; // grab before adjusting for which half we are in
        int k = (int)(head>>(52-8))  - (absx == 1.5); 

        return k;
}
#endif

static const double LogTaylor[]=
{
        // Polynomial by Ali Sazegari, minimax fit for (ln(1+x)-x)/x^2
        // Error is 0.4760672925851742e-17 < 2^-54 for x in [- 4/3 2^-9 - 2^-16, 2^-8  + 2^-16].

        .142372214866102850763683751341171,
        -.166667879188171146361052190972989,
        .200000004508805548728000821107988,
        -.249999999996709499926740747458343,
        .333333333333325027260811457536115,
        -.500000000000000001726903949064586,
};

/* Factored form 
 .142372214866102850763683751341171
 * (x^2+.828248034651122472190794934909920*x+1.51767606885304499387160779942961)
 * (x^2-1.99889695196211394019298943288314*x+1.54267491565007296487876747692402)
*/

static const double c0 = -.500000000000000001726903949064586; // c0
static const double c5_e = .142372214866102850763683751341171;  // c5 * ln(e) = c5

static const double a0 = .828248034651122472190794934909920, a1 = -1.99889695196211394019298943288314; //a0,a1
static const double b0 = 1.51767606885304499387160779942961, b1 = 1.54267491565007296487876747692402; //b0,b1


/*
x = 2^n (1+f)
if n == 0
    (1+f) = (1+a)(1+c)(1+s)
    c = invap1 * ( (1+f) - (1+a) )
    s = "1/(1+c)" * invap1 * ( ( ( (1+f) - (1+a) ) - c) - a*c )
      = ( (1 - c) * invap1 ) * ( ( ( (1+f) - (1+a) ) - c) - a*c )
  base e:       
    hi + lo = ln1pa + c
    cpp = c * c * p(c)
    s = ( (1 - c) * invap1 ) * ( ( ( (1+f) - (1+a) ) - c) - a*c )
    log(x) = hi + (ccp + s + lo)
  base b (2 or 10):
    hi + lo = logb1pa + "logb(e) * c"
    cpp = c * c * p(c)
    s = ( (1 - c) * invap1 ) * ( ( ( (1+f) - (1+a) ) - c) - a*c )
    log_b(x) = hi + (logb(e)*(ccp + s) + lo)

if n != 0
    (1+f) = (1+a)(1+c)
    c = invap1 * ( (1+f) - (1+a) )
  base e:       
    hi + lo = (ln2hi + ln2lo) * n + ln1pa
    cpp = c * c * p(c)
    log(x) = hi + (c + (ccp + lo))
  base b = 2:
    hi = n + logb1pa
    cpp = c * c * p(c)
    lo = (c + ccp) * logb(e)
    log_b(x) = hi + lo
  base b = 10:
    hi + lo = "n * logb2" + logb1pa
    cpp = c * c * p(c)
    log_b(x) = hi + ((c + ccp) * logb(e) + lo)
*/

static uint32_t small_cut = 0x00100000;
static uint32_t large_cut = 0x7fe00000;

/*
Here is the high level approach: 
x = 2^n (1+f)
if n == 0
    (1+f) = (1+a)(1+c)(1+s)
    c = invap1 * ( (1+f) - (1+a) )
    s = "1/(1+c)" * invap1 * ( ( ( (1+f) - (1+a) ) - c) - a*c )
      = ( (1 - c) * invap1 ) * ( ( ( (1+f) - (1+a) ) - c) - a*c )
  base b=2;
    hi + lo = logb1pa + "logb(e) * c"
    cpp = c * c * p(c)
    s = ( (1 - c) * invap1 ) * ( ( ( (1+f) - (1+a) ) - c) - a*c )
    log_b(x) = hi + (logb(e)*(ccp + s) + lo)

if n != 0
    (1+f) = (1+a)(1+c)
    c = invap1 * ( (1+f) - (1+a) )
  base b=2:
    hi = n + logb1pa
    cpp = c * c * p(c)
    lo = (c + ccp) * logb(e)
    log_b(x) = hi + lo
*/

double log10(double x){
    //In Ian's early out test [+Inf, nan, -nan, -Inf, -x, -0, 0, denorm]
    union{ double d; uint64_t u; }u = { x };

    uint32_t xhi = (uint32_t) (u.u >> 32);
    uint32_t xlo = (uint32_t) u.u;
    //on arm, no code should be generated up to this point

    int n; 
    int k; 
    double fp1;
    // Early out if x is NaN, Inf, 0, or <0
    // I.e., if the encoding of x is not in [0010000000000000, 7ff0000000000000)
    if( xhi - small_cut >= large_cut ) { 
            if((xhi & 0x7ff00000) == 0x7ff00000) { // nan, inf and -inf
                    return x+__builtin_inf(); // qnan+inf -> qnan, snan+inf -> qnan#I, +inf+inf -> +inf, -inf+inf -> NaN#I
            }
            if(xhi & 0x80000000) {
                    if(xhi == 0x80000000 && xlo == 0) { // x == -0
                            RAISE_ZERO_DIVIDE;  // Get the flags right
                            return -__builtin_inf();
                    }
                    // raise invalid
                    return sqrt(mone); // x < 0.0
            }
			// Treat denormals as zero, return -Inf
			RAISE_ZERO_DIVIDE; // Gets the flags right
			return -__builtin_inf();
    } else {
      fp1 = frexp_near1_normal_32(xhi, xlo, &n, &k);
      // Frexp gives value in [0.5, 1) we want [1, 2) (really want [0.75, 1.5))
    }
//    const double f = fp1 - 1.0;
    
// x = 2^n (1+f)
    double result;
    if(n == 0) {

/*
if n == 0
    (1+f) = (1+a)(1+c)(1+s)
    c = invap1 * ( (1+f) - (1+a) )
    s = "1/(1+c)" * invap1 * ( ( ( (1+f) - (1+a) ) - c) - a*c )
      = ( (1 - c) * invap1 ) * ( ( ( (1+f) - (1+a) ) - c) - a*c )
  base 10:
    hi + lo = logb1pa + "logb(e) * c"
    cpp = c * c * p(c)
    s = ( (1 - c) * invap1 ) * ( ( ( (1+f) - (1+a) ) - c) - a*c )
    log_b(x) = hi + (logb(e)*(ccp + s) + lo)
*/
        const double ap1 = LOG10_LOOKUP[k].ap1;
        const double va = LOG10_LOOKUP[k].inv;
/*
Solve (1+f)=(1+a)(1+f1) for f1:
    f1 = ((1+f) - (1+a) ) / (1+a) = (f - a) * Va
       = (fp1 - ap1) * Va
       = R(h * Va)
Here we let h = R(fp1 - ap1)
*/

        const double h = fp1 - ap1; // h = R(f-a)
        const double f1 = h * va; // f1 = R(h * Va)
#if FENVON
        if((xhi == 0x3ff00000)&&(xlo == 0)) return h; // return 0 without raising inexact if x == 1.0
#endif

        const double a = ap1 - one;

        const double c = f1;
        
        const double q0 = (c+a0)*c+b0;
        const double q1 = (c+a1)*c+b1;
        const double p = c0 + (c * c5_e) * (q0 * q1);

        const double log1pa = LOG10_LOOKUP[k].log1p; // log10(a+1) from table
	// Dekker's algorithm: http://www.cs.cmu.edu/~quake-papers/related/Priest.ps
	const double dekker_const = 0x1.0000004p26;
	const double cy = c * dekker_const;
	const double cz = cy - c;
	const double chi = cy - cz;
	const double clo = c - chi;
        const double clog10ehi = chi * log10ehi;

//      s = ( (1 - c) * invap1 ) * ( ( ( (1+f) - (1+a) ) - c) - a*c )
        const double cinvest = one - c;
        const double s = ((h - c) - (a * c)) * (cinvest * va);

        const double clog10elo = c * log10elo + clo * log10ehi;

        const double hi = log1pa + clog10ehi;
        const double lo = ((log1pa - hi) + clog10ehi) + clog10elo;
        
        const double cc = c * c;
        const double ccp = cc * p; // R(c*c*p(c))
        
        const double logx = hi + (log10e*(ccp + s) + lo);

#if TRACE
        if(c > Max_c) Max_c = c;
        if(c < Min_c) Min_c = c;
#if DEBUG
	if(Debug_Verbose>1) {
	  fprintf(stderr,"x  =\t%a = 2^%d (%a) (1 + %a) (1 + %a) = \n   =\t%a \n",x,0,ap1,c,s,(double)(ap1*(1.0+(long double)c)*(1.0 + (long double) s)));
	  fprintf(stderr,"log(1+c)\t= %La = %a + %a * %a\n\t\t= %La\n", (long double)logl(1.0+(long double)c), c, cc, p, (long double)c+ccp);
	  fprintf(stderr,"log10(x)\t= %La = %a + (%a * (%a + %a) + %a)\n   \t= %La\n", (long double)log10l((long double)x), hi, lge, ccp, s, lo, (long double)logx);
	  fprintf(stderr,"hi + lo = %La = %a + %a \n", (long double)hi+lo, hi, lo);
	}
#endif	
#endif
        result = logx;
        
            
    } else {

/*
  base b=2:
    hi = n + logb1pa
    (1+f) = (1+a)(1+c)
    c = invap1 * ( (1+f) - (1+a) )
    cpp = c * c * p(c)
    lo = (c + ccp) * logb(e)
    log_b(x) = hi + lo
*/
        const double nd = (double)n; 

        const double ap1 = LOG10_LOOKUP_BIG[k].ap1;
        const double va = LOG10_LOOKUP_BIG[k].inv;
/*
Solve (1+f)=(1+a)(1+f1) for f1:
    f1 = ((1+f) - (1+a) ) / (1+a) = (f - a) * Va
       = (fp1 - ap1) * Va
       = R(h * Va)
Here we let h = R(fp1 - ap1)
*/

        const double h = fp1 - ap1; // h = R(f-a)
        const double f1 = h * va; // f1 = R(h * Va)
        
        const double log1pa = LOG10_LOOKUP_BIG[k].log1p;
        const double nlog102hi = nd * log102hi; // exact
                
        const double c = f1;
#if EXACT_LOG10_10_N
        //check to see if we have and exact power of 10 and avoid inexact
        int index;

  #if AVOID_64_BIT_INTS
        union{ double d; uint64_t u; } u = { x };
        const uint32_t key = (((uint32_t)(u.u>>32)) >> (53-32)) & 0x003f;
        index = (int)key; //the top 6 of bottom 7 bits of exponent of x
  #else
        union{ double d; uint64_t u; } u = { x };
        uint64_t key = (u.u & 0x07e0000000000000ULL)>>53;
        index = (int)key; //the top 6 of bottom 7 bits of exponent of x
  #endif

        if(isPowerOf10[index][0] == x) {return  isPowerOf10[index][1];}
#endif 

        RAISE_INEXACT; // Raise inexact (possibly a false alarm for log10(10^n)

        const double q0 = (c+a0)*c+b0;
        const double q1 = (c+a1)*c+b1;
        const double p = c0 + (c * c5_e) * (q0 * q1);

#if TRACE
        if(c > Max_c) Max_c = c;
        if(c < Min_c) Min_c = c;
#endif

        const double clog10e = c * log10e;
        const double cclog10e = c * clog10e;
        const double ccplog10e = cclog10e * p; // R(c*c*p(c))
        
        const double nlog102lo = nd * log102lo;
        const double hi = nlog102hi + log1pa;
        const double lo = ((nlog102hi - hi) + log1pa) + nlog102lo;
        
        const double logx = hi + ((lo + clog10e) + ccplog10e);

        result = logx;
    }

    return result;
}


/*
if -1/4 < x < 1/2 for log(1+x)
    f = x
    (1+f) = (1+a)(1+c)(1+s)
    c = invap1 * ( (1+f) - (1+a) )
    s = "1/(1+c)" * invap1 * ( ( ( (1+f) - (1+a) ) - c) - a*c )
      = ( (1 - c) * invap1 ) * ( ( ( (1+f) - (1+a) ) - c) - a*c )
    hi + lo = ln1pa + c
    cpp = c * c * p(c)
    s = ( (1 - c) * invap1 ) * ( ( ( (1+f) - (1+a) ) - c) - a*c )
    log(x) = hi + (ccp + s + lo)

if -1 < x < -1/4 or 1/2 < x < Inf for log(1+x)
    xp1 = x+1
    if (xp1-1)==x:
        log(xp1)
    if (xp1-1)!=x: "log(xp1+(xp1-1))"
        u = xp1-1;
        (1+f+u) = (1+a)(1+c)
        c = invap1 * ( (1+f) - (1+a) + u)
        hi + lo = (ln2hi + ln2lo) * n + ln1pa
        cpp = c * c * p(c)
        log(x) = hi + (c + (ccp + lo))
*/

double log1p(double x) {
    //In Ian's early out test [+Inf, nan, -nan, -Inf, -x, -0, 0, denorm]
    union{ double d; uint64_t u; }u = { x };

    uint32_t xhi = (uint32_t) (u.u >> 32);
    uint32_t xlo = (uint32_t) u.u;
    //on arm, no code should be generated up to this point

    if(!( xhi < 0x7ff00000 || (0x80000000 <= xhi && xhi < 0xbff00000 ))) { 
        if((xhi & 0x7ff00000) == 0x7ff00000) { // nan, inf and -inf
            return x+__builtin_inf(); // qnan+inf -> qnan, snan+inf -> qnan#I, +inf+inf -> +inf, -inf+inf -> NaN#I
        }
        if(xhi >= 0xbff00000) { // x > -1.0
            if(xhi == 0xbff00000 && xlo == 0) { // x == -1.0
                RAISE_ZERO_DIVIDE;
                return -__builtin_inf(); // todo get flags right
            }
            // raise invalid
            return sqrt(mone); // x < 0.0
        }
    }
    double result;
    if(( 0x80000000 <= xhi && (0xbfd00000 + (1 >= xlo)) > xhi) || xhi < 0x3fe00000) { // if -0.25 <= x && x < 0.5
        const int32_t abs2xhi = xhi << 1; 
        if(abs2xhi < 0x7e200000) { // if(absx < 0x1p-14) ...
            if(abs2xhi < 0x79200000) { // if(absx < 0x1p-54) ...
                if(abs2xhi == 0 && xlo == 0) {
                        return x;  
                } else {
                        RAISE_INEXACT; 
                        return x; // Not quite right for other rounding modes
                }
            } else {
                    const double p = ( (-0.25 * x + 0x0.55555555555555p0) * x + -0.5) * (x*x) + x;
                    return p; 
            }
        }
        const int k = get_k(x); // get k, know n == 0
        const double f = x;
//        const double fp1 = f+one;

//        const double ap1 = LOG_LOOKUP[k].ap1;
        const double a = LOG_LOOKUP[k].a;
        const double va = LOG_LOOKUP[k].inv;
/*
Solve (1+f)=(1+a)(1+f1) for f1:
    f1 = ((1+f) - (1+a) ) / (1+a) = (f - a) * Va
       = (fp1 - ap1) * Va
       = R(h * Va)
Here we let h = R(fp1 - ap1)
*/

        const double h = f - a; // h = R(f-a)
        const double f1 = h * va; // f1 = R(h * Va)
        
//        const double a = ap1 - one;

        const double c = f1;
        
        const double q0 = (c+a0)*c+b0;
        const double q1 = (c+a1)*c+b1;
        const double p = c0 + (c * c5_e) * (q0 * q1);

//      s = ( (1 - c) * invap1 ) * ( ( ( (1+f) - (1+a) ) - c) - a*c )
        const double cinvest = one - c;
        const double s = ((h - c) - (a * c)) * (cinvest * va);

        const double log1pa = LOG_LOOKUP[k].log1p;

        const double hi = log1pa + c;
        const double lo = (log1pa - hi) + c;

        const double cc = c * c;
        const double ccp = cc * p; // R(c*c*p(c))
        
        const double logx = hi + (ccp + (s + lo));

        result = logx;
    } else {
/*
This does not work for x in [-0.5, 1.0) since we lose a bit when adding one. 
Let r = ULP(x). x = N * r =  2^e N/2^52 = 2^n (fp1 + u) with -3/4 <= fp1 < 3/2, representable in double. 
If N is odd, then x+1.0 might not be representable in double. 
In that case, we set
    xm1 = x - r.
    xp1 = xm1+1.0 = x + 1.0 - r = 2^n (fp1)
    If -1 < x < -1/4, ...
    if x > 1/2, ...
    If 3/2 > fp1 >= 1, u = 2^-n r = 2^-n 2^e-52 = 2^-53 since n = e+1
    If 3/4 <= fp1 < 1, u = 2^-n r = 2^-n 2^e-52 = 2^-54 since n = e+2
*/          
        double xp1 = x+one; // 1+x = xp1 + 2^n u
        double halfu = 0.0;
        const double r = x < one ? (one - xp1) + x : (x - xp1) + one;
        
        union{ double d; uint64_t u; }up1 = { xp1 };

        uint32_t xp1hi = (uint32_t) (up1.u >> 32);
        uint32_t xp1lo = (uint32_t) up1.u;
        double fp1; // = frexp_near1_normal_32(xp1hi, xp1lo, &n, &k);

        union{ double d; uint64_t u; } u ;

        const uint32_t        exponent = xp1hi >> (52-32);
        uint32_t        mantissahi = xp1hi & 0x000fffff;
        const uint32_t        mantissalo = xp1lo;

        // or in -1 + bias to the new exponent for the mantissa
        const uint32_t        isupperhalf = (xp1hi>> (51-32)) & 0x1;
        const uint32_t        upperhalf = xp1hi & 0x00080000;
        const uint32_t        head = mantissahi>>(52-32-8); // grab before adjusting for which half we are in
        const int k = head;
        mantissahi = (mantissahi | 0x3ff00000) ^ (upperhalf << 1); // Divide by two if significand >= 3/2
        const int n = exponent-1023 + isupperhalf; //right shift exponent and remove the bias, and store

        if(r) {
                // halfu = scalbn(r, -n);
                const uint32_t adjust = ((uint32_t)(-((n > 64) ? 64 : n)) << (52-32)); // clip everything over 64 or so to avoid denormals for x near Inf. 
                union { double d; uint64_t u; } ur;
                union { double d; uint64_t u; } uhalfu;
                ur.d = r;
                uhalfu.u = ur.u + (((uint64_t)adjust)<<32);
                halfu = uhalfu.d;
#if DEBUG
                if(Debug_Verbose>1) fprintf(stderr,"ur.u  = %llx\t ur.d = %a\t uh.u  = %llx\t uh.d = %a\t\n",ur.u, ur.d, uhalfu.u, uhalfu.d);
#endif
        }
        u.u = ((uint64_t)mantissahi << 32) | (uint64_t)mantissalo;
        fp1 = u.d;

/*
        hi + lo = "n * ln2hi + ln(ap1) + n * ln2lo"
        (1+f+u) = (1+a)(1+c)
        c = invap1 * ( (1+f) - (1+a) + u)
        lnest = hi + (c + (lo + ccp))
*/
        const double nd = (double)n; 

        const double ap1 = LOG_LOOKUP[k].ap1;
        const double va = LOG_LOOKUP[k].inv;
/*
Solve (1+f+u)=(1+a)(1+f1) for f1:
    f1 = ((1+f) - (1+a) + u ) / (1+a) = (f - a + u) * Va
       = (fp1 - ap1 + u) * Va
       = R(h * Va)
Here we let h = R(fp1 - ap1)
*/

        const double h = (fp1 - ap1) + halfu; // h = f - a + u
        const double f1 = h * va; // f1 = R(h * Va)
        
        const double log1pa = LOG_LOOKUP[k].log1p;
        const double nln2hi = nd * ln2hi;
        const double hi = nln2hi + log1pa;

        const double c = f1;
        
        const double q0 = (c+a0)*c+b0;
        const double q1 = (c+a1)*c+b1;
        const double p = c0 + (c * c5_e) * (q0 * q1);

        const double nln2lo = nd * ln2lo;
        const double lo = ((nln2hi - hi) + log1pa) + nln2lo;
        
        const double cc = c * c;
        const double ccp = cc * p; // R(c*c*p(c))
        
        const double logx = hi + (c + (lo + ccp));

#if TRACE
        if(c > Max_c) Max_c = c;
        if(c < Min_c) Min_c = c;
#if DEBUG
	if(Debug_Verbose>1) {
	  fprintf(stderr,"x =\t1+%La = 2^%d (%a) (1 + %a) = \n   =\t1+%La \n",(long double)x,n,ap1,c,((long double)(1<<n)*ap1*(1.0+(long double)c)-1.0l));
	  fprintf(stderr,"h =\t%La = (%a - %a) + %a = \n   =\t%La [r=%a]\n",(long double)h,fp1,ap1,halfu,((long double)fp1 - ap1) + halfu, r);
	  fprintf(stderr,"log(1+c) =\t%a = %a + %a * %a\n\t= \t%a\n", (double)logl(1.0+(long double)c), c, cc, p, c+ccp);
	  fprintf(stderr,"log(x) =\t%La = %a + (%a + (%a + %a))\n   =\t\t%La\n", log1pl((long double)x), hi, c, lo, ccp, (long double)logx);
	}
#endif	
#endif

        result = logx;
    }

    return result;
}
