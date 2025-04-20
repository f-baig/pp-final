# output file
OUTFILE="bst_results.txt"
> "$OUTFILE"

FACTORS=(1 2 5 10 25 50 75 100 200)

for f in "${FACTORS[@]}"; do
  printf "%3d ‑ " "$f" >> "$OUTFILE"
  ./final test_graphs/gplus_combined.adj "$f" >> "$OUTFILE"
done

echo "Done. Results saved in $OUTFILE"
