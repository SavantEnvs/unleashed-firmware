#!/usr/bin/env bash
#
# mayhem/build.sh — build the host-side fuzz harnesses and self-test oracles for unleashed-firmware.
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

# The firmware itself targets STM32 + FreeRTOS and cannot run on a host. The EMV card poller (lib/nfc/protocols/emv)
# is built for the host instead, out of tree under build/ (gitignored), over a scripted card: its ISO14443-4A
# transport is replaced by mayhem/emv_session.h, and the firmware headers it names by mayhem/emv_shim/.
OUT="$SRC/build"
M="$SRC/mayhem"

rm -rf "$OUT"; mkdir -p "$OUT/test"

EMV_SRCS="lib/nfc/protocols/emv/emv_poller_i.c lib/toolbox/bit_buffer.c"
EMV_INC="-I$M/emv_shim -I$M -I$SRC/lib/nfc -I$SRC"

# LeakSanitizer is switched off at build time for every target (docs/rules/c.md).
$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -c "$M/lsan_off.cc" -o "$OUT/lsan_off.o"

# fuzz <name> <harness.c> <srcs> <includes> [extra compile/link flags]
# Builds the fuzzer, and the run-once -standalone reproducer, with the project sources sanitized.
fuzz() {
  local name="$1" harness="$2" srcs="$3" inc="$4"; shift 4
  # shellcheck disable=SC2086
  $CC $SANITIZER_FLAGS "$@" $DEBUG_FLAGS -w $inc $LIB_FUZZING_ENGINE $srcs "$harness" "$OUT/lsan_off.o" -lm -o "$OUT/$name"
  # shellcheck disable=SC2086
  $CC $SANITIZER_FLAGS "$@" $DEBUG_FLAGS -w $inc "$STANDALONE_FUZZ_MAIN" $srcs "$harness" "$OUT/lsan_off.o" -lm -o "$OUT/$name-standalone"
}

# selftest <name> <selftest.c> <srcs> <includes>: the oracle, with the project's normal flags and no sanitizers.
selftest() {
  # shellcheck disable=SC2086
  $CC -O2 $COVERAGE_FLAGS -w $4 $3 "$2" -lm $COVERAGE_FLAGS -o "$OUT/test/$1_selftest"
}

# bit-buffer indexing in emv_poller_read_afl shifts by a negative amount for an SFI below 2 (benign on the
# target, and present before and after the fix); relaxing only shift-exponent keeps every input from halting there.
fuzz fuzz_emv "$M/fuzz_emv.c" "$EMV_SRCS" "$EMV_INC" -fno-sanitize=shift-exponent

selftest emv "$M/emv_selftest.c" "$EMV_SRCS" "$EMV_INC"
