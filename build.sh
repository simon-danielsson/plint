
#!/usr/bin/env bash

DIR_ROOT="$(dirname "$(readlink -f "$0")")"
cd "$DIR_ROOT"
BUILD_SCRIPT_NAME="$(basename "$0")"
NAME_PROJECT="$(basename "$(pwd)")"
BIN_NAME=$NAME_PROJECT
DIR_BIN="bin"
mkdir -p "$DIR_BIN"

DIR_SRC_MAIN="src"
DIR_SRC_EXTERNAL="src/external"
DIR_SRC_TESTS="tests"
BUILD_TYPE="debug"
ARG_VERBOSE=false
NO_RUN=false
LEAKS=false

compilation_flags_constant=(
    "-Wno-unused-function"
    "-std=c99"
)

compilation_flags_debug=(
    "-O0"
    "-DDEBUG"
    "-pedantic"
    "-pedantic-errors"
    "-g"
    "-Wall"
    "-Wextra"
    "-Wshadow"
)

compilation_flags_release=(
    "-flto"
    "-O2"
    "-DNDEBUG"
)

if [ -f "./static/embed.sh" ]; then
    chmod +x ./static/embed.sh
    ./static/embed.sh >/dev/null 2>&1
fi

log_details() {
    if [ $ARG_VERBOSE = true ]; then
        echo "[INFO] $1"
    fi
}

err() {
    echo "[ERROR] $1"
    exit 1
}

help() {
    echo "Usage: $BUILD_SCRIPT_NAME [options|commands]"
    echo "Execute without options to compile and run a debug build."
    echo "Program arguments are passed after a divider '--'."
    echo ""
    echo "Commands:"
    echo "(NONE)               Build (and run) with debug flags."
    echo "release              Build (and run) with release flags."
    echo "test                 Build (and run) tests with debug flags."
    echo "leaks                Build with debug flags and check for leaks."
    echo ""
    echo "Options:"
    echo "-n, --no-run         Don't run program after build."
    echo "-v, --verbose        Enable verbose build details."
    echo "-h, --help           Display help."
}

PASS_THROUGH_ARGS=()
arg_idx=0
for arg in "$@"
do
    case "$arg" in
        --)
            # Capture everything after this index
            ((idx++))
            PASS_THROUGH_ARGS=("${@:$((idx+1))}")
            break
            ;;

        release)
            BUILD_TYPE="release"
            BIN_NAME="${BIN_NAME}-release"
            ;;

        test)
            BUILD_TYPE="test"
            BIN_NAME="${BIN_NAME}-test"
            compilation_flags_debug+=("-DTEST")
            ;;

        leaks) LEAKS=true ;;

        -v|--verbose) ARG_VERBOSE=true ;;
        -h|--help) help; exit ;;
        -n|--no-run) NO_RUN=true ;;
    esac
    ((idx++))
done

if [[ "$BUILD_TYPE" == "debug" ]]; then
    if [[ "$LEAKS" == false ]]; then
        compilation_flags_debug+=("-fsanitize=address")
        compilation_flags_debug+=("-fsanitize=undefined")
        compilation_flags_debug+=("-fno-omit-frame-pointer")
        compilation_flags_debug+=("-fno-omit-frame-pointer")
    fi
    BIN_NAME="${BIN_NAME}-debug"
fi

if commit=$(git rev-parse --short=7 HEAD 2>/dev/null); then
    BIN_NAME="${BIN_NAME}-${commit}"
fi

if tag=$(git describe --tags --abbrev=0 2>/dev/null); then
    BIN_NAME="${BIN_NAME}-${tag}"
fi

src_files_str_src=$(find "$DIR_SRC_MAIN" "$DIR_SRC_EXTERNAL" | sed -n '/\.c/p')
src_files_str_tests=$(find "$DIR_SRC_TESTS" | sed -n '/\.c/p')
declare -a SRC_FILES=($src_files_str_src $src_files_str_tests)

log_details "source files: $SRC_FILES"
log_details "build type: $BUILD_TYPE"
log_details "bin name: $BIN_NAME"

compile() {
    local compiler=""
    if $(cc -v 2>/dev/null); then
        compiler="cc"
    elif $(gcc -v 2>/dev/null); then
        compiler="gcc"
    elif $(clang -v 2>/dev/null); then
        compiler="clang"
    elif $(tcc -v 2>/dev/null); then
        compiler="tcc"
    else
        err "no valid compiler could be found"
    fi

    log_details "compiler: $compiler"

    set -e

    if [[ "$BUILD_TYPE" == "release" ]]; then
        log_details "flags: ${compilation_flags_constant[*]} ${compilation_flags_release[*]}"

        $compiler "${compilation_flags_release[@]}" \
            "${compilation_flags_constant[@]}" \
            "${SRC_FILES[@]}" \
            -o "$DIR_BIN"/"$BIN_NAME"

    elif [[ "$BUILD_TYPE" == "test" ]]; then
        log_details "flags: ${compilation_flags_debug[*]} ${compilation_flags_constant[*]}"

        $compiler "${compilation_flags_debug[@]}" \
           "${compilation_flags_constant[@]}" \
           "${SRC_FILES[@]}" \
            -o "$DIR_BIN"/"$BIN_NAME"

    else # debug
        log_details "flags: ${compilation_flags_constant[*]} ${compilation_flags_debug[*]}"

        $compiler "${compilation_flags_debug[@]}" \
           "${compilation_flags_constant[@]}" \
           "${SRC_FILES[@]}" \
            -o "$DIR_BIN"/"$BIN_NAME"
    fi

    set +e
}

run() {
    log_details "destination: ./${DIR_BIN}/${BIN_NAME}"

    if [[ $LEAKS == true ]]; then
        MallocStackLogging=1 leaks -atExit -- \
            "$DIR_BIN"/"$BIN_NAME" \
            "${PASS_THROUGH_ARGS[@]}"
        exit
    fi

    if [[ $NO_RUN == false ]]; then
        if [[ "$BUILD_TYPE" == "test" ]]; then
            "$DIR_BIN"/"$BIN_NAME" "${PASS_THROUGH_ARGS[@]}" >/dev/null
        else
            "$DIR_BIN"/"$BIN_NAME" "${PASS_THROUGH_ARGS[@]}"
        fi
    fi

}

compile
run

