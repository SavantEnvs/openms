#!/usr/bin/env bash
#
# mayhem/build.sh — build this repo's fuzz harness(es). EDIT per repo.
#
# Runs inside the commit image (mayhem/Dockerfile) as `mayhem` in /mayhem. The base image
# (ghcr.io/savantenvs/base) already exports the build contract — use these, don't redefine:
#   CC, CXX             stock clang / clang++
#   LIB_FUZZING_ENGINE  -fsanitize=fuzzer   (link into each harness that has a LLVMFuzzer entry)
#   SANITIZER_FLAGS     -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer
#                       (ASan + UBSan, both set to HALT — so Mayhem catches memory AND UB defects)
#   DEBUG_FLAGS         -g -gdwarf-3   (DWARF debug info — always on for fuzz/standalone builds,
#                       independent of the sanitizer off-switch; DWARF version must be < 4)
#   RUST_DEBUG_FLAGS    -C debuginfo=2 -C force-frame-pointers=yes -C llvm-args=-gdwarf-3
#                       (thread through RUSTFLAGS on every cargo-fuzz build)
#   GO_DEBUG_FLAGS      -gcflags=all=-N -l
#                       (thread through go build / go-fuzz-build so the linked ELF keeps symbols)
#   SRC                 /mayhem (the repo source)
#
# Contract: build EVERYTHING here — one runnable binary per fuzz harness, AND the project's test
# suite (so mayhem/test.sh only has to RUN it, never compile). Keep it ADDITIVE (build upstream as
# upstream documents; don't edit upstream files). IMPORTANT: build the PROJECT ITSELF with
# $SANITIZER_FLAGS and $DEBUG_FLAGS (not just the harness) so the fuzzed code is instrumented
# AND carries DWARF < 4 symbols — otherwise ASan/UBSan only see the harness, not the library
# you're trying to find bugs in, and backtraces won't resolve project source lines. Build the TEST
# suite with the project's NORMAL flags (a clean, independent build) so test.sh stays an honest
# functional oracle and won't false-fail on benign UB. Leave the test binary/runner where test.sh
# expects it.
set -euo pipefail

# clang rejects SOURCE_DATE_EPOCH='' (empty) — it must be unset or a valid integer.
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

# Build knobs come from the ENVIRONMENT (overridable), with sane defaults — no if-statements,
# just parameter-expansion fallbacks. Default sanitizers are the base's ASan+UBSan (halting); override
# per build via the Dockerfile's `--build-arg SANITIZER_FLAGS="..."`.
# NB: SANITIZER_FLAGS uses `=` (no colon) on purpose — `=` only fills when the var is UNSET, so an
# explicit EMPTY value (`--build-arg SANITIZER_FLAGS=`) is honored and builds with NO sanitizers
# (useful when you want the program's natural crash / full backtrace, not an ASan report). The other
# knobs use `:=` (default on empty too). MAYHEM_JOBS sets build parallelism (falls back to nproc).
: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
# DEBUG_FLAGS carries DWARF debug info INDEPENDENTLY of the sanitizer off-switch (so an empty
# SANITIZER_FLAGS still yields DWARF symbols). DWARF MUST be < 4 (Mayhem triage can't read >=4); clang-19's
# plain `-g` emits DWARF-5, so `-gdwarf-3` is explicit. Apply $DEBUG_FLAGS to the fuzz/harness/standalone
# builds (NOT the test/oracle build). Rust/Go carry the same intent via their language flags below.
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${RUST_DEBUG_FLAGS:=-C debuginfo=2 -C force-frame-pointers=yes -C llvm-args=-gdwarf-3}"
: "${GO_DEBUG_FLAGS:=-gcflags=all=-N -l}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
# COVERAGE_FLAGS: empty by default → no effect on the normal oracle build. Set it via the Dockerfile's
# `--build-arg COVERAGE_FLAGS="-fprofile-instr-generate -fcoverage-mapping"` to instrument the TEST
# build for source-coverage measurement (how much of the project the test suite actually exercises —
# a quality signal for the oracle; complements the anti-reward-hack sabotage check). APPEND it to the
# test build's compile+link flags in step 3 (NOT the fuzz build); after `test.sh` runs, merge with
# `llvm-profdata` and report with `llvm-cov`. Empty value is honored (`=`, not `:=`).
: "${COVERAGE_FLAGS=}"
export SANITIZER_FLAGS DEBUG_FLAGS RUST_DEBUG_FLAGS GO_DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS

cd "$SRC"

# Upstream's CMake build needs Arrow/Parquet, HiGHS and libxml2>=2.13 (none usable offline in the image), so the
# readers under test are built from a curated source closure (mayhem/sources.txt) with the same generated
# headers CMake would produce. See mayhem/Makefile.
LX=/opt/toolchains/libxml2
B="$SRC/build"
OMS="$SRC/src/openms"
HARNESSES="mzml mzxml mzdata fasta idxml featurexml consensusxml traml mzid pepxml protxml mgf mztab paramxml"

cmake -DSRC="$SRC" -DOUT="$B/gen" -P "$SRC/mayhem/gen-config.cmake"

INCS="-I$B/gen -I$OMS/include -I$OMS/../openswathalgo/include -I$OMS/extern/nlohmann_json/include -I$OMS/extern/SQLiteCpp/include \
 -I$OMS/extern/IsoSpec -I$OMS/extern/evergreen/src -I$OMS/extern/GTE -I$OMS/extern/Quadtree/include -I$OMS/extern/simde \
 -I$OMS/extern/eol-bspline -I$OMS/thirdparty -I/usr/include/libsvm -I$LX/include/libxml2 -I/usr/include/eigen3"
BASE_CXXFLAGS="-std=c++23 -w -fPIC -DBOOST_PROCESS_USE_STD_FS $INCS"
LIBS="-Wl,--gc-sections -L$LX/lib -lxml2 -lsqlite3 -lcurl -lzip -lbz2 -lz -lzstd -lsvm -lpthread -lm -ldl"

build_lib() { # <outdir> <flags...>
  local out="$1"; shift
  mkdir -p "$out"
  local flags="$BASE_CXXFLAGS $*"
  [ "$(cat "$out/flags.stamp" 2>/dev/null)" = "$flags" ] || printf '%s' "$flags" > "$out/flags.stamp"
  make -f mayhem/Makefile OUT="$out" CXX="$CXX" CXXFLAGS_X="$flags" -j"$MAYHEM_JOBS" "$out/libopenms-lite.a"
}

# 1) Test/oracle build FIRST: project's normal flags (no sanitizers, no DWARF pin), independent of the fuzz build.
build_lib "$B/test" -O0 $COVERAGE_FLAGS
$CXX $BASE_CXXFLAGS -O0 $COVERAGE_FLAGS "$SRC/mayhem/kat/oms_kat.cc" "$B/test/libopenms-lite.a" $LIBS $COVERAGE_FLAGS -o "$B/test/oms_kat"

# 2) Fuzzed library: ASan+UBSan halting, SanCov on the library itself, DWARF-3.
LIB_FUZZ_FLAGS="-O1 -ffunction-sections -fdata-sections $SANITIZER_FLAGS -fsanitize=fuzzer-no-link $DEBUG_FLAGS -gline-tables-only"  # line tables only: the template-heavy library otherwise makes every binary >200MB
FUZZ_FLAGS="-O1 -ffunction-sections -fdata-sections $SANITIZER_FLAGS -fsanitize=fuzzer-no-link $DEBUG_FLAGS"
build_lib "$B/fuzz" $LIB_FUZZ_FLAGS

# 3) Harnesses: compile each once, link against the fuzzing engine and against the standalone driver.
mkdir -p "$B/harness"
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c -x c "$STANDALONE_FUZZ_MAIN" -o "$B/harness/standalone_main.o"
$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -c "$SRC/mayhem/lsan_off.cc" -o "$B/harness/lsan_off.o"
for t in $HARNESSES; do
  echo "$t"
done | xargs -P "$MAYHEM_JOBS" -I{} sh -c "$CXX $BASE_CXXFLAGS -I$SRC/mayhem/harness $FUZZ_FLAGS -c $SRC/mayhem/harness/fuzz_{}.cc -o $B/harness/fuzz_{}.o"
for t in $HARNESSES; do echo "$t"; done | xargs -P "$MAYHEM_JOBS" -I{} sh -c "$CXX $SANITIZER_FLAGS $DEBUG_FLAGS $LIB_FUZZING_ENGINE $B/harness/fuzz_{}.o $B/harness/lsan_off.o $B/fuzz/libopenms-lite.a $LIBS -o /mayhem/fuzz_{}"
for t in $HARNESSES; do echo "$t"; done | xargs -P "$MAYHEM_JOBS" -I{} sh -c "$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -fsanitize=fuzzer-no-link $B/harness/fuzz_{}.o $B/harness/standalone_main.o $B/harness/lsan_off.o $B/fuzz/libopenms-lite.a $LIBS -o /mayhem/fuzz_{}-standalone"
