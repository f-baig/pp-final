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
- Create GBBS pipeline
- Data collection
- Brain dataset processing
- Test executable in makefile

**Running Test Script**
- chmod +x ./test-pipeline.sh
- ./test-pipeline https://snap.stanford.edu/data/wiki-Vote.txt.gz

**Commands for Gbbs:**

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

**Relabeling procedure**
Takes in an input file (snap file in .txt format), and outputs a file that has:
1. nodes have been mapped to 0,...,n-1 2. in the form of an edge list 3. Each edge only appears once (if (u, v), then (v, u) does not appear)
make relabel
./relabel <input_file> <output_file>

**Command for calculating the arboricity of a graph:**

./arboricity/build/find_arboricity <path_to_graph>

Ex usage: ./arboricity/build/find_arboricity ./test_graphs/r_arxiv-gqc.txt

Note: expects to be run from the pp-final folder, and requires a graph that has
1. nodes have been mapped to 0,...,n-1 2. in the form of an edge list 3. Each edge only appears once (if (u, v), then (v, u) does not appear)

**To run our implementation of the triangle finding algorithm**
make final
./final <edge_file>
where edge_file is in the format that was described above (i.e. same as relabeling output and same as arboricity calculation input)

**Trial Runs with varying paralle reduction/increment strategies**

email-Enron Datset (times are median of 5 trials):

GBBS - 0.00350133 s

1. Commit "Fetch Add Per Query": 0.1595534 s

2. Commit "Private Triangle count per iteration, fetch_add once per iteration": 0.03855484 s

3. Commit "store triangle counts per iteration in a sequence, then parlay reduce sum": 0.03456398 s

4. Commit "add queries to a sequence, then filter out one's in edges_set": 0.04361992 s

5-7 builds on Commit 3
5. Commit "allocate size of edges_set to save time from rehashing": 0.03022596 s

6. Commit "parallelized edges_set construction in hopes of improving cache hits": 0.0355912 s

7. Commit "Tried doing parlay::tabulate instead of parlay::for to work on cache": 0.03203454 s
