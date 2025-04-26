# output file
OUTFILE="bst_results.txt"
> "$OUTFILE"

FACTORS=(1 2 4 8 16 32 64 128 256 512 1024)

for f in "${FACTORS[@]}"; do
  printf "%3d ‑ " "$f" >> "$OUTFILE"
  ./final test_graphs/gplus_combined.adj "$f" >> "$OUTFILE"
done

echo "Done. Results saved in $OUTFILE"
