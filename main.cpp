#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <omp.h>
#include <utility>
#include <atomic>

// Include ParlayLib (adjust the path if needed)
#include <parlay/primitives.h>
#include <parlay/parallel.h>
#include <parlay/sequence.h>
#include <parlay/hash_table.h>

struct PairHash {
	std::size_t operator()(const std::pair<int,int>& p) const {
		// return ~(std::hash<int>{}(p.first)) ^ (std::hash<int>{}(p.second));
		return (std::hash<int>{}(p.first)) ^ (std::hash<int>{}(p.second)) << 1;
	}
};

parlay::sequence<std::pair<int,int>> parseEdges(const std::string &filename) {
	parlay::sequence<std::pair<int,int>> edges_seq;
	std::ifstream in(filename);
	if (!in) {
		std::cerr << "Error opening file: " << filename << "\n";
		return edges_seq;
	}

	std::unordered_set<std::pair<int,int>, PairHash> edges_set;

	int u, v;
	while (in >> u >> v) {
		if (edges_set.count({u, v}) || edges_set.count({v, u})) {
			continue;
		}
		edges_seq.push_back({u, v});
		edges_seq.push_back({v, u});
		edges_set.insert({u, v});
	}

	return edges_seq;
}

parlay::sequence<std::pair<int,int>> halfEdges(const parlay::sequence<std::pair<int,int>> edges) {
	parlay::sequence<std::pair<int,int>> half_edges;
	for (auto& e : edges) {
		if (e.first < e.second) {
			half_edges.emplace_back(e);
		}
	}
	return half_edges;
}

// class
// stores mapping
// map from vertexID to sequences 
class Graph {

public:
	std::unordered_map<int,parlay::sequence<int>> adjList;

	Graph(parlay::sequence<std::pair<int,int>> &edges) {
		createAdjList(edges);
	}

	// void printAdjList() const {
	// 	for (int i = 0; i < adjList.size(); i++) {
	// 		std::cout << "Index: " << i << " Adjacent Nodes: ";

	// 		for (auto &e : adjList[i]) {
	// 			std::cout << e << ", ";
	// 		}
	// 		std::cout << std::endl;
	// 	}
  	// }

private:
	void createAdjList(const parlay::sequence<std::pair<int,int>>& edges) {
		if (edges.empty()) return;

		for (const auto& e : edges) {
			int u = e.first;
			int v = e.second;
			adjList[u].push_back(v);
    	}
	}
};

class Solver {

public:
	Solver(const Graph *g, const parlay::sequence<std::pair<int,int>> &edges) : 
		graph(g), edges_seq(edges), edges_set(edges.begin(), edges.end()) {}

	int getTriangleCount() { return triangle_count; }

	void computeTriangles() {
		parlay::sequence<long long> counts(edges_seq.size());

		parlay::parallel_for(0, edges_seq.size(), [&](int i) {
			// long long triangle_count_iter = 0;
			parlay::sequence<std::pair<int, int>> queries;

			auto e = edges_seq[i];
			int w, u;

			if (graph->adjList.at(e.first).size() <= graph->adjList.at(e.second).size()) { 
				w = e.first;
				u = e.second;
			} else { 
				w = e.second;
				u = e.first;
      		}
			

			for (auto &v : graph->adjList.at(w)) {		
				if (u < v) {
					queries.push_back({u, v});
				} else {
					queries.push_back({v, u});
				}
				// triangle_count_iter += queryEdges({w, u}, {w, v});
				// triangle_count.fetch_add(queryEdges({w, u}, {w, v}), std::memory_order_relaxed);		
    		}
			
			auto success = parlay::filter(queries, [&](auto e) {
				return edges_set.find(e) != edges_set.end();
			});

			// counts[i] = triangle_count_iter;
			counts[i] = success.size();
			//triangle_count.fetch_add(triangle_count_iter, std::memory_order_relaxed);
		});

		triangle_count = parlay::reduce(counts, parlay::addm<long long>());
	}
	
private:
	/**
	 * Private variables:
	 * triangle_count is a running total of triangles
	 * graph is a pointer to the 
	 */
	std::atomic<long long> triangle_count{0};
	const Graph *graph;
	parlay::sequence<std::pair<int,int>> edges_seq;
	std::unordered_set<std::pair<int,int>, PairHash> edges_set;

	int queryEdges(std::pair<int,int> e1, std::pair<int,int> e2) {
		if (e1.second < e2.second) {
			return edges_set.find({e1.second, e2.second}) != edges_set.end();
		}
		else { // e2.second < e1.second
			return edges_set.find({e2.second, e1.second}) != edges_set.end();
		}	
	}
};

int main(int argc, char** argv) {
	std::string data_file  = argv[1];
	parlay::sequence<std::pair<int,int>> edges = parseEdges(data_file);
	
	Graph *g = new Graph(edges);

	parlay::sequence<std::pair<int,int>> half_edges = halfEdges(edges);

	// g->printMapping();
	// g->printAdjList();

	double start_time = omp_get_wtime();
	Solver* s = new Solver(g, half_edges);
	s->computeTriangles();
	long long tot = s->getTriangleCount();
	long long triangles = tot / 3;
	
	std::cout << "Total: " << tot << " Triangles: " << triangles << std::endl;

	double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;
	std::cout << "Time Elapsed: " << elapsed << std::endl;
	return 0;
}