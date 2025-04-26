#!/bin/bash

# We didn't really know bash or regex so we used GPT help for a good bit of this

set -e

########################################################
# COLORS AND HELPERS
########################################################

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

log() {
  echo -e "${GREEN}==> $1${NC}" >&2
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

########################################################
# HELPER FUNCTION TO COMPUTE MEDIAN
########################################################

median_of_sorted() {                       # $1 = name of array variable
  local -n arr=$1
  local mid=$(( ${#arr[@]} / 2 ))
  echo "${arr[$mid]}"
}

########################################################
# LOGGING AND ACTUAL COMMANDS
########################################################

log "Getting .gz file"
run_in_dir test_graphs wget "$URL"

log "Decompressing"
run_in_dir test_graphs gzip --decompress "$FILENAME"

log "Relabeling .txt file"
run_in_dir . ./relabel "test_graphs/$BASENAME" "test_graphs/$RELABEL_FILENAME"

########################################################
# SNAP CONVERTING
########################################################

log "Snap Converting"
run_in_dir test_graphs ./snap_converter -s -i "$RELABEL_FILENAME" -o "$ADJ_FILENAME"

########################################################
# FINDING ARBORICITY
########################################################

log "Finding Arboricity"
readarray -t info < <( run_in_dir . ./arboricity/build/find_arboricity "./test_graphs/$RELABEL_FILENAME" )
arboricity=${info[0]}
num_nodes=${info[1]}
num_edges=${info[2]}

echo "Arboricity: $arboricity"

########################################################
# RUNNING ON GBBS BENCHMARK
########################################################

log "Running GBBS Benchmark"
GBBS_OUTPUT="results/GBBS_RESULT_${BASENAME%.txt}.txt"
> "$GBBS_OUTPUT"    

for i in {1..10}; do
  echo "=== Run $i ===" >> "$GBBS_OUTPUT"
  run_in_dir . ./TriangleCount -s "test_graphs/$ADJ_FILENAME" \
      >> "$GBBS_OUTPUT" 2>&1
  echo "" >> "$GBBS_OUTPUT"
done

clean_gbbs=$(sed 's/\x1b\[[0-9;]*m//g' "$GBBS_OUTPUT")

readarray -t gbbs_triangles_arr < <(
  grep "### Num triangles" <<<"$clean_gbbs" | awk -F'= ' '{print $2}'
)
readarray -t gbbs_times_arr < <(
  grep "# time per iter:"  <<<"$clean_gbbs" | awk '{print $5}'
)

IFS=$'\n' sorted_gbbs_times=($(sort -n <<<"${gbbs_times_arr[*]}"))
gbbs_time=$(median_of_sorted sorted_gbbs_times)
unique_counts=($(printf "%s\n" "${gbbs_triangles_arr[@]}" | sort -u))
gbbs_triangles="${unique_counts[0]}"

echo "GBBS Triangles: $gbbs_triangles"
echo "GBBS Time: $gbbs_time"

########################################################
# RUNNING DFFs (OUR) IMPLEMENTATION
########################################################

log "Running DFFs Implementation"
DFFS_OUT="results/DFFS_RESULT_${BASENAME%.txt}.txt"
> "$DFFS_OUT"

for i in {1..30}; do
  echo "=== Run $i ===" >> "$DFFS_OUT"
  run_in_dir . ./final "test_graphs/$ADJ_FILENAME" \
      >> "$DFFS_OUT" 2>&1
  echo >> "$DFFS_OUT"
done

clean_dffs=$(sed -E 's/\x1b\[[0-9;]*m//g' "$DFFS_OUT")

readarray -t dffs_triangles_arr < <(
  grep "Triangles:"   <<<"$clean_dffs" | sed -E 's/.*Triangles: \[?([0-9]+)\]?.*/\1/'
)
readarray -t dffs_times_arr < <(
grep "Computing Triangles Time:" <<<"$clean_dffs" | awk '{print $4}'
)

IFS=$'\n' sorted_dffs_times=($(sort -n <<<"${dffs_times_arr[*]}"))
dffs_time=$(median_of_sorted sorted_dffs_times)
uniq_dffs=($(printf "%s\n" "${dffs_triangles_arr[@]}" | sort -u))
dffs_triangles=${uniq_dffs[0]}

echo "DFFs Triangles: $dffs_triangles"
echo "DFFs Time: $dffs_time"

########################################################
# FINAL OUTPUT
########################################################

echo "${BASENAME%.txt},$arboricity,$num_nodes,$num_edges,$gbbs_triangles,$gbbs_time,$dffs_triangles,$dffs_time" >> results/summary_v2.csv
