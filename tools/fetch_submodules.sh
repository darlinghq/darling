#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
EXTERNAL_DIR="$PROJECT_ROOT/src/external"

if [ -z "$1" ]; then
    echo "Usage: $0 <component>"
    echo ""
    echo "Available components:"
    echo "  core           - Core system (base components)"
    echo "  system         - System components (includes core)"
    echo "  cli            - CLI tools (includes system)"
    echo "  cli_dev        - CLI development tools (includes cli)"
    echo "  cli_extra      - Extra CLI tools (includes cli)"
    echo "  gui            - GUI support (includes system)"
    echo "  gui_frameworks - GUI frameworks (includes gui)"
    echo "  gui_stubs      - GUI stubs (includes gui)"
    echo "  dev_gui_common - Common GUI development (includes system)"
    echo "  dev_gui_frameworks_common - GUI frameworks dev (includes dev_gui_common)"
    echo "  dev_gui_stubs_common - GUI stubs dev (includes dev_gui_common)"
    echo "  jsc            - JavaScriptCore (includes system)"
    echo "  webkit         - WebKit (includes dev_gui_frameworks_common, dev_gui_stubs_common)"
    echo "  iokitd         - IOKit daemon (includes system)"
    echo "  python         - Python (includes system)"
    echo "  ruby           - Ruby (includes system)"
    echo "  perl           - Perl (includes system)"
    echo "  stock          - Default build (includes cli, python, ruby, perl, dev_gui_common, dev_gui_frameworks_common, dev_gui_stubs_common, gui_frameworks, gui_stubs)"
    echo "  all            - All components"
    exit 1
fi

COMPONENT="$1"

declare -A COMPONENT_SUBMODULES

COMPONENT_SUBMODULES[core]="
librpcsvc
cfnetwork
security
mDNSResponder
AvailabilityVersions
IOKitUser
architecture
cctools-port
darlingserver
bootstrap_cmds
libkqueue
xnu
libplatform
libpthread
libtrace
libc
libstdcxx
dyld
libxpc
copyfile
Libinfo
compiler-rt
libnotify
syslog
libclosure
keymgr
libunwind
libmalloc
coretls
removefile
corecrypto
commoncrypto
configd
libsystem
objc4
corefoundation
icu
csu
libresolv
usertemplate
bsm
zlib
libedit
ncurses
vim
libcxxabi
libcxx
libdispatch
cctools
foundation
cocotron
DirectoryService
"

COMPONENT_SUBMODULES[system]="
libiconv
bzip2
libressl-2.8.3
libarchive
liblzma
OpenDirectory
openssl_certificates
bash
zsh
tcsh
files
crontabs
swift
"

COMPONENT_SUBMODULES[cli]="
darling-dmg
libutil
libxslt
openssl
nghttp2
passwordserver_sasl
curl
OpenLDAP
pcre
libtelnet
remote_cmds
IOKitTools
apr
file
shell_cmds
file_cmds
text_cmds
adv_cmds
network_cmds
system_cmds
mail_cmds
doc_cmds
basic_cmds
misc_cmds
patch_cmds
less
grep
awk
groff
nano
man
bc
zip
installer
bind9
netcat
gpatch
gnudiff
openssh
top
screen
rsync
DSTools
lzfse
libressl-2.2.9
libressl-2.5.5
libressl-2.6.5
dtrace
libauto
"

COMPONENT_SUBMODULES[cli_dev]="
libgmalloc
"

COMPONENT_SUBMODULES[cli_dev_gui_common]="
libxml2
sqlite
openpam
energytrace
IOKitUser
xar
Heimdal
fmdb
libnetwork
MITKerberosShim
"

COMPONENT_SUBMODULES[iokitd]="
iokitd
"

COMPONENT_SUBMODULES[dev_gui_common]="
openjdk
"

COMPONENT_SUBMODULES[cli_extra]="
gnutar
"

COMPONENT_SUBMODULES[gui]="
cups
dbuskit
"

COMPONENT_SUBMODULES[python]="
python_modules
python
BerkeleyDB
expat
libffi
"

COMPONENT_SUBMODULES[ruby]="
ruby
libffi
"

COMPONENT_SUBMODULES[perl]="
perl
"

COMPONENT_SUBMODULES[jsc_webkit_common]="
WTF
bmalloc
"

COMPONENT_SUBMODULES[jsc]="
JavaScriptCore
"

COMPONENT_SUBMODULES[dev_gui_frameworks_common]="
metal
"

COMPONENT_SUBMODULES[dev_gui_stubs_common]="
"

COMPONENT_SUBMODULES[cli_dev_gui_stubs]="
"

get_deps() {
    local comp="$1"
    local visited=()
    local queue=("$comp")
    
    while [ ${#queue[@]} -gt 0 ]; do
        local current="${queue[0]}"
        queue=("${queue[@]:1}")
        
        local found=0
        for v in "${visited[@]}"; do
            if [ "$v" = "$current" ]; then
                found=1
                break
            fi
        done
        if [ $found -eq 1 ]; then
            continue
        fi
        visited+=("$current")
        
        case "$current" in
            system|python|ruby|perl)
                queue+=("core")
                ;;
            cli)
                queue+=("system")
                ;;
            cli_dev)
                queue+=("cli")
                queue+=("python")
                queue+=("ruby")
                queue+=("perl")
                queue+=("dev_gui_common")
                queue+=("dev_gui_frameworks_common")
                queue+=("dev_gui_stubs_common")
                ;;
            cli_extra)
                queue+=("cli")
                ;;
            gui|iokitd|dev_gui_common|jsc)
                queue+=("system")
                ;;
            gui_frameworks)
                queue+=("gui")
                queue+=("dev_gui_frameworks_common")
                ;;
            gui_stubs)
                queue+=("gui")
                queue+=("dev_gui_stubs_common")
                ;;
            dev_gui_frameworks_common|dev_gui_stubs_common)
                queue+=("dev_gui_common")
                ;;
            webkit)
                queue+=("dev_gui_frameworks_common")
                queue+=("dev_gui_stubs_common")
                ;;
            stock)
                queue+=("cli")
                queue+=("python")
                queue+=("ruby")
                queue+=("perl")
                queue+=("dev_gui_common")
                queue+=("dev_gui_frameworks_common")
                queue+=("dev_gui_stubs_common")
                queue+=("gui_frameworks")
                queue+=("gui_stubs")
                ;;
            all)
                queue+=("stock")
                queue+=("jsc")
                queue+=("webkit")
                queue+=("cli_extra")
                queue+=("cli_dev_gui_stubs")
                ;;
        esac
    done
    
    printf '%s\n' "${visited[@]}"
}

resolve_submodules() {
    local components=("$@")
    local all_submodules=()
    
    for comp in "${components[@]}"; do
        if [ -n "${COMPONENT_SUBMODULES[$comp]}" ]; then
            while IFS= read -r submodule; do
                [ -z "$submodule" ] && continue
                all_submodules+=("$submodule")
            done <<< "${COMPONENT_SUBMODULES[$comp]}"
        fi
    done
    
    printf '%s\n' "${all_submodules[@]}" | sort -u
}

get_registered_submodules() {
    git -C "$PROJECT_ROOT" submodule status -- "$EXTERNAL_DIR" | awk '{print $2}' | sed "s|$EXTERNAL_DIR/||" | sort
}

is_registered_submodule() {
    local submodule="$1"
    git -C "$PROJECT_ROOT" submodule status -- "$EXTERNAL_DIR/$submodule" &>/dev/null
}

fetch_submodules() {
    local submodules=("$@")
    
    if [ ${#submodules[@]} -eq 0 ]; then
        return
    fi
    
    local git_paths=()
    for submodule in "${submodules[@]}"; do
        if is_registered_submodule "$submodule"; then
            git_paths+=("src/external/$submodule")
        else
            echo "Skipping $submodule (not a registered git submodule)"
        fi
    done
    
    if [ ${#git_paths[@]} -gt 0 ]; then
        echo "Fetching ${#git_paths[@]} submodules..."
        git -C "$PROJECT_ROOT" submodule update --init --recursive --depth=1 -- "${git_paths[@]}" 2>&1 || \
            git -C "$PROJECT_ROOT" submodule update --init --recursive -- "${git_paths[@]}" 2>&1 || true
    fi
}

case "$COMPONENT" in
    core|system|cli|cli_dev|cli_extra|gui|gui_frameworks|gui_stubs|dev_gui_common|dev_gui_frameworks_common|dev_gui_stubs_common|jsc|webkit|iokitd|python|ruby|perl|stock|all|cli_dev_gui_stubs|jsc_webkit_common)
        ;;
    *)
        echo "Error: Unknown component: $COMPONENT"
        exit 1
        ;;
esac

if [ "$COMPONENT" = "jsc_webkit_common" ]; then
    components=("jsc_webkit_common")
else
    components=($(get_deps "$COMPONENT"))
fi

submodules=($(resolve_submodules "${components[@]}"))

echo "Fetching submodules for component: $COMPONENT"
echo "Dependencies: ${components[*]}"
echo "Submodules to fetch: ${#submodules[@]}"
echo ""

fetch_submodules "${submodules[@]}"

echo ""
echo "Done fetching submodules for component: $COMPONENT"
