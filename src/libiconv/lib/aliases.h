/* ANSI-C code produced by gperf version 3.0.1 */
/* Command-line: gperf -m 10 lib/aliases.gperf  */
/* Computed positions: -k'1,3-11,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "lib/aliases.gperf"
struct alias { int name; unsigned int encoding_index; };

#define TOTAL_KEYWORDS 344
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 45
#define MIN_HASH_VALUE 8
#define MAX_HASH_VALUE 1033
/* maximum key range = 1026, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
aliases_hash (register const char *str, register unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034,
      1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034,
      1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034,
      1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034,
      1034, 1034, 1034, 1034, 1034,    0,  108, 1034,  135,    6,
        45,  105,   12,   21,    0,  126,   30,   18,  225, 1034,
      1034, 1034, 1034, 1034, 1034,   15,  162,    9,   36,   18,
       111,    9,   51,    0,  204,  204,   60,  153,    0,    0,
        90, 1034,    0,    9,   93,  273,  156,  198,  279,   21,
         0, 1034, 1034, 1034, 1034,    3, 1034, 1034, 1034, 1034,
      1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034,
      1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034,
      1034, 1034, 1034, 1034, 1034, 1034, 1034, 1034
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[10]];
      /*FALLTHROUGH*/
      case 10:
        hval += asso_values[(unsigned char)str[9]];
      /*FALLTHROUGH*/
      case 9:
        hval += asso_values[(unsigned char)str[8]];
      /*FALLTHROUGH*/
      case 8:
        hval += asso_values[(unsigned char)str[7]];
      /*FALLTHROUGH*/
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      /*FALLTHROUGH*/
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      /*FALLTHROUGH*/
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

struct stringpool_t
  {
    char stringpool_str8[sizeof("ISO-IR-6")];
    char stringpool_str11[sizeof("CN")];
    char stringpool_str16[sizeof("ISO-IR-166")];
    char stringpool_str28[sizeof("CHAR")];
    char stringpool_str29[sizeof("ASCII")];
    char stringpool_str30[sizeof("ISO646-CN")];
    char stringpool_str31[sizeof("SJIS")];
    char stringpool_str32[sizeof("R8")];
    char stringpool_str33[sizeof("866")];
    char stringpool_str39[sizeof("ISO-IR-14")];
    char stringpool_str41[sizeof("EUCCN")];
    char stringpool_str42[sizeof("EUC-CN")];
    char stringpool_str44[sizeof("CP866")];
    char stringpool_str48[sizeof("C99")];
    char stringpool_str49[sizeof("CSASCII")];
    char stringpool_str52[sizeof("ISO-IR-144")];
    char stringpool_str53[sizeof("HZ")];
    char stringpool_str58[sizeof("ISO-IR-165")];
    char stringpool_str61[sizeof("ISO-IR-126")];
    char stringpool_str62[sizeof("L6")];
    char stringpool_str64[sizeof("ISO-IR-149")];
    char stringpool_str65[sizeof("CP154")];
    char stringpool_str68[sizeof("L1")];
    char stringpool_str70[sizeof("ISO-IR-199")];
    char stringpool_str73[sizeof("ISO-IR-159")];
    char stringpool_str74[sizeof("L4")];
    char stringpool_str79[sizeof("CHINESE")];
    char stringpool_str80[sizeof("CP949")];
    char stringpool_str83[sizeof("L5")];
    char stringpool_str86[sizeof("CP819")];
    char stringpool_str87[sizeof("CP1256")];
    char stringpool_str88[sizeof("ISO-IR-148")];
    char stringpool_str90[sizeof("ISO-IR-58")];
    char stringpool_str92[sizeof("L8")];
    char stringpool_str94[sizeof("GEORGIAN-ACADEMY")];
    char stringpool_str99[sizeof("CP1251")];
    char stringpool_str100[sizeof("ISO-IR-226")];
    char stringpool_str107[sizeof("L2")];
    char stringpool_str108[sizeof("ISO8859-6")];
    char stringpool_str109[sizeof("ISO-8859-6")];
    char stringpool_str111[sizeof("CP1254")];
    char stringpool_str112[sizeof("ISO_8859-6")];
    char stringpool_str115[sizeof("ISO8859-16")];
    char stringpool_str116[sizeof("ISO-8859-16")];
    char stringpool_str119[sizeof("ISO_8859-16")];
    char stringpool_str120[sizeof("ISO8859-1")];
    char stringpool_str121[sizeof("ISO-8859-1")];
    char stringpool_str123[sizeof("862")];
    char stringpool_str124[sizeof("ISO_8859-1")];
    char stringpool_str127[sizeof("ISO8859-11")];
    char stringpool_str128[sizeof("ISO-8859-11")];
    char stringpool_str129[sizeof("CP1255")];
    char stringpool_str130[sizeof("ISO_8859-16:2001")];
    char stringpool_str131[sizeof("ISO_8859-11")];
    char stringpool_str132[sizeof("ISO8859-4")];
    char stringpool_str133[sizeof("ISO-8859-4")];
    char stringpool_str134[sizeof("CP862")];
    char stringpool_str136[sizeof("ISO_8859-4")];
    char stringpool_str137[sizeof("CP936")];
    char stringpool_str138[sizeof("CP1361")];
    char stringpool_str139[sizeof("ISO8859-14")];
    char stringpool_str140[sizeof("ISO-8859-14")];
    char stringpool_str143[sizeof("ISO_8859-14")];
    char stringpool_str144[sizeof("ISO8859-9")];
    char stringpool_str145[sizeof("ISO-8859-9")];
    char stringpool_str146[sizeof("PT154")];
    char stringpool_str147[sizeof("CP1258")];
    char stringpool_str148[sizeof("ISO_8859-9")];
    char stringpool_str150[sizeof("ISO8859-5")];
    char stringpool_str151[sizeof("ISO-8859-5")];
    char stringpool_str152[sizeof("GEORGIAN-PS")];
    char stringpool_str154[sizeof("ISO_8859-5")];
    char stringpool_str155[sizeof("CYRILLIC")];
    char stringpool_str157[sizeof("ISO8859-15")];
    char stringpool_str158[sizeof("ISO-8859-15")];
    char stringpool_str159[sizeof("LATIN6")];
    char stringpool_str161[sizeof("ISO_8859-15")];
    char stringpool_str163[sizeof("ISO-IR-101")];
    char stringpool_str166[sizeof("ISO_8859-14:1998")];
    char stringpool_str167[sizeof("L3")];
    char stringpool_str168[sizeof("ISO8859-8")];
    char stringpool_str169[sizeof("ISO-8859-8")];
    char stringpool_str171[sizeof("LATIN1")];
    char stringpool_str172[sizeof("ISO_8859-8")];
    char stringpool_str174[sizeof("MAC")];
    char stringpool_str175[sizeof("ISO_8859-15:1998")];
    char stringpool_str176[sizeof("CYRILLIC-ASIAN")];
    char stringpool_str177[sizeof("CP1252")];
    char stringpool_str178[sizeof("ISO-IR-179")];
    char stringpool_str180[sizeof("VISCII")];
    char stringpool_str181[sizeof("ISO-IR-138")];
    char stringpool_str183[sizeof("LATIN4")];
    char stringpool_str184[sizeof("MS-ANSI")];
    char stringpool_str187[sizeof("ISO-IR-109")];
    char stringpool_str188[sizeof("L7")];
    char stringpool_str189[sizeof("IBM866")];
    char stringpool_str191[sizeof("CSVISCII")];
    char stringpool_str194[sizeof("CP874")];
    char stringpool_str196[sizeof("LATIN-9")];
    char stringpool_str197[sizeof("CSISOLATIN6")];
    char stringpool_str198[sizeof("ISO8859-2")];
    char stringpool_str199[sizeof("ISO-8859-2")];
    char stringpool_str201[sizeof("LATIN5")];
    char stringpool_str202[sizeof("ISO_8859-2")];
    char stringpool_str208[sizeof("ISO-CELTIC")];
    char stringpool_str209[sizeof("CSISOLATIN1")];
    char stringpool_str212[sizeof("MS-EE")];
    char stringpool_str216[sizeof("ARABIC")];
    char stringpool_str217[sizeof("BIG5")];
    char stringpool_str218[sizeof("BIG-5")];
    char stringpool_str219[sizeof("LATIN8")];
    char stringpool_str221[sizeof("CSISOLATIN4")];
    char stringpool_str222[sizeof("CSISOLATINCYRILLIC")];
    char stringpool_str224[sizeof("CN-GB-ISOIR165")];
    char stringpool_str226[sizeof("CSISOLATINARABIC")];
    char stringpool_str227[sizeof("CP932")];
    char stringpool_str228[sizeof("CSBIG5")];
    char stringpool_str229[sizeof("CN-BIG5")];
    char stringpool_str230[sizeof("ECMA-114")];
    char stringpool_str231[sizeof("IBM819")];
    char stringpool_str234[sizeof("ROMAN8")];
    char stringpool_str236[sizeof("EUCKR")];
    char stringpool_str237[sizeof("EUC-KR")];
    char stringpool_str239[sizeof("CSISOLATIN5")];
    char stringpool_str240[sizeof("KOI8-R")];
    char stringpool_str243[sizeof("KOREAN")];
    char stringpool_str245[sizeof("ELOT_928")];
    char stringpool_str247[sizeof("PTCP154")];
    char stringpool_str249[sizeof("LATIN2")];
    char stringpool_str250[sizeof("CSKOI8R")];
    char stringpool_str253[sizeof("TCVN")];
    char stringpool_str255[sizeof("ARMSCII-8")];
    char stringpool_str261[sizeof("GB2312")];
    char stringpool_str266[sizeof("ECMA-118")];
    char stringpool_str275[sizeof("CSISO14JISC6220RO")];
    char stringpool_str279[sizeof("IBM862")];
    char stringpool_str281[sizeof("MS936")];
    char stringpool_str282[sizeof("ISO-IR-57")];
    char stringpool_str284[sizeof("US")];
    char stringpool_str287[sizeof("CSISOLATIN2")];
    char stringpool_str288[sizeof("HP-ROMAN8")];
    char stringpool_str289[sizeof("ISO-IR-157")];
    char stringpool_str290[sizeof("ISO-2022-CN")];
    char stringpool_str291[sizeof("ISO-IR-87")];
    char stringpool_str292[sizeof("ISO-IR-110")];
    char stringpool_str294[sizeof("UHC")];
    char stringpool_str296[sizeof("JP")];
    char stringpool_str297[sizeof("CP1253")];
    char stringpool_str301[sizeof("ISO_646.IRV:1991")];
    char stringpool_str303[sizeof("850")];
    char stringpool_str304[sizeof("ISO_8859-10:1992")];
    char stringpool_str305[sizeof("MACCROATIAN")];
    char stringpool_str308[sizeof("CSISO2022CN")];
    char stringpool_str310[sizeof("MS-CYRL")];
    char stringpool_str311[sizeof("UCS-4")];
    char stringpool_str312[sizeof("ISO646-US")];
    char stringpool_str313[sizeof("ISO-IR-127")];
    char stringpool_str314[sizeof("US-ASCII")];
    char stringpool_str315[sizeof("GREEK8")];
    char stringpool_str317[sizeof("VISCII1.1-1")];
    char stringpool_str318[sizeof("ISO8859-3")];
    char stringpool_str319[sizeof("ISO-8859-3")];
    char stringpool_str322[sizeof("ISO_8859-3")];
    char stringpool_str323[sizeof("CP950")];
    char stringpool_str325[sizeof("ISO8859-13")];
    char stringpool_str326[sizeof("ISO-8859-13")];
    char stringpool_str327[sizeof("MULELAO-1")];
    char stringpool_str328[sizeof("MACTHAI")];
    char stringpool_str329[sizeof("ISO_8859-13")];
    char stringpool_str330[sizeof("CSUCS4")];
    char stringpool_str333[sizeof("L10")];
    char stringpool_str335[sizeof("CP850")];
    char stringpool_str338[sizeof("MACROMAN")];
    char stringpool_str339[sizeof("CP1257")];
    char stringpool_str341[sizeof("MACCYRILLIC")];
    char stringpool_str342[sizeof("CP1133")];
    char stringpool_str346[sizeof("MACICELAND")];
    char stringpool_str347[sizeof("CN-GB")];
    char stringpool_str351[sizeof("CSPTCP154")];
    char stringpool_str357[sizeof("CP1250")];
    char stringpool_str360[sizeof("ISO8859-7")];
    char stringpool_str361[sizeof("ISO-8859-7")];
    char stringpool_str362[sizeof("CSIBM866")];
    char stringpool_str364[sizeof("ISO_8859-7")];
    char stringpool_str365[sizeof("UNICODE-1-1")];
    char stringpool_str369[sizeof("LATIN3")];
    char stringpool_str370[sizeof("MACROMANIA")];
    char stringpool_str371[sizeof("CP367")];
    char stringpool_str372[sizeof("CSUNICODE")];
    char stringpool_str374[sizeof("CSUNICODE11")];
    char stringpool_str375[sizeof("MACINTOSH")];
    char stringpool_str377[sizeof("UCS-2")];
    char stringpool_str378[sizeof("ISO_8859-9:1989")];
    char stringpool_str381[sizeof("MACARABIC")];
    char stringpool_str384[sizeof("ISO_8859-4:1988")];
    char stringpool_str385[sizeof("ISO8859-10")];
    char stringpool_str386[sizeof("ISO-8859-10")];
    char stringpool_str387[sizeof("ISO-2022-CN-EXT")];
    char stringpool_str388[sizeof("CSHPROMAN8")];
    char stringpool_str389[sizeof("ISO_8859-10")];
    char stringpool_str391[sizeof("MS-HEBR")];
    char stringpool_str392[sizeof("KSC_5601")];
    char stringpool_str393[sizeof("ISO_8859-5:1988")];
    char stringpool_str394[sizeof("JAVA")];
    char stringpool_str396[sizeof("UTF-16")];
    char stringpool_str397[sizeof("UCS-4LE")];
    char stringpool_str400[sizeof("ISO-IR-203")];
    char stringpool_str401[sizeof("CSMACINTOSH")];
    char stringpool_str402[sizeof("ISO_8859-8:1988")];
    char stringpool_str403[sizeof("CSISO57GB1988")];
    char stringpool_str405[sizeof("ISO646-JP")];
    char stringpool_str406[sizeof("GB_1988-80")];
    char stringpool_str407[sizeof("CSISOLATIN3")];
    char stringpool_str409[sizeof("MACCENTRALEUROPE")];
    char stringpool_str411[sizeof("LATIN7")];
    char stringpool_str414[sizeof("CSISOLATINGREEK")];
    char stringpool_str416[sizeof("EUCJP")];
    char stringpool_str417[sizeof("EUC-JP")];
    char stringpool_str419[sizeof("KS_C_5601-1989")];
    char stringpool_str420[sizeof("GBK")];
    char stringpool_str421[sizeof("ISO-IR-100")];
    char stringpool_str423[sizeof("TIS620")];
    char stringpool_str424[sizeof("TIS-620")];
    char stringpool_str425[sizeof("X0212")];
    char stringpool_str426[sizeof("KOI8-T")];
    char stringpool_str430[sizeof("UCS-2LE")];
    char stringpool_str431[sizeof("CSISO159JISX02121990")];
    char stringpool_str434[sizeof("CSGB2312")];
    char stringpool_str436[sizeof("LATIN10")];
    char stringpool_str440[sizeof("JIS_C6226-1983")];
    char stringpool_str444[sizeof("SHIFT-JIS")];
    char stringpool_str447[sizeof("SHIFT_JIS")];
    char stringpool_str448[sizeof("UTF8")];
    char stringpool_str449[sizeof("UTF-8")];
    char stringpool_str451[sizeof("CSISOLATINHEBREW")];
    char stringpool_str453[sizeof("ISO-10646-UCS-4")];
    char stringpool_str458[sizeof("GREEK")];
    char stringpool_str460[sizeof("WCHAR_T")];
    char stringpool_str468[sizeof("ISO_8859-6:1987")];
    char stringpool_str469[sizeof("TCVN5712-1")];
    char stringpool_str473[sizeof("JIS_C6220-1969-RO")];
    char stringpool_str474[sizeof("ISO_8859-1:1987")];
    char stringpool_str476[sizeof("X0201")];
    char stringpool_str477[sizeof("ISO_8859-3:1988")];
    char stringpool_str478[sizeof("HZ-GB-2312")];
    char stringpool_str479[sizeof("UCS-4-INTERNAL")];
    char stringpool_str480[sizeof("IBM850")];
    char stringpool_str481[sizeof("BIGFIVE")];
    char stringpool_str482[sizeof("BIG-FIVE")];
    char stringpool_str485[sizeof("ISO-2022-KR")];
    char stringpool_str486[sizeof("ISO-10646-UCS-2")];
    char stringpool_str491[sizeof("UNICODE-1-1-UTF-7")];
    char stringpool_str492[sizeof("BIG5HKSCS")];
    char stringpool_str493[sizeof("BIG5-HKSCS")];
    char stringpool_str494[sizeof("UTF-16LE")];
    char stringpool_str497[sizeof("ASMO-708")];
    char stringpool_str498[sizeof("CSUNICODE11UTF7")];
    char stringpool_str499[sizeof("UCS-4BE")];
    char stringpool_str501[sizeof("TCVN-5712")];
    char stringpool_str503[sizeof("CSISO2022KR")];
    char stringpool_str505[sizeof("CSSHIFTJIS")];
    char stringpool_str512[sizeof("UCS-2-INTERNAL")];
    char stringpool_str513[sizeof("ISO_8859-2:1987")];
    char stringpool_str514[sizeof("MS-ARAB")];
    char stringpool_str516[sizeof("IBM367")];
    char stringpool_str520[sizeof("CSEUCKR")];
    char stringpool_str521[sizeof("EUCTW")];
    char stringpool_str522[sizeof("EUC-TW")];
    char stringpool_str523[sizeof("GB_2312-80")];
    char stringpool_str524[sizeof("X0208")];
    char stringpool_str525[sizeof("WINDOWS-1256")];
    char stringpool_str526[sizeof("UNICODEBIG")];
    char stringpool_str527[sizeof("KS_C_5601-1987")];
    char stringpool_str531[sizeof("WINDOWS-1251")];
    char stringpool_str532[sizeof("UCS-2BE")];
    char stringpool_str537[sizeof("WINDOWS-1254")];
    char stringpool_str538[sizeof("TIS620.2529-1")];
    char stringpool_str540[sizeof("CSISO58GB231280")];
    char stringpool_str546[sizeof("WINDOWS-1255")];
    char stringpool_str551[sizeof("ANSI_X3.4-1986")];
    char stringpool_str555[sizeof("WINDOWS-1258")];
    char stringpool_str556[sizeof("CSKSC56011987")];
    char stringpool_str558[sizeof("EXTENDED_UNIX_CODE_PACKED_FORMAT_FOR_JAPANESE")];
    char stringpool_str560[sizeof("TIS620-0")];
    char stringpool_str562[sizeof("GB18030")];
    char stringpool_str567[sizeof("CSPC862LATINHEBREW")];
    char stringpool_str570[sizeof("WINDOWS-1252")];
    char stringpool_str573[sizeof("ISO_8859-7:2003")];
    char stringpool_str575[sizeof("WINDOWS-936")];
    char stringpool_str581[sizeof("ANSI_X3.4-1968")];
    char stringpool_str583[sizeof("ISO-2022-JP-1")];
    char stringpool_str585[sizeof("UTF-32")];
    char stringpool_str587[sizeof("MS_KANJI")];
    char stringpool_str589[sizeof("IBM-CP1133")];
    char stringpool_str594[sizeof("ISO_8859-7:1987")];
    char stringpool_str595[sizeof("JIS0208")];
    char stringpool_str596[sizeof("UTF-16BE")];
    char stringpool_str599[sizeof("JOHAB")];
    char stringpool_str608[sizeof("UTF8-MAC")];
    char stringpool_str609[sizeof("UTF-8-MAC")];
    char stringpool_str613[sizeof("UNICODELITTLE")];
    char stringpool_str614[sizeof("MS-GREEK")];
    char stringpool_str622[sizeof("ISO-2022-JP-2")];
    char stringpool_str623[sizeof("MACGREEK")];
    char stringpool_str630[sizeof("WINDOWS-1253")];
    char stringpool_str632[sizeof("WINDOWS-874")];
    char stringpool_str633[sizeof("HEBREW")];
    char stringpool_str638[sizeof("UTF-32LE")];
    char stringpool_str639[sizeof("CSISO2022JP2")];
    char stringpool_str641[sizeof("UTF-7")];
    char stringpool_str651[sizeof("WINDOWS-1257")];
    char stringpool_str658[sizeof("CSHALFWIDTHKATAKANA")];
    char stringpool_str660[sizeof("WINDOWS-1250")];
    char stringpool_str665[sizeof("ISO-2022-JP")];
    char stringpool_str680[sizeof("NEXTSTEP")];
    char stringpool_str683[sizeof("CSISO2022JP")];
    char stringpool_str685[sizeof("TIS620.2533-1")];
    char stringpool_str700[sizeof("MACUKRAINE")];
    char stringpool_str711[sizeof("CSISO87JISX0208")];
    char stringpool_str720[sizeof("BIG5-HKSCS:2001")];
    char stringpool_str726[sizeof("BIG5-HKSCS:2004")];
    char stringpool_str732[sizeof("BIG5-HKSCS:1999")];
    char stringpool_str740[sizeof("UTF-32BE")];
    char stringpool_str745[sizeof("UCS-4-SWAPPED")];
    char stringpool_str778[sizeof("UCS-2-SWAPPED")];
    char stringpool_str780[sizeof("JIS_X0212")];
    char stringpool_str786[sizeof("KOI8-U")];
    char stringpool_str787[sizeof("KOI8-RU")];
    char stringpool_str798[sizeof("TCVN5712-1:1993")];
    char stringpool_str805[sizeof("CSEUCTW")];
    char stringpool_str814[sizeof("TIS620.2533-0")];
    char stringpool_str816[sizeof("MACHEBREW")];
    char stringpool_str831[sizeof("JIS_X0201")];
    char stringpool_str844[sizeof("WINBALTRIM")];
    char stringpool_str850[sizeof("JISX0201-1976")];
    char stringpool_str853[sizeof("MACTURKISH")];
    char stringpool_str879[sizeof("JIS_X0208")];
    char stringpool_str881[sizeof("JIS_X0212-1990")];
    char stringpool_str934[sizeof("MS-TURK")];
    char stringpool_str952[sizeof("CSPC850MULTILINGUAL")];
    char stringpool_str965[sizeof("JIS_X0208-1983")];
    char stringpool_str991[sizeof("JIS_X0212.1990-0")];
    char stringpool_str995[sizeof("JIS_X0208-1990")];
    char stringpool_str1033[sizeof("CSEUCPKDFMTJAPANESE")];
  };
static const struct stringpool_t stringpool_contents =
  {
    "ISO-IR-6",
    "CN",
    "ISO-IR-166",
    "CHAR",
    "ASCII",
    "ISO646-CN",
    "SJIS",
    "R8",
    "866",
    "ISO-IR-14",
    "EUCCN",
    "EUC-CN",
    "CP866",
    "C99",
    "CSASCII",
    "ISO-IR-144",
    "HZ",
    "ISO-IR-165",
    "ISO-IR-126",
    "L6",
    "ISO-IR-149",
    "CP154",
    "L1",
    "ISO-IR-199",
    "ISO-IR-159",
    "L4",
    "CHINESE",
    "CP949",
    "L5",
    "CP819",
    "CP1256",
    "ISO-IR-148",
    "ISO-IR-58",
    "L8",
    "GEORGIAN-ACADEMY",
    "CP1251",
    "ISO-IR-226",
    "L2",
    "ISO8859-6",
    "ISO-8859-6",
    "CP1254",
    "ISO_8859-6",
    "ISO8859-16",
    "ISO-8859-16",
    "ISO_8859-16",
    "ISO8859-1",
    "ISO-8859-1",
    "862",
    "ISO_8859-1",
    "ISO8859-11",
    "ISO-8859-11",
    "CP1255",
    "ISO_8859-16:2001",
    "ISO_8859-11",
    "ISO8859-4",
    "ISO-8859-4",
    "CP862",
    "ISO_8859-4",
    "CP936",
    "CP1361",
    "ISO8859-14",
    "ISO-8859-14",
    "ISO_8859-14",
    "ISO8859-9",
    "ISO-8859-9",
    "PT154",
    "CP1258",
    "ISO_8859-9",
    "ISO8859-5",
    "ISO-8859-5",
    "GEORGIAN-PS",
    "ISO_8859-5",
    "CYRILLIC",
    "ISO8859-15",
    "ISO-8859-15",
    "LATIN6",
    "ISO_8859-15",
    "ISO-IR-101",
    "ISO_8859-14:1998",
    "L3",
    "ISO8859-8",
    "ISO-8859-8",
    "LATIN1",
    "ISO_8859-8",
    "MAC",
    "ISO_8859-15:1998",
    "CYRILLIC-ASIAN",
    "CP1252",
    "ISO-IR-179",
    "VISCII",
    "ISO-IR-138",
    "LATIN4",
    "MS-ANSI",
    "ISO-IR-109",
    "L7",
    "IBM866",
    "CSVISCII",
    "CP874",
    "LATIN-9",
    "CSISOLATIN6",
    "ISO8859-2",
    "ISO-8859-2",
    "LATIN5",
    "ISO_8859-2",
    "ISO-CELTIC",
    "CSISOLATIN1",
    "MS-EE",
    "ARABIC",
    "BIG5",
    "BIG-5",
    "LATIN8",
    "CSISOLATIN4",
    "CSISOLATINCYRILLIC",
    "CN-GB-ISOIR165",
    "CSISOLATINARABIC",
    "CP932",
    "CSBIG5",
    "CN-BIG5",
    "ECMA-114",
    "IBM819",
    "ROMAN8",
    "EUCKR",
    "EUC-KR",
    "CSISOLATIN5",
    "KOI8-R",
    "KOREAN",
    "ELOT_928",
    "PTCP154",
    "LATIN2",
    "CSKOI8R",
    "TCVN",
    "ARMSCII-8",
    "GB2312",
    "ECMA-118",
    "CSISO14JISC6220RO",
    "IBM862",
    "MS936",
    "ISO-IR-57",
    "US",
    "CSISOLATIN2",
    "HP-ROMAN8",
    "ISO-IR-157",
    "ISO-2022-CN",
    "ISO-IR-87",
    "ISO-IR-110",
    "UHC",
    "JP",
    "CP1253",
    "ISO_646.IRV:1991",
    "850",
    "ISO_8859-10:1992",
    "MACCROATIAN",
    "CSISO2022CN",
    "MS-CYRL",
    "UCS-4",
    "ISO646-US",
    "ISO-IR-127",
    "US-ASCII",
    "GREEK8",
    "VISCII1.1-1",
    "ISO8859-3",
    "ISO-8859-3",
    "ISO_8859-3",
    "CP950",
    "ISO8859-13",
    "ISO-8859-13",
    "MULELAO-1",
    "MACTHAI",
    "ISO_8859-13",
    "CSUCS4",
    "L10",
    "CP850",
    "MACROMAN",
    "CP1257",
    "MACCYRILLIC",
    "CP1133",
    "MACICELAND",
    "CN-GB",
    "CSPTCP154",
    "CP1250",
    "ISO8859-7",
    "ISO-8859-7",
    "CSIBM866",
    "ISO_8859-7",
    "UNICODE-1-1",
    "LATIN3",
    "MACROMANIA",
    "CP367",
    "CSUNICODE",
    "CSUNICODE11",
    "MACINTOSH",
    "UCS-2",
    "ISO_8859-9:1989",
    "MACARABIC",
    "ISO_8859-4:1988",
    "ISO8859-10",
    "ISO-8859-10",
    "ISO-2022-CN-EXT",
    "CSHPROMAN8",
    "ISO_8859-10",
    "MS-HEBR",
    "KSC_5601",
    "ISO_8859-5:1988",
    "JAVA",
    "UTF-16",
    "UCS-4LE",
    "ISO-IR-203",
    "CSMACINTOSH",
    "ISO_8859-8:1988",
    "CSISO57GB1988",
    "ISO646-JP",
    "GB_1988-80",
    "CSISOLATIN3",
    "MACCENTRALEUROPE",
    "LATIN7",
    "CSISOLATINGREEK",
    "EUCJP",
    "EUC-JP",
    "KS_C_5601-1989",
    "GBK",
    "ISO-IR-100",
    "TIS620",
    "TIS-620",
    "X0212",
    "KOI8-T",
    "UCS-2LE",
    "CSISO159JISX02121990",
    "CSGB2312",
    "LATIN10",
    "JIS_C6226-1983",
    "SHIFT-JIS",
    "SHIFT_JIS",
    "UTF8",
    "UTF-8",
    "CSISOLATINHEBREW",
    "ISO-10646-UCS-4",
    "GREEK",
    "WCHAR_T",
    "ISO_8859-6:1987",
    "TCVN5712-1",
    "JIS_C6220-1969-RO",
    "ISO_8859-1:1987",
    "X0201",
    "ISO_8859-3:1988",
    "HZ-GB-2312",
    "UCS-4-INTERNAL",
    "IBM850",
    "BIGFIVE",
    "BIG-FIVE",
    "ISO-2022-KR",
    "ISO-10646-UCS-2",
    "UNICODE-1-1-UTF-7",
    "BIG5HKSCS",
    "BIG5-HKSCS",
    "UTF-16LE",
    "ASMO-708",
    "CSUNICODE11UTF7",
    "UCS-4BE",
    "TCVN-5712",
    "CSISO2022KR",
    "CSSHIFTJIS",
    "UCS-2-INTERNAL",
    "ISO_8859-2:1987",
    "MS-ARAB",
    "IBM367",
    "CSEUCKR",
    "EUCTW",
    "EUC-TW",
    "GB_2312-80",
    "X0208",
    "WINDOWS-1256",
    "UNICODEBIG",
    "KS_C_5601-1987",
    "WINDOWS-1251",
    "UCS-2BE",
    "WINDOWS-1254",
    "TIS620.2529-1",
    "CSISO58GB231280",
    "WINDOWS-1255",
    "ANSI_X3.4-1986",
    "WINDOWS-1258",
    "CSKSC56011987",
    "EXTENDED_UNIX_CODE_PACKED_FORMAT_FOR_JAPANESE",
    "TIS620-0",
    "GB18030",
    "CSPC862LATINHEBREW",
    "WINDOWS-1252",
    "ISO_8859-7:2003",
    "WINDOWS-936",
    "ANSI_X3.4-1968",
    "ISO-2022-JP-1",
    "UTF-32",
    "MS_KANJI",
    "IBM-CP1133",
    "ISO_8859-7:1987",
    "JIS0208",
    "UTF-16BE",
    "JOHAB",
    "UTF8-MAC",
    "UTF-8-MAC",
    "UNICODELITTLE",
    "MS-GREEK",
    "ISO-2022-JP-2",
    "MACGREEK",
    "WINDOWS-1253",
    "WINDOWS-874",
    "HEBREW",
    "UTF-32LE",
    "CSISO2022JP2",
    "UTF-7",
    "WINDOWS-1257",
    "CSHALFWIDTHKATAKANA",
    "WINDOWS-1250",
    "ISO-2022-JP",
    "NEXTSTEP",
    "CSISO2022JP",
    "TIS620.2533-1",
    "MACUKRAINE",
    "CSISO87JISX0208",
    "BIG5-HKSCS:2001",
    "BIG5-HKSCS:2004",
    "BIG5-HKSCS:1999",
    "UTF-32BE",
    "UCS-4-SWAPPED",
    "UCS-2-SWAPPED",
    "JIS_X0212",
    "KOI8-U",
    "KOI8-RU",
    "TCVN5712-1:1993",
    "CSEUCTW",
    "TIS620.2533-0",
    "MACHEBREW",
    "JIS_X0201",
    "WINBALTRIM",
    "JISX0201-1976",
    "MACTURKISH",
    "JIS_X0208",
    "JIS_X0212-1990",
    "MS-TURK",
    "CSPC850MULTILINGUAL",
    "JIS_X0208-1983",
    "JIS_X0212.1990-0",
    "JIS_X0208-1990",
    "CSEUCPKDFMTJAPANESE"
  };
#define stringpool ((const char *) &stringpool_contents)

static const struct alias aliases[] =
  {
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 16 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str8, ei_ascii},
    {-1}, {-1},
#line 286 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str11, ei_iso646_cn},
    {-1}, {-1}, {-1}, {-1},
#line 250 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str16, ei_tis620},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1},
#line 354 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str28, ei_local_char},
#line 13 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str29, ei_ascii},
#line 284 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str30, ei_iso646_cn},
#line 306 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str31, ei_sjis},
#line 229 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str32, ei_hp_roman8},
#line 210 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str33, ei_cp866},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 262 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str39, ei_iso646_jp},
    {-1},
#line 316 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str41, ei_euc_cn},
#line 315 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str42, ei_euc_cn},
    {-1},
#line 208 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str44, ei_cp866},
    {-1}, {-1}, {-1},
#line 54 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str48, ei_c99},
#line 22 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str49, ei_ascii},
    {-1}, {-1},
#line 93 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str52, ei_iso8859_5},
#line 328 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str53, ei_hz},
    {-1}, {-1}, {-1}, {-1},
#line 292 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str58, ei_isoir165},
    {-1}, {-1},
#line 110 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str61, ei_iso8859_7},
#line 137 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str62, ei_iso8859_10},
    {-1},
#line 297 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str64, ei_ksc5601},
#line 238 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str65, ei_pt154},
    {-1}, {-1},
#line 63 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str68, ei_iso8859_1},
    {-1},
#line 152 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str70, ei_iso8859_14},
    {-1}, {-1},
#line 281 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str73, ei_jisx0212},
#line 87 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str74, ei_iso8859_4},
    {-1}, {-1}, {-1}, {-1},
#line 291 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str79, ei_gb2312},
#line 348 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str80, ei_cp949},
    {-1}, {-1},
#line 129 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str83, ei_iso8859_9},
    {-1}, {-1},
#line 60 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str86, ei_iso8859_1},
#line 192 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str87, ei_cp1256},
#line 127 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str88, ei_iso8859_9},
    {-1},
#line 289 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str90, ei_gb2312},
    {-1},
#line 154 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str92, ei_iso8859_14},
    {-1},
#line 233 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str94, ei_georgian_academy},
    {-1}, {-1}, {-1}, {-1},
#line 177 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str99, ei_cp1251},
#line 166 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str100, ei_iso8859_16},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 71 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str107, ei_iso8859_2},
#line 105 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str108, ei_iso8859_6},
#line 97 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str109, ei_iso8859_6},
    {-1},
#line 186 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str111, ei_cp1254},
#line 98 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str112, ei_iso8859_6},
    {-1}, {-1},
#line 169 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str115, ei_iso8859_16},
#line 163 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str116, ei_iso8859_16},
    {-1}, {-1},
#line 164 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str119, ei_iso8859_16},
#line 65 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str120, ei_iso8859_1},
#line 56 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str121, ei_iso8859_1},
    {-1},
#line 206 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str123, ei_cp862},
#line 57 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str124, ei_iso8859_1},
    {-1}, {-1},
#line 142 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str127, ei_iso8859_11},
#line 140 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str128, ei_iso8859_11},
#line 189 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str129, ei_cp1255},
#line 165 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str130, ei_iso8859_16},
#line 141 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str131, ei_iso8859_11},
#line 89 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str132, ei_iso8859_4},
#line 82 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str133, ei_iso8859_4},
#line 204 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str134, ei_cp862},
    {-1},
#line 83 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str136, ei_iso8859_4},
#line 321 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str137, ei_cp936},
#line 351 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str138, ei_johab},
#line 156 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str139, ei_iso8859_14},
#line 149 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str140, ei_iso8859_14},
    {-1}, {-1},
#line 150 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str143, ei_iso8859_14},
#line 131 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str144, ei_iso8859_9},
#line 124 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str145, ei_iso8859_9},
#line 236 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str146, ei_pt154},
#line 198 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str147, ei_cp1258},
#line 125 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str148, ei_iso8859_9},
    {-1},
#line 96 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str150, ei_iso8859_5},
#line 90 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str151, ei_iso8859_5},
#line 234 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str152, ei_georgian_ps},
    {-1},
#line 91 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str154, ei_iso8859_5},
#line 94 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str155, ei_iso8859_5},
    {-1},
#line 162 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str157, ei_iso8859_15},
#line 157 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str158, ei_iso8859_15},
#line 136 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str159, ei_iso8859_10},
    {-1},
#line 158 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str161, ei_iso8859_15},
    {-1},
#line 69 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str163, ei_iso8859_2},
    {-1}, {-1},
#line 151 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str166, ei_iso8859_14},
#line 79 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str167, ei_iso8859_3},
#line 123 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str168, ei_iso8859_8},
#line 117 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str169, ei_iso8859_8},
    {-1},
#line 62 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str171, ei_iso8859_1},
#line 118 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str172, ei_iso8859_8},
    {-1},
#line 214 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str174, ei_mac_roman},
#line 159 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str175, ei_iso8859_15},
#line 239 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str176, ei_pt154},
#line 180 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str177, ei_cp1252},
#line 145 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str178, ei_iso8859_13},
    {-1},
#line 253 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str180, ei_viscii},
#line 120 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str181, ei_iso8859_8},
    {-1},
#line 86 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str183, ei_iso8859_4},
#line 182 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str184, ei_cp1252},
    {-1}, {-1},
#line 77 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str187, ei_iso8859_3},
#line 147 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str188, ei_iso8859_13},
#line 209 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str189, ei_cp866},
    {-1},
#line 255 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str191, ei_viscii},
    {-1}, {-1},
#line 251 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str194, ei_cp874},
    {-1},
#line 161 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str196, ei_iso8859_15},
#line 138 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str197, ei_iso8859_10},
#line 73 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str198, ei_iso8859_2},
#line 66 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str199, ei_iso8859_2},
    {-1},
#line 128 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str201, ei_iso8859_9},
#line 67 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str202, ei_iso8859_2},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 155 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str208, ei_iso8859_14},
#line 64 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str209, ei_iso8859_1},
    {-1}, {-1},
#line 176 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str212, ei_cp1250},
    {-1}, {-1}, {-1},
#line 103 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str216, ei_iso8859_6},
#line 333 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str217, ei_ces_big5},
#line 334 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str218, ei_ces_big5},
#line 153 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str219, ei_iso8859_14},
    {-1},
#line 88 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str221, ei_iso8859_4},
#line 95 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str222, ei_iso8859_5},
    {-1},
#line 293 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str224, ei_isoir165},
    {-1},
#line 104 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str226, ei_iso8859_6},
#line 309 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str227, ei_cp932},
#line 338 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str228, ei_ces_big5},
#line 337 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str229, ei_ces_big5},
#line 101 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str230, ei_iso8859_6},
#line 61 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str231, ei_iso8859_1},
    {-1}, {-1},
#line 228 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str234, ei_hp_roman8},
    {-1},
#line 346 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str236, ei_euc_kr},
#line 345 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str237, ei_euc_kr},
    {-1},
#line 130 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str239, ei_iso8859_9},
#line 170 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str240, ei_koi8_r},
    {-1}, {-1},
#line 299 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str243, ei_ksc5601},
    {-1},
#line 112 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str245, ei_iso8859_7},
    {-1},
#line 237 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str247, ei_pt154},
    {-1},
#line 70 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str249, ei_iso8859_2},
#line 171 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str250, ei_koi8_r},
    {-1}, {-1},
#line 256 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str253, ei_tcvn},
    {-1},
#line 232 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str255, ei_armscii_8},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 317 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str261, ei_euc_cn},
    {-1}, {-1}, {-1}, {-1},
#line 111 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str266, ei_iso8859_7},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 264 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str275, ei_iso646_jp},
    {-1}, {-1}, {-1},
#line 205 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str279, ei_cp862},
    {-1},
#line 322 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str281, ei_cp936},
#line 285 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str282, ei_iso646_cn},
    {-1},
#line 21 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str284, ei_ascii},
    {-1}, {-1},
#line 72 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str287, ei_iso8859_2},
#line 227 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str288, ei_hp_roman8},
#line 135 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str289, ei_iso8859_10},
#line 325 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str290, ei_iso2022_cn},
#line 274 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str291, ei_jisx0208},
#line 85 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str292, ei_iso8859_4},
    {-1},
#line 349 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str294, ei_cp949},
    {-1},
#line 263 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str296, ei_iso646_jp},
#line 183 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str297, ei_cp1253},
    {-1}, {-1}, {-1},
#line 15 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str301, ei_ascii},
    {-1},
#line 202 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str303, ei_cp850},
#line 134 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str304, ei_iso8859_10},
#line 218 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str305, ei_mac_croatian},
    {-1}, {-1},
#line 326 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str308, ei_iso2022_cn},
    {-1},
#line 179 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str310, ei_cp1251},
#line 36 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str311, ei_ucs4},
#line 14 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str312, ei_ascii},
#line 100 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str313, ei_iso8859_6},
#line 12 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str314, ei_ascii},
#line 113 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str315, ei_iso8859_7},
    {-1},
#line 254 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str317, ei_viscii},
#line 81 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str318, ei_iso8859_3},
#line 74 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str319, ei_iso8859_3},
    {-1}, {-1},
#line 75 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str322, ei_iso8859_3},
#line 339 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str323, ei_cp950},
    {-1},
#line 148 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str325, ei_iso8859_13},
#line 143 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str326, ei_iso8859_13},
#line 241 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str327, ei_mulelao},
#line 226 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str328, ei_mac_thai},
#line 144 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str329, ei_iso8859_13},
#line 38 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str330, ei_ucs4},
    {-1}, {-1},
#line 168 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str333, ei_iso8859_16},
    {-1},
#line 200 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str335, ei_cp850},
    {-1}, {-1},
#line 212 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str338, ei_mac_roman},
#line 195 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str339, ei_cp1257},
    {-1},
#line 220 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str341, ei_mac_cyrillic},
#line 242 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str342, ei_cp1133},
    {-1}, {-1}, {-1},
#line 217 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str346, ei_mac_iceland},
#line 318 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str347, ei_euc_cn},
    {-1}, {-1}, {-1},
#line 240 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str351, ei_pt154},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 174 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str357, ei_cp1250},
    {-1}, {-1},
#line 116 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str360, ei_iso8859_7},
#line 106 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str361, ei_iso8859_7},
#line 211 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str362, ei_cp866},
    {-1},
#line 107 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str364, ei_iso8859_7},
#line 32 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str365, ei_ucs2be},
    {-1}, {-1}, {-1},
#line 78 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str369, ei_iso8859_3},
#line 219 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str370, ei_mac_romania},
#line 19 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str371, ei_ascii},
#line 29 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str372, ei_ucs2},
    {-1},
#line 33 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str374, ei_ucs2be},
#line 213 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str375, ei_mac_roman},
    {-1},
#line 27 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str377, ei_ucs2},
#line 126 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str378, ei_iso8859_9},
    {-1}, {-1},
#line 225 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str381, ei_mac_arabic},
    {-1}, {-1},
#line 84 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str384, ei_iso8859_4},
#line 139 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str385, ei_iso8859_10},
#line 132 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str386, ei_iso8859_10},
#line 327 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str387, ei_iso2022_cn_ext},
#line 230 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str388, ei_hp_roman8},
#line 133 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str389, ei_iso8859_10},
    {-1},
#line 191 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str391, ei_cp1255},
#line 294 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str392, ei_ksc5601},
#line 92 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str393, ei_iso8859_5},
#line 55 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str394, ei_java},
    {-1},
#line 41 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str396, ei_utf16},
#line 40 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str397, ei_ucs4le},
    {-1}, {-1},
#line 160 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str400, ei_iso8859_15},
#line 215 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str401, ei_mac_roman},
#line 119 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str402, ei_iso8859_8},
#line 287 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str403, ei_iso646_cn},
    {-1},
#line 261 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str405, ei_iso646_jp},
#line 283 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str406, ei_iso646_cn},
#line 80 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str407, ei_iso8859_3},
    {-1},
#line 216 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str409, ei_mac_centraleurope},
    {-1},
#line 146 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str411, ei_iso8859_13},
    {-1}, {-1},
#line 115 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str414, ei_iso8859_7},
    {-1},
#line 301 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str416, ei_euc_jp},
#line 300 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str417, ei_euc_jp},
    {-1},
#line 296 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str419, ei_ksc5601},
#line 320 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str420, ei_ces_gbk},
#line 59 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str421, ei_iso8859_1},
    {-1},
#line 245 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str423, ei_tis620},
#line 244 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str424, ei_tis620},
#line 280 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str425, ei_jisx0212},
#line 235 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str426, ei_koi8_t},
    {-1}, {-1}, {-1},
#line 34 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str430, ei_ucs2le},
#line 282 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str431, ei_jisx0212},
    {-1}, {-1},
#line 319 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str434, ei_euc_cn},
    {-1},
#line 167 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str436, ei_iso8859_16},
    {-1}, {-1}, {-1},
#line 275 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str440, ei_jisx0208},
    {-1}, {-1}, {-1},
#line 305 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str444, ei_sjis},
    {-1}, {-1},
#line 304 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str447, ei_sjis},
#line 24 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str448, ei_utf8},
#line 23 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str449, ei_utf8},
    {-1},
#line 122 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str451, ei_iso8859_8},
    {-1},
#line 37 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str453, ei_ucs4},
    {-1}, {-1}, {-1}, {-1},
#line 114 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str458, ei_iso8859_7},
    {-1},
#line 355 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str460, ei_local_wchar_t},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 99 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str468, ei_iso8859_6},
#line 258 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str469, ei_tcvn},
    {-1}, {-1}, {-1},
#line 260 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str473, ei_iso646_jp},
#line 58 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str474, ei_iso8859_1},
    {-1},
#line 267 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str476, ei_jisx0201},
#line 76 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str477, ei_iso8859_3},
#line 329 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str478, ei_hz},
#line 52 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str479, ei_ucs4internal},
#line 201 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str480, ei_cp850},
#line 336 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str481, ei_ces_big5},
#line 335 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str482, ei_ces_big5},
    {-1}, {-1},
#line 352 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str485, ei_iso2022_kr},
#line 28 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str486, ei_ucs2},
    {-1}, {-1}, {-1}, {-1},
#line 48 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str491, ei_utf7},
#line 343 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str492, ei_big5hkscs2004},
#line 342 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str493, ei_big5hkscs2004},
#line 43 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str494, ei_utf16le},
    {-1}, {-1},
#line 102 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str497, ei_iso8859_6},
#line 49 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str498, ei_utf7},
#line 39 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str499, ei_ucs4be},
    {-1},
#line 257 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str501, ei_tcvn},
    {-1},
#line 353 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str503, ei_iso2022_kr},
    {-1},
#line 308 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str505, ei_sjis},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 50 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str512, ei_ucs2internal},
#line 68 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str513, ei_iso8859_2},
#line 194 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str514, ei_cp1256},
    {-1},
#line 20 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str516, ei_ascii},
    {-1}, {-1}, {-1},
#line 347 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str520, ei_euc_kr},
#line 331 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str521, ei_euc_tw},
#line 330 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str522, ei_euc_tw},
#line 288 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str523, ei_gb2312},
#line 273 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str524, ei_jisx0208},
#line 193 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str525, ei_cp1256},
#line 31 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str526, ei_ucs2be},
#line 295 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str527, ei_ksc5601},
    {-1}, {-1}, {-1},
#line 178 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str531, ei_cp1251},
#line 30 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str532, ei_ucs2be},
    {-1}, {-1}, {-1}, {-1},
#line 187 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str537, ei_cp1254},
#line 247 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str538, ei_tis620},
    {-1},
#line 290 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str540, ei_gb2312},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 190 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str546, ei_cp1255},
    {-1}, {-1}, {-1}, {-1},
#line 18 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str551, ei_ascii},
    {-1}, {-1}, {-1},
#line 199 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str555, ei_cp1258},
#line 298 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str556, ei_ksc5601},
    {-1},
#line 302 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str558, ei_euc_jp},
    {-1},
#line 246 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str560, ei_tis620},
    {-1},
#line 324 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str562, ei_gb18030},
    {-1}, {-1}, {-1}, {-1},
#line 207 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str567, ei_cp862},
    {-1}, {-1},
#line 181 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str570, ei_cp1252},
    {-1}, {-1},
#line 109 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str573, ei_iso8859_7},
    {-1},
#line 323 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str575, ei_cp936},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 17 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str581, ei_ascii},
    {-1},
#line 312 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str583, ei_iso2022_jp1},
    {-1},
#line 44 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str585, ei_utf32},
    {-1},
#line 307 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str587, ei_sjis},
    {-1},
#line 243 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str589, ei_cp1133},
    {-1}, {-1}, {-1}, {-1},
#line 108 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str594, ei_iso8859_7},
#line 272 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str595, ei_jisx0208},
#line 42 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str596, ei_utf16be},
    {-1}, {-1},
#line 350 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str599, ei_johab},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 26 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str608, ei_utf8mac},
#line 25 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str609, ei_utf8mac},
    {-1}, {-1}, {-1},
#line 35 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str613, ei_ucs2le},
#line 185 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str614, ei_cp1253},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 313 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str622, ei_iso2022_jp2},
#line 222 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str623, ei_mac_greek},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 184 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str630, ei_cp1253},
    {-1},
#line 252 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str632, ei_cp874},
#line 121 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str633, ei_iso8859_8},
    {-1}, {-1}, {-1}, {-1},
#line 46 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str638, ei_utf32le},
#line 314 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str639, ei_iso2022_jp2},
    {-1},
#line 47 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str641, ei_utf7},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 196 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str651, ei_cp1257},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 268 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str658, ei_jisx0201},
    {-1},
#line 175 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str660, ei_cp1250},
    {-1}, {-1}, {-1}, {-1},
#line 310 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str665, ei_iso2022_jp},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 231 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str680, ei_nextstep},
    {-1}, {-1},
#line 311 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str683, ei_iso2022_jp},
    {-1},
#line 249 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str685, ei_tis620},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 221 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str700, ei_mac_ukraine},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1},
#line 276 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str711, ei_jisx0208},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 341 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str720, ei_big5hkscs2001},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 344 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str726, ei_big5hkscs2004},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 340 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str732, ei_big5hkscs1999},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 45 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str740, ei_utf32be},
    {-1}, {-1}, {-1}, {-1},
#line 53 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str745, ei_ucs4swapped},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 51 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str778, ei_ucs2swapped},
    {-1},
#line 277 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str780, ei_jisx0212},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 172 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str786, ei_koi8_u},
#line 173 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str787, ei_koi8_ru},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1},
#line 259 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str798, ei_tcvn},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 332 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str805, ei_euc_tw},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 248 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str814, ei_tis620},
    {-1},
#line 224 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str816, ei_mac_hebrew},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 265 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str831, ei_jisx0201},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1},
#line 197 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str844, ei_cp1257},
    {-1}, {-1}, {-1}, {-1}, {-1},
#line 266 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str850, ei_jisx0201},
    {-1}, {-1},
#line 223 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str853, ei_mac_turkish},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 269 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str879, ei_jisx0208},
    {-1},
#line 279 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str881, ei_jisx0212},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 188 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str934, ei_cp1254},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 203 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str952, ei_cp850},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1},
#line 270 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str965, ei_jisx0208},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
#line 278 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str991, ei_jisx0212},
    {-1}, {-1}, {-1},
#line 271 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str995, ei_jisx0208},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1}, {-1},
    {-1},
#line 303 "lib/aliases.gperf"
    {(int)(long)&((struct stringpool_t *)0)->stringpool_str1033, ei_euc_jp}
  };

const struct alias *
aliases_lookup (register const char *str, register unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = aliases_hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int o = aliases[key].name;
          if (o >= 0)
            {
              register const char *s = o + stringpool;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &aliases[key];
            }
        }
    }
  return 0;
}
