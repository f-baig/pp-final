# Shared Memory Parallel Triangle Counting
This repository contains our final project submission for CPSC 424, an implementation of a shared memory parallel triangle counting algorithm built using ParlayLib. Our starting point was a distributed memory triangle counting algorithm written by [Liu et. al.](https://arxiv.org/abs/2405.00262), from which we optimized for shared memory, practically matching the performance of the GBBS benchmark for triangle counting.

When starting with testing our algorithm, we recommend you follow the two-line tutorial below. Run both commands while you are in the main project folder. Note that our source code for the ./final executable is in main.cpp

```
$ make final
$ ./final test_graphs/rbl_email_enron.adj
```

To run the corresponding test for the GBBS benchmark, you can run the following commands. Run both commands while you are in the main project folder. *Note that if you are on the Zoo, you do not need to run the first command, as we include the precompiled ./TriangleCount executable in the repository.*

```
$ g++ -std=c++17 -O3 -pthread \
    benchmarks/TriangleCounting/ShunTangwongsan15/Triangle.cc \
    gbbs/graph_io.cc gbbs/io.cc gbbs/helpers/parse_command_line.cc \
    gbbs/encodings/byte.cc \
    gbbs/encodings/byte_pd.cc \
    gbbs/encodings/byte_pd_amortized.cc \
    -I. -Igbbs -Iparlaylib/include \
    -o TriangleCount
$ ./TriangleCount -s test_graphs/rbl_email_enron.adj
```

For more rigorous testing and finding graph arboricity, there are a few more steps which we would be happy to explain if reached out to. However for the sake of brevity, we will leave the testing tutorial here.
