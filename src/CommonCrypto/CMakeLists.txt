project(CommonCrypto)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -fblocks -w") 
add_definitions(-D_APPLE_COMMON_CRYPTO_)

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
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libdispatch)

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
	#Source/API/CommonDigest.c
	#Source/API/CommonHMAC.c
	#Source/API/CommonKeyDerivation.c
	#Source/API/CommonRandom.c
	#Source/API/CommonSymmetricKeywrap.c
	Source/libtomcrypt/src/misc/base64/base64_decode.c
	Source/libtomcrypt/src/misc/base64/base64_encode.c
	Source/libtomcrypt/src/misc/crypt/crypt_argchk.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/libcommonCrypto.dylib")
add_darling_library(CommonCrypto SHARED ${cc_SRCS})
target_link_libraries(CommonCrypto system_c system_m system_dyld)
set_target_properties(CommonCrypto PROPERTIES OUTPUT_NAME "commonCrypto")

install(TARGETS CommonCrypto DESTINATION libexec/darling/usr/lib)
