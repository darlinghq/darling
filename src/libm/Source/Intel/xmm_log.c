/*
 *  xmm_exp.c
 *  xmmLibm
 *
 *  Created by Ian Ollmann, Ph.D. on 7/14/05.
 *  Copyright © 2005 Apple Computer, Inc. All rights reserved.
 *
 *  Constants from original typing by Earl Killian of MIPS on March 23rd, 1992.         
 *  Converted from pairs of 32-bit hexadecimal words to C99 notation, by Ian Ollmann
 *
 *  Algorithm from Peter Tang:
 *
 *      ACM Transactions on Mathematical Software, Vol 16, 4, December 1990, pp 378-400
 *      
 *  
 */

#include "xmmLibm_prefix.h"

#include "math.h"
#include "fenv.h"

//Functions implemented here
static inline xDouble _xlog( xDouble x )   ALWAYS_INLINE;

static const double A1[2] = {
                                        0x1.5555555550286p-4, //0.0833333333330391334837
                                        0x1.999A0BC712416p-7  //0.0125000531680985842164
                                 };

static const double A2[4] = {
                                        0x1.55555555554E6p-4, //0.083333333333331788273
                                        0x1.9999999BAC6D4p-7, //0.0125000000037717509671
                                        0x1.2492307F1519Fp-9, //0.00223213998791944824227
                                        0x1.C8034C85DFFF0p-12  //0.000434887777707614574252
                                };

static const double C_lead[129] = {
                                         0x0,                  //                0
                                         0x1.FE02A6B100000p-8, //0.00778214044203195953742
                                         0x1.FC0A8B0FC0000p-7, //0.0155041865359635266941
                                         0x1.7B91B07D60000p-6, //0.0231670592816044518258
                                         0x1.F829B0E780000p-6, //0.0307716586667083902285
                                         0x1.39E87B9FE8000p-5, //0.0383188643020275776507
                                         0x1.77458F6330000p-5, //0.0458095360313564015087
                                         0x1.B42DD71198000p-5, //0.0532445145188376045553
                                         0x1.F0A30C0118000p-5, //0.060624621816486978787
                                         0x1.16536EEA38000p-4, //0.0679506619085259444546
                                         0x1.341D7961BC000p-4, //0.075223421237524235039
                                         0x1.51B073F060000p-4, //0.0824436692109884461388
                                         0x1.6F0D28AE58000p-4, //0.0896121586897606903221
                                         0x1.8C345D6318000p-4, //0.096729626458454731619
                                         0x1.A926D3A4AC000p-4, //0.10379679368156757846
                                         0x1.C5E548F5BC000p-4, //0.110814366340264314204
                                         0x1.E27076E2B0000p-4, //0.117783035656430001836
                                         0x1.FEC9131DC0000p-4, //0.12470347850103280507
                                         0x1.0D77E7CD08000p-3, //0.131576357788617315236
                                         0x1.1B72AD52F6000p-3, //0.138402322859064952354
                                         0x1.29552F8200000p-3, //0.145182009844575077295
                                         0x1.371FC201E8000p-3, //0.151916042025732167531
                                         0x1.44D2B6CCB8000p-3, //0.158605030176659056451
                                         0x1.526E5E3A1C000p-3, //0.165249572895390883787
                                         0x1.5FF3070A7A000p-3, //0.171850256926745714736
                                         0x1.6D60FE719E000p-3, //0.178407657472916980623
                                         0x1.7AB890210E000p-3, //0.184922338494061477832
                                         0x1.87FA06520C000p-3, //0.191394852999565046048
                                         0x1.9525A9CF46000p-3, //0.197825743329985925811
                                         0x1.A23BC1FE2C000p-3, //0.204215541428766300669
                                         0x1.AF3C94E80C000p-3, //0.210564769107350002741
                                         0x1.BC286742D8000p-3, //0.216873938300523150247
                                         0x1.C8FF7C79AA000p-3, //0.223143551314251453732
                                         0x1.D5C216B4FC000p-3, //0.229374101064877322642
                                         0x1.E27076E2B0000p-3, //0.235566071312860003673
                                         0x1.EF0ADCBDC6000p-3, //0.241719936887193398434
                                         0x1.FB9186D5E4000p-3, //0.247836163904594286578
                                         0x1.0402594B4D000p-2, //0.253915209980959843961
                                         0x1.0A324E2739000p-2, //0.259957524436913445243
                                         0x1.1058BF9AE5000p-2, //0.265963548497211377253
                                         0x1.1675CABABA000p-2, //0.271933715483555715764
                                         0x1.1C898C169A000p-2, //0.277868451003541849786
                                         0x1.22941FBCF8000p-2, //0.283768173130738432519
                                         0x1.2895A13DE8000p-2, //0.289633292582948342897
                                         0x1.2E8E2BAE12000p-2, //0.295464212893875810551
                                         0x1.347DD9A988000p-2, //0.301261330578199704178
                                         0x1.3A64C55694000p-2, //0.307025035294827830512
                                         0x1.404308686A000p-2, //0.312755710003784770379
                                         0x1.4618BC21C6000p-2, //0.318453731118552241242
                                         0x1.4BE5F95778000p-2, //0.324119468654316733591
                                         0x1.51AAD872E0000p-2, //0.329753286372579168528
                                         0x1.5767717456000p-2, //0.335355541921217081835
                                         0x1.5D1BDBF581000p-2, //0.340926586970681455568
                                         0x1.62C82F2B9C000p-2, //0.346466767346100823488
                                         0x1.686C81E9B1000p-2, //0.351976423157111639739
                                         0x1.6E08EAA2BA000p-2, //0.357455888921776931966
                                         0x1.739D7F6BBD000p-2, //0.362905493689368086052
                                         0x1.792A55FDD4000p-2, //0.368325561158599157352
                                         0x1.7EAF83B82B000p-2, //0.373716409793587445165
                                         0x1.842D1DA1E9000p-2, //0.379078352935039220029
                                         0x1.89A3386C14000p-2, //0.384411698910298582632
                                         0x1.8F11E87366000p-2, //0.3897167511399857176
                                         0x1.947941C211000p-2, //0.394993808240769794793
                                         0x1.99D958117E000p-2, //0.400243164127005002229
                                         0x1.9F323ECBFA000p-2, //0.405465108108273852849
                                         0x1.A484090E5C000p-2, //0.410659924985338875558
                                         0x1.A9CEC9A9A1000p-2, //0.415827895143820569501
                                         0x1.AF12932478000p-2, //0.420969294644237379543
                                         0x1.B44F77BCC9000p-2, //0.426084395310908803367
                                         0x1.B985896931000p-2, //0.43117346481835738814
                                         0x1.BEB4D9DA72000p-2, //0.436236766774982243078
                                         0x1.C3DD7A7CDB000p-2, //0.441274560804913562606
                                         0x1.C8FF7C79AA000p-2, //0.446287102628502907464
                                         0x1.CE1AF0B85F000p-2, //0.451274644139402880683
                                         0x1.D32FE7E00F000p-2, //0.456237433481646803557
                                         0x1.D83E7258A3000p-2, //0.461175715122180918115
                                         0x1.DD46A04C1C000p-2, //0.466089729924533457961
                                         0x1.E24881A7C7000p-2, //0.47097971521884574031
                                         0x1.E744261D68000p-2, //0.475845904869856894948
                                         0x1.EC399D2469000p-2, //0.480688529345798087888
                                         0x1.F128F5FAF0000p-2, //0.485507815781602403149
                                         0x1.F6123FA703000p-2, //0.490303988045297955978
                                         0x1.FAF588F78F000p-2, //0.495077266797807169496
                                         0x1.FFD2E0857F000p-2, //0.499827869556384030147
                                         0x1.02552A5A5D000p-1, //0.50455601075236700126
                                         0x1.04BDF9DA92800p-1, //0.509261901789841431309
                                         0x1.0723E5C1CE000p-1, //0.513945751102255599108
                                         0x1.0986F4F573800p-1, //0.518607764208127264283
                                         0x1.0BE72E4252800p-1, //0.52324814376447648101
                                         0x1.0E44985D1D000p-1, //0.527867089620940532768
                                         0x1.109F39E2D5000p-1, //0.532464798869568767259
                                         0x1.12F719593F000p-1, //0.537041465896891168086
                                         0x1.154C3D2F4D800p-1, //0.541597282432803694974
                                         0x1.179EABBD89800p-1, //0.546132437598089381936
                                         0x1.19EE6B467C800p-1, //0.550647117952621556469
                                         0x1.1C3B81F714000p-1, //0.555141507540611200966
                                         0x1.1E85F5E704000p-1, //0.559615787935399566777
                                         0x1.20CDCD192A800p-1, //0.564070138284705535625
                                         0x1.23130D7BEC000p-1, //0.568504735352689749561
                                         0x1.2555BCE98F800p-1, //0.572919753561791367247
                                         0x1.2795E1289B000p-1, //0.57731536503479219391
                                         0x1.29D37FEC2B000p-1, //0.581691739634607074549
                                         0x1.2C0E9ED449000p-1, //0.586049045003619539784
                                         0x1.2E47436E40000p-1, //0.590387446602107957006
                                         0x1.307D7334F1000p-1, //0.594707107746671681525
                                         0x1.32B1339122000p-1, //0.599008189646156097297
                                         0x1.34E289D9CE000p-1, //0.603290851438032404985
                                         0x1.37117B5474800p-1, //0.607555250224550036364
                                         0x1.393E0D3562800p-1, //0.611801541105933210929
                                         0x1.3B6844A000000p-1, //0.616029877215623855591
                                         0x1.3D9026A715800p-1, //0.620240409751886545564
                                         0x1.3FB5B84D17000p-1, //0.624433288011914555682
                                         0x1.41D8FE8467000p-1, //0.628608659422297932906
                                         0x1.43F9FE2F9D000p-1, //0.632766669571083184564
                                         0x1.4618BC21C6000p-1, //0.636907462237104482483
                                         0x1.48353D1EA8800p-1, //0.641031179420906482846
                                         0x1.4A4F85DB04000p-1, //0.645137961373620782979
                                         0x1.4C679AFCCF000p-1, //0.649227946625160257099
                                         0x1.4E7D811B75800p-1, //0.653301272012640765752
                                         0x1.50913CC016800p-1, //0.657358072708348117885
                                         0x1.52A2D265BC800p-1, //0.661398482245431296178
                                         0x1.54B2467999800p-1, //0.665422632545187298092
                                         0x1.56BF9D5B3F000p-1, //0.669430653942526987521
                                         0x1.58CADB5CD7800p-1, //0.673422675212123067467
                                         0x1.5AD404C35A000p-1, //0.677398823591829568613
                                         0x1.5CDB1DC6C1800p-1, //0.681359224807920327294
                                         0x1.5EE02A9241800p-1, //0.685304003098963221419
                                         0x1.60E32F4478800p-1, //0.689233281238784911693
                                         0x1.62E42FEFA3800p-1, //0.693147180559890330187      
                                    };
                                    
static const double C_trail[129] = {
                                         0.0,                   //                0
                                         0x1.9E23F0DDA40E4p-46, //2.29894100462035112076e-14
                                         0x1.F1E7CF6D3A69Cp-50, //1.72745674997061065553e-15
                                        -0x1.3B955B602ACE4p-44, //-7.00735970431003565857e-14
                                         0x1.980267C7E09E4p-45, //4.52981425779092882775e-14
                                         0x1.EAFD480AD9015p-44, //1.09021543022033016421e-13
                                        -0x1.181DCE586AF09p-44, //-6.21983419947579227529e-14
                                        -0x1.C827AE5D6704Cp-46, //-2.53216894311744497863e-14
                                        -0x1.D599E83368E91p-45, //-5.21362063913650408235e-14
                                        -0x1.47C5E768FA309p-46, //-1.81950600301688149235e-14
                                         0x1.1D09299837610p-44, //6.32906595872454402199e-14
                                         0x1.83F69278E686Ap-44, //8.61451293608781447223e-14
                                        -0x1.4B4641B664613p-44, //-7.35577021943502867846e-14
                                         0x1.B20F5ACB42A66p-44, //9.63806765855227740728e-14
                                         0x1.563650BD22A9Cp-44, //7.59863659719414144342e-14
                                         0x1.D0C57585FBE06p-46, //2.57999912830699022513e-14
                                        -0x1.A342C2AF0003Cp-45, //-4.65472974759844472568e-14
                                        -0x1.54555D1AE6607p-44, //-7.55692068745133691756e-14
                                         0x1.CB2CD2EE2F482p-44, //1.01957352237084734958e-13
                                         0x1.E80A41811A396p-45, //5.41833313790089940464e-14
                                        -0x1.5B967F4471DFCp-44, //-7.71800133682809851086e-14
                                         0x1.EE8779B2D8ABCp-44, //1.09807540998552379211e-13
                                        -0x1.70CC16135783Cp-46, //-2.04723578004619553937e-14
                                        -0x1.790BA37FC5238p-44, //-8.37209109923591205585e-14
                                        -0x1.8586F183BEBF2p-44, //-8.64923960721207091374e-14
                                        -0x1.BC6E557134767p-44, //-9.86835038673494943912e-14
                                        -0x1.BDB9072534A58p-45, //-4.94851676612509959777e-14
                                         0x1.22120401202FCp-44, //6.44085615069689207389e-14
                                        -0x1.297137D9F158Fp-44, //-6.60454487708238395939e-14
                                        -0x1.539CD91DC9F0Bp-44, //-7.54091651195618882501e-14
                                        -0x1.A4E633FCD9066p-52, //-3.65071888317905767114e-16
                                         0x1.9AC53F39D121Cp-44, //9.12093724991498410553e-14
                                        -0x1.7794F689F8434p-45, //-4.16979658452719528642e-14
                                        -0x1.1BA91BBCA681Bp-45, //-3.14926506519148377243e-14
                                        -0x1.A342C2AF0003Cp-44, //-9.30945949519688945136e-14
                                        -0x1.B26B79C86AF24p-45, //-4.82302894299408858477e-14
                                        -0x1.D572AAB993C87p-47, //-1.30297971733086634357e-14
                                         0x1.036B89EF42D7Fp-48, //3.60017673263733462441e-15
                                         0x1.C6BEE7EF4030Ep-47, //1.26217293988853160748e-14
                                        -0x1.4AB9D817D52CDp-44, //-7.34359136986779711761e-14
                                         0x1.8380E731F55C4p-44, //8.60430677280873279668e-14
                                        -0x1.81410E5C62AFFp-44, //-8.55436000656632193091e-14
                                        -0x1.A6976F5EB0963p-44, //-9.38341722366369999987e-14
                                         0x1.A8D7AD24C13F0p-44, //9.43339818951269030846e-14
                                        -0x1.67B1E99B72BD8p-45, //-3.99341638438784391272e-14
                                        -0x1.5594DD4C58092p-45, //-3.7923164802093146798e-14
                                         0x1.7A71CBCD735D0p-44, //8.40315630479242455758e-14
                                         0x1.F8EF43049F7D3p-44, //1.1211800740360981983e-13
                                        -0x1.3D82F484C84CCp-46, //-1.76254313121726620573e-14
                                        -0x1.D7C92CD9AD824p-44, //-1.04757500587765412913e-13
                                        -0x1.F4BD8DB0A7CC1p-44, //-1.11186713895593226425e-13
                                        -0x1.64EAD9524D7CAp-44, //-7.92515783138655870267e-14
                                        -0x1.8D6BDC9C7C238p-44, //-8.8245263321256400121e-14
                                         0x1.E54BDBD7C8A98p-44, //1.07757430375726404546e-13
                                         0x1.2BB110AF84054p-44, //6.65449164332479482515e-14
                                         0x1.E38C139318D71p-46, //2.68422602858563731995e-14
                                         0x1.A7389314FEB50p-52, //3.67085697163493829481e-16
                                         0x1.E89F057691FEAp-44, //1.08495696229679121506e-13
                                        -0x1.E4DA62D0C25ADp-49, //-3.36434401382552911606e-15
                                        -0x1.3A2DB13AE687Cp-44, //-6.97616377035377091917e-14
                                         0x1.2D5AD38C40882p-45, //3.3457102695440823738e-14
                                         0x1.63BF0BB4EAB4Cp-45, //3.949577025210288071e-14
                                         0x1.BEAE9337451F4p-44, //9.91833135258393974771e-14
                                         0x1.1597525DD88F0p-47, //7.70470078193964863175e-15
                                        -0x1.ED03525CA2643p-44, //-1.09470871366066397592e-13
                                        -0x1.3D7500D6523C5p-44, //-7.04896239210974659112e-14
                                        -0x1.ED9CADEC02B43p-44, //-1.09603887929539904968e-13
                                        -0x1.E53BB31EED7A9p-44, //-1.07743414616095792458e-13
                                        -0x1.3AE68224AA2CEp-47, //-8.74024251107295313295e-15
                                         0x1.F6B31F629F11Ep-47, //1.39527194700992522593e-14
                                        -0x1.21021E78B2151p-44, //-6.41727287881571093141e-14
                                        -0x1.5946261F5A42Bp-45, //-3.83331165923754571982e-14
                                        -0x1.7794F689F8434p-44, //-8.33959316905439057284e-14
                                         0x1.F5BDBE95E5568p-45, //5.57044620824077391343e-14
                                        -0x1.0AA7884DCD050p-44, //-5.92091761359114736879e-14
                                        -0x1.835F5D48BA26Dp-47, //-1.07517471912360339462e-14
                                         0x1.282FB989A9274p-44, //6.57665976858006147528e-14
                                        -0x1.ECF1A1385D356p-45, //-5.47277630185806178777e-14
                                         0x1.E1F8DF68DBCF3p-44, //1.07019317621142549209e-13
                                        -0x1.9FF45188D6065p-45, //-4.61802117788209510607e-14
                                         0x1.BB2CD720EC44Cp-44, //9.84046527823262695501e-14
                                        -0x1.D4E7AEA4F0D25p-44, //-1.04117827384293371195e-13
                                         0x1.8F6CD7D9F2754p-45, //4.43451018828153751026e-14
                                         0x1.261565F40D932p-44, //6.52996738757825591006e-14
                                         0x1.FD8D38D2BAFDDp-46, //2.8285798609067893876e-14
                                        -0x1.2D9A033EFF74Ep-45, //-3.34845053941249831574e-14
                                        -0x1.7F6350D38EDDDp-46, //-2.12823065872096837292e-14
                                        -0x1.6FA37012B5806p-44, //-8.16321296891503919914e-14
                                         0x1.415B4C4BDD99Fp-44, //7.13555066011812146304e-14
                                        -0x1.BA048A8D10B4Bp-44, //-9.81476542529858082436e-14
                                        -0x1.B4810E09B27A4p-44, //-9.69233849737002813365e-14
                                        -0x1.0EB3FB7398E0Cp-47, //-7.51351912898166894415e-15
                                        -0x1.0B2B38662E34Dp-44, //-5.93233971574446149215e-14
                                         0x1.A0BFC60E6FA08p-45, //4.62684463909612350375e-14
                                         0x1.6ECC5CBDD7782p-45, //4.07227907088846767786e-14
                                        -0x1.EDA1B58389902p-44, //-1.09608250460592783688e-13
                                         0x1.A07BD8B34BE7Cp-46, //2.3119493838005377632e-14
                                         0x1.B6C9A81E87BAEp-44, //9.74304462767037064935e-14
                                        -0x1.7AFA4392F1BA7p-46, //-2.10374825114449422873e-14
                                        -0x1.A61FDE292977Ep-48, //-5.85815401264202219115e-15
                                         0x1.1AEB783F3DB97p-45, //3.14104080050449590607e-14
                                         0x1.1590B9AD974BAp-46, //1.54079711890856738893e-14
                                        -0x1.74468563CE45Dp-45, //-4.13308801481084566682e-14
                                         0x1.34202A10C3491p-44, //6.84176364159146659095e-14
                                         0x1.7C3F6B2143EADp-46, //2.11079891578422983965e-14
                                        -0x1.4766FD54A4C27p-44, //-7.26979150253512871478e-14
                                         0x1.D316EB92D885Dp-45, //5.18573553063418286042e-14
                                        -0x1.28E88BF6DEEC9p-47, //-8.24086314983113070392e-15
                                         0x1.0CD4E221301B7p-44, //5.96926009653846962309e-14
                                        -0x1.EEA838909F3D3p-44, //-1.0983594325438429833e-13
                                        -0x1.055BFBD9C2F53p-45, //-2.90167125533596631915e-14
                                        -0x1.7B4962C55F46Bp-46, //-2.10546393306435734475e-14
                                         0x1.5732325E617A3p-44, //7.6204838231893709023e-14
                                        -0x1.98858D84649F1p-45, //-4.53550186996774688761e-14
                                        -0x1.3D82F484C84CCp-45, //-3.52508626243453241145e-14
                                         0x1.BEE7ABD176604p-46, //2.48082091251967673736e-14
                                        -0x1.44FDD840B8591p-45, //-3.60813136042255739798e-14
                                        -0x1.C64E971322CE8p-45, //-5.04382083563449091526e-14
                                         0x1.D84E584C2B22Cp-44, //1.04873006903856996874e-13
                                         0x1.AD2F2CE96C2D6p-47, //1.19122567102055698791e-14
                                        -0x1.2A88C41BA8752p-44, //-6.62879179039074743232e-14
                                        -0x1.B42B755EBA5E1p-44, //-9.68491419671810783613e-14
                                         0x1.CCA08E310B9B2p-44, //1.02279777907416200886e-13
                                         0x1.893092F25D931p-45, //4.36528304869414810551e-14
                                        -0x1.A609ACAAB41FCp-46, //-2.34278036379790696248e-14
                                        -0x1.36E612387451Fp-46, //-1.72583456150091690167e-14
                                        -0x1.8A8F29F6A02DCp-45, //-4.38048746232309815355e-14
                                         0x1.B194F912B416Ap-46, //2.40686318405286681994e-14
                                         0x1.EF35793C76730p-45, //5.49792301870837115524e-14
                                     };
                                    

static const double T1      =  0x1.E0FABFBC702A3p-1;  //0.939413062813475696622  // e^(-1/16)
static const double T2      =  0x1.1082B577D34EEp+0;  //1.06449445891785954288 // e^(+1/16)
static const double tiny    =  0x0.0000000000001p-1022; //4.94065645841246544177e-324
static const xDouble plusinf =  { INFINITY, 0.0 };
static const xDouble smallestNormal = { 0x1.0p-1022, 0 };
static const double denormBias        =  0.0;
static const xDouble one =  { 1.0, 0.0 };
static const double d128 = 128.0;
static const double d0_0078125 = 0.0078125;
static const xSInt64 bias[2] = { {2045, 0},  {1023, 0} };
static const xSInt64 * const biasp = bias + 1;
static const double mOne = -1.0;
static const double two = 2.0;
static const xSInt64 logNaN = { 0x7ff8000000000024LL, 0 };
static const xSInt64 logNaNf = { 0x7ff8000480000000LL, 0 };

//x must be finite, non-NaN, positive
static inline xDouble mantissa( xDouble x, xDouble *exponent ) ALWAYS_INLINE;
static inline xDouble mantissa( xDouble x, xDouble *exponent )
{

    xDouble mantissa = _mm_andnot_pd( plusinf, x );                          // x & 0x800FFFFFFFFFFFFF   (note: always positive)
    xDouble isDenormal = _mm_cmplt_sdm( x, (double*) &smallestNormal );		 // -1LL if denormal, 0 otherwise
    xDouble denormExp = _mm_and_pd( one, isDenormal );                       // 1.0 if denormal, 0.0 otherwise
    int biasSelector = _mm_cvtsi128_si32( (__m128i) isDenormal );            // -1 if denormal, 0 otherwise
    x = _mm_or_pd( x, denormExp );                                           // or in 1.0 as exponent if denormal
    x = _mm_sub_sd( x, denormExp );                                          // subtract away 1.0 as exponent if denormal
    mantissa = _mm_andnot_pd( plusinf, x );                                  // get the mantissa of x
    xUInt64 iExp = (__m128i) _mm_and_pd( x, plusinf );                       // extract the exponent of x
    iExp = _mm_srli_epi64( iExp, 52 );                                       // right shift to make an integer
    iExp = _mm_sub_epi64( iExp, biasp[ biasSelector ] );                     // subtract away either 1023 or the denormal bias
    *exponent = _mm_cvtepi32_pd( iExp );                                     // convert to double
    return _mm_or_pd( mantissa, one );                             // set the mantissa to have a range 1.0 <= mantissa < 2.0
}

static inline xDouble _xlog( xDouble x )
{
	static const double half = 0.5;
  xDouble Y, F, f, f1, L_lead, L_trail, R, u, u1, u2, v, q, g, m;
	int j;

	xDouble xLET1 = _mm_cmple_sdm( x, &T1 );
	xDouble xGET2 = _mm_cmpge_sdm( x, &T2 );
	if( _mm_istrue_sd( _mm_or_pd( xLET1, xGET2 ) ) )
	{
		Y = mantissa( x, &m );
		j = _mm_cvttsd_si32( _mm_add_sdm( _mm_mul_sdm( Y, &d128 ), &half) );
		F = _mm_mul_sdm( _mm_cvtsi32_sd( Y, j) , &d0_0078125 );
		j -= 128;
		f = _mm_sub_sd( Y, F );
		L_lead = _mm_add_sdm( _mm_mul_sdm( m, &C_lead[128] ), &C_lead[j] );
		L_trail = _mm_add_sdm( _mm_mul_sdm( m, &C_trail[128] ), &C_trail[j] );
		u = _mm_div_sd( _mm_add_sd( f, f), _mm_add_sd( Y, F ) );
		v = _mm_mul_sd( u, u );
		q = _mm_add_sdm( _mm_mul_sdm( v, &A1[1] ), &A1[0] );
		q = _mm_mul_sd( q, v );
		q = _mm_mul_sd( q, u );
		R = _mm_add_sd( L_lead, _mm_add_sd( u , _mm_add_sd( q, L_trail ) ) );
	}
	else
	{
		xDouble xEQone = _mm_cmpeq_sdm( x, (double*) &one );
		
		f = _mm_sub_sdm( x, (double*) &one );
		g = _mm_div_sd( one, _mm_add_sdm( f, &two ) );
		u = _mm_mul_sd( f, g );     u = _mm_add_sd( u, u );     //u = 2.0 * (f * g)
		v = _mm_mul_sd( u, u );
		q = _mm_add_sdm( _mm_mul_sdm( v, &A2[3] ), &A2[2] );
		q = _mm_add_sdm( _mm_mul_sd( q, v ), &A2[1] );
		q = _mm_add_sdm( _mm_mul_sd( q, v ), &A2[0] );
		q = _mm_mul_sd( _mm_mul_sd( q, v ), u );
		u1 = _mm_cvtss_sd( u, _mm_cvtsd_ss( (xFloat) u, u ) );
		f1 = _mm_cvtss_sd( f, _mm_cvtsd_ss( (xFloat) f, f ) );
		
		//			u2 = ( ( 2 * ( f - u1 ) - u1 * f1 ) - u1 * ( f - f1 ) ) * g;
		xDouble t0 = _mm_sub_sd( f, u1 );       //f - u1
		xDouble t1 = _mm_sub_sd( f, f1 );       //f - f1
		xDouble t2 = _mm_mul_sd( u1, f1 );      //u1 * f1
		t0 = _mm_add_sd( t0, t0 );              //2 * (f- u1)
		t1 = _mm_mul_sd( t1, u1 );              //u1 * ( f - f1 )
		t0 = _mm_sub_sd( t0, t2 );              //2 * (f - u1) - u1 * f1
		t0 = _mm_sub_sd( t0, t1 );              //(2 * (f - u1) - u1 * f1) - u1 * (f- f1)
		u2 = _mm_mul_sd( t0, g );               //((2 * (f - u1) - u1 * f1) - u1 * (f- f1)) * g
		R = _mm_add_sd( u2, q );
		R = _mm_add_sd( R, u1 );
		
		R = _mm_andnot_pd( xEQone, R );
	}
	return R;
}

/*
static inline double _xlog2( double x ) ALWAYS_INLINE;
static inline double _xlog2( double x )
{
    static const double conversion = 1.0 / 6.9314718055994530942E-1;
    xDouble r = _xlog( DOUBLE_2_XDOUBLE( x ) );
    r = _mm_mul_sdm( r, &conversion );
    return XDOUBLE_2_DOUBLE( r );
}
*/

#pragma mark -
#pragma mark log2

#if defined( BUILDING_FOR_CARBONCORE_LEGACY )

#warning goofy use of scaled log() to do log2()
double log2( double x )
{
	static const double conversion = 1.0 / 6.9314718055994530942E-1;
	xDouble xd = DOUBLE_2_XDOUBLE( x );
	
	xDouble xIsNaN = _mm_cmpunord_sd( xd, xd );
	xDouble safeX = _mm_andnot_pd( xIsNaN, xd );
	
	xDouble xGTzero = _mm_cmpgt_sdm( safeX, (double*) &minusZeroD );
	xDouble xLTinf =  _mm_cmplt_sdm( safeX, (double*) &plusinf );
	
	// mainline: 0 < x < inf
	if( _mm_istrue_sd( _mm_and_pd( xGTzero, xLTinf ) ) ) {
		xd = _xlog(xd);
		xd = _mm_mul_sdm(xd, &conversion);
	}
	
	// edge cases
	else {
		if( _mm_istrue_sd( _mm_cmpeq_sd( xd, minusZeroD ) ) )
			xd = _mm_div_sd( _mm_load_sd( &two ), minusZeroD );
		else {
			xDouble inf = plusinf;
			xDouble isInf = _mm_cmpeq_sd( xd, inf );
			inf = _mm_andnot_pd( xIsNaN, inf );
			inf = _mm_andnot_pd( isInf, inf );
			
			xDouble xLTzero = _mm_cmplt_sdm( safeX, (double*) &minusZeroD );
			
			xd = _mm_sub_sd( xd, inf );				//silence NaN, set finite values to -Inf
			xd = _mm_add_sd( xd, inf );
			xLTzero = _mm_and_pd( xLTzero, (xDouble) logNaN );
			xd = _mm_add_sd( xLTzero, xd );   //set to NaN if x < 0
		}
	}
	
	// return
	return XDOUBLE_2_DOUBLE( xd );
}

#else

/*
float log2f( float x )
{
	//cheesy fallback on double, that probably fails to get the edge cases right.
	static const double conversion = 1.0 / 6.9314718055994530942E-1;
	xDouble xd = FLOAT_2_XDOUBLE( x );
		
	xDouble xIsNaN = _mm_cmpunord_sd( xd, xd );
	xDouble safeX = _mm_andnot_pd( xIsNaN, xd );
	
	xDouble xGTzero = _mm_cmpgt_sdm( safeX, (double*) &minusZeroD );
	xDouble xLTinf =  _mm_cmplt_sdm( safeX, (double*) &plusinf );
	
	// mainline: 0 < x < inf
	if( _mm_istrue_sd( _mm_and_pd( xGTzero, xLTinf ) ) ) {
		xd = _xlog(xd);
		xd = _mm_mul_sdm(xd, &conversion);
	}
	
	// edge cases
	else {
		if( _mm_istrue_sd( _mm_cmpeq_sd( xd, minusZeroD ) ) )
			xd = _mm_div_sd( _mm_load_sd( &two ), minusZeroD );
		else {
			xDouble inf = plusinf;
			xDouble isInf = _mm_cmpeq_sd( xd, inf );
			inf = _mm_andnot_pd( xIsNaN, inf );
			inf = _mm_andnot_pd( isInf, inf );
			
			xDouble xLTzero = _mm_cmplt_sdm( safeX, (double*) &minusZeroD );
			
			xd = _mm_sub_sd( xd, inf );				//silence NaN, set finite values to -Inf
			xd = _mm_add_sd( xd, inf );
			xLTzero = _mm_and_pd( xLTzero, (xDouble) logNaNf );
			xd = _mm_add_sd( xLTzero, xd );   //set to NaN if x < 0
		}
	}
	
	// return
	return XDOUBLE_2_FLOAT( xd );
}
*/

#pragma mark -
#pragma mark log

double log ( double x )
{
	xDouble xd = DOUBLE_2_XDOUBLE( x );
	
	xDouble xIsNaN = _mm_cmpunord_sd( xd, xd );
	xDouble safeX = _mm_andnot_pd( xIsNaN, xd );
	
	xDouble xGTzero = _mm_cmpgt_sdm( safeX, (double*) &minusZeroD );
	xDouble xLTinf =  _mm_cmplt_sdm( safeX, (double*) &plusinf );
	
	// mainline: 0 < x < inf
	if( _mm_istrue_sd( _mm_and_pd( xGTzero, xLTinf ) ) )
		xd = _xlog(xd);
	
	// edge cases
	else {
		if( _mm_istrue_sd( _mm_cmpeq_sd( xd, minusZeroD ) ) )
			xd = _mm_div_sd( _mm_load_sd( &two ), minusZeroD );
		else {
			xDouble inf = plusinf;
			xDouble isInf = _mm_cmpeq_sd( xd, inf );
			inf = _mm_andnot_pd( xIsNaN, inf );
			inf = _mm_andnot_pd( isInf, inf );
			
			xDouble xLTzero = _mm_cmplt_sdm( safeX, (double*) &minusZeroD );
			
			xd = _mm_sub_sd( xd, inf );				//silence NaN, set finite values to -Inf
			xd = _mm_add_sd( xd, inf );
			xLTzero = _mm_and_pd( xLTzero, (xDouble) logNaN );
			xd = _mm_add_sd( xLTzero, xd );   //set to NaN if x < 0
		}
	}
	
	// return
	return XDOUBLE_2_DOUBLE( xd );
}

/*
float    logf( float x )
{
	//cheesy fallback on double, that probably fails to get the edge cases right.
	xDouble xd = FLOAT_2_XDOUBLE( x );
		
	xDouble xIsNaN = _mm_cmpunord_sd( xd, xd );
	xDouble safeX = _mm_andnot_pd( xIsNaN, xd );
	
	xDouble xGTzero = _mm_cmpgt_sdm( safeX, (double*) &minusZeroD );
	xDouble xLTinf =  _mm_cmplt_sdm( safeX, (double*) &plusinf );
	
	// mainline: 0 < x < inf
	if( _mm_istrue_sd( _mm_and_pd( xGTzero, xLTinf ) ) )
		xd = _xlog(xd);
	
	// edge cases
	else {
		if( _mm_istrue_sd( _mm_cmpeq_sd( xd, minusZeroD ) ) )
			xd = _mm_div_sd( _mm_load_sd( &two ), minusZeroD );
		else {
			xDouble inf = plusinf;
			xDouble isInf = _mm_cmpeq_sd( xd, inf );
			inf = _mm_andnot_pd( xIsNaN, inf );
			inf = _mm_andnot_pd( isInf, inf );
			
			xDouble xLTzero = _mm_cmplt_sdm( safeX, (double*) &minusZeroD );
			
			xd = _mm_sub_sd( xd, inf );				//silence NaN, set finite values to -Inf
			xd = _mm_add_sd( xd, inf );
			xLTzero = _mm_and_pd( xLTzero, (xDouble) logNaNf );
			xd = _mm_add_sd( xLTzero, xd );   //set to NaN if x < 0
		}
	}
	
	// return
	return XDOUBLE_2_FLOAT( xd );
}
*/

#pragma mark -
#pragma mark log1p

static inline xDouble _xlog1p( xDouble x )   ALWAYS_INLINE;        

static const double T1p     = -0x1.F0540438FD5C4p-5;  //-0.0605869371865242201114  // e^(-1/16) - 1
static const double T2p     =  0x1.082B577D34ED8p-4;  //0.0644944589178594318568;  // e^(+1/16) - 1
static const double T4p     =  0x1.0000000000000p-53; //1.11022302462515654042e-16;  // 2^(-53)

static inline xDouble _xlog1p( xDouble x )
{
	static const double mTwo = -2.0;
	static const double x52 = 52.0;
	static const double eight = 8.0;
	static const double oneEighth = 0.125;
	static const double twom1 = 0x1.0p-1;
	static const double twom9 = 0x1.0p-9;

	xDouble Y, F, f, f1, L_lead, L_trail, R, u, u1, u2, v, q, g, m;
	xSInt64 im, imc;
	int j;
	   
	xDouble xLEt1p = _mm_cmple_sdm( x, &T1p );
	xDouble xGEt2p = _mm_cmpge_sdm( x, &T2p );
	xDouble fabsX = _mm_andnot_pd( minusZeroD, x );
	xDouble bt = _mm_or_pd( xLEt1p, xGEt2p );

	if( _mm_istrue_sd( bt ) )          // x <= T1p || x >= T2p
	{
/*			Y = mantissa(x + 1.0, &m);  //  m = logb(x+1);  Y = scalb(x+1, -m);
			j  = (int)(128.0*Y + 0.5);
			F  = j*0.0078125;
			j -= 128;
			
			if ( m <= -2 ) 
				f = Y - F;
			else if ( m <= 52 )
				f = ( scalb ( 1.0, - m ) - F ) + scalb ( x, - m );
			else 
				f = ( scalb ( x, - m ) - F ) + scalb ( 1.0, - m );
*/
           
		//We can simplify a number of ways here: 
		//  1) x + 1.0 cannot give us a denormal result.
		//  2) (x+1.0) * 2**-8 cannot underfow. 
		//  3) In this particular case, x+1.0 is also always positive, so certain hacks 
		//      that fail with negative numbers work here.
		//
		//Lets do this a different way. Don't reduce Y to a mantissa section until later.
		//Just use x + 1.0 instead.  We can extract out the top 8 bits with rounding by
		//adding the exponent of (x+1) * 2-8 to x+1. Use a mask to trim off the remaining 
		//mantissa bits below the first 7. This is okay for the overflow case where the rounding
		//value triggers a promotion in the exponent, since the leading bits in the mantissa are
		//all zeros in this case, so using the wrong mask isn't a problem. Note that at this case,
		//F and Y are scaled larger than the old code by 2**m. This means we can do the calculation
		//without scalb above, and just use 1.0 or x where scalb ( 1.0, - m ) and scalb ( x, - m )
		//appear.
		
		double fparts[4];
		int index;
		Y = _mm_sub_sdm( x, &mOne );
		im = _mm_srli_epi64( (xSInt64) Y, 52 );     //Grab the exponent before we destroy it. Y is always positive, so we need only shift right
		R = _mm_mul_sdm( Y, &twom9 );        //rounding value
		imc = _mm_sub_epi64( im, bias[1] );              //m as an integer (eponent - 1023)
		im = _mm_add_epi64( im, (xSInt64) bt );			//subtract 1 from im, to correct for 0.5 multiplication to follow
		Y = _mm_mul_sdm( Y, &twom1 );        //divide Y by two so that the F calculation doesn't overflow to infinity. This cannot underflow.
		R = _mm_and_pd( R, plusinf );       //trim off mantissa of the rounding value
		m = _mm_cvtepi32_pd( imc );                      //-m as a double
		F = _mm_add_sd( Y, R );             //Round Y up to give F 
		x = _mm_mul_sdm( x, &twom1 );                            //convert x to 0.5x (exact, unless x is denorm, in which case, it does not influence the answer) 
		F = (xDouble) _mm_srli_epi64( (xSInt64) F, 52-7 );         //truncate F
		im = _mm_slli_epi64( im, 7 );
		im = _mm_sub_epi64( (xSInt64) F, im );
		F = (xDouble) _mm_slli_epi64( (xSInt64) F, 52-7 );
		j = _mm_cvtsi128_si32( im );
		
		//F now has the mantissa we want. Both Y and F are currently too large by 2**(m-1)
		fparts[0] = 0.0;
		fparts[1] = 0.5;
		_mm_store_sd( &fparts[2], x );
		_mm_store_sd( &fparts[3], Y );
		xDouble mLEmtwo = _mm_cmple_sdm( m, &mTwo );  
		xDouble mLE52 = _mm_cmple_sdm( m, &x52 );  
		index = _mm_cvtsi128_si32( _mm_add_epi64( (xSInt64) mLEmtwo, (xSInt64) mLE52 ) ); //-2 for m <= -2, -1 for -2 < m <=52, 0 otherwise
		
		f = _mm_sub_sdm( F, fparts + 2 + index );        // f = { F - 0,     F - 0.5,     F - x       }
		f = _mm_sub_sdm( f, fparts + 1 - index );        // f = { F - 0 - Y, F - 0.5 - x, F - x - 0.5 }
		
		//f currently has the wrong exponent ( off by 2**(m-1), and the wrong sign )
		//However, the rest of the code looks like this:
		//  L_lead  = m * C_lead[128].d + C_lead[j].d;
		//  L_trail = m * C_trail[128].d + C_trail[j].d;
		//  u = ( 2.0 * f ) / ( Y + F );
		//  v = u * u;
		//  q = u * v * ( A1[0].d + v * A1[1].d );
		//  R = L_lead + ( u + ( q + L_trail ) );
		//
		// f is only used in one place and there it is divided by Y + F, which also are 
		// off by 2**(m-1), so it cancels out, presuming the intermediate terms don't overflow
		// The maximum possible value of F is exactly 2**1023, and Y must be slightly less than that, so the
		// denomenator cannot overflow. f is considerably smaller than F, so 2 * f cannot overflow.
		L_lead = _mm_add_sdm( _mm_mul_sdm( m, C_lead + 128 ), C_lead + j );
		L_trail = _mm_add_sdm( _mm_mul_sdm( m, C_trail + 128 ), C_trail + j);
		Y = _mm_add_sd( Y, F );             //Y + F, too large by 2**(m-1)
		f = _mm_mul_sdm( f, &mTwo );        //2 * f, sign is correct now, too large by 2**(m-1)
		u = _mm_div_sd( f, Y );             //u is correct
		v = _mm_mul_sd( u, u );
		q = _mm_add_sdm( _mm_mul_sdm( v, A1 + 1 ), A1 );
		v = _mm_mul_sd( v, u );
		q = _mm_mul_sd( q, v );
		R = _mm_add_sd( q, L_trail );
		R = _mm_add_sd( R, u );
		R = _mm_add_sd( R, L_lead );
	}
	else if( _mm_istrue_sd( _mm_cmple_sdm( fabsX, &T4p ) ) )
	{
		xDouble xEQzero = _mm_cmpeq_sdm( x, (double*) &minusZeroD );
		xDouble ittyBit = _mm_load_sd( &tiny );
		ittyBit = _mm_andnot_pd( xEQzero, ittyBit );
		//R = ( 8.0 * x - tiny.d ) / 8.0;
		R = _mm_mul_sdm( x, &eight );
		R = _mm_sub_sd( R, ittyBit );
		R = _mm_mul_sdm( R, &oneEighth );
		
		R = _mm_sel_pd( R, x, xEQzero );                        //make sure the x == 0 case is zero
	}
	else
	{
		//			g = 1.0 / ( 2.0 + x );
		g = _mm_div_sd( one, _mm_sub_sdm( x, &mTwo ) );
		//			u = 2 * ( x * g );
		u = _mm_mul_sd( x, g );
		u = _mm_add_sd( u, u ); 
		//			v = u * u;
		v = _mm_mul_sd( u, u );
		//			q = u * v * ( A2[0].d +  v * ( A2[1].d + v * ( A2[2].d + v * A2[3].d ) ) );
		q = _mm_add_sdm( _mm_mul_sdm( v, A2 + 3 ), A2 + 2 );
		q = _mm_add_sdm( _mm_mul_sd( q, v ), A2 + 1 );
		q = _mm_add_sdm( _mm_mul_sd( q, v ), A2 + 0 );
		v = _mm_mul_sd( v, u );
		q = _mm_mul_sd( q, v );
		//			u1 = ( float ) u;
		u1 = _mm_cvtss_sd( u, _mm_cvtsd_ss( (xFloat) u, u ) );
		//			f1 = ( float ) x;
		f1 = _mm_cvtss_sd( x, _mm_cvtsd_ss( (xFloat) x, x ) );
		//			u2 = ( ( 2 * ( x - u1 ) - u1 * f1 ) - u1 * ( x - f1 ) ) * g;
		xDouble t0 = _mm_sub_sd( x, u1 );       //f - u1
		xDouble t1 = _mm_sub_sd( x, f1 );       //f - f1
		xDouble t2 = _mm_mul_sd( u1, f1 );      //u1 * f1
		t0 = _mm_add_sd( t0, t0 );              //2 * (f- u1)
		t1 = _mm_mul_sd( t1, u1 );              //u1 * ( f - f1 )
		t0 = _mm_sub_sd( t0, t2 );              //2 * (f - u1) - u1 * f1
		t0 = _mm_sub_sd( t0, t1 );              //(2 * (f - u1) - u1 * f1) - u1 * (f- f1)
		u2 = _mm_mul_sd( t0, g );               //((2 * (f - u1) - u1 * f1) - u1 * (f- f1)) * g
		
		//			R = u1 + ( u2 + q );
		R = _mm_add_sd( u2, q );
		R = _mm_add_sd( R, u1 );
	}
	
	return R;
}

double log1p ( double x ) {
	xDouble xd = DOUBLE_2_XDOUBLE( x );
	
	xDouble xIsNaN = _mm_cmpunord_sd( xd, xd );
	if( _mm_istrue_sd( xIsNaN ) )
		return XDOUBLE_2_DOUBLE( (xDouble)logNaN );
	
	xDouble xGTminusOne = _mm_cmpgt_sdm( xd, &mOne );
	xDouble xLTinfinity = _mm_cmplt_sdm( xd, (double*) &plusinf );
	
	// mainline: -1 < x < inf
	if( _mm_istrue_sd( _mm_and_pd(  xGTminusOne, xLTinfinity ) ) ) {
		xd = _xlog1p( xd );
	}
	
	// edge cases
	else {
		if( _mm_istrue_sd( _mm_cmpeq_sdm( xd, &mOne ) ) )
			xd = _mm_div_sd( _mm_load_sd( &two ), minusZeroD );
		else {
			xDouble inf = plusinf;
			xDouble isInf = _mm_cmpeq_sd( xd, inf );
			inf = _mm_andnot_pd( xIsNaN, inf );
			inf = _mm_andnot_pd( isInf, inf );
			
			xDouble xLTzero = _mm_cmplt_sdm( xd, (double*) &minusZeroD );
		
			xd = _mm_sub_sd( xd, inf );   //silence NaN, set finite values to -Inf
			xd = _mm_add_sd( xd, inf );
			xLTzero = _mm_and_pd( xLTzero, (xDouble) logNaN );
			xd = _mm_add_sd( xLTzero, xd );   //set to NaN if x < 0
		}
	}
	
	// return
	return XDOUBLE_2_DOUBLE( xd );
}

/*
float    log1pf( float x )
{		//cheesy fallback on double, that probably fails to get the edge cases right.
	xDouble xd = FLOAT_2_XDOUBLE( x );
	
	xDouble xIsNaN = _mm_cmpunord_sd( xd, xd );
	if( _mm_istrue_sd( xIsNaN ) )
		return XDOUBLE_2_FLOAT( (xDouble)logNaNf );
	
	xDouble xGTminusOne = _mm_cmpgt_sdm( xd, &mOne );
	xDouble xLTinfinity = _mm_cmplt_sdm( xd, (double*) &plusinf );
	
	// mainline: -1 < x < inf
	if( _mm_istrue_sd( _mm_and_pd(  xGTminusOne, xLTinfinity ) ) ) {
		xd = _xlog1p( xd );
	}
	
	// edge cases
	else {
		if( _mm_istrue_sd( _mm_cmpeq_sdm( xd, &mOne ) ) )
			xd = _mm_div_sd( _mm_load_sd( &two ), minusZeroD );
		else {
			xDouble inf = plusinf;
			xDouble isInf = _mm_cmpeq_sd( xd, inf );
			inf = _mm_andnot_pd( xIsNaN, inf );
			inf = _mm_andnot_pd( isInf, inf );
			
			xDouble xLTzero = _mm_cmplt_sdm( xd, (double*) &minusZeroD );
			
			xd = _mm_sub_sd( xd, inf );   //silence NaN, set finite values to -Inf
			xd = _mm_add_sd( xd, inf );
			xLTzero = _mm_and_pd( xLTzero, (xDouble) logNaNf );
			xd = _mm_add_sd( xLTzero, xd );   //set to NaN if x < 0
		}
	}
	
	// return
	return XDOUBLE_2_FLOAT( xd );
}
*/

#pragma mark -
#pragma mark log10

static inline double _xlog10( double ) ALWAYS_INLINE;

/* static xUInt64 LOGORITHMIC_NAN = { 0x7FF8000000000000ULL, 0 };  [sic!] */

static inline double _xlog10( double x )
{
    static const double half = 0.5;
    static const double log10e = 0.434294481903251827651128918916605082;  // log_10(e) 
    xDouble xx = DOUBLE_2_XDOUBLE( x );
	xDouble xIsNaN = _mm_cmpunord_sd( xx, xx );
	xDouble safeX = _mm_andnot_pd( xIsNaN, xx );
    xDouble xGTzero = _mm_cmpgt_sdm( safeX, (double*) &minusZeroD );
    xDouble xLTinf =  _mm_cmplt_sdm( safeX, (double*) &plusinf );
    xDouble R;

    if( _mm_istrue_sd( _mm_and_pd( xGTzero, xLTinf ) ) ) // x > 0 && x < inf
    {
        if( (x <= T1) || x >= T2 )
        {
            xDouble m;
            xDouble Y = mantissa( xx, &m );
            int j = _mm_cvttsd_si32( _mm_add_sdm( _mm_mul_sdm( Y, &d128 ), &half ));
            xDouble F = _mm_mul_sdm( _mm_cvtsi32_sd( Y, j) , &d0_0078125 );
            j -= 128;
            xDouble f = _mm_sub_sd( Y, F );
            xDouble L_lead = _mm_add_sdm( _mm_mul_sdm( m, &C_lead[128] ), &C_lead[j] );
            xDouble L_trail = _mm_add_sdm( _mm_mul_sdm( m, &C_trail[128] ), &C_trail[j] );
            xDouble u = _mm_div_sd( _mm_add_sd( f, f), _mm_add_sd( Y, F ) );
            xDouble v = _mm_mul_sd( u, u );
            xDouble q = _mm_add_sdm( _mm_mul_sdm( v, &A1[1] ), &A1[0] );
            q = _mm_mul_sd( q, v );
            q = _mm_mul_sd( q, u );
            R = _mm_add_sd( L_lead, _mm_add_sd( u , _mm_add_sd( q, L_trail ) ) );
            R = _mm_mul_sdm( R, &log10e );
        }
        else
        {
            xDouble xEQone = _mm_cmpeq_sdm( xx, (double*) &one );
            
            xDouble f = _mm_sub_sdm( xx, (double*) &one );
            xDouble g = _mm_div_sd( one, _mm_add_sdm( f, &two ) );
            xDouble u = _mm_mul_sd( f, g );     u = _mm_add_sd( u, u );     //u = 2.0 * (f * g)
            xDouble v = _mm_mul_sd( u, u );
            xDouble q = _mm_add_sdm( _mm_mul_sdm( v, &A2[3] ), &A2[2] );
            q = _mm_add_sdm( _mm_mul_sd( q, v ), &A2[1] );
            q = _mm_add_sdm( _mm_mul_sd( q, v ), &A2[0] );
            q = _mm_mul_sd( _mm_mul_sd( q, v ), u );
            xDouble u1 = _mm_cvtss_sd( u, _mm_cvtsd_ss( (xFloat) u, u ) );
            xDouble f1 = _mm_cvtss_sd( f, _mm_cvtsd_ss( (xFloat) f, f ) );

            //			u2 = ( ( 2 * ( f - u1 ) - u1 * f1 ) - u1 * ( f - f1 ) ) * g;
            xDouble t0 = _mm_sub_sd( f, u1 );       //f - u1
            xDouble t1 = _mm_sub_sd( f, f1 );       //f - f1
            xDouble t2 = _mm_mul_sd( u1, f1 );      //u1 * f1
            t0 = _mm_add_sd( t0, t0 );              //2 * (f- u1)
            t1 = _mm_mul_sd( t1, u1 );              //u1 * ( f - f1 )
            t0 = _mm_sub_sd( t0, t2 );              //2 * (f - u1) - u1 * f1
            t0 = _mm_sub_sd( t0, t1 );              //(2 * (f - u1) - u1 * f1) - u1 * (f- f1)
            xDouble u2 = _mm_mul_sd( t0, g );               //((2 * (f - u1) - u1 * f1) - u1 * (f- f1)) * g
            R = _mm_add_sd( u2, q );
            R = _mm_add_sd( R, u1 );
            R = _mm_mul_sdm( R, &log10e );
            R = _mm_andnot_pd( xEQone, R );
        }
    }
    else
    { //edge cases
        if( _mm_istrue_sd( _mm_cmpeq_sd( xx, minusZeroD ) ) )
            R = _mm_div_sd( _mm_load_sd( &two ), minusZeroD );
        else
        {
			xDouble inf = plusinf;
			xDouble isInf = _mm_cmpeq_sd( xx, inf );
			inf = _mm_andnot_pd( xIsNaN, inf );
			inf = _mm_andnot_pd( isInf, inf );
			
            xDouble xLTzero = _mm_cmplt_sdm( safeX, (double*) &minusZeroD );

            R = _mm_sub_sd( xx, inf );   //silence NaN, set finite values to -Inf
			R = _mm_add_sd( R, inf );
            xLTzero = _mm_and_pd( xLTzero, (xDouble) logNaN );
            R = _mm_add_sd( xLTzero, R );   //set to NaN if x < 0
		}
    
    }
    
    return XDOUBLE_2_DOUBLE( R );
}

double log10 ( double x )
{
    return _xlog10( x );
}

/*
float    log10f( float x )
{//cheesy fallback on double, that probably fails to get the edge cases right.
    return (float) _xlog10( (double) x );
}
*/

#endif /* defined( BUILDING_FOR_CARBONCORE_LEGACY ) */
