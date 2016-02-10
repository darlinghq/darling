/*
 *  cbrt.c
 *  armLibm
 *
 *  Created by Jeff Kidder on 5/6/08.
 *  Polynomial by Steve Canon. 
 *  Copyright 2008 Apple Inc. All rights reserved.
 *
 */


/*
Given y we seek r with r*r*r ~ y. 
Write y = 2^(3n) z (1 + (y-z)/z)
where z is chosen so cz = cbrt(z) exactly
and |y-z| < POLY_RANGE
Then cbrt(y) = 2^n cbrt(z) (1 + p((y-z)/z))

Given y0 reduced into [1,8] we use the bottom two bits of exponent and the first six bits of fraction
to look up cz, z, zinv with
z = cz*cz*cz exactly
and zinv ~ 1/z

Compute w = (y-z)*zinv

The table is designed by looking through all cubes of 17-bit significands
which fit in 51-bits. 
 */

#include <math.h>
#include <stdint.h>

#ifndef DAZ
#define DAZ 1
#endif

#ifndef FENV_ACCESS_ON
#define FENV_ACCESS_ON 0
#endif

#ifndef AVOID_UINT64
#define AVOID_UINT64 !defined(__x86_64__)
#endif 

//gcc cbrttable.c -o xct -Wall && ./xct 
static const double cbrt_table[][4] = { // z, zinv, cbrt(z), y0
    {  0x1.0000000000000p+0,   0x1.0000000000000p+0,   0x1.0000000000000p+0, 0x1.000p+0},
    {  0x1.01ff52f6f7680p+0,   0x1.fc0944da7bf12p-1,   0x1.00aa000000000p+0, 0x1.020p+0},
    {  0x1.03fe450d748b0p+0,   0x1.f822dcd67af49p-1,   0x1.0153000000000p+0, 0x1.040p+0},
    {  0x1.05ffd8005fc00p+0,   0x1.f446a644b2ad4p-1,   0x1.01fc000000000p+0, 0x1.060p+0},
    {  0x1.07fdedafcafb0p+0,   0x1.f08004618b8b3p-1,   0x1.02a3000000000p+0, 0x1.080p+0},
    {  0x1.09fe97c0b2e80p+0,   0x1.ecc3168ac46e4p-1,   0x1.034a000000000p+0, 0x1.0a0p+0},
    {  0x1.0bfec00bf0000p+0,   0x1.e91562a43a245p-1,   0x1.03f0000000000p+0, 0x1.0c0p+0},
    {  0x1.0dfe5c5e25ad0p+0,   0x1.e5769f1123ff8p-1,   0x1.0495000000000p+0, 0x1.0e0p+0},
    {  0x1.0ffd628732690p+0,   0x1.e1e683fb1c33bp-1,   0x1.0539000000000p+0, 0x1.100p+0},
    {  0x1.11feebec7a850p+0,   0x1.de5f503ed23bdp-1,   0x1.05dd000000000p+0, 0x1.120p+0},
    {  0x1.13ffd2a000000p+0,   0x1.dae6557eadd2ep-1,   0x1.0680000000000p+0, 0x1.140p+0},
    {  0x1.15fce10fe1610p+0,   0x1.d780b05602805p-1,   0x1.0721000000000p+0, 0x1.160p+0},
    {  0x1.17ff8f34615b0p+0,   0x1.d41dfe67bb981p-1,   0x1.07c3000000000p+0, 0x1.180p+0},
    {  0x1.19fe50c0a63b0p+0,   0x1.d0ce1fc421c47p-1,   0x1.0863000000000p+0, 0x1.1a0p+0},
    {  0x1.1bff7df4f31b0p+0,   0x1.cd863be50966cp-1,   0x1.0903000000000p+0, 0x1.1c0p+0},
    {  0x1.1dffdd62cb880p+0,   0x1.ca4b67cd60a30p-1,   0x1.09a2000000000p+0, 0x1.1e0p+0},
    {  0x1.1fff64e400000p+0,   0x1.c71d66e4123bep-1,   0x1.0a40000000000p+0, 0x1.200p+0},
    {  0x1.21fe0a5741850p+0,   0x1.c3fbfdfac741ep-1,   0x1.0add000000000p+0, 0x1.220p+0},
    {  0x1.23ff0a0459280p+0,   0x1.c0e1ea5c76a8cp-1,   0x1.0b7a000000000p+0, 0x1.240p+0},
    {  0x1.25ff1b1039980p+0,   0x1.bdd413c3714e1p-1,   0x1.0c16000000000p+0, 0x1.260p+0},
    {  0x1.27fe335d41110p+0,   0x1.bad2426aa85cbp-1,   0x1.0cb1000000000p+0, 0x1.280p+0},
    {  0x1.29ff9aaaa2c00p+0,   0x1.b7d7596e80007p-1,   0x1.0d4c000000000p+0, 0x1.2a0p+0},
    {  0x1.2bfffca2a3580p+0,   0x1.b4e82034f4c4bp-1,   0x1.0de6000000000p+0, 0x1.2c0p+0},
    {  0x1.2dff4f2c6b7f0p+0,   0x1.b2046226c200bp-1,   0x1.0e7f000000000p+0, 0x1.2e0p+0},
    {  0x1.2ffd88352c870p+0,   0x1.af2bebe0bbfcbp-1,   0x1.0f17000000000p+0, 0x1.300p+0},
    {  0x1.31fdfea9140f0p+0,   0x1.ac59d0441d895p-1,   0x1.0faf000000000p+0, 0x1.320p+0},
    {  0x1.33fd4f07fbd80p+0,   0x1.a992adfec1582p-1,   0x1.1046000000000p+0, 0x1.340p+0},
    {  0x1.35fed7c563850p+0,   0x1.a6d1ae764bb51p-1,   0x1.10dd000000000p+0, 0x1.360p+0},
    {  0x1.37ff2dd5dfeb0p+0,   0x1.a41b5ca078b6cp-1,   0x1.1173000000000p+0, 0x1.380p+0},
    {  0x1.39fe472d82000p+0,   0x1.a16f89a08f150p-1,   0x1.1208000000000p+0, 0x1.3a0p+0},
    {  0x1.3bff8d8172c50p+0,   0x1.9ec97f9a8da23p-1,   0x1.129d000000000p+0, 0x1.3c0p+0},
    {  0x1.3dff8a8664910p+0,   0x1.9c2dad3273edcp-1,   0x1.1331000000000p+0, 0x1.3e0p+0},
    {  0x1.3ffe343874400p+0,   0x1.999be62171209p-1,   0x1.13c4000000000p+0, 0x1.400p+0},
    {  0x1.41feff7b08470p+0,   0x1.970f93c925efcp-1,   0x1.1457000000000p+0, 0x1.420p+0},
    {  0x1.43fe6ad7ff790p+0,   0x1.948d09afad30dp-1,   0x1.14e9000000000p+0, 0x1.440p+0},
    {  0x1.45fff29b8c030p+0,   0x1.920fc5216d6aep-1,   0x1.157b000000000p+0, 0x1.460p+0},
    {  0x1.47fc83ee37330p+0,   0x1.8fa057d9553bap-1,   0x1.160b000000000p+0, 0x1.480p+0},
    {  0x1.49feb2bc0dc00p+0,   0x1.8d31a9f2d47fbp-1,   0x1.169c000000000p+0, 0x1.4a0p+0},
    {  0x1.4bff687f1cc00p+0,   0x1.8acc451fcfb49p-1,   0x1.172c000000000p+0, 0x1.4c0p+0},
    {  0x1.4dfe9b4104c30p+0,   0x1.887002348663ap-1,   0x1.17bb000000000p+0, 0x1.4e0p+0},
    {  0x1.4fffd9b64ee80p+0,   0x1.86188df9db60ep-1,   0x1.184a000000000p+0, 0x1.500p+0},
    {  0x1.51ff889bc6000p+0,   0x1.83ca00a5a8f32p-1,   0x1.18d8000000000p+0, 0x1.520p+0},
    {  0x1.53fd9e064b9d0p+0,   0x1.81843523cc8a7p-1,   0x1.1965000000000p+0, 0x1.540p+0},
    {  0x1.55fdb39fad480p+0,   0x1.7f42f32ce62adp-1,   0x1.19f2000000000p+0, 0x1.560p+0},
    {  0x1.57ffca688f7f0p+0,   0x1.7d062f73dabb5p-1,   0x1.1a7f000000000p+0, 0x1.580p+0},
    {  0x1.59fc8db9a7e80p+0,   0x1.7ad5e68ed5f8cp-1,   0x1.1b0a000000000p+0, 0x1.5a0p+0},
    {  0x1.5bfef420a3980p+0,   0x1.78a5ea042bcf5p-1,   0x1.1b96000000000p+0, 0x1.5c0p+0},
    {  0x1.5dffa558e0610p+0,   0x1.767e2f4271824p-1,   0x1.1c21000000000p+0, 0x1.5e0p+0},
    {  0x1.5ffe978200130p+0,   0x1.745e94a033f66p-1,   0x1.1cab000000000p+0, 0x1.600p+0},
    {  0x1.61ff79fee48d0p+0,   0x1.7243141d712b5p-1,   0x1.1d35000000000p+0, 0x1.620p+0},
    {  0x1.63fe90eb04f80p+0,   0x1.702f816650015p-1,   0x1.1dbe000000000p+0, 0x1.640p+0},
    {  0x1.65ff92d8b0170p+0,   0x1.6e1fe655abc69p-1,   0x1.1e47000000000p+0, 0x1.660p+0},
    {  0x1.67febcb6716f0p+0,   0x1.6c180862ccf0dp-1,   0x1.1ecf000000000p+0, 0x1.680p+0},
    {  0x1.69ffcc3fb9470p+0,   0x1.6a1400d859f3ep-1,   0x1.1f57000000000p+0, 0x1.6a0p+0},
    {  0x1.6bfef73ce6780p+0,   0x1.6817875379f1cp-1,   0x1.1fde000000000p+0, 0x1.6c0p+0},
    {  0x1.6dfc33e542400p+0,   0x1.66227ddbe3339p-1,   0x1.2064000000000p+0, 0x1.6e0p+0},
    {  0x1.6fff1cd666d30p+0,   0x1.642d616dfe741p-1,   0x1.20eb000000000p+0, 0x1.700p+0},
    {  0x1.71fc3c5870000p+0,   0x1.6243421ae7a84p-1,   0x1.2170000000000p+0, 0x1.720p+0},
    {  0x1.73ff09dbd4180p+0,   0x1.6058ff28cfec5p-1,   0x1.21f6000000000p+0, 0x1.740p+0},
    {  0x1.75ffd7375b030p+0,   0x1.5e75e1c484452p-1,   0x1.227b000000000p+0, 0x1.760p+0},
    {  0x1.77fe9aa868ff0p+0,   0x1.5c99ce06a4e7cp-1,   0x1.22ff000000000p+0, 0x1.780p+0},
    {  0x1.79ff2e4cfe9b0p+0,   0x1.5ac1170d9d9c5p-1,   0x1.2383000000000p+0, 0x1.7a0p+0},
    {  0x1.7bfdab9b30d80p+0,   0x1.58ef4064450c1p-1,   0x1.2406000000000p+0, 0x1.7c0p+0},
    {  0x1.7dfdf3b868590p+0,   0x1.5720aab2d03bfp-1,   0x1.2489000000000p+0, 0x1.7e0p+0},
    {  0x1.7ffc19187c330p+0,   0x1.5558cd4897700p-1,   0x1.250b000000000p+0, 0x1.800p+0},
    {  0x1.81fff5b8acb80p+0,   0x1.53909d9a091b4p-1,   0x1.258e000000000p+0, 0x1.820p+0},
    {  0x1.83fdb4db3f2f0p+0,   0x1.51d27de463703p-1,   0x1.260f000000000p+0, 0x1.840p+0},
    {  0x1.85fd33ff90000p+0,   0x1.50176a58004f0p-1,   0x1.2690000000000p+0, 0x1.860p+0},
    {  0x1.87fe73ea28310p+0,   0x1.4e5f5c4e64e37p-1,   0x1.2711000000000p+0, 0x1.880p+0},
    {  0x1.89fd759d99b10p+0,   0x1.4cadad9b3188bp-1,   0x1.2791000000000p+0, 0x1.8a0p+0},
    {  0x1.8bfe32aa8b310p+0,   0x1.4afeeba007be7p-1,   0x1.2811000000000p+0, 0x1.8c0p+0},
    {  0x1.8dfca52590000p+0,   0x1.495664ea7f47dp-1,   0x1.2890000000000p+0, 0x1.8e0p+0},
    {  0x1.8ffccd8c282f0p+0,   0x1.47b0b2fa2bda1p-1,   0x1.290f000000000p+0, 0x1.900p+0},
    {  0x1.91feac99dcb80p+0,   0x1.460dcfc6ed440p-1,   0x1.298e000000000p+0, 0x1.920p+0},
    {  0x1.93fe3206e6c00p+0,   0x1.4470f955a1827p-1,   0x1.2a0c000000000p+0, 0x1.940p+0},
    {  0x1.95ff68a951e80p+0,   0x1.42d6dab45c848p-1,   0x1.2a8a000000000p+0, 0x1.960p+0},
    {  0x1.97fe3958b2570p+0,   0x1.4142a7401970ep-1,   0x1.2b07000000000p+0, 0x1.980p+0},
    {  0x1.99feb5cb28400p+0,   0x1.3fb115744cd56p-1,   0x1.2b84000000000p+0, 0x1.9a0p+0},
    {  0x1.9bfcc00000000p+0,   0x1.3e254e465d72cp-1,   0x1.2c00000000000p+0, 0x1.9c0p+0},
    {  0x1.9dfc708557c00p+0,   0x1.3c9c1357411b6p-1,   0x1.2c7c000000000p+0, 0x1.9e0p+0},
    {  0x1.9ffdc809be000p+0,   0x1.3b155f697ed3ap-1,   0x1.2cf8000000000p+0, 0x1.a00p+0},
    {  0x1.a1fc9e5153eb0p+0,   0x1.39944c2e925eap-1,   0x1.2d73000000000p+0, 0x1.a20p+0},
    {  0x1.a3fd162291380p+0,   0x1.3815ab70c411ep-1,   0x1.2dee000000000p+0, 0x1.a40p+0},
    {  0x1.a5ff3027d3f90p+0,   0x1.36997838092b7p-1,   0x1.2e69000000000p+0, 0x1.a60p+0},
    {  0x1.a7feb9f2d5bb0p+0,   0x1.3522bd6b8de66p-1,   0x1.2ee3000000000p+0, 0x1.a80p+0},
    {  0x1.a9ffe079fb050p+0,   0x1.33ae5c7a1ab67p-1,   0x1.2f5d000000000p+0, 0x1.aa0p+0},
    {  0x1.abfe6a8b1e980p+0,   0x1.323f56c09d10dp-1,   0x1.2fd6000000000p+0, 0x1.ac0p+0},
    {  0x1.adfe8be115ef0p+0,   0x1.30d297ddf683ap-1,   0x1.304f000000000p+0, 0x1.ae0p+0},
    {  0x1.affc048ecf970p+0,   0x1.2f6b17dd4ba38p-1,   0x1.30c7000000000p+0, 0x1.b00p+0},
    {  0x1.b1ff52f400000p+0,   0x1.2e02d4701d501p-1,   0x1.3140000000000p+0, 0x1.b20p+0},
    {  0x1.b3fff337ce000p+0,   0x1.2c9fbda84458dp-1,   0x1.31b8000000000p+0, 0x1.b40p+0},
    {  0x1.b5fddbe6eb8f0p+0,   0x1.2b41c14adb3a1p-1,   0x1.322f000000000p+0, 0x1.b60p+0},
    {  0x1.b7fd518be4580p+0,   0x1.29e5e36014bd4p-1,   0x1.32a6000000000p+0, 0x1.b80p+0},
    {  0x1.b9fe54c100450p+0,   0x1.288c1fce9874bp-1,   0x1.331d000000000p+0, 0x1.ba0p+0},
    {  0x1.bbfc917f394b0p+0,   0x1.273753a633b82p-1,   0x1.3393000000000p+0, 0x1.bc0p+0},
    {  0x1.bdfc56545ed90p+0,   0x1.25e490e965a48p-1,   0x1.3409000000000p+0, 0x1.be0p+0},
    {  0x1.bffda3d6dd7f0p+0,   0x1.2493d3b473c59p-1,   0x1.347f000000000p+0, 0x1.c00p+0},
    {  0x1.c1fc1c0569400p+0,   0x1.2347ec39d66b0p-1,   0x1.34f4000000000p+0, 0x1.c20p+0},
    {  0x1.c3fc176738f90p+0,   0x1.21fdfa077410fp-1,   0x1.3569000000000p+0, 0x1.c40p+0},
    {  0x1.c5fd968eee780p+0,   0x1.20b5f96c5276cp-1,   0x1.35de000000000p+0, 0x1.c60p+0},
    {  0x1.c7fc318b71c80p+0,   0x1.1f72ae201c271p-1,   0x1.3652000000000p+0, 0x1.c80p+0},
    {  0x1.c9fc4ad339d80p+0,   0x1.1e3144d16fd97p-1,   0x1.36c6000000000p+0, 0x1.ca0p+0},
    {  0x1.cbfde2f52e280p+0,   0x1.1cf1b9ff60523p-1,   0x1.373a000000000p+0, 0x1.cc0p+0},
    {  0x1.cdfc881b2e750p+0,   0x1.1bb6c545bdcc4p-1,   0x1.37ad000000000p+0, 0x1.ce0p+0},
    {  0x1.cffca6a080000p+0,   0x1.1a7da00d343a3p-1,   0x1.3820000000000p+0, 0x1.d00p+0},
    {  0x1.d1fe3f10604b0p+0,   0x1.1946470258170p-1,   0x1.3893000000000p+0, 0x1.d20p+0},
    {  0x1.d3fcd5bab37d0p+0,   0x1.18136600a7357p-1,   0x1.3905000000000p+0, 0x1.d40p+0},
    {  0x1.d5fce0d4d1a70p+0,   0x1.16e242c804dcap-1,   0x1.3977000000000p+0, 0x1.d60p+0},
    {  0x1.d7fe60e65e790p+0,   0x1.15b2da30214b8p-1,   0x1.39e9000000000p+0, 0x1.d80p+0},
    {  0x1.d9fcd07097a80p+0,   0x1.1487ccac6a7c7p-1,   0x1.3a5a000000000p+0, 0x1.da0p+0},
    {  0x1.dbfcaf77e3730p+0,   0x1.135e6bf4cdb3cp-1,   0x1.3acb000000000p+0, 0x1.dc0p+0},
    {  0x1.ddfdfe805bc00p+0,   0x1.1236b509d4023p-1,   0x1.3b3c000000000p+0, 0x1.de0p+0},
    {  0x1.dffc2e4834c00p+0,   0x1.11133d4bfd103p-1,   0x1.3bac000000000p+0, 0x1.e00p+0},
    {  0x1.e1fbc89795c00p+0,   0x1.0ff1620f911f9p-1,   0x1.3c1c000000000p+0, 0x1.e20p+0},
    {  0x1.e3fccdef1ec00p+0,   0x1.0ed1207bcff2ep-1,   0x1.3c8c000000000p+0, 0x1.e40p+0},
    {  0x1.e5ff3ecf6fc00p+0,   0x1.0db275be001a6p-1,   0x1.3cfc000000000p+0, 0x1.e60p+0},
    {  0x1.e7fe7efef0530p+0,   0x1.0c97e8e372825p-1,   0x1.3d6b000000000p+0, 0x1.e80p+0},
    {  0x1.e9ff253c51a80p+0,   0x1.0b7ee62f4c332p-1,   0x1.3dda000000000p+0, 0x1.ea0p+0},
    {  0x1.ebfc8edc32000p+0,   0x1.0a69edc7b41dbp-1,   0x1.3e48000000000p+0, 0x1.ec0p+0},
    {  0x1.edffff705d670p+0,   0x1.0953f3dd361c7p-1,   0x1.3eb7000000000p+0, 0x1.ee0p+0},
    {  0x1.effb845986400p+0,   0x1.084473fb1fdadp-1,   0x1.3f24000000000p+0, 0x1.f00p+0},
    {  0x1.f1fd112ab0c80p+0,   0x1.0733ed7907e73p-1,   0x1.3f92000000000p+0, 0x1.f20p+0},
    {  0x1.f400000000000p+0,   0x1.0624dd2f1a9fcp-1,   0x1.4000000000000p+0, 0x1.f40p+0},
    {  0x1.f5ff9e1d82b50p+0,   0x1.0519b266e0edep-1,   0x1.406d000000000p+0, 0x1.f60p+0},
    {  0x1.f7fbe263ab490p+0,   0x1.041260b1fc714p-1,   0x1.40d9000000000p+0, 0x1.f80p+0},
    {  0x1.f9fe36d7a7d80p+0,   0x1.030a055aebeddp-1,   0x1.4146000000000p+0, 0x1.fa0p+0},
    {  0x1.fbfd2bfe5a480p+0,   0x1.020577cc283d6p-1,   0x1.41b2000000000p+0, 0x1.fc0p+0},
    {  0x1.fdfd78ac81780p+0,   0x1.0102473781e2ep-1,   0x1.421e000000000p+0, 0x1.fe0p+0},
    {  0x1.ffff1d5571e80p+0,   0x1.0000715579388p-1,   0x1.428a000000000p+0, 0x1.000p+1},
    {  0x1.01fe6eb8c0000p+1,   0x1.fc0b064e0ed42p-2,   0x1.4360000000000p+0, 0x1.020p+1},
    {  0x1.03fff4b10b8c0p+1,   0x1.f81f97e54183ap-2,   0x1.4436000000000p+0, 0x1.040p+1},
    {  0x1.05ff4c356ff40p+1,   0x1.f447b132ca3acp-2,   0x1.450a000000000p+0, 0x1.060p+1},
    {  0x1.07fed2419d428p+1,   0x1.f07e5681bb219p-2,   0x1.45dd000000000p+0, 0x1.080p+1},
    {  0x1.09fe7f156c878p+1,   0x1.ecc3443e00aeap-2,   0x1.46af000000000p+0, 0x1.0a0p+1},
    {  0x1.0bfe4af000000p+1,   0x1.e916385d22026p-2,   0x1.4780000000000p+0, 0x1.0c0p+1},
    {  0x1.0dfe2e0fe8000p+1,   0x1.e576f253c657bp-2,   0x1.4850000000000p+0, 0x1.0e0p+1},
    {  0x1.0ffe20b347af8p+1,   0x1.e1e5330b8cb6ap-2,   0x1.491f000000000p+0, 0x1.100p+1},
    {  0x1.11fe1b17f99a8p+1,   0x1.de60bcd933127p-2,   0x1.49ed000000000p+0, 0x1.120p+1},
    {  0x1.13fe157bb4140p+1,   0x1.dae953730a179p-2,   0x1.4aba000000000p+0, 0x1.140p+1},
    {  0x1.15fe081c2d6c0p+1,   0x1.d77ebbe7b308bp-2,   0x1.4b86000000000p+0, 0x1.160p+1},
    {  0x1.17fdeb373ff88p+1,   0x1.d420bc9525242p-2,   0x1.4c51000000000p+0, 0x1.180p+1},
    {  0x1.19fdb70b0df18p+1,   0x1.d0cf1d1ff8234p-2,   0x1.4d1b000000000p+0, 0x1.1a0p+1},
    {  0x1.1bfff110868e8p+1,   0x1.cd8580d5a9203p-2,   0x1.4de5000000000p+0, 0x1.1c0p+1},
    {  0x1.1dff7a217dba8p+1,   0x1.ca4c06da42de5p-2,   0x1.4ead000000000p+0, 0x1.1e0p+1},
    {  0x1.1ffed4a690a00p+1,   0x1.c71e4ad4c7f13p-2,   0x1.4f74000000000p+0, 0x1.200p+1},
    {  0x1.21fdf8e05d140p+1,   0x1.c3fc193327136p-2,   0x1.503a000000000p+0, 0x1.220p+1},
    {  0x1.23ff788000000p+1,   0x1.c0e1408494bbcp-2,   0x1.5100000000000p+0, 0x1.240p+1},
    {  0x1.25fe1bf00a200p+1,   0x1.bdd596a6d86afp-2,   0x1.51c4000000000p+0, 0x1.260p+1},
    {  0x1.27ff1159f1000p+1,   0x1.bad0f650df847p-2,   0x1.5288000000000p+0, 0x1.280p+1},
    {  0x1.29ffb78cd0798p+1,   0x1.b7d72eccd2063p-2,   0x1.534b000000000p+0, 0x1.2a0p+1},
    {  0x1.2bfd613ee3600p+1,   0x1.b4ebec32620a1p-2,   0x1.540c000000000p+0, 0x1.2c0p+1},
    {  0x1.2dfff74f29dc0p+1,   0x1.b203708429799p-2,   0x1.54ce000000000p+0, 0x1.2e0p+1},
    {  0x1.2fff8163765c0p+1,   0x1.af291f5c88af1p-2,   0x1.558e000000000p+0, 0x1.300p+1},
    {  0x1.31fe9d4aa4aa8p+1,   0x1.ac58f2343e704p-2,   0x1.564d000000000p+0, 0x1.320p+1},
    {  0x1.33fff4d36b600p+1,   0x1.a98f0577115b3p-2,   0x1.570c000000000p+0, 0x1.340p+1},
    {  0x1.35fe202fff0c8p+1,   0x1.a6d2a8dd1463fp-2,   0x1.57c9000000000p+0, 0x1.360p+1},
    {  0x1.37fe7db2eb6c0p+1,   0x1.a41c49cc6d94fp-2,   0x1.5886000000000p+0, 0x1.380p+1},
    {  0x1.39fe541ac7840p+1,   0x1.a16f787114257p-2,   0x1.5942000000000p+0, 0x1.3a0p+1},
    {  0x1.3bfd9bb03ef28p+1,   0x1.9ecc0d110a8a2p-2,   0x1.59fd000000000p+0, 0x1.3c0p+1},
    {  0x1.3dff0d1d47000p+1,   0x1.9c2e4fc0421fep-2,   0x1.5ab8000000000p+0, 0x1.3e0p+1},
    {  0x1.3fffe635cfa40p+1,   0x1.9999ba9c69854p-2,   0x1.5b72000000000p+0, 0x1.400p+1},
    {  0x1.41fd58fd78b40p+1,   0x1.9711a9e674cc2p-2,   0x1.5c2a000000000p+0, 0x1.420p+1},
    {  0x1.43ffb09017dd8p+1,   0x1.948b72fcbd74cp-2,   0x1.5ce3000000000p+0, 0x1.440p+1},
    {  0x1.45fe926c3b540p+1,   0x1.9211777e6b069p-2,   0x1.5d9a000000000p+0, 0x1.460p+1},
    {  0x1.47ff8c393af88p+1,   0x1.8f9ca60787426p-2,   0x1.5e51000000000p+0, 0x1.480p+1},
    {  0x1.49ffcd0f472b8p+1,   0x1.8d305622d9ea4p-2,   0x1.5f07000000000p+0, 0x1.4a0p+1},
    {  0x1.4bff4d3e99e00p+1,   0x1.8acc65880024fp-2,   0x1.5fbc000000000p+0, 0x1.4c0p+1},
    {  0x1.4dfe051ab8000p+1,   0x1.8870b2a123748p-2,   0x1.6070000000000p+0, 0x1.4e0p+1},
    {  0x1.4ffec7aef3200p+1,   0x1.8619cc20b8cf4p-2,   0x1.6124000000000p+0, 0x1.500p+1},
    {  0x1.51feb8683c638p+1,   0x1.83caef8563317p-2,   0x1.61d7000000000p+0, 0x1.520p+1},
    {  0x1.53fdcf9ca12c8p+1,   0x1.8183fce9b0418p-2,   0x1.6289000000000p+0, 0x1.540p+1},
    {  0x1.55fee906d1a18p+1,   0x1.7f4198707d934p-2,   0x1.633b000000000p+0, 0x1.560p+1},
    {  0x1.57ff1f6250600p+1,   0x1.7d06ece2cbd90p-2,   0x1.63ec000000000p+0, 0x1.580p+1},
    {  0x1.59fe6b0856e00p+1,   0x1.7ad3dbf1d07c5p-2,   0x1.649c000000000p+0, 0x1.5a0p+1},
    {  0x1.5bffb05791600p+1,   0x1.78a51e4e89e8dp-2,   0x1.654c000000000p+0, 0x1.5c0p+1},
    {  0x1.5dfd128783940p+1,   0x1.7680f0344cfa3p-2,   0x1.65fa000000000p+0, 0x1.5e0p+1},
    {  0x1.5fff5691fc5c8p+1,   0x1.745dca8168ee1p-2,   0x1.66a9000000000p+0, 0x1.600p+1},
    {  0x1.61fda83b744c0p+1,   0x1.7244fb4988919p-2,   0x1.6756000000000p+0, 0x1.620p+1},
    {  0x1.63fde632fc0d8p+1,   0x1.703031f757122p-2,   0x1.6803000000000p+0, 0x1.640p+1},
    {  0x1.65fd17207f878p+1,   0x1.6e227080885dcp-2,   0x1.68af000000000p+0, 0x1.660p+1},
    {  0x1.67fe308281518p+1,   0x1.6c18962f9c4e0p-2,   0x1.695b000000000p+0, 0x1.680p+1},
    {  0x1.69fe33505c6c0p+1,   0x1.6a1599e058224p-2,   0x1.6a06000000000p+0, 0x1.6a0p+1},
    {  0x1.6bfd17f098000p+1,   0x1.6819617e32041p-2,   0x1.6ab0000000000p+0, 0x1.6c0p+1},
    {  0x1.6dfddc60e1d40p+1,   0x1.6620de7d14ff3p-2,   0x1.6b5a000000000p+0, 0x1.6e0p+1},
    {  0x1.6ffd791b320d8p+1,   0x1.642ef7afe7ed9p-2,   0x1.6c03000000000p+0, 0x1.700p+1},
    {  0x1.71fef1bff2600p+1,   0x1.6240aa2fa0dfdp-2,   0x1.6cac000000000p+0, 0x1.720p+1},
    {  0x1.73ff3924fda00p+1,   0x1.6058d25f0f1d2p-2,   0x1.6d54000000000p+0, 0x1.740p+1},
    {  0x1.75fe47b79cc18p+1,   0x1.5e77582101fccp-2,   0x1.6dfb000000000p+0, 0x1.760p+1},
    {  0x1.77ff298583c40p+1,   0x1.5c99499255997p-2,   0x1.6ea2000000000p+0, 0x1.780p+1},
    {  0x1.79fec8fa79000p+1,   0x1.5ac1740057116p-2,   0x1.6f48000000000p+0, 0x1.7a0p+1},
    {  0x1.7bfd1e8a5a9a8p+1,   0x1.58efc071ad723p-2,   0x1.6fed000000000p+0, 0x1.7c0p+1},
    {  0x1.7dfd3ea25e640p+1,   0x1.57214d5c9f55fp-2,   0x1.7092000000000p+0, 0x1.7e0p+1},
    {  0x1.7fff2a1026738p+1,   0x1.5556138047c4dp-2,   0x1.7137000000000p+0, 0x1.800p+1},
    {  0x1.81ffc01990118p+1,   0x1.5390ccc5d4dcbp-2,   0x1.71db000000000p+0, 0x1.820p+1},
    {  0x1.83fef935eefc0p+1,   0x1.51d1637b58131p-2,   0x1.727e000000000p+0, 0x1.840p+1},
    {  0x1.85fff4ed12b08p+1,   0x1.50150adb1403fp-2,   0x1.7321000000000p+0, 0x1.860p+1},
    {  0x1.87ff8a34e2ad8p+1,   0x1.4e5e6eecb42a3p-2,   0x1.73c3000000000p+0, 0x1.880p+1},
    {  0x1.89fdb18c81200p+1,   0x1.4cad7afff0972p-2,   0x1.7464000000000p+0, 0x1.8a0p+1},
    {  0x1.8bfd92c223be8p+1,   0x1.4aff7149754e8p-2,   0x1.7505000000000p+0, 0x1.8c0p+1},
    {  0x1.8dff2e94d42c0p+1,   0x1.49544b844d5bap-2,   0x1.75a6000000000p+0, 0x1.8e0p+1},
    {  0x1.8fff50f881ec0p+1,   0x1.47aea3dd6c456p-2,   0x1.7646000000000p+0, 0x1.900p+1},
    {  0x1.91fdf271280e8p+1,   0x1.460e66c512f7ap-2,   0x1.76e5000000000p+0, 0x1.920p+1},
    {  0x1.93fe45c4b4200p+1,   0x1.4470e97afe4a4p-2,   0x1.7784000000000p+0, 0x1.940p+1},
    {  0x1.95fd0eb31cc40p+1,   0x1.42d8b9616138fp-2,   0x1.7822000000000p+0, 0x1.960p+1},
    {  0x1.97fd857600000p+1,   0x1.414334e568ed9p-2,   0x1.78c0000000000p+0, 0x1.980p+1},
    {  0x1.99ffaac1ec3c0p+1,   0x1.3fb056724ebb2p-2,   0x1.795e000000000p+0, 0x1.9a0p+1},
    {  0x1.9bfcf50bbb940p+1,   0x1.3e25254fd0abap-2,   0x1.79fa000000000p+0, 0x1.9c0p+1},
    {  0x1.9dff2c38eb838p+1,   0x1.3c99fc3c0ecc7p-2,   0x1.7a97000000000p+0, 0x1.9e0p+1},
    {  0x1.9fffc4098f958p+1,   0x1.3b13dea578788p-2,   0x1.7b33000000000p+0, 0x1.a00p+1},
    {  0x1.a1feb50d73dc0p+1,   0x1.3992bb06457b4p-2,   0x1.7bce000000000p+0, 0x1.a20p+1},
    {  0x1.a3ff47c4aefc8p+1,   0x1.38140a1c79146p-2,   0x1.7c69000000000p+0, 0x1.a40p+1},
    {  0x1.a5fe2a3c978d8p+1,   0x1.369a38ffc6242p-2,   0x1.7d03000000000p+0, 0x1.a60p+1},
    {  0x1.a7feaa57fde28p+1,   0x1.3522c8cc34702p-2,   0x1.7d9d000000000p+0, 0x1.a80p+1},
    {  0x1.a9fd70c4078c0p+1,   0x1.33b01ef76839dp-2,   0x1.7e36000000000p+0, 0x1.aa0p+1},
    {  0x1.abfdd0c1c8b78p+1,   0x1.323fc4cb2b3e1p-2,   0x1.7ecf000000000p+0, 0x1.ac0p+1},
    {  0x1.adffcaf535000p+1,   0x1.30d1b5accf7d2p-2,   0x1.7f68000000000p+0, 0x1.ae0p+1},
    {  0x1.b000000000000p+1,   0x1.2f684bda12f68p-2,   0x1.8000000000000p+0, 0x1.b00p+1},
    {  0x1.b1fe688084838p+1,   0x1.2e037796f3561p-2,   0x1.8097000000000p+0, 0x1.b20p+1},
    {  0x1.b3fe626b241c0p+1,   0x1.2ca0d203aca14p-2,   0x1.812e000000000p+0, 0x1.b40p+1},
    {  0x1.b5ffee5d79de8p+1,   0x1.2b4056dc7efdfp-2,   0x1.81c5000000000p+0, 0x1.b60p+1},
    {  0x1.b7ffa2510ad18p+1,   0x1.29e4520b5d433p-2,   0x1.825b000000000p+0, 0x1.b80p+1},
    {  0x1.b9fd76ec78000p+1,   0x1.288cb4a41a9b5p-2,   0x1.82f0000000000p+0, 0x1.ba0p+1},
    {  0x1.bbfcd4c4a37e8p+1,   0x1.273726eb58088p-2,   0x1.8385000000000p+0, 0x1.bc0p+1},
    {  0x1.bdfdbc70fa540p+1,   0x1.25e3a4edaa4cbp-2,   0x1.841a000000000p+0, 0x1.be0p+1},
    {  0x1.bffcb956c91c0p+1,   0x1.24946cdb00998p-2,   0x1.84ae000000000p+0, 0x1.c00p+1},
    {  0x1.c1fd3bf5cf840p+1,   0x1.234731d751cccp-2,   0x1.8542000000000p+0, 0x1.c20p+1},
    {  0x1.c3ff44e2734c0p+1,   0x1.21fbf01dbcc84p-2,   0x1.85d6000000000p+0, 0x1.c40p+1},
    {  0x1.c5ff5798adfc8p+1,   0x1.20b4dbddb8b22p-2,   0x1.8669000000000p+0, 0x1.c60p+1},
    {  0x1.c7fd6cc9c6418p+1,   0x1.1f71e766caf88p-2,   0x1.86fb000000000p+0, 0x1.c80p+1},
    {  0x1.c9fcff7c918a8p+1,   0x1.1e30d3ecab2d3p-2,   0x1.878d000000000p+0, 0x1.ca0p+1},
    {  0x1.cbfe103f862f8p+1,   0x1.1cf19df136177p-2,   0x1.881f000000000p+0, 0x1.cc0p+1},
    {  0x1.cdfd181598000p+1,   0x1.1bb66cda74540p-2,   0x1.88b0000000000p+0, 0x1.ce0p+1},
    {  0x1.cffd99dca5e08p+1,   0x1.1a7d0bf6948b7p-2,   0x1.8941000000000p+0, 0x1.d00p+1},
    {  0x1.d1ff96203de40p+1,   0x1.194577f0415c8p-2,   0x1.89d2000000000p+0, 0x1.d20p+1},
    {  0x1.d3ff7e0eea440p+1,   0x1.1811cedb334c3p-2,   0x1.8a62000000000p+0, 0x1.d40p+1},
    {  0x1.d5fd4a6639e88p+1,   0x1.16e20423988e5p-2,   0x1.8af1000000000p+0, 0x1.d60p+1},
    {  0x1.d7fc887000000p+1,   0x1.15b3f02aaa7f7p-2,   0x1.8b80000000000p+0, 0x1.d80p+1},
    {  0x1.d9fd38b218978p+1,   0x1.14878fd9777fbp-2,   0x1.8c0f000000000p+0, 0x1.da0p+1},
    {  0x1.dbff5bb25fbc0p+1,   0x1.135ce01fba91bp-2,   0x1.8c9e000000000p+0, 0x1.dc0p+1},
    {  0x1.ddff55aa1e600p+1,   0x1.1235f02ce295ap-2,   0x1.8d2c000000000p+0, 0x1.de0p+1},
    {  0x1.dffd1f5f09f48p+1,   0x1.1112b422eaaf9p-2,   0x1.8db9000000000p+0, 0x1.e00p+1},
    {  0x1.e1fff478800b8p+1,   0x1.0fef07911ab67p-2,   0x1.8e47000000000p+0, 0x1.e20p+1},
    {  0x1.e3fcf12a08858p+1,   0x1.0ed10cc540ecbp-2,   0x1.8ed3000000000p+0, 0x1.e40p+1},
    {  0x1.e5fefa40c0000p+1,   0x1.0db29bc986108p-2,   0x1.8f60000000000p+0, 0x1.e60p+1},
    {  0x1.e7fec5a970600p+1,   0x1.0c97c1fe67aafp-2,   0x1.8fec000000000p+0, 0x1.e80p+1},
    {  0x1.e9fff7e32f000p+1,   0x1.0b7e733018fafp-2,   0x1.9078000000000p+0, 0x1.ea0p+1},
    {  0x1.ebfee329a58d8p+1,   0x1.0a68aae26ff8bp-2,   0x1.9103000000000p+0, 0x1.ec0p+1},
    {  0x1.edff311c5e5c0p+1,   0x1.095462af12046p-2,   0x1.918e000000000p+0, 0x1.ee0p+1},
    {  0x1.effd2edcdb000p+1,   0x1.084390bb22179p-2,   0x1.9218000000000p+0, 0x1.f00p+1},
    {  0x1.f1fc8b255bc40p+1,   0x1.0734344eaebefp-2,   0x1.92a2000000000p+0, 0x1.f20p+1},
    {  0x1.f3fd466e2e600p+1,   0x1.06264aeb4dfe1p-2,   0x1.932c000000000p+0, 0x1.f40p+1},
    {  0x1.f5ff612fa08c0p+1,   0x1.0519d217b1777p-2,   0x1.93b6000000000p+0, 0x1.f60p+1},
    {  0x1.f7ff1e5d465f8p+1,   0x1.0410b571c6692p-2,   0x1.943f000000000p+0, 0x1.f80p+1},
    {  0x1.f9fc76d31dcb8p+1,   0x1.030aeab7a83cep-2,   0x1.94c7000000000p+0, 0x1.fa0p+1},
    {  0x1.fbfee88f68000p+1,   0x1.020495fe39c9dp-2,   0x1.9550000000000p+0, 0x1.fc0p+1},
    {  0x1.fdfef16dc3000p+1,   0x1.0101895a920fbp-2,   0x1.95d8000000000p+0, 0x1.fe0p+1},
    {  0x1.fffc8a4e2b8f8p+1,   0x1.0001badbe84f3p-2,   0x1.965f000000000p+0, 0x1.000p+2},
    {  0x1.01ffc11b68ce0p+2,   0x1.fc086bf7be35fp-3,   0x1.976e000000000p+0, 0x1.020p+2},
    {  0x1.03fe2f65f54a0p+2,   0x1.f82306d38867bp-3,   0x1.987a000000000p+0, 0x1.040p+2},
    {  0x1.05ff3d3150b60p+2,   0x1.f447cddf17c9bp-3,   0x1.9986000000000p+0, 0x1.060p+2},
    {  0x1.07ff1086e4000p+2,   0x1.f07de1653d505p-3,   0x1.9a90000000000p+0, 0x1.080p+2},
    {  0x1.09ff8c6d39e24p+2,   0x1.ecc1514885e00p-3,   0x1.9b99000000000p+0, 0x1.0a0p+2},
    {  0x1.0bfeb843a0000p+2,   0x1.e91570d824941p-3,   0x1.9ca0000000000p+0, 0x1.0c0p+2},
    {  0x1.0dfe7c0fa2160p+2,   0x1.e5766614ac00bp-3,   0x1.9da6000000000p+0, 0x1.0e0p+2},
    {  0x1.0ffed1fe3584cp+2,   0x1.e1e3f8ef59221p-3,   0x1.9eab000000000p+0, 0x1.100p+2},
    {  0x1.11ffb43a3103cp+2,   0x1.de5df2897c620p-3,   0x1.9faf000000000p+0, 0x1.120p+2},
    {  0x1.13ff203b5f444p+2,   0x1.dae788736ea8dp-3,   0x1.a0b1000000000p+0, 0x1.140p+2},
    {  0x1.15ff07eeb6920p+2,   0x1.d77d0a049390ep-3,   0x1.a1b2000000000p+0, 0x1.160p+2},
    {  0x1.17ff657c09920p+2,   0x1.d41e4427c8534p-3,   0x1.a2b2000000000p+0, 0x1.180p+2},
    {  0x1.19fe2f010c000p+2,   0x1.d0ce5764ac9ddp-3,   0x1.a3b0000000000p+0, 0x1.1a0p+2},
    {  0x1.1bff6441c48e0p+2,   0x1.cd8665a8dc5dfp-3,   0x1.a4ae000000000p+0, 0x1.1c0p+2},
    {  0x1.1dfef4e374da0p+2,   0x1.ca4cdc5e38b4fp-3,   0x1.a5aa000000000p+0, 0x1.1e0p+2},
    {  0x1.1ffedf132b574p+2,   0x1.c71e3a5b98854p-3,   0x1.a6a5000000000p+0, 0x1.200p+2},
    {  0x1.21ff1cf442b7cp+2,   0x1.c3fa51f8aa0afp-3,   0x1.a79f000000000p+0, 0x1.220p+2},
    {  0x1.23ffa8a8e5800p+2,   0x1.c0e0f67bc6bcap-3,   0x1.a898000000000p+0, 0x1.240p+2},
    {  0x1.25fe69bfce6bcp+2,   0x1.bdd520a702f83p-3,   0x1.a98f000000000p+0, 0x1.260p+2},
    {  0x1.27ff7d151bb60p+2,   0x1.bad055262311fp-3,   0x1.aa86000000000p+0, 0x1.280p+2},
    {  0x1.29feb53f5f80cp+2,   0x1.b7d8ac0df92bep-3,   0x1.ab7b000000000p+0, 0x1.2a0p+2},
    {  0x1.2bfe1ef29893cp+2,   0x1.b4ead7e836b48p-3,   0x1.ac6f000000000p+0, 0x1.2c0p+2},
    {  0x1.2dffd076334ecp+2,   0x1.b203a858682cbp-3,   0x1.ad63000000000p+0, 0x1.2e0p+2},
    {  0x1.2fff8dff263b4p+2,   0x1.af290d7ab4dc8p-3,   0x1.ae55000000000p+0, 0x1.300p+2},
    {  0x1.31ff6b7103f60p+2,   0x1.ac57d1a0a3a79p-3,   0x1.af46000000000p+0, 0x1.320p+2},
    {  0x1.33ff62eb29c60p+2,   0x1.a98fcf107c213p-3,   0x1.b036000000000p+0, 0x1.340p+2},
    {  0x1.35ff6e8c98374p+2,   0x1.a6d0e0cf35543p-3,   0x1.b125000000000p+0, 0x1.360p+2},
    {  0x1.37ff88740832cp+2,   0x1.a41ae29c0d9c3p-3,   0x1.b213000000000p+0, 0x1.380p+2},
    {  0x1.39ffaac000000p+2,   0x1.a16db0ec408b2p-3,   0x1.b300000000000p+0, 0x1.3a0p+2},
    {  0x1.3bffcf8ee8300p+2,   0x1.9ec928e6d9f5dp-3,   0x1.b3ec000000000p+0, 0x1.3c0p+2},
    {  0x1.3dfff0ff2071cp+2,   0x1.9c2d2860a546ep-3,   0x1.b4d7000000000p+0, 0x1.3e0p+2},
    {  0x1.3ffdd7c700000p+2,   0x1.999c5c76af5a0p-3,   0x1.b5c0000000000p+0, 0x1.400p+2},
    {  0x1.41fdde7efa2e4p+2,   0x1.9711011e7f5e1p-3,   0x1.b6a9000000000p+0, 0x1.420p+2},
    {  0x1.43fdd035526c4p+2,   0x1.948dcac574e71p-3,   0x1.b791000000000p+0, 0x1.440p+2},
    {  0x1.45ffdf6fec0a4p+2,   0x1.920fdcc619066p-3,   0x1.b879000000000p+0, 0x1.460p+2},
    {  0x1.47ff97d2c007cp+2,   0x1.8f9c97e5a8a7ep-3,   0x1.b95f000000000p+0, 0x1.480p+2},
    {  0x1.49ff2990eb100p+2,   0x1.8d311aeb5c884p-3,   0x1.ba44000000000p+0, 0x1.4a0p+2},
    {  0x1.4bfe8ecace800p+2,   0x1.8acd4803076e4p-3,   0x1.bb28000000000p+0, 0x1.4c0p+2},
    {  0x1.4dfdc1a1664ccp+2,   0x1.887101e950c8fp-3,   0x1.bc0b000000000p+0, 0x1.4e0p+2},
    {  0x1.4fff002ce14e0p+2,   0x1.86198a8a05330p-3,   0x1.bcee000000000p+0, 0x1.500p+2},
    {  0x1.51fdbeedcb9bcp+2,   0x1.83cc0dc244b6ap-3,   0x1.bdcf000000000p+0, 0x1.520p+2},
    {  0x1.53fe823f4c000p+2,   0x1.8183325c32c1bp-3,   0x1.beb0000000000p+0, 0x1.540p+2},
    {  0x1.55ff004aa4000p+2,   0x1.7f417e5e20dedp-3,   0x1.bf90000000000p+0, 0x1.560p+2},
    {  0x1.57ff33308793cp+2,   0x1.7d06d6f2c9b6cp-3,   0x1.c06f000000000p+0, 0x1.580p+2},
    {  0x1.59ff1512ac954p+2,   0x1.7ad321c4f107fp-3,   0x1.c14d000000000p+0, 0x1.5a0p+2},
    {  0x1.5bfea013de5a0p+2,   0x1.78a644fc9b7e4p-3,   0x1.c22a000000000p+0, 0x1.5c0p+2},
    {  0x1.5dfdce5811360p+2,   0x1.7680273c586edp-3,   0x1.c306000000000p+0, 0x1.5e0p+2},
    {  0x1.5ffef03fb1a20p+2,   0x1.745e36bf9a9f8p-3,   0x1.c3e2000000000p+0, 0x1.600p+2},
    {  0x1.61ffae3c2c094p+2,   0x1.7242dd79c5d7bp-3,   0x1.c4bd000000000p+0, 0x1.620p+2},
    {  0x1.63fda7ae36e60p+2,   0x1.703072a07c8fbp-3,   0x1.c596000000000p+0, 0x1.640p+2},
    {  0x1.65ffe701dc000p+2,   0x1.6e1f9043853d7p-3,   0x1.c670000000000p+0, 0x1.660p+2},
    {  0x1.67ff5615bc800p+2,   0x1.6c176d44b9477p-3,   0x1.c748000000000p+0, 0x1.680p+2},
    {  0x1.69fe49d3f317cp+2,   0x1.6a15835b4e485p-3,   0x1.c81f000000000p+0, 0x1.6a0p+2},
    {  0x1.6bff202662060p+2,   0x1.68175eda585dfp-3,   0x1.c8f6000000000p+0, 0x1.6c0p+2},
    {  0x1.6dff73efaeb00p+2,   0x1.661f4fb1a0608p-3,   0x1.c9cc000000000p+0, 0x1.6e0p+2},
    {  0x1.6fff3f57a2f84p+2,   0x1.642d400879d8cp-3,   0x1.caa1000000000p+0, 0x1.700p+2},
    {  0x1.71fe7c87d2534p+2,   0x1.62411a6b0ce8fp-3,   0x1.cb75000000000p+0, 0x1.720p+2},
    {  0x1.73ff925b24e64p+2,   0x1.60587ddf5d7bdp-3,   0x1.cc49000000000p+0, 0x1.740p+2},
    {  0x1.75fda3d6e6f8cp+2,   0x1.5e77f1b2d4024p-3,   0x1.cd1b000000000p+0, 0x1.760p+2},
    {  0x1.77fff7e0844e0p+2,   0x1.5c988a411d6eap-3,   0x1.cdee000000000p+0, 0x1.780p+2},
    {  0x1.79ff3be92f0fcp+2,   0x1.5ac10a9168160p-3,   0x1.cebf000000000p+0, 0x1.7a0p+2},
    {  0x1.7bfdd9085aebcp+2,   0x1.58ef1727fb30fp-3,   0x1.cf8f000000000p+0, 0x1.7c0p+2},
    {  0x1.7dfe412fa147cp+2,   0x1.5720651d4d5c8p-3,   0x1.d05f000000000p+0, 0x1.7e0p+2},
    {  0x1.7ffdfb35820e0p+2,   0x1.555720b668692p-3,   0x1.d12e000000000p+0, 0x1.800p+2},
    {  0x1.81ff7d7349794p+2,   0x1.53910767a7e9dp-3,   0x1.d1fd000000000p+0, 0x1.820p+2},
    {  0x1.83fdcbf8773a0p+2,   0x1.51d269c44b634p-3,   0x1.d2ca000000000p+0, 0x1.840p+2},
    {  0x1.85fddb7be481cp+2,   0x1.5016da018b848p-3,   0x1.d397000000000p+0, 0x1.860p+2},
    {  0x1.87ffacc2c0900p+2,   0x1.4e5e517350cf3p-3,   0x1.d464000000000p+0, 0x1.880p+2},
    {  0x1.89fe36b4dd23cp+2,   0x1.4cad0a90993f5p-3,   0x1.d52f000000000p+0, 0x1.8a0p+2},
    {  0x1.8bfe7b2f91ca0p+2,   0x1.4afeaf0242604p-3,   0x1.d5fa000000000p+0, 0x1.8c0p+2},
    {  0x1.8dfdf1a9c1100p+2,   0x1.495551c26a875p-3,   0x1.d6c4000000000p+0, 0x1.8e0p+2},
    {  0x1.8fff1fcd612e0p+2,   0x1.47aecc24f4847p-3,   0x1.d78e000000000p+0, 0x1.900p+2},
    {  0x1.91ff78b50511cp+2,   0x1.460d2a3ade09ep-3,   0x1.d857000000000p+0, 0x1.920p+2},
    {  0x1.93fef6970fd7cp+2,   0x1.44705b7a7ca45p-3,   0x1.d91f000000000p+0, 0x1.940p+2},
    {  0x1.95fd93acacd60p+2,   0x1.42d84fa34911ep-3,   0x1.d9e6000000000p+0, 0x1.960p+2},
    {  0x1.97fdde4c4fdd4p+2,   0x1.4142eef190046p-3,   0x1.daad000000000p+0, 0x1.980p+2},
    {  0x1.99ffd72a58500p+2,   0x1.3fb033d201f0dp-3,   0x1.db74000000000p+0, 0x1.9a0p+2},
    {  0x1.9bfe4e1f89da4p+2,   0x1.3e241ad6ffe2ap-3,   0x1.dc39000000000p+0, 0x1.9c0p+2},
    {  0x1.9dfe6c1816fe0p+2,   0x1.3c9a8f2a1f8a5p-3,   0x1.dcfe000000000p+0, 0x1.9e0p+2},
    {  0x1.9ffd950c31420p+2,   0x1.3b158608a35b4p-3,   0x1.ddc2000000000p+0, 0x1.a00p+2},
    {  0x1.a1fe621497b60p+2,   0x1.3992f944e0082p-3,   0x1.de86000000000p+0, 0x1.a20p+2},
    {  0x1.a3fe32de87264p+2,   0x1.3814d7dcdaa81p-3,   0x1.df49000000000p+0, 0x1.a40p+2},
    {  0x1.a5ffa4ca81b00p+2,   0x1.3699225f72272p-3,   0x1.e00c000000000p+0, 0x1.a60p+2},
    {  0x1.a7fd6dfa85354p+2,   0x1.3523af768b748p-3,   0x1.e0cd000000000p+0, 0x1.a80p+2},
    {  0x1.a9ff7876b86bcp+2,   0x1.33aea799e6168p-3,   0x1.e18f000000000p+0, 0x1.aa0p+2},
    {  0x1.abfdcebbdafbcp+2,   0x1.323fc63dc5c59p-3,   0x1.e24f000000000p+0, 0x1.ac0p+2},
    {  0x1.adfdbbf3a58bcp+2,   0x1.30d32b448c48ap-3,   0x1.e30f000000000p+0, 0x1.ae0p+2},
    {  0x1.afff40c0181bcp+2,   0x1.2f68d22c69be3p-3,   0x1.e3cf000000000p+0, 0x1.b00p+2},
    {  0x1.b1ffade2d82e0p+2,   0x1.2e029528d4f7dp-3,   0x1.e48e000000000p+0, 0x1.b20p+2},
    {  0x1.b3fefda1c8b00p+2,   0x1.2ca066fdd32b7p-3,   0x1.e54c000000000p+0, 0x1.b40p+2},
    {  0x1.b5ffde5c62fa0p+2,   0x1.2b4061cbb8c88p-3,   0x1.e60a000000000p+0, 0x1.b60p+2},
    {  0x1.b7ff9a7c1465cp+2,   0x1.29e45758c86a6p-3,   0x1.e6c7000000000p+0, 0x1.b80p+2},
    {  0x1.b9fe2c4b6aa6cp+2,   0x1.288c3af3d5cdcp-3,   0x1.e783000000000p+0, 0x1.ba0p+2},
    {  0x1.bbfe487b622fcp+2,   0x1.27362fc35f9d7p-3,   0x1.e83f000000000p+0, 0x1.bc0p+2},
    {  0x1.bdffefa410a0cp+2,   0x1.25e231cf996f8p-3,   0x1.e8fb000000000p+0, 0x1.be0p+2},
    {  0x1.bffda53057834p+2,   0x1.2493d2d2d3588p-3,   0x1.e9b5000000000p+0, 0x1.c00p+2},
    {  0x1.c1ff9f2cdc000p+2,   0x1.2345a635acc4dp-3,   0x1.ea70000000000p+0, 0x1.c20p+2},
    {  0x1.c3fd9c2b178e4p+2,   0x1.21fd009a10925p-3,   0x1.eb29000000000p+0, 0x1.c40p+2},
    {  0x1.c5ffdec0212ecp+2,   0x1.20b485eb4e5cfp-3,   0x1.ebe3000000000p+0, 0x1.c60p+2},
    {  0x1.c7fe18f7fd98cp+2,   0x1.1f717add1edcdp-3,   0x1.ec9b000000000p+0, 0x1.c80p+2},
    {  0x1.c9fdd0e87622cp+2,   0x1.1e30510f8c8ecp-3,   0x1.ed53000000000p+0, 0x1.ca0p+2},
    {  0x1.cbff07201fcccp+2,   0x1.1cf10503b6bcap-3,   0x1.ee0b000000000p+0, 0x1.cc0p+2},
    {  0x1.cdfeef0724420p+2,   0x1.1bb54ba55bb8ep-3,   0x1.eec2000000000p+0, 0x1.ce0p+2},
    {  0x1.cffd82f367800p+2,   0x1.1a7d19e97b606p-3,   0x1.ef78000000000p+0, 0x1.d00p+2},
    {  0x1.d1fd8e832f0e0p+2,   0x1.1946b193a1280p-3,   0x1.f02e000000000p+0, 0x1.d20p+2},
    {  0x1.d3ff124076900p+2,   0x1.18120f5f532e7p-3,   0x1.f0e4000000000p+0, 0x1.d40p+2},
    {  0x1.d5ff394d32624p+2,   0x1.16e0de7a64731p-3,   0x1.f199000000000p+0, 0x1.d60p+2},
    {  0x1.d7fdfe030f554p+2,   0x1.15b3145e80a1fp-3,   0x1.f24d000000000p+0, 0x1.d80p+2},
    {  0x1.d9fe344036404p+2,   0x1.1486fd17547b9p-3,   0x1.f301000000000p+0, 0x1.da0p+2},
    {  0x1.dbffdc8a23034p+2,   0x1.135c9596d2896p-3,   0x1.f3b5000000000p+0, 0x1.dc0p+2},
    {  0x1.ddfd3c2cb21dcp+2,   0x1.1237248528065p-3,   0x1.f467000000000p+0, 0x1.de0p+2},
    {  0x1.dffee657c82a0p+2,   0x1.1111b14cb8f39p-3,   0x1.f51a000000000p+0, 0x1.e00p+2},
    {  0x1.e1ff1e4c3eb00p+2,   0x1.0fef806635fe8p-3,   0x1.f5cc000000000p+0, 0x1.e20p+2},
    {  0x1.e3fdde6a2fd94p+2,   0x1.0ed0880474f71p-3,   0x1.f67d000000000p+0, 0x1.e40p+2},
    {  0x1.e5fe06d9140e0p+2,   0x1.0db322dce8431p-3,   0x1.f72e000000000p+0, 0x1.e60p+2},
    {  0x1.e7ff9817d6e7cp+2,   0x1.0c974e2c4e003p-3,   0x1.f7df000000000p+0, 0x1.e80p+2},
    {  0x1.e9ffa8cd61abcp+2,   0x1.0b7e9e5c6ec09p-3,   0x1.f88f000000000p+0, 0x1.ea0p+2},
    {  0x1.ebfe335e93be0p+2,   0x1.0a690a131c86ap-3,   0x1.f93e000000000p+0, 0x1.ec0p+2},
    {  0x1.edfe201620cd4p+2,   0x1.0954f55423991p-3,   0x1.f9ed000000000p+0, 0x1.ee0p+2},
    {  0x1.efff6f6eb3700p+2,   0x1.08425d89f8618p-3,   0x1.fa9c000000000p+0, 0x1.f00p+2},
    {  0x1.f1ff2ff2d4ba0p+2,   0x1.0732ce999c3d1p-3,   0x1.fb4a000000000p+0, 0x1.f20p+2},
    {  0x1.f3fd5c0c8c49cp+2,   0x1.06263f9594b61p-3,   0x1.fbf7000000000p+0, 0x1.f40p+2},
    {  0x1.f5ffda1397d74p+2,   0x1.05199336faf21p-3,   0x1.fca5000000000p+0, 0x1.f60p+2},
    {  0x1.f7fdc89c9cbc4p+2,   0x1.041165caebb1fp-3,   0x1.fd51000000000p+0, 0x1.f80p+2},
    {  0x1.f9fd100472794p+2,   0x1.030a9c4a34299p-3,   0x1.fdfd000000000p+0, 0x1.fa0p+2},
    {  0x1.fbfdb0bf902e4p+2,   0x1.0205345e3761ep-3,   0x1.fea9000000000p+0, 0x1.fc0p+2},
    {  0x1.fdffab426cfb4p+2,   0x1.01012bb50ef58p-3,   0x1.ff55000000000p+0, 0x1.fe0p+2},
};


// Describe the destination floating-point type.
#define	SignificandBits	53
#define	ExponentBits	11

#if AVOID_UINT64
double cbrt(double x) {
        // deal with exponent
        
	// Define a structure for accessing the internal components of a float.
	typedef union
	{
		double d;
		struct
		{
			#if defined __BIG_ENDIAN__
				unsigned int sign     : 1;
				unsigned int exponent : ExponentBits;
				uint64_t fraction : SignificandBits-1;
			#else	// defined __BIG_ENDIAN__
				uint64_t fraction : SignificandBits-1;
				unsigned int exponent : ExponentBits;
				unsigned int sign     : 1;
			#endif	// defined __BIG_ENDIAN__
		} s;
	} Double;
        
        Double X;
        X.d = x;
        int sign = X.s.sign;
        int exponent = X.s.exponent;
        uint64_t fraction = X.s.fraction;

        if( exponent == 0x7ff) {
        #if FENV_ACCESS_ON
                return x+x; // inf, -inf, NaN, raise invalid for signalling NaNs
        #else
                return x; // inf, -inf, NaN
        #endif
        }

	if (exponent == 0x000) { // denormal/zero path
                #if DAZ
                return 0.0 * x; //cbrt(+/-denorm) = +/-0 if DAZ
                #else
                if(fraction == 0)
                        return x; //cbrt(+/-0) = +/-0
                Double fixed;
                fixed.s.sign = 0;
                fixed.s.exponent = 0x400;
                fixed.s.fraction = fraction;
                double newx = fixed.d - 2.0;
                Double newscale;
                newscale.s.sign = sign;
                newscale.s.exponent = 0x2aa;
                newscale.s.fraction = 0;
                return newscale.d * cbrt(newx);
                #endif
        }

	Double Y;
        Y.d = 0.0;
        int e = exponent; // - 0x3ff;

        int expdiv3 = (e * 0x000aaaab)>>21; // e/3
        int expmod3 = 0x003 & ((e * 0x002aaaab)>>21); // e%3

        Y.s.exponent = expmod3 + 0x3ff;
        Y.s.fraction = fraction;
        Double scale;
        scale.d = 0.0;
        scale.s.exponent = expdiv3 + 0x2aa; // need to add 2/3 of the bias = bias - bias/3
        scale.s.sign = sign;

        double y = Y.d; // reduced argument we try to take the cube root of

        int ylookup = (int)((expmod3 << 7) | (fraction >> (52-7)));

        const double z = cbrt_table[ylookup][0];
        const double zinv = cbrt_table[ylookup][1]; // 1.0/z;
        const double cbrtz = cbrt_table[ylookup][2]; //cbrt(z); exactly

        const double w = (y - z) * zinv;
/*
 Remes of order 5 on [-0x1p-10, 0x1.33a3c38cf408p-7] converged to:

 P(x) = 9.99999999999999988582933092574e-1 + 3.33333333333348975533452008402e-1 x^1 - 1.11111111092259896957294965013e-1 x^2 + 6.17283764468193710192949460754e-2 x^3 - 4.11470773517990145835234033841e-2 x^4 + 2.95939821802870215940915362711e-2 x^5
 rounded to double P(x) = 0x1p+0 + 0x1.555555555566fp-2 x^1 - 0x1.c71c71c5d0cf7p-4 x^2 + 0x1.f9adc9c26062dp-5 x^3 - 0x1.5113acf1a9a61p-5 x^4 + 0x1.e4de2867daf6fp-6 x^5
 with eps = 0x1.ffffffffffeaap-57
 We pull off the leading 1.0
 */
        const double pcoef[5] = {
                0x1.555555555566fp-2,
                - 0x1.c71c71c5d0cf7p-4,
                0x1.f9adc9c26062dp-5,
                - 0x1.5113acf1a9a61p-5,
                0x1.e4de2867daf6fp-6,
        };
        const double pw = pcoef[0] + w*( pcoef[1] + w*(pcoef[2] + w * (pcoef[3] + w * pcoef[4])));
        double res = scale.d * (cbrtz + (cbrtz * w) * pw);
        return res;
}
#else // AVOID_UINT64
double cbrt(double x) {
        // deal with exponent
        
	// Define a structure for accessing the internal components of a float.
	typedef union
	{
		double d;
                uint64_t u;
	} Double;
        
        Double X;
        X.d = x;
//        int sign = X.u >> 63;
        uint64_t signx = X.u & 0x8000000000000000LL;
        int exponent = (int)(X.u >> 52) & 0x7ff;
        uint64_t fraction = X.u & 0x000fffffffffffffLL;

        if( exponent == 0x7ff) {
        #if FENV_ACCESS_ON
                return x+x; // inf, -inf, NaN, raise invalid for signalling NaNs
        #else
                return x; // inf, -inf, NaN
        #endif
        }

	if (exponent == 0x000) { // denormal/zero path
                #if DAZ
                return 0.0 * x; //cbrt(+/-denorm) = +/-0 if DAZ
                #else
                if(fraction == 0)
                        return x; //cbrt(+/-0) = +/-0
                Double fixed;
//                fixed.s.sign = 0;
//                fixed.s.exponent = 0x400;
//                fixed.s.fraction = fraction;
                fixed.u = 0x4000000000000000LL | fraction;
                double newx = fixed.d - 2.0;
                Double newscale;
//                newscale.s.sign = sign;
//                newscale.s.exponent = 0x2aa;
//                newscale.s.fraction = 0;
                newscale.u = signx | 0x2aa0000000000000LL;
                return newscale.d * cbrt(newx);
                #endif
        }

        int e = exponent; // - 0x3ff;

        int expdiv3 = (e * 0x000aaaab)>>21; // e/3
        int expmod3 = 0x003 & ((e * 0x002aaaab)>>21); // e%3

	Double Y;
//        Y.d = 0.0;
//        Y.s.exponent = expmod3 + 0x3ff;
//        Y.s.fraction = fraction;
        Y.u = ((uint64_t)(expmod3 + 0x3ff)<<52) | fraction;
        Double scale;
//        scale.d = 0.0;
//        scale.s.exponent = expdiv3 + 0x2aa; // need to add 2/3 of the bias = bias - bias/3
//        scale.s.sign = sign;
        scale.u = signx | ((uint64_t)(expdiv3 + 0x2aa) << 52);
        double y = Y.d; // reduced argument we try to take the cube root of

        int ylookup = (int)((expmod3 << 7) | (fraction >> (52-7)));

        const double z = cbrt_table[ylookup][0];
        const double zinv = cbrt_table[ylookup][1]; // 1.0/z;
        const double cbrtz = cbrt_table[ylookup][2]; //cbrt(z); exactly

        const double w = (y - z) * zinv;
/*
 Remes of order 5 on [-0x1p-10, 0x1.33a3c38cf408p-7] converged to:

 P(x) = 9.99999999999999988582933092574e-1 + 3.33333333333348975533452008402e-1 x^1 - 1.11111111092259896957294965013e-1 x^2 + 6.17283764468193710192949460754e-2 x^3 - 4.11470773517990145835234033841e-2 x^4 + 2.95939821802870215940915362711e-2 x^5
 rounded to double P(x) = 0x1p+0 + 0x1.555555555566fp-2 x^1 - 0x1.c71c71c5d0cf7p-4 x^2 + 0x1.f9adc9c26062dp-5 x^3 - 0x1.5113acf1a9a61p-5 x^4 + 0x1.e4de2867daf6fp-6 x^5
 with eps = 0x1.ffffffffffeaap-57
 We pull off the leading 1.0
 */
        const double pcoef[5] = {
                0x1.555555555566fp-2,
                - 0x1.c71c71c5d0cf7p-4,
                0x1.f9adc9c26062dp-5,
                - 0x1.5113acf1a9a61p-5,
                0x1.e4de2867daf6fp-6,
        };
        const double pw = pcoef[0] + w*( pcoef[1] + w*(pcoef[2] + w * (pcoef[3] + w * pcoef[4])));
        double res = scale.d * (cbrtz + (cbrtz * w) * pw);
        return res;
}
#endif // AVOID_UINT64

/* 
// Code to demonstrate how to compute (e/3,e%3) either with the exponent where you want it to end up
// or with an integer result. 
#include <stdio.h>
#include <stdint.h>
int main(void) {
        uint32_t e;
        int bad = 0;
        for(e = 0; e <= 0x7ff; e++) {
                uint32_t f = e << 20;
                uint32_t ediv3 = 0x7ff00000 & (e * 0x00055556);
                uint32_t emod3 = 0x00300000 & (e * 0x00155556);
                uint32_t exp = 3 * ediv3 + emod3;
                if (exp == f) printf("ok\t%03x\t%08x\t%08x\t%08x\n", e, ediv3, emod3, f);
                else bad++, printf("not ok\t%03x\t%08x\t%08x\t%08x\n", e, ediv3, emod3, f);
        }                
        for(e = 0; e <= 0x7ff; e++) {
                uint32_t f = e << 20;
                uint32_t ediv3 = (e * 0x000aaaab)>>21;
                uint32_t emod3 = 0x003 & ((e * 0x002aaaab)>>21);
                uint32_t exp = 3 * ediv3 + emod3;
                if (exp == e) printf("ok\t%03x\t%08x\t%08x\t%08x\n", e, ediv3, emod3, f);
                else bad++, printf("not ok\t%03x\t%08x\t%08x\t%08x\n", e, ediv3, emod3, f);
        }                
        if(bad) printf ("%d failures.\n",bad);
        else printf("done. passed.\n");
        return 0;
}
*/
