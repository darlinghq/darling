#!/usr/bin/env bash

export DARLING_SRC=`pwd`
VERSION=1.0

ANSI_RESET="\e[0m"
ANSI_COLORS="${ANSI_RESET}\e[1m\e[32m"
ANSI_ERROR="${ANSI_RESET}\e[31m"
ANSI_QUIT="${ANSI_RESET}\e[1m\e[91m"

# Make sure we are root
if [ `whoami` != "root" ]; then
  echo -e "${ANSI_QUIT}dkms.sh must be run as root${ANSI_RESET}"
  exit 1
fi

# Test if darling-mach is already installed
modinfo darling-mach &> /dev/null

if (( $? == 0 )); then
  echo -e "${ANSI_COLORS}Uninstalling all versions...${ANSI_ERROR}"
  dkms remove darling-mach/$VERSION --all > /dev/null
fi

if [ -d /var/lib/dkms/darling-mach ]; then
  echo -e "${ANSI_COLORS}Removing darling-mach DKMS build folder...${ANSI_ERROR}"
  rm -rf /var/lib/dkms/darling-mach > /dev/null
fi


if [ -d /usr/src/darling-mach-* ]; then
  echo -e "${ANSI_COLORS}Removing darling-mach DKMS source folder...${ANSI_ERROR}"
  rm -rf /usr/src/darling-mach-* > /dev/null
fi

# Build it from scratch and install

echo -e "${ANSI_COLORS}Registering darling-mach with DKMS...${ANSI_ERROR}"
dkms add . > /dev/null

if (( $? != 0 )); then
  echo -e "${ANSI_QUIT}Registering darling-mach failed${ANSI_RESET}"
  exit $?
fi

echo -e "${ANSI_COLORS}Building darling-mach...${ANSI_ERROR}"
dkms build darling-mach -v $VERSION > /dev/null

if (( $? != 0 )); then
  echo -e "${ANSI_QUIT}Building darling-mach failed${ANSI_RESET}"
  exit $?
fi

echo -e "${ANSI_COLORS}Installing darling-mach...${ANSI_ERROR}"
dkms install darling-mach -v $VERSION  > /dev/null

if (( $? != 0 )); then
  echo -e "${ANSI_QUIT}Installing darling-mach failed${ANSI_RESET}"
  exit $?
fi

echo -en ${ANSI_RESET}
