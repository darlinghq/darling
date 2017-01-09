/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

///
/// Test that lazy binding is thread safe
///

extern int do_000();
extern int do_001();
extern int do_002();
extern int do_003();
extern int do_004();
extern int do_005();
extern int do_006();
extern int do_007();
extern int do_008();
extern int do_009();
extern int do_010();
extern int do_011();
extern int do_012();
extern int do_013();
extern int do_014();
extern int do_015();
extern int do_016();
extern int do_017();
extern int do_018();
extern int do_019();
extern int do_020();
extern int do_021();
extern int do_022();
extern int do_023();
extern int do_024();
extern int do_025();
extern int do_026();
extern int do_027();
extern int do_028();
extern int do_029();
extern int do_030();
extern int do_031();
extern int do_032();
extern int do_033();
extern int do_034();
extern int do_035();
extern int do_036();
extern int do_037();
extern int do_038();
extern int do_039();
extern int do_040();
extern int do_041();
extern int do_042();
extern int do_043();
extern int do_044();
extern int do_045();
extern int do_046();
extern int do_047();
extern int do_048();
extern int do_049();
extern int do_050();
extern int do_051();
extern int do_052();
extern int do_053();
extern int do_054();
extern int do_055();
extern int do_056();
extern int do_057();
extern int do_058();
extern int do_059();
extern int do_060();
extern int do_061();
extern int do_062();
extern int do_063();
extern int do_064();
extern int do_065();
extern int do_066();
extern int do_067();
extern int do_068();
extern int do_069();
extern int do_070();
extern int do_071();
extern int do_072();
extern int do_073();
extern int do_074();
extern int do_075();
extern int do_076();
extern int do_077();
extern int do_078();
extern int do_079();
extern int do_080();
extern int do_081();
extern int do_082();
extern int do_083();
extern int do_084();
extern int do_085();
extern int do_086();
extern int do_087();
extern int do_088();
extern int do_089();
extern int do_090();
extern int do_091();
extern int do_092();
extern int do_093();
extern int do_094();
extern int do_095();
extern int do_096();
extern int do_097();
extern int do_098();
extern int do_099();
extern int do_100();
extern int do_101();
extern int do_102();
extern int do_103();
extern int do_104();
extern int do_105();
extern int do_106();
extern int do_107();
extern int do_108();
extern int do_109();
extern int do_110();
extern int do_111();
extern int do_112();
extern int do_113();
extern int do_114();
extern int do_115();
extern int do_116();
extern int do_117();
extern int do_118();
extern int do_119();
extern int do_120();
extern int do_121();
extern int do_122();
extern int do_123();
extern int do_124();
extern int do_125();
extern int do_126();
extern int do_127();
extern int do_128();
extern int do_129();
extern int do_130();
extern int do_131();
extern int do_132();
extern int do_133();
extern int do_134();
extern int do_135();
extern int do_136();
extern int do_137();
extern int do_138();
extern int do_139();
extern int do_140();
extern int do_141();
extern int do_142();
extern int do_143();
extern int do_144();
extern int do_145();
extern int do_146();
extern int do_147();
extern int do_148();
extern int do_149();
extern int do_150();
extern int do_151();
extern int do_152();
extern int do_153();
extern int do_154();
extern int do_155();
extern int do_156();
extern int do_157();
extern int do_158();
extern int do_159();
extern int do_160();
extern int do_161();
extern int do_162();
extern int do_163();
extern int do_164();
extern int do_165();
extern int do_166();
extern int do_167();
extern int do_168();
extern int do_169();
extern int do_170();
extern int do_171();
extern int do_172();
extern int do_173();
extern int do_174();
extern int do_175();
extern int do_176();
extern int do_177();
extern int do_178();
extern int do_179();
extern int do_180();
extern int do_181();
extern int do_182();
extern int do_183();
extern int do_184();
extern int do_185();
extern int do_186();
extern int do_187();
extern int do_188();
extern int do_189();
extern int do_190();
extern int do_191();
extern int do_192();
extern int do_193();
extern int do_194();
extern int do_195();
extern int do_196();
extern int do_197();
extern int do_198();
extern int do_199();
extern int do_200();
extern int do_201();
extern int do_202();
extern int do_203();
extern int do_204();
extern int do_205();
extern int do_206();
extern int do_207();
extern int do_208();
extern int do_209();
extern int do_210();
extern int do_211();
extern int do_212();
extern int do_213();
extern int do_214();
extern int do_215();
extern int do_216();
extern int do_217();
extern int do_218();
extern int do_219();
extern int do_220();
extern int do_221();
extern int do_222();
extern int do_223();
extern int do_224();
extern int do_225();
extern int do_226();
extern int do_227();
extern int do_228();
extern int do_229();
extern int do_230();
extern int do_231();
extern int do_232();
extern int do_233();
extern int do_234();
extern int do_235();
extern int do_236();
extern int do_237();
extern int do_238();
extern int do_239();
extern int do_240();
extern int do_241();
extern int do_242();
extern int do_243();
extern int do_244();
extern int do_245();
extern int do_246();
extern int do_247();
extern int do_248();
extern int do_249();
extern int do_250();
extern int do_251();
extern int do_252();
extern int do_253();
extern int do_254();
extern int do_255();
extern int do_256();
extern int do_257();
extern int do_258();
extern int do_259();
extern int do_260();
extern int do_261();
extern int do_262();
extern int do_263();
extern int do_264();
extern int do_265();
extern int do_266();
extern int do_267();
extern int do_268();
extern int do_269();
extern int do_270();
extern int do_271();
extern int do_272();
extern int do_273();
extern int do_274();
extern int do_275();
extern int do_276();
extern int do_277();
extern int do_278();
extern int do_279();
extern int do_280();
extern int do_281();
extern int do_282();
extern int do_283();
extern int do_284();
extern int do_285();
extern int do_286();
extern int do_287();
extern int do_288();
extern int do_289();
extern int do_290();
extern int do_291();
extern int do_292();
extern int do_293();
extern int do_294();
extern int do_295();
extern int do_296();
extern int do_297();
extern int do_298();
extern int do_299();
extern int do_300();
extern int do_301();
extern int do_302();
extern int do_303();
extern int do_304();
extern int do_305();
extern int do_306();
extern int do_307();
extern int do_308();
extern int do_309();
extern int do_310();
extern int do_311();
extern int do_312();
extern int do_313();
extern int do_314();
extern int do_315();
extern int do_316();
extern int do_317();
extern int do_318();
extern int do_319();
extern int do_320();
extern int do_321();
extern int do_322();
extern int do_323();
extern int do_324();
extern int do_325();
extern int do_326();
extern int do_327();
extern int do_328();
extern int do_329();
extern int do_330();
extern int do_331();
extern int do_332();
extern int do_333();
extern int do_334();
extern int do_335();
extern int do_336();
extern int do_337();
extern int do_338();
extern int do_339();
extern int do_340();
extern int do_341();
extern int do_342();
extern int do_343();
extern int do_344();
extern int do_345();
extern int do_346();
extern int do_347();
extern int do_348();
extern int do_349();
extern int do_350();
extern int do_351();
extern int do_352();
extern int do_353();
extern int do_354();
extern int do_355();
extern int do_356();
extern int do_357();
extern int do_358();
extern int do_359();
extern int do_360();
extern int do_361();
extern int do_362();
extern int do_363();
extern int do_364();
extern int do_365();
extern int do_366();
extern int do_367();
extern int do_368();
extern int do_369();
extern int do_370();
extern int do_371();
extern int do_372();
extern int do_373();
extern int do_374();
extern int do_375();
extern int do_376();
extern int do_377();
extern int do_378();
extern int do_379();
extern int do_380();
extern int do_381();
extern int do_382();
extern int do_383();
extern int do_384();
extern int do_385();
extern int do_386();
extern int do_387();
extern int do_388();
extern int do_389();
extern int do_390();
extern int do_391();
extern int do_392();
extern int do_393();
extern int do_394();
extern int do_395();
extern int do_396();
extern int do_397();
extern int do_398();
extern int do_399();
extern int do_400();
extern int do_401();
extern int do_402();
extern int do_403();
extern int do_404();
extern int do_405();
extern int do_406();
extern int do_407();
extern int do_408();
extern int do_409();
extern int do_410();
extern int do_411();
extern int do_412();
extern int do_413();
extern int do_414();
extern int do_415();
extern int do_416();
extern int do_417();
extern int do_418();
extern int do_419();
extern int do_420();
extern int do_421();
extern int do_422();
extern int do_423();
extern int do_424();
extern int do_425();
extern int do_426();
extern int do_427();
extern int do_428();
extern int do_429();
extern int do_430();
extern int do_431();
extern int do_432();
extern int do_433();
extern int do_434();
extern int do_435();
extern int do_436();
extern int do_437();
extern int do_438();
extern int do_439();
extern int do_440();
extern int do_441();
extern int do_442();
extern int do_443();
extern int do_444();
extern int do_445();
extern int do_446();
extern int do_447();
extern int do_448();
extern int do_449();
extern int do_450();
extern int do_451();
extern int do_452();
extern int do_453();
extern int do_454();
extern int do_455();
extern int do_456();
extern int do_457();
extern int do_458();
extern int do_459();
extern int do_460();
extern int do_461();
extern int do_462();
extern int do_463();
extern int do_464();
extern int do_465();
extern int do_466();
extern int do_467();
extern int do_468();
extern int do_469();
extern int do_470();
extern int do_471();
extern int do_472();
extern int do_473();
extern int do_474();
extern int do_475();
extern int do_476();
extern int do_477();
extern int do_478();
extern int do_479();
extern int do_480();
extern int do_481();
extern int do_482();
extern int do_483();
extern int do_484();
extern int do_485();
extern int do_486();
extern int do_487();
extern int do_488();
extern int do_489();
extern int do_490();
extern int do_491();
extern int do_492();
extern int do_493();
extern int do_494();
extern int do_495();
extern int do_496();
extern int do_497();
extern int do_498();
extern int do_499();
extern int do_500();
extern int do_501();
extern int do_502();
extern int do_503();
extern int do_504();
extern int do_505();
extern int do_506();
extern int do_507();
extern int do_508();
extern int do_509();
extern int do_510();
extern int do_511();
extern int do_512();
extern int do_513();
extern int do_514();
extern int do_515();
extern int do_516();
extern int do_517();
extern int do_518();
extern int do_519();
extern int do_520();
extern int do_521();
extern int do_522();
extern int do_523();
extern int do_524();
extern int do_525();
extern int do_526();
extern int do_527();
extern int do_528();
extern int do_529();
extern int do_530();
extern int do_531();
extern int do_532();
extern int do_533();
extern int do_534();
extern int do_535();
extern int do_536();
extern int do_537();
extern int do_538();
extern int do_539();
extern int do_540();
extern int do_541();
extern int do_542();
extern int do_543();
extern int do_544();
extern int do_545();
extern int do_546();
extern int do_547();
extern int do_548();
extern int do_549();
extern int do_550();
extern int do_551();
extern int do_552();
extern int do_553();
extern int do_554();
extern int do_555();
extern int do_556();
extern int do_557();
extern int do_558();
extern int do_559();
extern int do_560();
extern int do_561();
extern int do_562();
extern int do_563();
extern int do_564();
extern int do_565();
extern int do_566();
extern int do_567();
extern int do_568();
extern int do_569();
extern int do_570();
extern int do_571();
extern int do_572();
extern int do_573();
extern int do_574();
extern int do_575();
extern int do_576();
extern int do_577();
extern int do_578();
extern int do_579();
extern int do_580();
extern int do_581();
extern int do_582();
extern int do_583();
extern int do_584();
extern int do_585();
extern int do_586();
extern int do_587();
extern int do_588();
extern int do_589();
extern int do_590();
extern int do_591();
extern int do_592();
extern int do_593();
extern int do_594();
extern int do_595();
extern int do_596();
extern int do_597();
extern int do_598();
extern int do_599();
extern int do_600();
extern int do_601();
extern int do_602();
extern int do_603();
extern int do_604();
extern int do_605();
extern int do_606();
extern int do_607();
extern int do_608();
extern int do_609();
extern int do_610();
extern int do_611();
extern int do_612();
extern int do_613();
extern int do_614();
extern int do_615();
extern int do_616();
extern int do_617();
extern int do_618();
extern int do_619();
extern int do_620();
extern int do_621();
extern int do_622();
extern int do_623();
extern int do_624();
extern int do_625();
extern int do_626();
extern int do_627();
extern int do_628();
extern int do_629();
extern int do_630();
extern int do_631();
extern int do_632();
extern int do_633();
extern int do_634();
extern int do_635();
extern int do_636();
extern int do_637();
extern int do_638();
extern int do_639();
extern int do_640();
extern int do_641();
extern int do_642();
extern int do_643();
extern int do_644();
extern int do_645();
extern int do_646();
extern int do_647();
extern int do_648();
extern int do_649();
extern int do_650();
extern int do_651();
extern int do_652();
extern int do_653();
extern int do_654();
extern int do_655();
extern int do_656();
extern int do_657();
extern int do_658();
extern int do_659();
extern int do_660();
extern int do_661();
extern int do_662();
extern int do_663();
extern int do_664();
extern int do_665();
extern int do_666();
extern int do_667();
extern int do_668();
extern int do_669();
extern int do_670();
extern int do_671();
extern int do_672();
extern int do_673();
extern int do_674();
extern int do_675();
extern int do_676();
extern int do_677();
extern int do_678();
extern int do_679();
extern int do_680();
extern int do_681();
extern int do_682();
extern int do_683();
extern int do_684();
extern int do_685();
extern int do_686();
extern int do_687();
extern int do_688();
extern int do_689();
extern int do_690();
extern int do_691();
extern int do_692();
extern int do_693();
extern int do_694();
extern int do_695();
extern int do_696();
extern int do_697();
extern int do_698();
extern int do_699();
extern int do_700();
extern int do_701();
extern int do_702();
extern int do_703();
extern int do_704();
extern int do_705();
extern int do_706();
extern int do_707();
extern int do_708();
extern int do_709();
extern int do_710();
extern int do_711();
extern int do_712();
extern int do_713();
extern int do_714();
extern int do_715();
extern int do_716();
extern int do_717();
extern int do_718();
extern int do_719();
extern int do_720();
extern int do_721();
extern int do_722();
extern int do_723();
extern int do_724();
extern int do_725();
extern int do_726();
extern int do_727();
extern int do_728();
extern int do_729();
extern int do_730();
extern int do_731();
extern int do_732();
extern int do_733();
extern int do_734();
extern int do_735();
extern int do_736();
extern int do_737();
extern int do_738();
extern int do_739();
extern int do_740();
extern int do_741();
extern int do_742();
extern int do_743();
extern int do_744();
extern int do_745();
extern int do_746();
extern int do_747();
extern int do_748();
extern int do_749();
extern int do_750();
extern int do_751();
extern int do_752();
extern int do_753();
extern int do_754();
extern int do_755();
extern int do_756();
extern int do_757();
extern int do_758();
extern int do_759();
extern int do_760();
extern int do_761();
extern int do_762();
extern int do_763();
extern int do_764();
extern int do_765();
extern int do_766();
extern int do_767();
extern int do_768();
extern int do_769();
extern int do_770();
extern int do_771();
extern int do_772();
extern int do_773();
extern int do_774();
extern int do_775();
extern int do_776();
extern int do_777();
extern int do_778();
extern int do_779();
extern int do_780();
extern int do_781();
extern int do_782();
extern int do_783();
extern int do_784();
extern int do_785();
extern int do_786();
extern int do_787();
extern int do_788();
extern int do_789();
extern int do_790();
extern int do_791();
extern int do_792();
extern int do_793();
extern int do_794();
extern int do_795();
extern int do_796();
extern int do_797();
extern int do_798();
extern int do_799();
extern int do_800();
extern int do_801();
extern int do_802();
extern int do_803();
extern int do_804();
extern int do_805();
extern int do_806();
extern int do_807();
extern int do_808();
extern int do_809();
extern int do_810();
extern int do_811();
extern int do_812();
extern int do_813();
extern int do_814();
extern int do_815();
extern int do_816();
extern int do_817();
extern int do_818();
extern int do_819();
extern int do_820();
extern int do_821();
extern int do_822();
extern int do_823();
extern int do_824();
extern int do_825();
extern int do_826();
extern int do_827();
extern int do_828();
extern int do_829();
extern int do_830();
extern int do_831();
extern int do_832();
extern int do_833();
extern int do_834();
extern int do_835();
extern int do_836();
extern int do_837();
extern int do_838();
extern int do_839();
extern int do_840();
extern int do_841();
extern int do_842();
extern int do_843();
extern int do_844();
extern int do_845();
extern int do_846();
extern int do_847();
extern int do_848();
extern int do_849();
extern int do_850();
extern int do_851();
extern int do_852();
extern int do_853();
extern int do_854();
extern int do_855();
extern int do_856();
extern int do_857();
extern int do_858();
extern int do_859();
extern int do_860();
extern int do_861();
extern int do_862();
extern int do_863();
extern int do_864();
extern int do_865();
extern int do_866();
extern int do_867();
extern int do_868();
extern int do_869();
extern int do_870();
extern int do_871();
extern int do_872();
extern int do_873();
extern int do_874();
extern int do_875();
extern int do_876();
extern int do_877();
extern int do_878();
extern int do_879();
extern int do_880();
extern int do_881();
extern int do_882();
extern int do_883();
extern int do_884();
extern int do_885();
extern int do_886();
extern int do_887();
extern int do_888();
extern int do_889();
extern int do_890();
extern int do_891();
extern int do_892();
extern int do_893();
extern int do_894();
extern int do_895();
extern int do_896();
extern int do_897();
extern int do_898();
extern int do_899();
extern int do_900();
extern int do_901();
extern int do_902();
extern int do_903();
extern int do_904();
extern int do_905();
extern int do_906();
extern int do_907();
extern int do_908();
extern int do_909();
extern int do_910();
extern int do_911();
extern int do_912();
extern int do_913();
extern int do_914();
extern int do_915();
extern int do_916();
extern int do_917();
extern int do_918();
extern int do_919();
extern int do_920();
extern int do_921();
extern int do_922();
extern int do_923();
extern int do_924();
extern int do_925();
extern int do_926();
extern int do_927();
extern int do_928();
extern int do_929();
extern int do_930();
extern int do_931();
extern int do_932();
extern int do_933();
extern int do_934();
extern int do_935();
extern int do_936();
extern int do_937();
extern int do_938();
extern int do_939();
extern int do_940();
extern int do_941();
extern int do_942();
extern int do_943();
extern int do_944();
extern int do_945();
extern int do_946();
extern int do_947();
extern int do_948();
extern int do_949();
extern int do_950();
extern int do_951();
extern int do_952();
extern int do_953();
extern int do_954();
extern int do_955();
extern int do_956();
extern int do_957();
extern int do_958();
extern int do_959();
extern int do_960();
extern int do_961();
extern int do_962();
extern int do_963();
extern int do_964();
extern int do_965();
extern int do_966();
extern int do_967();
extern int do_968();
extern int do_969();
extern int do_970();
extern int do_971();
extern int do_972();
extern int do_973();
extern int do_974();
extern int do_975();
extern int do_976();
extern int do_977();
extern int do_978();
extern int do_979();
extern int do_980();
extern int do_981();
extern int do_982();
extern int do_983();
extern int do_984();
extern int do_985();
extern int do_986();
extern int do_987();
extern int do_988();
extern int do_989();
extern int do_990();
extern int do_991();
extern int do_992();
extern int do_993();
extern int do_994();
extern int do_995();
extern int do_996();
extern int do_997();
extern int do_998();
extern int do_999();


static void* work(void* ignore)
{
	if ( do_000() != 0 ) { FAIL("iteration 0"); exit(0); }
	if ( do_001() != 1 ) { FAIL("iteration 1"); exit(0); }
	if ( do_002() != 2 ) { FAIL("iteration 2"); exit(0); }
	if ( do_003() != 3 ) { FAIL("iteration 3"); exit(0); }
	if ( do_004() != 4 ) { FAIL("iteration 4"); exit(0); }
	if ( do_005() != 5 ) { FAIL("iteration 5"); exit(0); }
	if ( do_006() != 6 ) { FAIL("iteration 6"); exit(0); }
	if ( do_007() != 7 ) { FAIL("iteration 7"); exit(0); }
	if ( do_008() != 8 ) { FAIL("iteration 8"); exit(0); }
	if ( do_009() != 9 ) { FAIL("iteration 9"); exit(0); }
	if ( do_010() != 10 ) { FAIL("iteration 10"); exit(0); }
	if ( do_011() != 11 ) { FAIL("iteration 11"); exit(0); }
	if ( do_012() != 12 ) { FAIL("iteration 12"); exit(0); }
	if ( do_013() != 13 ) { FAIL("iteration 13"); exit(0); }
	if ( do_014() != 14 ) { FAIL("iteration 14"); exit(0); }
	if ( do_015() != 15 ) { FAIL("iteration 15"); exit(0); }
	if ( do_016() != 16 ) { FAIL("iteration 16"); exit(0); }
	if ( do_017() != 17 ) { FAIL("iteration 17"); exit(0); }
	if ( do_018() != 18 ) { FAIL("iteration 18"); exit(0); }
	if ( do_019() != 19 ) { FAIL("iteration 19"); exit(0); }
	if ( do_020() != 20 ) { FAIL("iteration 20"); exit(0); }
	if ( do_021() != 21 ) { FAIL("iteration 21"); exit(0); }
	if ( do_022() != 22 ) { FAIL("iteration 22"); exit(0); }
	if ( do_023() != 23 ) { FAIL("iteration 23"); exit(0); }
	if ( do_024() != 24 ) { FAIL("iteration 24"); exit(0); }
	if ( do_025() != 25 ) { FAIL("iteration 25"); exit(0); }
	if ( do_026() != 26 ) { FAIL("iteration 26"); exit(0); }
	if ( do_027() != 27 ) { FAIL("iteration 27"); exit(0); }
	if ( do_028() != 28 ) { FAIL("iteration 28"); exit(0); }
	if ( do_029() != 29 ) { FAIL("iteration 29"); exit(0); }
	if ( do_030() != 30 ) { FAIL("iteration 30"); exit(0); }
	if ( do_031() != 31 ) { FAIL("iteration 31"); exit(0); }
	if ( do_032() != 32 ) { FAIL("iteration 32"); exit(0); }
	if ( do_033() != 33 ) { FAIL("iteration 33"); exit(0); }
	if ( do_034() != 34 ) { FAIL("iteration 34"); exit(0); }
	if ( do_035() != 35 ) { FAIL("iteration 35"); exit(0); }
	if ( do_036() != 36 ) { FAIL("iteration 36"); exit(0); }
	if ( do_037() != 37 ) { FAIL("iteration 37"); exit(0); }
	if ( do_038() != 38 ) { FAIL("iteration 38"); exit(0); }
	if ( do_039() != 39 ) { FAIL("iteration 39"); exit(0); }
	if ( do_040() != 40 ) { FAIL("iteration 40"); exit(0); }
	if ( do_041() != 41 ) { FAIL("iteration 41"); exit(0); }
	if ( do_042() != 42 ) { FAIL("iteration 42"); exit(0); }
	if ( do_043() != 43 ) { FAIL("iteration 43"); exit(0); }
	if ( do_044() != 44 ) { FAIL("iteration 44"); exit(0); }
	if ( do_045() != 45 ) { FAIL("iteration 45"); exit(0); }
	if ( do_046() != 46 ) { FAIL("iteration 46"); exit(0); }
	if ( do_047() != 47 ) { FAIL("iteration 47"); exit(0); }
	if ( do_048() != 48 ) { FAIL("iteration 48"); exit(0); }
	if ( do_049() != 49 ) { FAIL("iteration 49"); exit(0); }
	if ( do_050() != 50 ) { FAIL("iteration 50"); exit(0); }
	if ( do_051() != 51 ) { FAIL("iteration 51"); exit(0); }
	if ( do_052() != 52 ) { FAIL("iteration 52"); exit(0); }
	if ( do_053() != 53 ) { FAIL("iteration 53"); exit(0); }
	if ( do_054() != 54 ) { FAIL("iteration 54"); exit(0); }
	if ( do_055() != 55 ) { FAIL("iteration 55"); exit(0); }
	if ( do_056() != 56 ) { FAIL("iteration 56"); exit(0); }
	if ( do_057() != 57 ) { FAIL("iteration 57"); exit(0); }
	if ( do_058() != 58 ) { FAIL("iteration 58"); exit(0); }
	if ( do_059() != 59 ) { FAIL("iteration 59"); exit(0); }
	if ( do_060() != 60 ) { FAIL("iteration 60"); exit(0); }
	if ( do_061() != 61 ) { FAIL("iteration 61"); exit(0); }
	if ( do_062() != 62 ) { FAIL("iteration 62"); exit(0); }
	if ( do_063() != 63 ) { FAIL("iteration 63"); exit(0); }
	if ( do_064() != 64 ) { FAIL("iteration 64"); exit(0); }
	if ( do_065() != 65 ) { FAIL("iteration 65"); exit(0); }
	if ( do_066() != 66 ) { FAIL("iteration 66"); exit(0); }
	if ( do_067() != 67 ) { FAIL("iteration 67"); exit(0); }
	if ( do_068() != 68 ) { FAIL("iteration 68"); exit(0); }
	if ( do_069() != 69 ) { FAIL("iteration 69"); exit(0); }
	if ( do_070() != 70 ) { FAIL("iteration 70"); exit(0); }
	if ( do_071() != 71 ) { FAIL("iteration 71"); exit(0); }
	if ( do_072() != 72 ) { FAIL("iteration 72"); exit(0); }
	if ( do_073() != 73 ) { FAIL("iteration 73"); exit(0); }
	if ( do_074() != 74 ) { FAIL("iteration 74"); exit(0); }
	if ( do_075() != 75 ) { FAIL("iteration 75"); exit(0); }
	if ( do_076() != 76 ) { FAIL("iteration 76"); exit(0); }
	if ( do_077() != 77 ) { FAIL("iteration 77"); exit(0); }
	if ( do_078() != 78 ) { FAIL("iteration 78"); exit(0); }
	if ( do_079() != 79 ) { FAIL("iteration 79"); exit(0); }
	if ( do_080() != 80 ) { FAIL("iteration 80"); exit(0); }
	if ( do_081() != 81 ) { FAIL("iteration 81"); exit(0); }
	if ( do_082() != 82 ) { FAIL("iteration 82"); exit(0); }
	if ( do_083() != 83 ) { FAIL("iteration 83"); exit(0); }
	if ( do_084() != 84 ) { FAIL("iteration 84"); exit(0); }
	if ( do_085() != 85 ) { FAIL("iteration 85"); exit(0); }
	if ( do_086() != 86 ) { FAIL("iteration 86"); exit(0); }
	if ( do_087() != 87 ) { FAIL("iteration 87"); exit(0); }
	if ( do_088() != 88 ) { FAIL("iteration 88"); exit(0); }
	if ( do_089() != 89 ) { FAIL("iteration 89"); exit(0); }
	if ( do_090() != 90 ) { FAIL("iteration 90"); exit(0); }
	if ( do_091() != 91 ) { FAIL("iteration 91"); exit(0); }
	if ( do_092() != 92 ) { FAIL("iteration 92"); exit(0); }
	if ( do_093() != 93 ) { FAIL("iteration 93"); exit(0); }
	if ( do_094() != 94 ) { FAIL("iteration 94"); exit(0); }
	if ( do_095() != 95 ) { FAIL("iteration 95"); exit(0); }
	if ( do_096() != 96 ) { FAIL("iteration 96"); exit(0); }
	if ( do_097() != 97 ) { FAIL("iteration 97"); exit(0); }
	if ( do_098() != 98 ) { FAIL("iteration 98"); exit(0); }
	if ( do_099() != 99 ) { FAIL("iteration 99"); exit(0); }
	if ( do_100() != 100 ) { FAIL("iteration 100"); exit(0); }
	if ( do_101() != 101 ) { FAIL("iteration 101"); exit(0); }
	if ( do_102() != 102 ) { FAIL("iteration 102"); exit(0); }
	if ( do_103() != 103 ) { FAIL("iteration 103"); exit(0); }
	if ( do_104() != 104 ) { FAIL("iteration 104"); exit(0); }
	if ( do_105() != 105 ) { FAIL("iteration 105"); exit(0); }
	if ( do_106() != 106 ) { FAIL("iteration 106"); exit(0); }
	if ( do_107() != 107 ) { FAIL("iteration 107"); exit(0); }
	if ( do_108() != 108 ) { FAIL("iteration 108"); exit(0); }
	if ( do_109() != 109 ) { FAIL("iteration 109"); exit(0); }
	if ( do_110() != 110 ) { FAIL("iteration 110"); exit(0); }
	if ( do_111() != 111 ) { FAIL("iteration 111"); exit(0); }
	if ( do_112() != 112 ) { FAIL("iteration 112"); exit(0); }
	if ( do_113() != 113 ) { FAIL("iteration 113"); exit(0); }
	if ( do_114() != 114 ) { FAIL("iteration 114"); exit(0); }
	if ( do_115() != 115 ) { FAIL("iteration 115"); exit(0); }
	if ( do_116() != 116 ) { FAIL("iteration 116"); exit(0); }
	if ( do_117() != 117 ) { FAIL("iteration 117"); exit(0); }
	if ( do_118() != 118 ) { FAIL("iteration 118"); exit(0); }
	if ( do_119() != 119 ) { FAIL("iteration 119"); exit(0); }
	if ( do_120() != 120 ) { FAIL("iteration 120"); exit(0); }
	if ( do_121() != 121 ) { FAIL("iteration 121"); exit(0); }
	if ( do_122() != 122 ) { FAIL("iteration 122"); exit(0); }
	if ( do_123() != 123 ) { FAIL("iteration 123"); exit(0); }
	if ( do_124() != 124 ) { FAIL("iteration 124"); exit(0); }
	if ( do_125() != 125 ) { FAIL("iteration 125"); exit(0); }
	if ( do_126() != 126 ) { FAIL("iteration 126"); exit(0); }
	if ( do_127() != 127 ) { FAIL("iteration 127"); exit(0); }
	if ( do_128() != 128 ) { FAIL("iteration 128"); exit(0); }
	if ( do_129() != 129 ) { FAIL("iteration 129"); exit(0); }
	if ( do_130() != 130 ) { FAIL("iteration 130"); exit(0); }
	if ( do_131() != 131 ) { FAIL("iteration 131"); exit(0); }
	if ( do_132() != 132 ) { FAIL("iteration 132"); exit(0); }
	if ( do_133() != 133 ) { FAIL("iteration 133"); exit(0); }
	if ( do_134() != 134 ) { FAIL("iteration 134"); exit(0); }
	if ( do_135() != 135 ) { FAIL("iteration 135"); exit(0); }
	if ( do_136() != 136 ) { FAIL("iteration 136"); exit(0); }
	if ( do_137() != 137 ) { FAIL("iteration 137"); exit(0); }
	if ( do_138() != 138 ) { FAIL("iteration 138"); exit(0); }
	if ( do_139() != 139 ) { FAIL("iteration 139"); exit(0); }
	if ( do_140() != 140 ) { FAIL("iteration 140"); exit(0); }
	if ( do_141() != 141 ) { FAIL("iteration 141"); exit(0); }
	if ( do_142() != 142 ) { FAIL("iteration 142"); exit(0); }
	if ( do_143() != 143 ) { FAIL("iteration 143"); exit(0); }
	if ( do_144() != 144 ) { FAIL("iteration 144"); exit(0); }
	if ( do_145() != 145 ) { FAIL("iteration 145"); exit(0); }
	if ( do_146() != 146 ) { FAIL("iteration 146"); exit(0); }
	if ( do_147() != 147 ) { FAIL("iteration 147"); exit(0); }
	if ( do_148() != 148 ) { FAIL("iteration 148"); exit(0); }
	if ( do_149() != 149 ) { FAIL("iteration 149"); exit(0); }
	if ( do_150() != 150 ) { FAIL("iteration 150"); exit(0); }
	if ( do_151() != 151 ) { FAIL("iteration 151"); exit(0); }
	if ( do_152() != 152 ) { FAIL("iteration 152"); exit(0); }
	if ( do_153() != 153 ) { FAIL("iteration 153"); exit(0); }
	if ( do_154() != 154 ) { FAIL("iteration 154"); exit(0); }
	if ( do_155() != 155 ) { FAIL("iteration 155"); exit(0); }
	if ( do_156() != 156 ) { FAIL("iteration 156"); exit(0); }
	if ( do_157() != 157 ) { FAIL("iteration 157"); exit(0); }
	if ( do_158() != 158 ) { FAIL("iteration 158"); exit(0); }
	if ( do_159() != 159 ) { FAIL("iteration 159"); exit(0); }
	if ( do_160() != 160 ) { FAIL("iteration 160"); exit(0); }
	if ( do_161() != 161 ) { FAIL("iteration 161"); exit(0); }
	if ( do_162() != 162 ) { FAIL("iteration 162"); exit(0); }
	if ( do_163() != 163 ) { FAIL("iteration 163"); exit(0); }
	if ( do_164() != 164 ) { FAIL("iteration 164"); exit(0); }
	if ( do_165() != 165 ) { FAIL("iteration 165"); exit(0); }
	if ( do_166() != 166 ) { FAIL("iteration 166"); exit(0); }
	if ( do_167() != 167 ) { FAIL("iteration 167"); exit(0); }
	if ( do_168() != 168 ) { FAIL("iteration 168"); exit(0); }
	if ( do_169() != 169 ) { FAIL("iteration 169"); exit(0); }
	if ( do_170() != 170 ) { FAIL("iteration 170"); exit(0); }
	if ( do_171() != 171 ) { FAIL("iteration 171"); exit(0); }
	if ( do_172() != 172 ) { FAIL("iteration 172"); exit(0); }
	if ( do_173() != 173 ) { FAIL("iteration 173"); exit(0); }
	if ( do_174() != 174 ) { FAIL("iteration 174"); exit(0); }
	if ( do_175() != 175 ) { FAIL("iteration 175"); exit(0); }
	if ( do_176() != 176 ) { FAIL("iteration 176"); exit(0); }
	if ( do_177() != 177 ) { FAIL("iteration 177"); exit(0); }
	if ( do_178() != 178 ) { FAIL("iteration 178"); exit(0); }
	if ( do_179() != 179 ) { FAIL("iteration 179"); exit(0); }
	if ( do_180() != 180 ) { FAIL("iteration 180"); exit(0); }
	if ( do_181() != 181 ) { FAIL("iteration 181"); exit(0); }
	if ( do_182() != 182 ) { FAIL("iteration 182"); exit(0); }
	if ( do_183() != 183 ) { FAIL("iteration 183"); exit(0); }
	if ( do_184() != 184 ) { FAIL("iteration 184"); exit(0); }
	if ( do_185() != 185 ) { FAIL("iteration 185"); exit(0); }
	if ( do_186() != 186 ) { FAIL("iteration 186"); exit(0); }
	if ( do_187() != 187 ) { FAIL("iteration 187"); exit(0); }
	if ( do_188() != 188 ) { FAIL("iteration 188"); exit(0); }
	if ( do_189() != 189 ) { FAIL("iteration 189"); exit(0); }
	if ( do_190() != 190 ) { FAIL("iteration 190"); exit(0); }
	if ( do_191() != 191 ) { FAIL("iteration 191"); exit(0); }
	if ( do_192() != 192 ) { FAIL("iteration 192"); exit(0); }
	if ( do_193() != 193 ) { FAIL("iteration 193"); exit(0); }
	if ( do_194() != 194 ) { FAIL("iteration 194"); exit(0); }
	if ( do_195() != 195 ) { FAIL("iteration 195"); exit(0); }
	if ( do_196() != 196 ) { FAIL("iteration 196"); exit(0); }
	if ( do_197() != 197 ) { FAIL("iteration 197"); exit(0); }
	if ( do_198() != 198 ) { FAIL("iteration 198"); exit(0); }
	if ( do_199() != 199 ) { FAIL("iteration 199"); exit(0); }
	if ( do_200() != 200 ) { FAIL("iteration 200"); exit(0); }
	if ( do_201() != 201 ) { FAIL("iteration 201"); exit(0); }
	if ( do_202() != 202 ) { FAIL("iteration 202"); exit(0); }
	if ( do_203() != 203 ) { FAIL("iteration 203"); exit(0); }
	if ( do_204() != 204 ) { FAIL("iteration 204"); exit(0); }
	if ( do_205() != 205 ) { FAIL("iteration 205"); exit(0); }
	if ( do_206() != 206 ) { FAIL("iteration 206"); exit(0); }
	if ( do_207() != 207 ) { FAIL("iteration 207"); exit(0); }
	if ( do_208() != 208 ) { FAIL("iteration 208"); exit(0); }
	if ( do_209() != 209 ) { FAIL("iteration 209"); exit(0); }
	if ( do_210() != 210 ) { FAIL("iteration 210"); exit(0); }
	if ( do_211() != 211 ) { FAIL("iteration 211"); exit(0); }
	if ( do_212() != 212 ) { FAIL("iteration 212"); exit(0); }
	if ( do_213() != 213 ) { FAIL("iteration 213"); exit(0); }
	if ( do_214() != 214 ) { FAIL("iteration 214"); exit(0); }
	if ( do_215() != 215 ) { FAIL("iteration 215"); exit(0); }
	if ( do_216() != 216 ) { FAIL("iteration 216"); exit(0); }
	if ( do_217() != 217 ) { FAIL("iteration 217"); exit(0); }
	if ( do_218() != 218 ) { FAIL("iteration 218"); exit(0); }
	if ( do_219() != 219 ) { FAIL("iteration 219"); exit(0); }
	if ( do_220() != 220 ) { FAIL("iteration 220"); exit(0); }
	if ( do_221() != 221 ) { FAIL("iteration 221"); exit(0); }
	if ( do_222() != 222 ) { FAIL("iteration 222"); exit(0); }
	if ( do_223() != 223 ) { FAIL("iteration 223"); exit(0); }
	if ( do_224() != 224 ) { FAIL("iteration 224"); exit(0); }
	if ( do_225() != 225 ) { FAIL("iteration 225"); exit(0); }
	if ( do_226() != 226 ) { FAIL("iteration 226"); exit(0); }
	if ( do_227() != 227 ) { FAIL("iteration 227"); exit(0); }
	if ( do_228() != 228 ) { FAIL("iteration 228"); exit(0); }
	if ( do_229() != 229 ) { FAIL("iteration 229"); exit(0); }
	if ( do_230() != 230 ) { FAIL("iteration 230"); exit(0); }
	if ( do_231() != 231 ) { FAIL("iteration 231"); exit(0); }
	if ( do_232() != 232 ) { FAIL("iteration 232"); exit(0); }
	if ( do_233() != 233 ) { FAIL("iteration 233"); exit(0); }
	if ( do_234() != 234 ) { FAIL("iteration 234"); exit(0); }
	if ( do_235() != 235 ) { FAIL("iteration 235"); exit(0); }
	if ( do_236() != 236 ) { FAIL("iteration 236"); exit(0); }
	if ( do_237() != 237 ) { FAIL("iteration 237"); exit(0); }
	if ( do_238() != 238 ) { FAIL("iteration 238"); exit(0); }
	if ( do_239() != 239 ) { FAIL("iteration 239"); exit(0); }
	if ( do_240() != 240 ) { FAIL("iteration 240"); exit(0); }
	if ( do_241() != 241 ) { FAIL("iteration 241"); exit(0); }
	if ( do_242() != 242 ) { FAIL("iteration 242"); exit(0); }
	if ( do_243() != 243 ) { FAIL("iteration 243"); exit(0); }
	if ( do_244() != 244 ) { FAIL("iteration 244"); exit(0); }
	if ( do_245() != 245 ) { FAIL("iteration 245"); exit(0); }
	if ( do_246() != 246 ) { FAIL("iteration 246"); exit(0); }
	if ( do_247() != 247 ) { FAIL("iteration 247"); exit(0); }
	if ( do_248() != 248 ) { FAIL("iteration 248"); exit(0); }
	if ( do_249() != 249 ) { FAIL("iteration 249"); exit(0); }
	if ( do_250() != 250 ) { FAIL("iteration 250"); exit(0); }
	if ( do_251() != 251 ) { FAIL("iteration 251"); exit(0); }
	if ( do_252() != 252 ) { FAIL("iteration 252"); exit(0); }
	if ( do_253() != 253 ) { FAIL("iteration 253"); exit(0); }
	if ( do_254() != 254 ) { FAIL("iteration 254"); exit(0); }
	if ( do_255() != 255 ) { FAIL("iteration 255"); exit(0); }
	if ( do_256() != 256 ) { FAIL("iteration 256"); exit(0); }
	if ( do_257() != 257 ) { FAIL("iteration 257"); exit(0); }
	if ( do_258() != 258 ) { FAIL("iteration 258"); exit(0); }
	if ( do_259() != 259 ) { FAIL("iteration 259"); exit(0); }
	if ( do_260() != 260 ) { FAIL("iteration 260"); exit(0); }
	if ( do_261() != 261 ) { FAIL("iteration 261"); exit(0); }
	if ( do_262() != 262 ) { FAIL("iteration 262"); exit(0); }
	if ( do_263() != 263 ) { FAIL("iteration 263"); exit(0); }
	if ( do_264() != 264 ) { FAIL("iteration 264"); exit(0); }
	if ( do_265() != 265 ) { FAIL("iteration 265"); exit(0); }
	if ( do_266() != 266 ) { FAIL("iteration 266"); exit(0); }
	if ( do_267() != 267 ) { FAIL("iteration 267"); exit(0); }
	if ( do_268() != 268 ) { FAIL("iteration 268"); exit(0); }
	if ( do_269() != 269 ) { FAIL("iteration 269"); exit(0); }
	if ( do_270() != 270 ) { FAIL("iteration 270"); exit(0); }
	if ( do_271() != 271 ) { FAIL("iteration 271"); exit(0); }
	if ( do_272() != 272 ) { FAIL("iteration 272"); exit(0); }
	if ( do_273() != 273 ) { FAIL("iteration 273"); exit(0); }
	if ( do_274() != 274 ) { FAIL("iteration 274"); exit(0); }
	if ( do_275() != 275 ) { FAIL("iteration 275"); exit(0); }
	if ( do_276() != 276 ) { FAIL("iteration 276"); exit(0); }
	if ( do_277() != 277 ) { FAIL("iteration 277"); exit(0); }
	if ( do_278() != 278 ) { FAIL("iteration 278"); exit(0); }
	if ( do_279() != 279 ) { FAIL("iteration 279"); exit(0); }
	if ( do_280() != 280 ) { FAIL("iteration 280"); exit(0); }
	if ( do_281() != 281 ) { FAIL("iteration 281"); exit(0); }
	if ( do_282() != 282 ) { FAIL("iteration 282"); exit(0); }
	if ( do_283() != 283 ) { FAIL("iteration 283"); exit(0); }
	if ( do_284() != 284 ) { FAIL("iteration 284"); exit(0); }
	if ( do_285() != 285 ) { FAIL("iteration 285"); exit(0); }
	if ( do_286() != 286 ) { FAIL("iteration 286"); exit(0); }
	if ( do_287() != 287 ) { FAIL("iteration 287"); exit(0); }
	if ( do_288() != 288 ) { FAIL("iteration 288"); exit(0); }
	if ( do_289() != 289 ) { FAIL("iteration 289"); exit(0); }
	if ( do_290() != 290 ) { FAIL("iteration 290"); exit(0); }
	if ( do_291() != 291 ) { FAIL("iteration 291"); exit(0); }
	if ( do_292() != 292 ) { FAIL("iteration 292"); exit(0); }
	if ( do_293() != 293 ) { FAIL("iteration 293"); exit(0); }
	if ( do_294() != 294 ) { FAIL("iteration 294"); exit(0); }
	if ( do_295() != 295 ) { FAIL("iteration 295"); exit(0); }
	if ( do_296() != 296 ) { FAIL("iteration 296"); exit(0); }
	if ( do_297() != 297 ) { FAIL("iteration 297"); exit(0); }
	if ( do_298() != 298 ) { FAIL("iteration 298"); exit(0); }
	if ( do_299() != 299 ) { FAIL("iteration 299"); exit(0); }
	if ( do_300() != 300 ) { FAIL("iteration 300"); exit(0); }
	if ( do_301() != 301 ) { FAIL("iteration 301"); exit(0); }
	if ( do_302() != 302 ) { FAIL("iteration 302"); exit(0); }
	if ( do_303() != 303 ) { FAIL("iteration 303"); exit(0); }
	if ( do_304() != 304 ) { FAIL("iteration 304"); exit(0); }
	if ( do_305() != 305 ) { FAIL("iteration 305"); exit(0); }
	if ( do_306() != 306 ) { FAIL("iteration 306"); exit(0); }
	if ( do_307() != 307 ) { FAIL("iteration 307"); exit(0); }
	if ( do_308() != 308 ) { FAIL("iteration 308"); exit(0); }
	if ( do_309() != 309 ) { FAIL("iteration 309"); exit(0); }
	if ( do_310() != 310 ) { FAIL("iteration 310"); exit(0); }
	if ( do_311() != 311 ) { FAIL("iteration 311"); exit(0); }
	if ( do_312() != 312 ) { FAIL("iteration 312"); exit(0); }
	if ( do_313() != 313 ) { FAIL("iteration 313"); exit(0); }
	if ( do_314() != 314 ) { FAIL("iteration 314"); exit(0); }
	if ( do_315() != 315 ) { FAIL("iteration 315"); exit(0); }
	if ( do_316() != 316 ) { FAIL("iteration 316"); exit(0); }
	if ( do_317() != 317 ) { FAIL("iteration 317"); exit(0); }
	if ( do_318() != 318 ) { FAIL("iteration 318"); exit(0); }
	if ( do_319() != 319 ) { FAIL("iteration 319"); exit(0); }
	if ( do_320() != 320 ) { FAIL("iteration 320"); exit(0); }
	if ( do_321() != 321 ) { FAIL("iteration 321"); exit(0); }
	if ( do_322() != 322 ) { FAIL("iteration 322"); exit(0); }
	if ( do_323() != 323 ) { FAIL("iteration 323"); exit(0); }
	if ( do_324() != 324 ) { FAIL("iteration 324"); exit(0); }
	if ( do_325() != 325 ) { FAIL("iteration 325"); exit(0); }
	if ( do_326() != 326 ) { FAIL("iteration 326"); exit(0); }
	if ( do_327() != 327 ) { FAIL("iteration 327"); exit(0); }
	if ( do_328() != 328 ) { FAIL("iteration 328"); exit(0); }
	if ( do_329() != 329 ) { FAIL("iteration 329"); exit(0); }
	if ( do_330() != 330 ) { FAIL("iteration 330"); exit(0); }
	if ( do_331() != 331 ) { FAIL("iteration 331"); exit(0); }
	if ( do_332() != 332 ) { FAIL("iteration 332"); exit(0); }
	if ( do_333() != 333 ) { FAIL("iteration 333"); exit(0); }
	if ( do_334() != 334 ) { FAIL("iteration 334"); exit(0); }
	if ( do_335() != 335 ) { FAIL("iteration 335"); exit(0); }
	if ( do_336() != 336 ) { FAIL("iteration 336"); exit(0); }
	if ( do_337() != 337 ) { FAIL("iteration 337"); exit(0); }
	if ( do_338() != 338 ) { FAIL("iteration 338"); exit(0); }
	if ( do_339() != 339 ) { FAIL("iteration 339"); exit(0); }
	if ( do_340() != 340 ) { FAIL("iteration 340"); exit(0); }
	if ( do_341() != 341 ) { FAIL("iteration 341"); exit(0); }
	if ( do_342() != 342 ) { FAIL("iteration 342"); exit(0); }
	if ( do_343() != 343 ) { FAIL("iteration 343"); exit(0); }
	if ( do_344() != 344 ) { FAIL("iteration 344"); exit(0); }
	if ( do_345() != 345 ) { FAIL("iteration 345"); exit(0); }
	if ( do_346() != 346 ) { FAIL("iteration 346"); exit(0); }
	if ( do_347() != 347 ) { FAIL("iteration 347"); exit(0); }
	if ( do_348() != 348 ) { FAIL("iteration 348"); exit(0); }
	if ( do_349() != 349 ) { FAIL("iteration 349"); exit(0); }
	if ( do_350() != 350 ) { FAIL("iteration 350"); exit(0); }
	if ( do_351() != 351 ) { FAIL("iteration 351"); exit(0); }
	if ( do_352() != 352 ) { FAIL("iteration 352"); exit(0); }
	if ( do_353() != 353 ) { FAIL("iteration 353"); exit(0); }
	if ( do_354() != 354 ) { FAIL("iteration 354"); exit(0); }
	if ( do_355() != 355 ) { FAIL("iteration 355"); exit(0); }
	if ( do_356() != 356 ) { FAIL("iteration 356"); exit(0); }
	if ( do_357() != 357 ) { FAIL("iteration 357"); exit(0); }
	if ( do_358() != 358 ) { FAIL("iteration 358"); exit(0); }
	if ( do_359() != 359 ) { FAIL("iteration 359"); exit(0); }
	if ( do_360() != 360 ) { FAIL("iteration 360"); exit(0); }
	if ( do_361() != 361 ) { FAIL("iteration 361"); exit(0); }
	if ( do_362() != 362 ) { FAIL("iteration 362"); exit(0); }
	if ( do_363() != 363 ) { FAIL("iteration 363"); exit(0); }
	if ( do_364() != 364 ) { FAIL("iteration 364"); exit(0); }
	if ( do_365() != 365 ) { FAIL("iteration 365"); exit(0); }
	if ( do_366() != 366 ) { FAIL("iteration 366"); exit(0); }
	if ( do_367() != 367 ) { FAIL("iteration 367"); exit(0); }
	if ( do_368() != 368 ) { FAIL("iteration 368"); exit(0); }
	if ( do_369() != 369 ) { FAIL("iteration 369"); exit(0); }
	if ( do_370() != 370 ) { FAIL("iteration 370"); exit(0); }
	if ( do_371() != 371 ) { FAIL("iteration 371"); exit(0); }
	if ( do_372() != 372 ) { FAIL("iteration 372"); exit(0); }
	if ( do_373() != 373 ) { FAIL("iteration 373"); exit(0); }
	if ( do_374() != 374 ) { FAIL("iteration 374"); exit(0); }
	if ( do_375() != 375 ) { FAIL("iteration 375"); exit(0); }
	if ( do_376() != 376 ) { FAIL("iteration 376"); exit(0); }
	if ( do_377() != 377 ) { FAIL("iteration 377"); exit(0); }
	if ( do_378() != 378 ) { FAIL("iteration 378"); exit(0); }
	if ( do_379() != 379 ) { FAIL("iteration 379"); exit(0); }
	if ( do_380() != 380 ) { FAIL("iteration 380"); exit(0); }
	if ( do_381() != 381 ) { FAIL("iteration 381"); exit(0); }
	if ( do_382() != 382 ) { FAIL("iteration 382"); exit(0); }
	if ( do_383() != 383 ) { FAIL("iteration 383"); exit(0); }
	if ( do_384() != 384 ) { FAIL("iteration 384"); exit(0); }
	if ( do_385() != 385 ) { FAIL("iteration 385"); exit(0); }
	if ( do_386() != 386 ) { FAIL("iteration 386"); exit(0); }
	if ( do_387() != 387 ) { FAIL("iteration 387"); exit(0); }
	if ( do_388() != 388 ) { FAIL("iteration 388"); exit(0); }
	if ( do_389() != 389 ) { FAIL("iteration 389"); exit(0); }
	if ( do_390() != 390 ) { FAIL("iteration 390"); exit(0); }
	if ( do_391() != 391 ) { FAIL("iteration 391"); exit(0); }
	if ( do_392() != 392 ) { FAIL("iteration 392"); exit(0); }
	if ( do_393() != 393 ) { FAIL("iteration 393"); exit(0); }
	if ( do_394() != 394 ) { FAIL("iteration 394"); exit(0); }
	if ( do_395() != 395 ) { FAIL("iteration 395"); exit(0); }
	if ( do_396() != 396 ) { FAIL("iteration 396"); exit(0); }
	if ( do_397() != 397 ) { FAIL("iteration 397"); exit(0); }
	if ( do_398() != 398 ) { FAIL("iteration 398"); exit(0); }
	if ( do_399() != 399 ) { FAIL("iteration 399"); exit(0); }
	if ( do_400() != 400 ) { FAIL("iteration 400"); exit(0); }
	if ( do_401() != 401 ) { FAIL("iteration 401"); exit(0); }
	if ( do_402() != 402 ) { FAIL("iteration 402"); exit(0); }
	if ( do_403() != 403 ) { FAIL("iteration 403"); exit(0); }
	if ( do_404() != 404 ) { FAIL("iteration 404"); exit(0); }
	if ( do_405() != 405 ) { FAIL("iteration 405"); exit(0); }
	if ( do_406() != 406 ) { FAIL("iteration 406"); exit(0); }
	if ( do_407() != 407 ) { FAIL("iteration 407"); exit(0); }
	if ( do_408() != 408 ) { FAIL("iteration 408"); exit(0); }
	if ( do_409() != 409 ) { FAIL("iteration 409"); exit(0); }
	if ( do_410() != 410 ) { FAIL("iteration 410"); exit(0); }
	if ( do_411() != 411 ) { FAIL("iteration 411"); exit(0); }
	if ( do_412() != 412 ) { FAIL("iteration 412"); exit(0); }
	if ( do_413() != 413 ) { FAIL("iteration 413"); exit(0); }
	if ( do_414() != 414 ) { FAIL("iteration 414"); exit(0); }
	if ( do_415() != 415 ) { FAIL("iteration 415"); exit(0); }
	if ( do_416() != 416 ) { FAIL("iteration 416"); exit(0); }
	if ( do_417() != 417 ) { FAIL("iteration 417"); exit(0); }
	if ( do_418() != 418 ) { FAIL("iteration 418"); exit(0); }
	if ( do_419() != 419 ) { FAIL("iteration 419"); exit(0); }
	if ( do_420() != 420 ) { FAIL("iteration 420"); exit(0); }
	if ( do_421() != 421 ) { FAIL("iteration 421"); exit(0); }
	if ( do_422() != 422 ) { FAIL("iteration 422"); exit(0); }
	if ( do_423() != 423 ) { FAIL("iteration 423"); exit(0); }
	if ( do_424() != 424 ) { FAIL("iteration 424"); exit(0); }
	if ( do_425() != 425 ) { FAIL("iteration 425"); exit(0); }
	if ( do_426() != 426 ) { FAIL("iteration 426"); exit(0); }
	if ( do_427() != 427 ) { FAIL("iteration 427"); exit(0); }
	if ( do_428() != 428 ) { FAIL("iteration 428"); exit(0); }
	if ( do_429() != 429 ) { FAIL("iteration 429"); exit(0); }
	if ( do_430() != 430 ) { FAIL("iteration 430"); exit(0); }
	if ( do_431() != 431 ) { FAIL("iteration 431"); exit(0); }
	if ( do_432() != 432 ) { FAIL("iteration 432"); exit(0); }
	if ( do_433() != 433 ) { FAIL("iteration 433"); exit(0); }
	if ( do_434() != 434 ) { FAIL("iteration 434"); exit(0); }
	if ( do_435() != 435 ) { FAIL("iteration 435"); exit(0); }
	if ( do_436() != 436 ) { FAIL("iteration 436"); exit(0); }
	if ( do_437() != 437 ) { FAIL("iteration 437"); exit(0); }
	if ( do_438() != 438 ) { FAIL("iteration 438"); exit(0); }
	if ( do_439() != 439 ) { FAIL("iteration 439"); exit(0); }
	if ( do_440() != 440 ) { FAIL("iteration 440"); exit(0); }
	if ( do_441() != 441 ) { FAIL("iteration 441"); exit(0); }
	if ( do_442() != 442 ) { FAIL("iteration 442"); exit(0); }
	if ( do_443() != 443 ) { FAIL("iteration 443"); exit(0); }
	if ( do_444() != 444 ) { FAIL("iteration 444"); exit(0); }
	if ( do_445() != 445 ) { FAIL("iteration 445"); exit(0); }
	if ( do_446() != 446 ) { FAIL("iteration 446"); exit(0); }
	if ( do_447() != 447 ) { FAIL("iteration 447"); exit(0); }
	if ( do_448() != 448 ) { FAIL("iteration 448"); exit(0); }
	if ( do_449() != 449 ) { FAIL("iteration 449"); exit(0); }
	if ( do_450() != 450 ) { FAIL("iteration 450"); exit(0); }
	if ( do_451() != 451 ) { FAIL("iteration 451"); exit(0); }
	if ( do_452() != 452 ) { FAIL("iteration 452"); exit(0); }
	if ( do_453() != 453 ) { FAIL("iteration 453"); exit(0); }
	if ( do_454() != 454 ) { FAIL("iteration 454"); exit(0); }
	if ( do_455() != 455 ) { FAIL("iteration 455"); exit(0); }
	if ( do_456() != 456 ) { FAIL("iteration 456"); exit(0); }
	if ( do_457() != 457 ) { FAIL("iteration 457"); exit(0); }
	if ( do_458() != 458 ) { FAIL("iteration 458"); exit(0); }
	if ( do_459() != 459 ) { FAIL("iteration 459"); exit(0); }
	if ( do_460() != 460 ) { FAIL("iteration 460"); exit(0); }
	if ( do_461() != 461 ) { FAIL("iteration 461"); exit(0); }
	if ( do_462() != 462 ) { FAIL("iteration 462"); exit(0); }
	if ( do_463() != 463 ) { FAIL("iteration 463"); exit(0); }
	if ( do_464() != 464 ) { FAIL("iteration 464"); exit(0); }
	if ( do_465() != 465 ) { FAIL("iteration 465"); exit(0); }
	if ( do_466() != 466 ) { FAIL("iteration 466"); exit(0); }
	if ( do_467() != 467 ) { FAIL("iteration 467"); exit(0); }
	if ( do_468() != 468 ) { FAIL("iteration 468"); exit(0); }
	if ( do_469() != 469 ) { FAIL("iteration 469"); exit(0); }
	if ( do_470() != 470 ) { FAIL("iteration 470"); exit(0); }
	if ( do_471() != 471 ) { FAIL("iteration 471"); exit(0); }
	if ( do_472() != 472 ) { FAIL("iteration 472"); exit(0); }
	if ( do_473() != 473 ) { FAIL("iteration 473"); exit(0); }
	if ( do_474() != 474 ) { FAIL("iteration 474"); exit(0); }
	if ( do_475() != 475 ) { FAIL("iteration 475"); exit(0); }
	if ( do_476() != 476 ) { FAIL("iteration 476"); exit(0); }
	if ( do_477() != 477 ) { FAIL("iteration 477"); exit(0); }
	if ( do_478() != 478 ) { FAIL("iteration 478"); exit(0); }
	if ( do_479() != 479 ) { FAIL("iteration 479"); exit(0); }
	if ( do_480() != 480 ) { FAIL("iteration 480"); exit(0); }
	if ( do_481() != 481 ) { FAIL("iteration 481"); exit(0); }
	if ( do_482() != 482 ) { FAIL("iteration 482"); exit(0); }
	if ( do_483() != 483 ) { FAIL("iteration 483"); exit(0); }
	if ( do_484() != 484 ) { FAIL("iteration 484"); exit(0); }
	if ( do_485() != 485 ) { FAIL("iteration 485"); exit(0); }
	if ( do_486() != 486 ) { FAIL("iteration 486"); exit(0); }
	if ( do_487() != 487 ) { FAIL("iteration 487"); exit(0); }
	if ( do_488() != 488 ) { FAIL("iteration 488"); exit(0); }
	if ( do_489() != 489 ) { FAIL("iteration 489"); exit(0); }
	if ( do_490() != 490 ) { FAIL("iteration 490"); exit(0); }
	if ( do_491() != 491 ) { FAIL("iteration 491"); exit(0); }
	if ( do_492() != 492 ) { FAIL("iteration 492"); exit(0); }
	if ( do_493() != 493 ) { FAIL("iteration 493"); exit(0); }
	if ( do_494() != 494 ) { FAIL("iteration 494"); exit(0); }
	if ( do_495() != 495 ) { FAIL("iteration 495"); exit(0); }
	if ( do_496() != 496 ) { FAIL("iteration 496"); exit(0); }
	if ( do_497() != 497 ) { FAIL("iteration 497"); exit(0); }
	if ( do_498() != 498 ) { FAIL("iteration 498"); exit(0); }
	if ( do_499() != 499 ) { FAIL("iteration 499"); exit(0); }
	if ( do_500() != 500 ) { FAIL("iteration 500"); exit(0); }
	if ( do_501() != 501 ) { FAIL("iteration 501"); exit(0); }
	if ( do_502() != 502 ) { FAIL("iteration 502"); exit(0); }
	if ( do_503() != 503 ) { FAIL("iteration 503"); exit(0); }
	if ( do_504() != 504 ) { FAIL("iteration 504"); exit(0); }
	if ( do_505() != 505 ) { FAIL("iteration 505"); exit(0); }
	if ( do_506() != 506 ) { FAIL("iteration 506"); exit(0); }
	if ( do_507() != 507 ) { FAIL("iteration 507"); exit(0); }
	if ( do_508() != 508 ) { FAIL("iteration 508"); exit(0); }
	if ( do_509() != 509 ) { FAIL("iteration 509"); exit(0); }
	if ( do_510() != 510 ) { FAIL("iteration 510"); exit(0); }
	if ( do_511() != 511 ) { FAIL("iteration 511"); exit(0); }
	if ( do_512() != 512 ) { FAIL("iteration 512"); exit(0); }
	if ( do_513() != 513 ) { FAIL("iteration 513"); exit(0); }
	if ( do_514() != 514 ) { FAIL("iteration 514"); exit(0); }
	if ( do_515() != 515 ) { FAIL("iteration 515"); exit(0); }
	if ( do_516() != 516 ) { FAIL("iteration 516"); exit(0); }
	if ( do_517() != 517 ) { FAIL("iteration 517"); exit(0); }
	if ( do_518() != 518 ) { FAIL("iteration 518"); exit(0); }
	if ( do_519() != 519 ) { FAIL("iteration 519"); exit(0); }
	if ( do_520() != 520 ) { FAIL("iteration 520"); exit(0); }
	if ( do_521() != 521 ) { FAIL("iteration 521"); exit(0); }
	if ( do_522() != 522 ) { FAIL("iteration 522"); exit(0); }
	if ( do_523() != 523 ) { FAIL("iteration 523"); exit(0); }
	if ( do_524() != 524 ) { FAIL("iteration 524"); exit(0); }
	if ( do_525() != 525 ) { FAIL("iteration 525"); exit(0); }
	if ( do_526() != 526 ) { FAIL("iteration 526"); exit(0); }
	if ( do_527() != 527 ) { FAIL("iteration 527"); exit(0); }
	if ( do_528() != 528 ) { FAIL("iteration 528"); exit(0); }
	if ( do_529() != 529 ) { FAIL("iteration 529"); exit(0); }
	if ( do_530() != 530 ) { FAIL("iteration 530"); exit(0); }
	if ( do_531() != 531 ) { FAIL("iteration 531"); exit(0); }
	if ( do_532() != 532 ) { FAIL("iteration 532"); exit(0); }
	if ( do_533() != 533 ) { FAIL("iteration 533"); exit(0); }
	if ( do_534() != 534 ) { FAIL("iteration 534"); exit(0); }
	if ( do_535() != 535 ) { FAIL("iteration 535"); exit(0); }
	if ( do_536() != 536 ) { FAIL("iteration 536"); exit(0); }
	if ( do_537() != 537 ) { FAIL("iteration 537"); exit(0); }
	if ( do_538() != 538 ) { FAIL("iteration 538"); exit(0); }
	if ( do_539() != 539 ) { FAIL("iteration 539"); exit(0); }
	if ( do_540() != 540 ) { FAIL("iteration 540"); exit(0); }
	if ( do_541() != 541 ) { FAIL("iteration 541"); exit(0); }
	if ( do_542() != 542 ) { FAIL("iteration 542"); exit(0); }
	if ( do_543() != 543 ) { FAIL("iteration 543"); exit(0); }
	if ( do_544() != 544 ) { FAIL("iteration 544"); exit(0); }
	if ( do_545() != 545 ) { FAIL("iteration 545"); exit(0); }
	if ( do_546() != 546 ) { FAIL("iteration 546"); exit(0); }
	if ( do_547() != 547 ) { FAIL("iteration 547"); exit(0); }
	if ( do_548() != 548 ) { FAIL("iteration 548"); exit(0); }
	if ( do_549() != 549 ) { FAIL("iteration 549"); exit(0); }
	if ( do_550() != 550 ) { FAIL("iteration 550"); exit(0); }
	if ( do_551() != 551 ) { FAIL("iteration 551"); exit(0); }
	if ( do_552() != 552 ) { FAIL("iteration 552"); exit(0); }
	if ( do_553() != 553 ) { FAIL("iteration 553"); exit(0); }
	if ( do_554() != 554 ) { FAIL("iteration 554"); exit(0); }
	if ( do_555() != 555 ) { FAIL("iteration 555"); exit(0); }
	if ( do_556() != 556 ) { FAIL("iteration 556"); exit(0); }
	if ( do_557() != 557 ) { FAIL("iteration 557"); exit(0); }
	if ( do_558() != 558 ) { FAIL("iteration 558"); exit(0); }
	if ( do_559() != 559 ) { FAIL("iteration 559"); exit(0); }
	if ( do_560() != 560 ) { FAIL("iteration 560"); exit(0); }
	if ( do_561() != 561 ) { FAIL("iteration 561"); exit(0); }
	if ( do_562() != 562 ) { FAIL("iteration 562"); exit(0); }
	if ( do_563() != 563 ) { FAIL("iteration 563"); exit(0); }
	if ( do_564() != 564 ) { FAIL("iteration 564"); exit(0); }
	if ( do_565() != 565 ) { FAIL("iteration 565"); exit(0); }
	if ( do_566() != 566 ) { FAIL("iteration 566"); exit(0); }
	if ( do_567() != 567 ) { FAIL("iteration 567"); exit(0); }
	if ( do_568() != 568 ) { FAIL("iteration 568"); exit(0); }
	if ( do_569() != 569 ) { FAIL("iteration 569"); exit(0); }
	if ( do_570() != 570 ) { FAIL("iteration 570"); exit(0); }
	if ( do_571() != 571 ) { FAIL("iteration 571"); exit(0); }
	if ( do_572() != 572 ) { FAIL("iteration 572"); exit(0); }
	if ( do_573() != 573 ) { FAIL("iteration 573"); exit(0); }
	if ( do_574() != 574 ) { FAIL("iteration 574"); exit(0); }
	if ( do_575() != 575 ) { FAIL("iteration 575"); exit(0); }
	if ( do_576() != 576 ) { FAIL("iteration 576"); exit(0); }
	if ( do_577() != 577 ) { FAIL("iteration 577"); exit(0); }
	if ( do_578() != 578 ) { FAIL("iteration 578"); exit(0); }
	if ( do_579() != 579 ) { FAIL("iteration 579"); exit(0); }
	if ( do_580() != 580 ) { FAIL("iteration 580"); exit(0); }
	if ( do_581() != 581 ) { FAIL("iteration 581"); exit(0); }
	if ( do_582() != 582 ) { FAIL("iteration 582"); exit(0); }
	if ( do_583() != 583 ) { FAIL("iteration 583"); exit(0); }
	if ( do_584() != 584 ) { FAIL("iteration 584"); exit(0); }
	if ( do_585() != 585 ) { FAIL("iteration 585"); exit(0); }
	if ( do_586() != 586 ) { FAIL("iteration 586"); exit(0); }
	if ( do_587() != 587 ) { FAIL("iteration 587"); exit(0); }
	if ( do_588() != 588 ) { FAIL("iteration 588"); exit(0); }
	if ( do_589() != 589 ) { FAIL("iteration 589"); exit(0); }
	if ( do_590() != 590 ) { FAIL("iteration 590"); exit(0); }
	if ( do_591() != 591 ) { FAIL("iteration 591"); exit(0); }
	if ( do_592() != 592 ) { FAIL("iteration 592"); exit(0); }
	if ( do_593() != 593 ) { FAIL("iteration 593"); exit(0); }
	if ( do_594() != 594 ) { FAIL("iteration 594"); exit(0); }
	if ( do_595() != 595 ) { FAIL("iteration 595"); exit(0); }
	if ( do_596() != 596 ) { FAIL("iteration 596"); exit(0); }
	if ( do_597() != 597 ) { FAIL("iteration 597"); exit(0); }
	if ( do_598() != 598 ) { FAIL("iteration 598"); exit(0); }
	if ( do_599() != 599 ) { FAIL("iteration 599"); exit(0); }
	if ( do_600() != 600 ) { FAIL("iteration 600"); exit(0); }
	if ( do_601() != 601 ) { FAIL("iteration 601"); exit(0); }
	if ( do_602() != 602 ) { FAIL("iteration 602"); exit(0); }
	if ( do_603() != 603 ) { FAIL("iteration 603"); exit(0); }
	if ( do_604() != 604 ) { FAIL("iteration 604"); exit(0); }
	if ( do_605() != 605 ) { FAIL("iteration 605"); exit(0); }
	if ( do_606() != 606 ) { FAIL("iteration 606"); exit(0); }
	if ( do_607() != 607 ) { FAIL("iteration 607"); exit(0); }
	if ( do_608() != 608 ) { FAIL("iteration 608"); exit(0); }
	if ( do_609() != 609 ) { FAIL("iteration 609"); exit(0); }
	if ( do_610() != 610 ) { FAIL("iteration 610"); exit(0); }
	if ( do_611() != 611 ) { FAIL("iteration 611"); exit(0); }
	if ( do_612() != 612 ) { FAIL("iteration 612"); exit(0); }
	if ( do_613() != 613 ) { FAIL("iteration 613"); exit(0); }
	if ( do_614() != 614 ) { FAIL("iteration 614"); exit(0); }
	if ( do_615() != 615 ) { FAIL("iteration 615"); exit(0); }
	if ( do_616() != 616 ) { FAIL("iteration 616"); exit(0); }
	if ( do_617() != 617 ) { FAIL("iteration 617"); exit(0); }
	if ( do_618() != 618 ) { FAIL("iteration 618"); exit(0); }
	if ( do_619() != 619 ) { FAIL("iteration 619"); exit(0); }
	if ( do_620() != 620 ) { FAIL("iteration 620"); exit(0); }
	if ( do_621() != 621 ) { FAIL("iteration 621"); exit(0); }
	if ( do_622() != 622 ) { FAIL("iteration 622"); exit(0); }
	if ( do_623() != 623 ) { FAIL("iteration 623"); exit(0); }
	if ( do_624() != 624 ) { FAIL("iteration 624"); exit(0); }
	if ( do_625() != 625 ) { FAIL("iteration 625"); exit(0); }
	if ( do_626() != 626 ) { FAIL("iteration 626"); exit(0); }
	if ( do_627() != 627 ) { FAIL("iteration 627"); exit(0); }
	if ( do_628() != 628 ) { FAIL("iteration 628"); exit(0); }
	if ( do_629() != 629 ) { FAIL("iteration 629"); exit(0); }
	if ( do_630() != 630 ) { FAIL("iteration 630"); exit(0); }
	if ( do_631() != 631 ) { FAIL("iteration 631"); exit(0); }
	if ( do_632() != 632 ) { FAIL("iteration 632"); exit(0); }
	if ( do_633() != 633 ) { FAIL("iteration 633"); exit(0); }
	if ( do_634() != 634 ) { FAIL("iteration 634"); exit(0); }
	if ( do_635() != 635 ) { FAIL("iteration 635"); exit(0); }
	if ( do_636() != 636 ) { FAIL("iteration 636"); exit(0); }
	if ( do_637() != 637 ) { FAIL("iteration 637"); exit(0); }
	if ( do_638() != 638 ) { FAIL("iteration 638"); exit(0); }
	if ( do_639() != 639 ) { FAIL("iteration 639"); exit(0); }
	if ( do_640() != 640 ) { FAIL("iteration 640"); exit(0); }
	if ( do_641() != 641 ) { FAIL("iteration 641"); exit(0); }
	if ( do_642() != 642 ) { FAIL("iteration 642"); exit(0); }
	if ( do_643() != 643 ) { FAIL("iteration 643"); exit(0); }
	if ( do_644() != 644 ) { FAIL("iteration 644"); exit(0); }
	if ( do_645() != 645 ) { FAIL("iteration 645"); exit(0); }
	if ( do_646() != 646 ) { FAIL("iteration 646"); exit(0); }
	if ( do_647() != 647 ) { FAIL("iteration 647"); exit(0); }
	if ( do_648() != 648 ) { FAIL("iteration 648"); exit(0); }
	if ( do_649() != 649 ) { FAIL("iteration 649"); exit(0); }
	if ( do_650() != 650 ) { FAIL("iteration 650"); exit(0); }
	if ( do_651() != 651 ) { FAIL("iteration 651"); exit(0); }
	if ( do_652() != 652 ) { FAIL("iteration 652"); exit(0); }
	if ( do_653() != 653 ) { FAIL("iteration 653"); exit(0); }
	if ( do_654() != 654 ) { FAIL("iteration 654"); exit(0); }
	if ( do_655() != 655 ) { FAIL("iteration 655"); exit(0); }
	if ( do_656() != 656 ) { FAIL("iteration 656"); exit(0); }
	if ( do_657() != 657 ) { FAIL("iteration 657"); exit(0); }
	if ( do_658() != 658 ) { FAIL("iteration 658"); exit(0); }
	if ( do_659() != 659 ) { FAIL("iteration 659"); exit(0); }
	if ( do_660() != 660 ) { FAIL("iteration 660"); exit(0); }
	if ( do_661() != 661 ) { FAIL("iteration 661"); exit(0); }
	if ( do_662() != 662 ) { FAIL("iteration 662"); exit(0); }
	if ( do_663() != 663 ) { FAIL("iteration 663"); exit(0); }
	if ( do_664() != 664 ) { FAIL("iteration 664"); exit(0); }
	if ( do_665() != 665 ) { FAIL("iteration 665"); exit(0); }
	if ( do_666() != 666 ) { FAIL("iteration 666"); exit(0); }
	if ( do_667() != 667 ) { FAIL("iteration 667"); exit(0); }
	if ( do_668() != 668 ) { FAIL("iteration 668"); exit(0); }
	if ( do_669() != 669 ) { FAIL("iteration 669"); exit(0); }
	if ( do_670() != 670 ) { FAIL("iteration 670"); exit(0); }
	if ( do_671() != 671 ) { FAIL("iteration 671"); exit(0); }
	if ( do_672() != 672 ) { FAIL("iteration 672"); exit(0); }
	if ( do_673() != 673 ) { FAIL("iteration 673"); exit(0); }
	if ( do_674() != 674 ) { FAIL("iteration 674"); exit(0); }
	if ( do_675() != 675 ) { FAIL("iteration 675"); exit(0); }
	if ( do_676() != 676 ) { FAIL("iteration 676"); exit(0); }
	if ( do_677() != 677 ) { FAIL("iteration 677"); exit(0); }
	if ( do_678() != 678 ) { FAIL("iteration 678"); exit(0); }
	if ( do_679() != 679 ) { FAIL("iteration 679"); exit(0); }
	if ( do_680() != 680 ) { FAIL("iteration 680"); exit(0); }
	if ( do_681() != 681 ) { FAIL("iteration 681"); exit(0); }
	if ( do_682() != 682 ) { FAIL("iteration 682"); exit(0); }
	if ( do_683() != 683 ) { FAIL("iteration 683"); exit(0); }
	if ( do_684() != 684 ) { FAIL("iteration 684"); exit(0); }
	if ( do_685() != 685 ) { FAIL("iteration 685"); exit(0); }
	if ( do_686() != 686 ) { FAIL("iteration 686"); exit(0); }
	if ( do_687() != 687 ) { FAIL("iteration 687"); exit(0); }
	if ( do_688() != 688 ) { FAIL("iteration 688"); exit(0); }
	if ( do_689() != 689 ) { FAIL("iteration 689"); exit(0); }
	if ( do_690() != 690 ) { FAIL("iteration 690"); exit(0); }
	if ( do_691() != 691 ) { FAIL("iteration 691"); exit(0); }
	if ( do_692() != 692 ) { FAIL("iteration 692"); exit(0); }
	if ( do_693() != 693 ) { FAIL("iteration 693"); exit(0); }
	if ( do_694() != 694 ) { FAIL("iteration 694"); exit(0); }
	if ( do_695() != 695 ) { FAIL("iteration 695"); exit(0); }
	if ( do_696() != 696 ) { FAIL("iteration 696"); exit(0); }
	if ( do_697() != 697 ) { FAIL("iteration 697"); exit(0); }
	if ( do_698() != 698 ) { FAIL("iteration 698"); exit(0); }
	if ( do_699() != 699 ) { FAIL("iteration 699"); exit(0); }
	if ( do_700() != 700 ) { FAIL("iteration 700"); exit(0); }
	if ( do_701() != 701 ) { FAIL("iteration 701"); exit(0); }
	if ( do_702() != 702 ) { FAIL("iteration 702"); exit(0); }
	if ( do_703() != 703 ) { FAIL("iteration 703"); exit(0); }
	if ( do_704() != 704 ) { FAIL("iteration 704"); exit(0); }
	if ( do_705() != 705 ) { FAIL("iteration 705"); exit(0); }
	if ( do_706() != 706 ) { FAIL("iteration 706"); exit(0); }
	if ( do_707() != 707 ) { FAIL("iteration 707"); exit(0); }
	if ( do_708() != 708 ) { FAIL("iteration 708"); exit(0); }
	if ( do_709() != 709 ) { FAIL("iteration 709"); exit(0); }
	if ( do_710() != 710 ) { FAIL("iteration 710"); exit(0); }
	if ( do_711() != 711 ) { FAIL("iteration 711"); exit(0); }
	if ( do_712() != 712 ) { FAIL("iteration 712"); exit(0); }
	if ( do_713() != 713 ) { FAIL("iteration 713"); exit(0); }
	if ( do_714() != 714 ) { FAIL("iteration 714"); exit(0); }
	if ( do_715() != 715 ) { FAIL("iteration 715"); exit(0); }
	if ( do_716() != 716 ) { FAIL("iteration 716"); exit(0); }
	if ( do_717() != 717 ) { FAIL("iteration 717"); exit(0); }
	if ( do_718() != 718 ) { FAIL("iteration 718"); exit(0); }
	if ( do_719() != 719 ) { FAIL("iteration 719"); exit(0); }
	if ( do_720() != 720 ) { FAIL("iteration 720"); exit(0); }
	if ( do_721() != 721 ) { FAIL("iteration 721"); exit(0); }
	if ( do_722() != 722 ) { FAIL("iteration 722"); exit(0); }
	if ( do_723() != 723 ) { FAIL("iteration 723"); exit(0); }
	if ( do_724() != 724 ) { FAIL("iteration 724"); exit(0); }
	if ( do_725() != 725 ) { FAIL("iteration 725"); exit(0); }
	if ( do_726() != 726 ) { FAIL("iteration 726"); exit(0); }
	if ( do_727() != 727 ) { FAIL("iteration 727"); exit(0); }
	if ( do_728() != 728 ) { FAIL("iteration 728"); exit(0); }
	if ( do_729() != 729 ) { FAIL("iteration 729"); exit(0); }
	if ( do_730() != 730 ) { FAIL("iteration 730"); exit(0); }
	if ( do_731() != 731 ) { FAIL("iteration 731"); exit(0); }
	if ( do_732() != 732 ) { FAIL("iteration 732"); exit(0); }
	if ( do_733() != 733 ) { FAIL("iteration 733"); exit(0); }
	if ( do_734() != 734 ) { FAIL("iteration 734"); exit(0); }
	if ( do_735() != 735 ) { FAIL("iteration 735"); exit(0); }
	if ( do_736() != 736 ) { FAIL("iteration 736"); exit(0); }
	if ( do_737() != 737 ) { FAIL("iteration 737"); exit(0); }
	if ( do_738() != 738 ) { FAIL("iteration 738"); exit(0); }
	if ( do_739() != 739 ) { FAIL("iteration 739"); exit(0); }
	if ( do_740() != 740 ) { FAIL("iteration 740"); exit(0); }
	if ( do_741() != 741 ) { FAIL("iteration 741"); exit(0); }
	if ( do_742() != 742 ) { FAIL("iteration 742"); exit(0); }
	if ( do_743() != 743 ) { FAIL("iteration 743"); exit(0); }
	if ( do_744() != 744 ) { FAIL("iteration 744"); exit(0); }
	if ( do_745() != 745 ) { FAIL("iteration 745"); exit(0); }
	if ( do_746() != 746 ) { FAIL("iteration 746"); exit(0); }
	if ( do_747() != 747 ) { FAIL("iteration 747"); exit(0); }
	if ( do_748() != 748 ) { FAIL("iteration 748"); exit(0); }
	if ( do_749() != 749 ) { FAIL("iteration 749"); exit(0); }
	if ( do_750() != 750 ) { FAIL("iteration 750"); exit(0); }
	if ( do_751() != 751 ) { FAIL("iteration 751"); exit(0); }
	if ( do_752() != 752 ) { FAIL("iteration 752"); exit(0); }
	if ( do_753() != 753 ) { FAIL("iteration 753"); exit(0); }
	if ( do_754() != 754 ) { FAIL("iteration 754"); exit(0); }
	if ( do_755() != 755 ) { FAIL("iteration 755"); exit(0); }
	if ( do_756() != 756 ) { FAIL("iteration 756"); exit(0); }
	if ( do_757() != 757 ) { FAIL("iteration 757"); exit(0); }
	if ( do_758() != 758 ) { FAIL("iteration 758"); exit(0); }
	if ( do_759() != 759 ) { FAIL("iteration 759"); exit(0); }
	if ( do_760() != 760 ) { FAIL("iteration 760"); exit(0); }
	if ( do_761() != 761 ) { FAIL("iteration 761"); exit(0); }
	if ( do_762() != 762 ) { FAIL("iteration 762"); exit(0); }
	if ( do_763() != 763 ) { FAIL("iteration 763"); exit(0); }
	if ( do_764() != 764 ) { FAIL("iteration 764"); exit(0); }
	if ( do_765() != 765 ) { FAIL("iteration 765"); exit(0); }
	if ( do_766() != 766 ) { FAIL("iteration 766"); exit(0); }
	if ( do_767() != 767 ) { FAIL("iteration 767"); exit(0); }
	if ( do_768() != 768 ) { FAIL("iteration 768"); exit(0); }
	if ( do_769() != 769 ) { FAIL("iteration 769"); exit(0); }
	if ( do_770() != 770 ) { FAIL("iteration 770"); exit(0); }
	if ( do_771() != 771 ) { FAIL("iteration 771"); exit(0); }
	if ( do_772() != 772 ) { FAIL("iteration 772"); exit(0); }
	if ( do_773() != 773 ) { FAIL("iteration 773"); exit(0); }
	if ( do_774() != 774 ) { FAIL("iteration 774"); exit(0); }
	if ( do_775() != 775 ) { FAIL("iteration 775"); exit(0); }
	if ( do_776() != 776 ) { FAIL("iteration 776"); exit(0); }
	if ( do_777() != 777 ) { FAIL("iteration 777"); exit(0); }
	if ( do_778() != 778 ) { FAIL("iteration 778"); exit(0); }
	if ( do_779() != 779 ) { FAIL("iteration 779"); exit(0); }
	if ( do_780() != 780 ) { FAIL("iteration 780"); exit(0); }
	if ( do_781() != 781 ) { FAIL("iteration 781"); exit(0); }
	if ( do_782() != 782 ) { FAIL("iteration 782"); exit(0); }
	if ( do_783() != 783 ) { FAIL("iteration 783"); exit(0); }
	if ( do_784() != 784 ) { FAIL("iteration 784"); exit(0); }
	if ( do_785() != 785 ) { FAIL("iteration 785"); exit(0); }
	if ( do_786() != 786 ) { FAIL("iteration 786"); exit(0); }
	if ( do_787() != 787 ) { FAIL("iteration 787"); exit(0); }
	if ( do_788() != 788 ) { FAIL("iteration 788"); exit(0); }
	if ( do_789() != 789 ) { FAIL("iteration 789"); exit(0); }
	if ( do_790() != 790 ) { FAIL("iteration 790"); exit(0); }
	if ( do_791() != 791 ) { FAIL("iteration 791"); exit(0); }
	if ( do_792() != 792 ) { FAIL("iteration 792"); exit(0); }
	if ( do_793() != 793 ) { FAIL("iteration 793"); exit(0); }
	if ( do_794() != 794 ) { FAIL("iteration 794"); exit(0); }
	if ( do_795() != 795 ) { FAIL("iteration 795"); exit(0); }
	if ( do_796() != 796 ) { FAIL("iteration 796"); exit(0); }
	if ( do_797() != 797 ) { FAIL("iteration 797"); exit(0); }
	if ( do_798() != 798 ) { FAIL("iteration 798"); exit(0); }
	if ( do_799() != 799 ) { FAIL("iteration 799"); exit(0); }
	if ( do_800() != 800 ) { FAIL("iteration 800"); exit(0); }
	if ( do_801() != 801 ) { FAIL("iteration 801"); exit(0); }
	if ( do_802() != 802 ) { FAIL("iteration 802"); exit(0); }
	if ( do_803() != 803 ) { FAIL("iteration 803"); exit(0); }
	if ( do_804() != 804 ) { FAIL("iteration 804"); exit(0); }
	if ( do_805() != 805 ) { FAIL("iteration 805"); exit(0); }
	if ( do_806() != 806 ) { FAIL("iteration 806"); exit(0); }
	if ( do_807() != 807 ) { FAIL("iteration 807"); exit(0); }
	if ( do_808() != 808 ) { FAIL("iteration 808"); exit(0); }
	if ( do_809() != 809 ) { FAIL("iteration 809"); exit(0); }
	if ( do_810() != 810 ) { FAIL("iteration 810"); exit(0); }
	if ( do_811() != 811 ) { FAIL("iteration 811"); exit(0); }
	if ( do_812() != 812 ) { FAIL("iteration 812"); exit(0); }
	if ( do_813() != 813 ) { FAIL("iteration 813"); exit(0); }
	if ( do_814() != 814 ) { FAIL("iteration 814"); exit(0); }
	if ( do_815() != 815 ) { FAIL("iteration 815"); exit(0); }
	if ( do_816() != 816 ) { FAIL("iteration 816"); exit(0); }
	if ( do_817() != 817 ) { FAIL("iteration 817"); exit(0); }
	if ( do_818() != 818 ) { FAIL("iteration 818"); exit(0); }
	if ( do_819() != 819 ) { FAIL("iteration 819"); exit(0); }
	if ( do_820() != 820 ) { FAIL("iteration 820"); exit(0); }
	if ( do_821() != 821 ) { FAIL("iteration 821"); exit(0); }
	if ( do_822() != 822 ) { FAIL("iteration 822"); exit(0); }
	if ( do_823() != 823 ) { FAIL("iteration 823"); exit(0); }
	if ( do_824() != 824 ) { FAIL("iteration 824"); exit(0); }
	if ( do_825() != 825 ) { FAIL("iteration 825"); exit(0); }
	if ( do_826() != 826 ) { FAIL("iteration 826"); exit(0); }
	if ( do_827() != 827 ) { FAIL("iteration 827"); exit(0); }
	if ( do_828() != 828 ) { FAIL("iteration 828"); exit(0); }
	if ( do_829() != 829 ) { FAIL("iteration 829"); exit(0); }
	if ( do_830() != 830 ) { FAIL("iteration 830"); exit(0); }
	if ( do_831() != 831 ) { FAIL("iteration 831"); exit(0); }
	if ( do_832() != 832 ) { FAIL("iteration 832"); exit(0); }
	if ( do_833() != 833 ) { FAIL("iteration 833"); exit(0); }
	if ( do_834() != 834 ) { FAIL("iteration 834"); exit(0); }
	if ( do_835() != 835 ) { FAIL("iteration 835"); exit(0); }
	if ( do_836() != 836 ) { FAIL("iteration 836"); exit(0); }
	if ( do_837() != 837 ) { FAIL("iteration 837"); exit(0); }
	if ( do_838() != 838 ) { FAIL("iteration 838"); exit(0); }
	if ( do_839() != 839 ) { FAIL("iteration 839"); exit(0); }
	if ( do_840() != 840 ) { FAIL("iteration 840"); exit(0); }
	if ( do_841() != 841 ) { FAIL("iteration 841"); exit(0); }
	if ( do_842() != 842 ) { FAIL("iteration 842"); exit(0); }
	if ( do_843() != 843 ) { FAIL("iteration 843"); exit(0); }
	if ( do_844() != 844 ) { FAIL("iteration 844"); exit(0); }
	if ( do_845() != 845 ) { FAIL("iteration 845"); exit(0); }
	if ( do_846() != 846 ) { FAIL("iteration 846"); exit(0); }
	if ( do_847() != 847 ) { FAIL("iteration 847"); exit(0); }
	if ( do_848() != 848 ) { FAIL("iteration 848"); exit(0); }
	if ( do_849() != 849 ) { FAIL("iteration 849"); exit(0); }
	if ( do_850() != 850 ) { FAIL("iteration 850"); exit(0); }
	if ( do_851() != 851 ) { FAIL("iteration 851"); exit(0); }
	if ( do_852() != 852 ) { FAIL("iteration 852"); exit(0); }
	if ( do_853() != 853 ) { FAIL("iteration 853"); exit(0); }
	if ( do_854() != 854 ) { FAIL("iteration 854"); exit(0); }
	if ( do_855() != 855 ) { FAIL("iteration 855"); exit(0); }
	if ( do_856() != 856 ) { FAIL("iteration 856"); exit(0); }
	if ( do_857() != 857 ) { FAIL("iteration 857"); exit(0); }
	if ( do_858() != 858 ) { FAIL("iteration 858"); exit(0); }
	if ( do_859() != 859 ) { FAIL("iteration 859"); exit(0); }
	if ( do_860() != 860 ) { FAIL("iteration 860"); exit(0); }
	if ( do_861() != 861 ) { FAIL("iteration 861"); exit(0); }
	if ( do_862() != 862 ) { FAIL("iteration 862"); exit(0); }
	if ( do_863() != 863 ) { FAIL("iteration 863"); exit(0); }
	if ( do_864() != 864 ) { FAIL("iteration 864"); exit(0); }
	if ( do_865() != 865 ) { FAIL("iteration 865"); exit(0); }
	if ( do_866() != 866 ) { FAIL("iteration 866"); exit(0); }
	if ( do_867() != 867 ) { FAIL("iteration 867"); exit(0); }
	if ( do_868() != 868 ) { FAIL("iteration 868"); exit(0); }
	if ( do_869() != 869 ) { FAIL("iteration 869"); exit(0); }
	if ( do_870() != 870 ) { FAIL("iteration 870"); exit(0); }
	if ( do_871() != 871 ) { FAIL("iteration 871"); exit(0); }
	if ( do_872() != 872 ) { FAIL("iteration 872"); exit(0); }
	if ( do_873() != 873 ) { FAIL("iteration 873"); exit(0); }
	if ( do_874() != 874 ) { FAIL("iteration 874"); exit(0); }
	if ( do_875() != 875 ) { FAIL("iteration 875"); exit(0); }
	if ( do_876() != 876 ) { FAIL("iteration 876"); exit(0); }
	if ( do_877() != 877 ) { FAIL("iteration 877"); exit(0); }
	if ( do_878() != 878 ) { FAIL("iteration 878"); exit(0); }
	if ( do_879() != 879 ) { FAIL("iteration 879"); exit(0); }
	if ( do_880() != 880 ) { FAIL("iteration 880"); exit(0); }
	if ( do_881() != 881 ) { FAIL("iteration 881"); exit(0); }
	if ( do_882() != 882 ) { FAIL("iteration 882"); exit(0); }
	if ( do_883() != 883 ) { FAIL("iteration 883"); exit(0); }
	if ( do_884() != 884 ) { FAIL("iteration 884"); exit(0); }
	if ( do_885() != 885 ) { FAIL("iteration 885"); exit(0); }
	if ( do_886() != 886 ) { FAIL("iteration 886"); exit(0); }
	if ( do_887() != 887 ) { FAIL("iteration 887"); exit(0); }
	if ( do_888() != 888 ) { FAIL("iteration 888"); exit(0); }
	if ( do_889() != 889 ) { FAIL("iteration 889"); exit(0); }
	if ( do_890() != 890 ) { FAIL("iteration 890"); exit(0); }
	if ( do_891() != 891 ) { FAIL("iteration 891"); exit(0); }
	if ( do_892() != 892 ) { FAIL("iteration 892"); exit(0); }
	if ( do_893() != 893 ) { FAIL("iteration 893"); exit(0); }
	if ( do_894() != 894 ) { FAIL("iteration 894"); exit(0); }
	if ( do_895() != 895 ) { FAIL("iteration 895"); exit(0); }
	if ( do_896() != 896 ) { FAIL("iteration 896"); exit(0); }
	if ( do_897() != 897 ) { FAIL("iteration 897"); exit(0); }
	if ( do_898() != 898 ) { FAIL("iteration 898"); exit(0); }
	if ( do_899() != 899 ) { FAIL("iteration 899"); exit(0); }
	if ( do_900() != 900 ) { FAIL("iteration 900"); exit(0); }
	if ( do_901() != 901 ) { FAIL("iteration 901"); exit(0); }
	if ( do_902() != 902 ) { FAIL("iteration 902"); exit(0); }
	if ( do_903() != 903 ) { FAIL("iteration 903"); exit(0); }
	if ( do_904() != 904 ) { FAIL("iteration 904"); exit(0); }
	if ( do_905() != 905 ) { FAIL("iteration 905"); exit(0); }
	if ( do_906() != 906 ) { FAIL("iteration 906"); exit(0); }
	if ( do_907() != 907 ) { FAIL("iteration 907"); exit(0); }
	if ( do_908() != 908 ) { FAIL("iteration 908"); exit(0); }
	if ( do_909() != 909 ) { FAIL("iteration 909"); exit(0); }
	if ( do_910() != 910 ) { FAIL("iteration 910"); exit(0); }
	if ( do_911() != 911 ) { FAIL("iteration 911"); exit(0); }
	if ( do_912() != 912 ) { FAIL("iteration 912"); exit(0); }
	if ( do_913() != 913 ) { FAIL("iteration 913"); exit(0); }
	if ( do_914() != 914 ) { FAIL("iteration 914"); exit(0); }
	if ( do_915() != 915 ) { FAIL("iteration 915"); exit(0); }
	if ( do_916() != 916 ) { FAIL("iteration 916"); exit(0); }
	if ( do_917() != 917 ) { FAIL("iteration 917"); exit(0); }
	if ( do_918() != 918 ) { FAIL("iteration 918"); exit(0); }
	if ( do_919() != 919 ) { FAIL("iteration 919"); exit(0); }
	if ( do_920() != 920 ) { FAIL("iteration 920"); exit(0); }
	if ( do_921() != 921 ) { FAIL("iteration 921"); exit(0); }
	if ( do_922() != 922 ) { FAIL("iteration 922"); exit(0); }
	if ( do_923() != 923 ) { FAIL("iteration 923"); exit(0); }
	if ( do_924() != 924 ) { FAIL("iteration 924"); exit(0); }
	if ( do_925() != 925 ) { FAIL("iteration 925"); exit(0); }
	if ( do_926() != 926 ) { FAIL("iteration 926"); exit(0); }
	if ( do_927() != 927 ) { FAIL("iteration 927"); exit(0); }
	if ( do_928() != 928 ) { FAIL("iteration 928"); exit(0); }
	if ( do_929() != 929 ) { FAIL("iteration 929"); exit(0); }
	if ( do_930() != 930 ) { FAIL("iteration 930"); exit(0); }
	if ( do_931() != 931 ) { FAIL("iteration 931"); exit(0); }
	if ( do_932() != 932 ) { FAIL("iteration 932"); exit(0); }
	if ( do_933() != 933 ) { FAIL("iteration 933"); exit(0); }
	if ( do_934() != 934 ) { FAIL("iteration 934"); exit(0); }
	if ( do_935() != 935 ) { FAIL("iteration 935"); exit(0); }
	if ( do_936() != 936 ) { FAIL("iteration 936"); exit(0); }
	if ( do_937() != 937 ) { FAIL("iteration 937"); exit(0); }
	if ( do_938() != 938 ) { FAIL("iteration 938"); exit(0); }
	if ( do_939() != 939 ) { FAIL("iteration 939"); exit(0); }
	if ( do_940() != 940 ) { FAIL("iteration 940"); exit(0); }
	if ( do_941() != 941 ) { FAIL("iteration 941"); exit(0); }
	if ( do_942() != 942 ) { FAIL("iteration 942"); exit(0); }
	if ( do_943() != 943 ) { FAIL("iteration 943"); exit(0); }
	if ( do_944() != 944 ) { FAIL("iteration 944"); exit(0); }
	if ( do_945() != 945 ) { FAIL("iteration 945"); exit(0); }
	if ( do_946() != 946 ) { FAIL("iteration 946"); exit(0); }
	if ( do_947() != 947 ) { FAIL("iteration 947"); exit(0); }
	if ( do_948() != 948 ) { FAIL("iteration 948"); exit(0); }
	if ( do_949() != 949 ) { FAIL("iteration 949"); exit(0); }
	if ( do_950() != 950 ) { FAIL("iteration 950"); exit(0); }
	if ( do_951() != 951 ) { FAIL("iteration 951"); exit(0); }
	if ( do_952() != 952 ) { FAIL("iteration 952"); exit(0); }
	if ( do_953() != 953 ) { FAIL("iteration 953"); exit(0); }
	if ( do_954() != 954 ) { FAIL("iteration 954"); exit(0); }
	if ( do_955() != 955 ) { FAIL("iteration 955"); exit(0); }
	if ( do_956() != 956 ) { FAIL("iteration 956"); exit(0); }
	if ( do_957() != 957 ) { FAIL("iteration 957"); exit(0); }
	if ( do_958() != 958 ) { FAIL("iteration 958"); exit(0); }
	if ( do_959() != 959 ) { FAIL("iteration 959"); exit(0); }
	if ( do_960() != 960 ) { FAIL("iteration 960"); exit(0); }
	if ( do_961() != 961 ) { FAIL("iteration 961"); exit(0); }
	if ( do_962() != 962 ) { FAIL("iteration 962"); exit(0); }
	if ( do_963() != 963 ) { FAIL("iteration 963"); exit(0); }
	if ( do_964() != 964 ) { FAIL("iteration 964"); exit(0); }
	if ( do_965() != 965 ) { FAIL("iteration 965"); exit(0); }
	if ( do_966() != 966 ) { FAIL("iteration 966"); exit(0); }
	if ( do_967() != 967 ) { FAIL("iteration 967"); exit(0); }
	if ( do_968() != 968 ) { FAIL("iteration 968"); exit(0); }
	if ( do_969() != 969 ) { FAIL("iteration 969"); exit(0); }
	if ( do_970() != 970 ) { FAIL("iteration 970"); exit(0); }
	if ( do_971() != 971 ) { FAIL("iteration 971"); exit(0); }
	if ( do_972() != 972 ) { FAIL("iteration 972"); exit(0); }
	if ( do_973() != 973 ) { FAIL("iteration 973"); exit(0); }
	if ( do_974() != 974 ) { FAIL("iteration 974"); exit(0); }
	if ( do_975() != 975 ) { FAIL("iteration 975"); exit(0); }
	if ( do_976() != 976 ) { FAIL("iteration 976"); exit(0); }
	if ( do_977() != 977 ) { FAIL("iteration 977"); exit(0); }
	if ( do_978() != 978 ) { FAIL("iteration 978"); exit(0); }
	if ( do_979() != 979 ) { FAIL("iteration 979"); exit(0); }
	if ( do_980() != 980 ) { FAIL("iteration 980"); exit(0); }
	if ( do_981() != 981 ) { FAIL("iteration 981"); exit(0); }
	if ( do_982() != 982 ) { FAIL("iteration 982"); exit(0); }
	if ( do_983() != 983 ) { FAIL("iteration 983"); exit(0); }
	if ( do_984() != 984 ) { FAIL("iteration 984"); exit(0); }
	if ( do_985() != 985 ) { FAIL("iteration 985"); exit(0); }
	if ( do_986() != 986 ) { FAIL("iteration 986"); exit(0); }
	if ( do_987() != 987 ) { FAIL("iteration 987"); exit(0); }
	if ( do_988() != 988 ) { FAIL("iteration 988"); exit(0); }
	if ( do_989() != 989 ) { FAIL("iteration 989"); exit(0); }
	if ( do_990() != 990 ) { FAIL("iteration 990"); exit(0); }
	if ( do_991() != 991 ) { FAIL("iteration 991"); exit(0); }
	if ( do_992() != 992 ) { FAIL("iteration 992"); exit(0); }
	if ( do_993() != 993 ) { FAIL("iteration 993"); exit(0); }
	if ( do_994() != 994 ) { FAIL("iteration 994"); exit(0); }
	if ( do_995() != 995 ) { FAIL("iteration 995"); exit(0); }
	if ( do_996() != 996 ) { FAIL("iteration 996"); exit(0); }
	if ( do_997() != 997 ) { FAIL("iteration 997"); exit(0); }
	if ( do_998() != 998 ) { FAIL("iteration 998"); exit(0); }
	if ( do_999() != 999 ) { FAIL("iteration 999"); exit(0); }
	return NULL;
}


int main()
{
	pthread_t worker[10];
	for (int i=0; i < 10; ++i) {
		if ( pthread_create(&worker[i], NULL, work, NULL) != 0 ) {
			FAIL("pthread_create failed");
			exit(0);
		}
	}
		
	void* result;
	for (int i=0; i < 10; ++i) {
		pthread_join(worker[i], &result);
	}
	
	PASS("thread-lazy-bind");
	return 0;
}
