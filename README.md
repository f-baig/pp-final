# pp-final
parallel shared-memory implementation of triangle counting algorithm

Triangle Counts:
arxiv-gqc.txt: 48260
email-Enron.txt: 727044

Relabel:
- Doesn't Double
- Relabel vertices
- gets rid of comments

TODO:
- Make the edges list not have both directions
- Deal with large vertex IDs (use strings)
- Add parallelism to main.cpp
- Create GBBS pipeline
- Do arboricity stuff
- Data collection
- Brain dataset processing
- Test executable in makefile

Questions:
Directed graphs??

Commands for Gbbs:

wget https://snap.stanford.edu/data/wiki-Vote.txt.gz

gzip --decompress ${PWD}/wiki-Vote.txt.gz

g++ -std=c++17 -O3 -pthread  -DGBBSEDGELONG -DGBBSLONG   utils/snap_converter.cc     gbbs/graph_io.cc gbbs/io.cc     gbbs/helpers/parse_command_line.cc     -I. -Igbbs -Iparlaylib/include -o snap_converter

./snap_converter -s -i wiki-Vote.txt -o wiki-Vote.adj

g++ -std=c++17 -O3 -pthread \
    benchmarks/TriangleCounting/ShunTangwongsan15/Triangle.cc \
    gbbs/graph_io.cc gbbs/io.cc gbbs/helpers/parse_command_line.cc \
    gbbs/encodings/byte.cc \
    gbbs/encodings/byte_pd.cc \
    gbbs/encodings/byte_pd_amortized.cc \
    -I. -Igbbs -Iparlaylib/include \
    -o TriangleCount

./TriangleCount -s wiki-Vote.adj

email-Enron Datset (times are median of 5 trials):

GBBS - 0.00350133 s

1. Commit "Fetch Add Per Query": 0.1595534 s

2. Commit "Private Triangle count per iteration, fetch_add once per iteration": 0.03855484 s

3. Commit "store triangle counts per iteration in a sequence, then parlay reduce sum": 0.03456398 s

4. Commit "add queries to a sequence, then filter out one's in edges_set": 0.04361992 s

5. Builds on Commit 3, Commit "allocate size of edges_set to save time from rehashing": 0.03022596 s