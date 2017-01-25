project(CommonCrypto)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -fblocks -w") 
add_definitions(-D_APPLE_COMMON_CRYPTO_ -DLTC_SHA224 -DLTC_SHA384 -DSHA256_USE_ASSEMBLY -DPRIVATE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source/API)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source/libtomcrypt/src/headers)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source/libtomcrypt/src/ciphers/aesedpport/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source/libtomcrypt/src/hashes/skein/Optimized_64bit/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source/libtomcrypt/src/ciphers/RC4/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source/libtomcrypt/src/hashes/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source/libtomcrypt/src/padding/pkcs7)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/CommonCrypto)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source/ccUtilities)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source/CommonCryptoSPI)
include_directories(${CMAKE_SOURCE_DIR}/src/external/libdispatch)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/darwin)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/locale)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/locale/FreeBSD)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/stdtime/FreeBSD)
include_directories(${CMAKE_SOURCE_DIR}/src/kernel/libsyscall/wrappers)

set(cc_SRCS
	Source/Digest/md2_dgst.c
	Source/Digest/md4_dgst.c
	Source/Digest/md5_dgst.c
	# TODO - Too much work :-/
	Source/Digest/sha1.c
	Source/Digest/sha1-586.S
	Source/Digest/sha1-x86_64.S
	Source/Digest/sha2.c
	Source/Digest/sha256.S
	Source/Digest/sha256_nossse3.S
	#Source/Digest/sha1edpBigEndian.S

	#Source/RC2/ccRC2.c
	#Source/RC2/rc2.c
	#Source/GladmanAES/aescrypt.c
	#Source/GladmanAES/aeskey.c
	#Source/GladmanAES/aestab.c
	#Source/GladmanAES/ccNewGladman.c
	#Source/CAST/ccCast.c
	#Source/CAST/c_ecb.c
	#Source/CAST/c_enc.c
	#Source/CAST/c_skey.c
	#Source/ccUtilities/ccdebug.c
	#Source/ccOpenssl/des_enc.c
	#Source/ccOpenssl/opensslDES.c
	#Source/ccOpenssl/set_key.c

	#Source/API/CommonCMAC.c
	#Source/API/CommonCryptor.c
	Source/API/CommonDigest.c
	#Source/API/CommonHMAC.c
	#Source/API/CommonKeyDerivation.c
	#Source/API/CommonRandom.c
	#Source/API/CommonSymmetricKeywrap.c
	Source/libtomcrypt/src/misc/base64/base64_decode.c
	Source/libtomcrypt/src/misc/base64/base64_encode.c
	Source/libtomcrypt/src/misc/crypt/crypt_argchk.c
	Source/libtomcrypt/src/misc/crypt/crypt_hash_descriptor.c
	Source/libtomcrypt/src/misc/crypt/crypt_register_hash.c
	Source/libtomcrypt/src/misc/zeromem.c
	Source/libtomcrypt/src/hashes/md2.c
	Source/libtomcrypt/src/hashes/md4.c
	Source/libtomcrypt/src/hashes/md5.c
	Source/libtomcrypt/src/hashes/rmd128.c
	Source/libtomcrypt/src/hashes/rmd160.c
	Source/libtomcrypt/src/hashes/rmd256.c
	Source/libtomcrypt/src/hashes/rmd320.c
	Source/libtomcrypt/src/hashes/sha1.c
	Source/libtomcrypt/src/hashes/skein_ltc.c
	Source/libtomcrypt/src/hashes/skein/Optimized_64bit/SHA3api_ref.c
	Source/libtomcrypt/src/hashes/skein/Optimized_64bit/skein_block.c
	Source/libtomcrypt/src/hashes/skein/Optimized_64bit/skein.c
	Source/libtomcrypt/src/hashes/skein/Optimized_64bit/skein_debug.c
	Source/libtomcrypt/src/hashes/skein/Optimized_64bit/skein_dropin.c
	Source/libtomcrypt/src/hashes/sha2/sha256.c
	Source/libtomcrypt/src/hashes/sha2/sha512.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/libcommonCrypto.dylib")
add_darling_library(CommonCrypto SHARED ${cc_SRCS})
target_link_libraries(CommonCrypto system_c system_m system_dyld system_malloc)
set_target_properties(CommonCrypto PROPERTIES OUTPUT_NAME "commonCrypto")
make_fat(CommonCrypto)

install(TARGETS CommonCrypto DESTINATION libexec/darling/usr/lib)
