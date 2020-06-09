#!/bin/bash -x

if [ $# -ne 5 ]; then
    echo "Usage: $0 <archs> <variants> <outputdir> <sdkroot> <srcroot>" 1>&2
    exit 1
fi

shopt -s nullglob

ARCHS="$1"
VARIANTS="$2"
OUTPUTDIR="$3"
SDKROOT="$4"
SRCROOT="$5"

LIBSYS="${SDKROOT}/${SDK_INSTALL_ROOT}/usr/local/lib/system"
LSYS="${SDKROOT}/${SDK_INSTALL_ROOT}/usr/lib/system"

mkdir -p "${OUTPUTDIR}"

for arch in ${ARCHS}; do
    for variant in ${VARIANTS}; do
	OUTPUTFILE="${OUTPUTDIR}/linker_arguments.${arch}.${variant}.txt"
	OUTPUTCONFIG="${OUTPUTDIR}/config.${arch}.${variant}.h"
	TEMPDIR="${OUTPUTDIR}/linker_arguments_tmp"

	SDK_INSTALL_VARIANT="${DRIVERKIT:+_driverkit}"

	mkdir -p "${TEMPDIR}"

	ACTUALLIBS="${TEMPDIR}/${arch}.${variant}.actuallibs"
	ALLLIBS="${TEMPDIR}/${arch}.${variant}.alllibs"
	FROMUSRLIBSYSTEM="${TEMPDIR}/${arch}.${variant}.fromusrlibsystem"
	FROMUSRLOCALLIBSYSTEM="${TEMPDIR}/${arch}.${variant}.fromusrlocallibsystem"
	INUSRLIBSYSTEM="${TEMPDIR}/${arch}.${variant}.inusrlibsystem"
	INUSRLOCALLIBSYSTEM="${TEMPDIR}/${arch}.${variant}.inusrlocallibsystem"
	MISSINGLIBS="${TEMPDIR}/${arch}.${variant}.missinglibs"
	POSSIBLEUSRLOCALLIBSYSTEM="${TEMPDIR}/${arch}.${variant}.possibleusrlocallibsystem"
	OPTIONALLIBS="${SRCROOT}/optionallibs${SDK_INSTALL_VARIANT}"
	REQUIREDLIBS="${SRCROOT}/requiredlibs${SDK_INSTALL_VARIANT}"

	if [ ${variant} = "normal" ]; then
	    SUFFIX=""
	else
	    SUFFIX="_${variant}"
	fi

	rm -f "${OUTPUTFILE}"

	cat ${REQUIREDLIBS} ${OPTIONALLIBS} | sort > ${ALLLIBS}

	# Strip away variant information to do presence checks
	# non-normal variants can always link against normal clients
	cd ${LSYS} && ls lib*.{dylib,tbd,api,spi} | sed -E -e 's/_(debug|profile|static)\././' | while read l; do
		if [ "${l: -6}" == ".dylib" ]; then
			xcrun -sdk "${SDKROOT}" lipo "${LSYS}/${l}" -verify_arch "${arch}" 2>/dev/null
		else
			xcrun -sdk "${SDKROOT}" tapi archive "${LSYS}/${l}" --verify-arch "${arch}" 2>/dev/null
		fi
		if [ $? -eq 0 ]; then
			  echo "${l}"
		fi
	done | sed -E -e 's/^lib//' -e 's/\..*$$//'  | sort -u > ${INUSRLIBSYSTEM}
	[ -d ${LIBSYS} ] && cd ${LIBSYS} && ls lib*.a | sed -E -e 's/_(debug|profile|static)\././' | while read l; do
	    xcrun -sdk "${SDKROOT}" lipo "${LIBSYS}/${l}" -verify_arch "${arch}" 2>/dev/null
	    if [ $? -eq 0 ]; then
		echo "${l}"
	    fi
	done | sed -E -e 's/^lib//' -e 's/\..*$$//' | sort -u > ${INUSRLOCALLIBSYSTEM}

	while read line ; do
	    for lib in ${line} ; do
		egrep "^${lib}$" ${INUSRLIBSYSTEM} && break
	    done
	done < ${ALLLIBS} > ${FROMUSRLIBSYSTEM}

	while read line ; do
	    for lib in ${line} ; do
		egrep -q "^${lib}$" ${INUSRLIBSYSTEM} && break
		egrep "^${lib}$" ${INUSRLOCALLIBSYSTEM} && break
	    done
	done < ${ALLLIBS} > ${POSSIBLEUSRLOCALLIBSYSTEM}

	comm -13 ${FROMUSRLIBSYSTEM} ${POSSIBLEUSRLOCALLIBSYSTEM} > ${FROMUSRLOCALLIBSYSTEM}
	cat ${FROMUSRLIBSYSTEM} ${FROMUSRLOCALLIBSYSTEM} | sort > ${ACTUALLIBS}

	while read line ; do
	    found=0
	    for lib in ${line} ; do
		egrep -q "^${lib}$" ${ACTUALLIBS} && found=1
	    done
	    [[ $found == 0 ]] && echo ${line}
	done < ${REQUIREDLIBS} > ${MISSINGLIBS}

	if [ -s ${MISSINGLIBS} ]; then
	    echo '*** missing required libs ***'
	    cat ${MISSINGLIBS}
	    exit 1
	fi

	cat ${FROMUSRLIBSYSTEM} | while read lib; do
		echo "#define HAVE_"$(echo "${lib/_sim/}" | tr 'a-z' 'A-Z')" 1"
	done >> "${OUTPUTCONFIG}"

	cat ${FROMUSRLIBSYSTEM} | while read lib; do
	    echo "-Wl,-reexport-l${lib}"
	done >> "${OUTPUTFILE}"

	# Allow static archives to degrade to their non-variant versions
	cat ${FROMUSRLOCALLIBSYSTEM} | while read lib; do
	    LPATH="${LIBSYS}/lib${lib}${SUFFIX}.a"
	    xcrun -sdk "${SDKROOT}" lipo "${LPATH}" -verify_arch "${arch}" 2>/dev/null
	    if [ $? -ne 0 ]; then
		LPATH="${LIBSYS}/lib${lib}.a"
	    fi
	    echo "-Wl,-force_load,${LPATH}"
	done >> "${OUTPUTFILE}"	    

	echo "Final contents of ${OUTPUTFILE}:"
	cat "${OUTPUTFILE}"
    done
done

MARKERFILE="${OUTPUTDIR}/linker_arguments_changed.c"

for arch in ${ARCHS}; do
    for variant in ${VARIANTS}; do
	OUTPUTFILE="${OUTPUTDIR}/linker_arguments.${arch}.${variant}.txt"
	cat "${OUTPUTFILE}"
    done
done | shasum | sed 's,^,// ,' > "${MARKERFILE}"

exit 0
