#!/usr/bin/env bash
#
# mayhem/test.sh — RUN the known-answer self-test that mayhem/build.sh built, and diff it against mayhem/golden/.
# exit 0 = pass. EDIT per repo. PATCH-grade oracle: after an agent patches the source, the grader
# rebuilds (build.sh) then runs this. DELETE this file if the repo has no meaningful tests.
#
# IMPORTANT:
#  * Must assert BEHAVIOR/OUTPUT, not just exit status. The oracle has to check asserted values /
#    golden-output diffs / known-answer results — so a PATCH that "fixes" a bug by making the program
#    exit(0) (or any no-op) FAILS here. Running inputs and checking only "exit 0 / didn't crash" is
#    NOT a functional test (it's trivially reward-hackable) — use the project's real assertion suite.
#  * Do NOT build here — mayhem/build.sh already compiled the test suite (with the project's normal
#    flags). This script only RUNS the pre-built tests and reports counts. If the test runner is
#    missing, that's a build.sh bug — fail loudly rather than silently rebuilding.
#  * REQUIRED OUTPUT — a CTRF (https://ctrf.io) summary so Mayhem/the PATCH grader reads the counts:
#      - writes a CTRF JSON report to ${CTRF_REPORT:-$SRC/ctrf-report.json}, and
#      - prints a one-line `CTRF {...}` marker to stdout (same JSON, compact).
#    Only `results.summary` (with tests/passed/failed/pending/skipped/other) is required.
#    Use the emit_ctrf helper below; it computes tests = passed+failed+skipped and sets the exit
#    code (0 iff failed==0). Map your framework's output to passed/failed/skipped.
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${MAYHEM_JOBS:=$(nproc)}"   # build parallelism; env-overridable, falls back to nproc (use -j"$MAYHEM_JOBS")
cd "$SRC"

# emit_ctrf <tool> <passed> <failed> [skipped] [pending] [other]
# Writes a CTRF report (file + stdout `CTRF {...}` marker) and returns non-zero iff failed>0.
emit_ctrf() {
  local tool="$1" passed="$2" failed="$3" skipped="${4:-0}" pending="${5:-0}" other="${6:-0}"
  local tests=$(( passed + failed + skipped + pending + other ))
  cat > "${CTRF_REPORT:-$SRC/ctrf-report.json}" <<JSON
{
  "results": {
    "tool": { "name": "$tool" },
    "summary": {
      "tests": $tests,
      "passed": $passed,
      "failed": $failed,
      "pending": $pending,
      "skipped": $skipped,
      "other": $other
    }
  }
}
JSON
  printf 'CTRF {"results":{"tool":{"name":"%s"},"summary":{"tests":%d,"passed":%d,"failed":%d,"pending":%d,"skipped":%d,"other":%d}}}\n' \
    "$tool" "$tests" "$passed" "$failed" "$pending" "$skipped" "$other"
  [ "$failed" -eq 0 ]
}

# textbox_selftest drives the real TextBox module (gui/modules/text_box.c) on fixed inputs and prints one result per
# line (first visible line, scrollbar position); every line is compared with mayhem/golden/textbox.txt and counts as one test.
passed=0; failed=0
for name in textbox; do
  bin="$SRC/build/test/${name}_selftest"
  if [ ! -x "$bin" ]; then echo "missing $bin — mayhem/build.sh did not build it" >&2; failed=$((failed + 1)); continue; fi
  mapfile -t want < "$SRC/mayhem/golden/$name.txt"
  mapfile -t got < <("$bin" 2>&1)
  n=${#want[@]}; [ "${#got[@]}" -gt "$n" ] && n=${#got[@]}
  for ((i = 0; i < n; i++)); do
    if [ "${want[i]+set}" = set ] && [ "${got[i]+set}" = set ] && [ "${want[i]}" = "${got[i]}" ]; then
      passed=$((passed + 1))
    else
      failed=$((failed + 1)); echo "FAIL $name line $((i + 1)): want '${want[i]-<none>}' got '${got[i]-<none>}'" >&2
    fi
  done
done
emit_ctrf "unleashed-textbox-selftest" "$passed" "$failed" 0
