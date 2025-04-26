#!/bin/bash

executable="$1"
input_file="$2"

if [[ ! -x "$executable" ]]; then
  echo "Error: $executable is not executable or does not exist."
  exit 1
fi

if [[ ! -f "$input_file" ]]; then
  echo "Error: $input_file does not exist."
  exit 1
fi

while IFS= read -r line; do
  # Skip empty lines
  [[ -z "$line" ]] && continue

  # Extract the link (before the first comma)
  link="${line%%,*}"

  echo "Running: $executable $link"
  "$executable" "$link"
done < "$input_file"
