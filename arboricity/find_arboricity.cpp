#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_set>
#include "arboricity.h"

struct PairHash {
    size_t operator()(const std::pair<int,int>& p) const noexcept {
        // pack into 64 bits
        return std::hash<long long>()(
            (static_cast<long long>(p.first) << 32) ^ p.second
        );
    }
};

int main(int argc, char **argv) {
    std::vector<std::pair<int, int>> edges;
    std::unordered_set<std::pair<int,int>, PairHash> seen;
    std::unordered_set<int> nodes;

    std::string data_file  = argv[1];
    std::ifstream input(data_file);

    size_t num_edges = 0;
    size_t num_nodes;

    if (!input) {
		std::cerr << "Error opening an input file.\n";
        return 1;
	}

    std::string line;
    while (std::getline(input, line)) {
        std::istringstream iss(line);
        int a, b;
        
        if (iss >> a >> b) {
            // Make the reverse
            std::pair<int,int> e{a, b}, rev{b,a};
            //Check if the reverse is in the set
            if (seen.count(rev) || seen.count(e)) {
                continue;
            }

            edges.emplace_back(a, b);
            seen.insert(e);
            num_edges++;

            nodes.insert(a);
            nodes.insert(b);
            
            // if (a == b) {
            //     std::cout << "Self Edge" << std::endl;
            // }

        } else {
            std::cerr << "Skipping malformed line: " << line << '\n';
        }
    }

    num_nodes = nodes.size();

    UndirectedGraph* g = new UndirectedGraph(num_nodes, num_edges);
    for (const auto& e : edges) {
        int u = e.first;
        int v = e.second;
        g->AddEdge(u, v, 1);
    }

    std::cout << g->Solve() << std::endl;
}

