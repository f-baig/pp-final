#!/bin/bash

# We didn't really know bash or regex so we used GPT help for a good bit of this

set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

log() {
  echo -e "${GREEN}==> $1${NC}"
}

error() {
  echo -e "${RED}Error: $1${NC}" >&2
  exit 1
}

run_in_dir() {
  local dir="$1"
  shift
  log "Running in $dir: $*"
  (cd "$dir" && "$@") || error "Failed in $dir: $*"
}

if [ $# -ne 1 ]; then
  error "Usage: $0 https://snap.stanford.edu/data/NAME.txt.gz"
fi

URL="$1"

if [[ ! "$URL" =~ ^https://snap.stanford.edu/data/.+\.txt\.gz$ ]]; then
  error "URL must match format: https://snap.stanford.edu/data/NAME.txt.gz"
fi

FILENAME=$(basename "$URL")
BASENAME="${FILENAME%.gz}"
ADJ_FILENAME="${BASENAME%.txt}.adj"
RELABEL_FILENAME="RBL_${BASENAME%.txt}.txt"

# Logging + actual Commands hHeheeasdalnd DYRAANANNA

log "Getting .gz file"
run_in_dir test_graphs wget "$URL"

log "Decompressing"
run_in_dir test_graphs gzip --decompress "$FILENAME"

log "Snap Converting"
run_in_dir test_graphs ./snap_converter -s -i "$BASENAME" -o "$ADJ_FILENAME"

log "Relabeling .txt file"
run_in_dir . ./relabel "test_graphs/$BASENAME" "test_graphs/$RELABEL_FILENAME"

log "Finding Arboricity"
arboricity=$(run_in_dir . ./arboricity/build/find_arboricity "./test_graphs/$RELABEL_FILENAME" | tail -n 1)

echo "Arboricity: $arboricity"

log "Running GBBS Benchmark"
run_in_dir . ./TriangleCount -s "test_graphs/$ADJ_FILENAME" \
  | tee "results/GBBS_RESULT_${BASENAME%.txt}.txt" > /dev/null

GBBS_RESULT_FILE="results/GBBS_RESULT_${BASENAME%.txt}.txt"

gbbs_triangles=$(grep "### Num triangles" "$GBBS_RESULT_FILE" | tail -1 | awk -F'= ' '{print $2}')

gbbs_time=$(grep "# time per iter:" "$GBBS_RESULT_FILE" | awk '{print $5}')

echo "GBBS Triangles: $gbbs_triangles"
echo "GBBS Time: $gbbs_time"

log "Running DFFs Implementation 3 times"
DFFS_OUTPUT="results/DFFS_RESULT_${BASENAME%.txt}.txt"
> "$DFFS_OUTPUT"

for i in {1..3}; do
  echo "=== Run $i ===" >> "$DFFS_OUTPUT"
  run_in_dir . ./final "test_graphs/$ADJ_FILENAME" >> "$DFFS_OUTPUT" 2>&1
  echo "" >> "$DFFS_OUTPUT"
done

DFFS_RESULT_FILE="results/DFFS_RESULT_${BASENAME%.txt}.txt"

clean_output=$(sed 's/\x1b\[[0-9;]*m//g' "$DFFS_RESULT_FILE")

readarray -t triangle_counts < <(echo "$clean_output" | grep "Triangles:" | sed -E 's/.*Triangles: \[?([0-9]+)\]?.*/\1/')

if [ "${triangle_counts[0]}" = "${triangle_counts[1]}" ] && [ "${triangle_counts[1]}" = "${triangle_counts[2]}" ]; then
  dffs_triangles="${triangle_counts[0]}"
else
  echo "❌ Triangle count mismatch across DFFs runs: ${triangle_counts[*]}"
  exit 1
fi

readarray -t dffs_times < <(echo "$clean_output" | grep "Time Elapsed:" | awk '{print $3}')

IFS=$'\n' sorted=($(sort -n <<<"${dffs_times[*]}"))
dffs_time=${sorted[1]}

echo "DFFs Triangles: $dffs_triangles"
echo "DFFs Time: $dffs_time"

echo "Dataset,Arboricity,GBBS_Triangles,GBBS_Time,DFFS_Triangles,DFFS_Time" > results/summary.csv
echo "${BASENAME%.txt},$arboricity,$gbbs_triangles,$gbbs_time,$dffs_triangles,$dffs_time" >> results/summary.csv