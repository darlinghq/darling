#!/bin/bash -e -x

if [[ ${UID} -eq 0 ]]; then
	chown -hR root:wheel ${DSTROOT}/usr
fi

if [[ "${ACTION}" == "install" ]]; then
  ln -s libiconv.2.4.0.dylib   ${DSTROOT}/usr/lib/libiconv.dylib
  ln -s libiconv.2.dylib       ${DSTROOT}/usr/lib/libiconv.2.4.0.dylib
  ln -s libcharset.1.0.0.dylib ${DSTROOT}/usr/lib/libcharset.dylib
  ln -s libcharset.1.dylib     ${DSTROOT}/usr/lib/libcharset.1.0.0.dylib
fi

if [[ "${ACTION}" == "install" || "${ACTION}" == "installapi" ]]; then
  ln -s libiconv.2.4.0.tbd   ${DSTROOT}/usr/lib/libiconv.tbd
  ln -s libiconv.2.tbd       ${DSTROOT}/usr/lib/libiconv.2.4.0.tbd
  ln -s libcharset.1.0.0.tbd ${DSTROOT}/usr/lib/libcharset.tbd
  ln -s libcharset.1.tbd     ${DSTROOT}/usr/lib/libcharset.1.0.0.tbd
fi

find ${DSTROOT}/usr -type f -perm +u+w -print0 | xargs -t -0 chmod -h u-w
