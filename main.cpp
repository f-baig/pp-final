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
	parlay::sequence<std::pair<int,int>> edges;
	std::ifstream in(filename);
	if (!in) {
		std::cerr << "Error opening file: " << filename << "\n";
		return edges;
	}

	std::string burn_header;
	std::getline(in, burn_header);

	size_t num_vertices, num_edges;
	in >> num_vertices >> num_edges;

	std::vector<int> offsets(num_vertices);
	for (size_t i = 0; i < num_vertices; i++) {
		in >> offsets[i];
	}

	std::vector<int> target_vertices(num_edges);
	for (size_t i = 0; i < num_edges; i++) {
		in >> target_vertices[i];
	}

	edges = parlay::sequence<std::pair<int,int>>(num_edges);
	size_t idx = 0;
	for (size_t i = 0; i < num_vertices; i++) {
		size_t start = offsets[i];
		size_t end;
		if (i + 1 < num_vertices) {
			end = offsets[i+1];
		} else {
			end = num_edges;
		}
		for (size_t j = start; j < end; j++) {
			edges[idx] = {i, target_vertices[j]};
			idx++;
		}
	}
	
	return edges;
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
	parlay::sequence<parlay::sequence<int>> adjList;

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

		int max_vertex = 0;
		for (const auto& e : edges) {
			max_vertex = std::max({max_vertex, e.first, e.second});
		}

		adjList = parlay::sequence<parlay::sequence<int>>(max_vertex + 1);

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
		graph(g), edges(edges) {}

	int getTriangleCount() { return triangle_count; }

	void computeTriangles() {
		parlay::sequence<long long> counts(edges.size());

		parlay::parallel_for(0, edges.size(), [&](int i) {
			auto e = edges[i];
			counts[i] = countSharedVertices(graph->adjList[e.first], graph->adjList[e.second]);
		});

		triangle_count = parlay::reduce(counts, parlay::addm<long long>());
	}
	
private:
	/**
	 * Private variables:
	 * triangle_count is a running total of triangles
	 * graph is a pointer to the 
	 */
	long long triangle_count = 0;
	const Graph *graph;
	parlay::sequence<std::pair<int,int>> edges;

	long long countSharedVertices(const parlay::sequence<int>& w, const parlay::sequence<int>& u) {
		size_t w_idx = 0;
		size_t u_idx = 0;
		long long count = 0;
		while (w_idx < w.size() && u_idx < u.size()) {
			if (w[w_idx] < u[u_idx]) {
				w_idx++;
			} else if (w[w_idx] > u[u_idx]) {
				u_idx++;
			} else { // w[w_idx] = u[u_udx]
				count++; 
				w_idx++;
				u_idx++;
			}
		}
		return count;
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
	
	double end_time = omp_get_wtime();
	
	std::cout << "Total: " << tot << " Triangles: " << triangles << std::endl;
	
    double elapsed = end_time - start_time;
	std::cout << "Time Elapsed: " << elapsed << std::endl;
	return 0;
}