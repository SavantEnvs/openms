#!/usr/bin/env bash
#
# mayhem/test.sh — RUN this repo's OWN functional test suite (already built by mayhem/build.sh).
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

# Known-answer tests: mayhem/build.sh built build/test/oms_kat (normal flags, no sanitizers) from the same OpenMS
# readers that are fuzzed. Each case parses a known input (or evaluates a known formula/sequence) and the probe's
# `KAT:` lines must equal the golden values in mayhem/kat/golden/<case>.txt. A neutered or silent binary prints
# nothing and therefore fails every case.
KAT="$SRC/build/test/oms_kat"
GOLDEN="$SRC/mayhem/kat/golden"
if [ ! -x "$KAT" ]; then
  echo "missing $KAT: mayhem/build.sh must build it" >&2
  emit_ctrf "oms-kat" 0 1 0
  exit 1
fi

CASES='
mzml_basic|mzml|mayhem/mzml/testsuite/MzMLFile_1.mzML
mzml_indexed|mzml|mayhem/mzml/testsuite/MzMLFile_4_indexed.mzML
mzml_truncated|mzml|mayhem/kat/truncated.mzML
mzxml_basic|mzxml|mayhem/mzxml/testsuite/MzXMLFile_1.mzXML
mzdata_basic|mzdata|mayhem/mzdata/testsuite/MzDataFile_1.mzData
mgf_gnps|mgf|mayhem/mgf/testsuite/MascotGenericFile_GNPS.mgf
fasta_basic|fasta|mayhem/fasta/testsuite/FASTAFile_test.fasta
idxml_quant|idxml|mayhem/idxml/testsuite/ProteinQuantifier_input.idXML
mzid_msgf|mzid|mayhem/mzid/testsuite/MzIdentMLFile_msgf_mini.mzid
pepxml_mascot|pepxml|mayhem/pepxml/testsuite/PepXMLFileMascot_test.pepXML
protxml_basic|protxml|mayhem/protxml/testsuite/ProtXMLFile_input_1.protXML
featurexml_basic|featurexml|mayhem/featurexml/testsuite/FeatureXMLFile_1.featureXML
consensusxml_basic|consensusxml|mayhem/consensusxml/testsuite/ConsensusXMLFile_1.consensusXML
traml_generic|traml|mayhem/traml/testsuite/OpenSwath_generic_input.TraML
mztab_labelfree|mztab|mayhem/mztab/testsuite/MzTabFile_labelfree.mzTab
paramxml_basic|paramxml|mayhem/paramxml/testsuite/Param_post16_update.ini
formula_glucose|formula|C6H12O6
formula_water|formula|H2O
peptide_peptider|peptide|PEPTIDER
peptide_glycine|peptide|GGG
'

pass=0; fail=0
while IFS='|' read -r id mode arg; do
  [ -n "$id" ] || continue
  got="$("$KAT" "$mode" "$arg" 2>/dev/null | grep '^KAT: ' || true)"
  want="$(cat "$GOLDEN/$id.txt" 2>/dev/null || true)"
  if [ -n "$want" ] && [ "$got" = "$want" ]; then
    pass=$((pass + 1))
  else
    fail=$((fail + 1))
    echo "FAIL $id: expected [$want] got [$got]" >&2
  fi
done <<< "$CASES"

emit_ctrf "oms-kat" "$pass" "$fail" 0
