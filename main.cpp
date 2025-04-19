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

// Include ParlayLib (adjust the path if needed)
#include <parlay/primitives.h>
#include <parlay/parallel.h>
#include <parlay/sequence.h>
#include <parlay/hash_table.h>

parlay::sequence<std::pair<int,int>> parseEdges(const std::string &filename) {
	parlay::sequence<std::pair<int,int>> edges;
	std::ifstream infile(filename);
	if (!infile) {
		std::cerr << "Error opening file: " << filename << "\n";
		return edges;
	}

 	std::string line;
  	while (std::getline(infile, line)) {
		if (line.empty()) {
			continue;
		}

		std::istringstream iss(line);
		int u, v;
		if (iss >> u >> v) {
			if (u != v) {
				edges.push_back({u, v});
			}
		} else {
			std::cerr << "Warning: skipping invalid line: " << line << "\n";
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
// sequence of sequences 

class Graph {

public:
	parlay::sequence<parlay::sequence<int>> adjList;

	Graph(parlay::sequence<std::pair<int,int>> &edges) {
		createNodeCellMapping(edges);
		createAdjList(edges);
	}

	int map(int vertex) const {
		return mapping.at(vertex);
	}
	
	void printMapping() {
		for (auto &ele : mapping) {
			// if (ele.second  < 5) {std::cout << "Key: " << ele.first << " Index: " << ele.second << "\n";}
			std::cout << "Key: " << ele.first << " Index: " << ele.second << "\n";
		}
	}

	void printAdjList() const {
		for (int i = 0; i < adjList.size(); i++) {
			std::cout << "Index: " << i << " Adjacent Nodes: ";

			for (auto &e : adjList[i]) {
				std::cout << e << ", ";
			}
			std::cout << std::endl;
		}
  	}

private:
	std::unordered_map<int,int> mapping;

	void createNodeCellMapping(const parlay::sequence<std::pair<int,int>>& edges) {
		if (edges.empty()) return;

		int cnt = 0;
		int prev = edges[0].first;

		mapping[prev] = cnt;

		for (const auto &e : edges) {
			int curr = e.first;
			if (curr != prev) {
				cnt++;
				prev = curr;
				mapping[curr] = cnt;
			}
		}
	}	

	void createAdjList(const parlay::sequence<std::pair<int,int>>& edges) {
		if (edges.empty()) return;

		adjList.emplace_back();
		int cnt = 0;
		int curr = edges[0].first;

		for (const auto& e : edges) {
			if (e.first != curr) {
				cnt++;
				curr = e.first;
				adjList.emplace_back();
			}
			adjList[cnt].push_back(e.second);
    	}
	}
};

struct PairHash {
	std::size_t operator()(const std::pair<int,int>& p) const {
		// return ~(std::hash<int>{}(p.first)) ^ (std::hash<int>{}(p.second));
		return (std::hash<int>{}(p.first)) ^ (std::hash<int>{}(p.second)) << 1;
	}
};

class Solver {

public:
	Solver(const Graph *g, const parlay::sequence<std::pair<int,int>> &edges) : graph(g), edges(edges.begin(), edges.end()) {}
	int getTriangleCount() { return triangle_count; }

	void computeTriangles() {
		// parlay::parallel_for(0, Q, [&](int i){
		// 	KNNHelper helper(data_points, k);
		// 	helper.search(root, query_points[i]);
		// 	results[i] = helper.get_results();
		// });

		for (auto &e : edges) {
			// auto e = edges[i];
			int w, u;
			if (graph->adjList[graph->map(e.first)].size() <= graph->adjList[graph->map(e.second)].size()) { 
				w = e.first;
				u = e.second;
			} else { 
				w = e.second;
				u = e.first;
      		}
			

			for (auto &v : graph->adjList[graph->map(w)]) {				
				triangle_count += queryEdges({w, u}, {w, v});
    		}
    	}
	}
	
private:
	/**
	 * Private variables:
	 * triangle_count is a running total of triangles
	 * graph is a pointer to the 
	 */
	int triangle_count = 0;
	const Graph *graph;
	std::unordered_set<std::pair<int,int>, PairHash> edges;

	int queryEdges(std::pair<int,int> e1, std::pair<int,int> e2) {
		if (e1.second < e2.second) {
			return edges.find({e1.second, e2.second}) != edges.end();
		}
		else { // e2.second < e1.second
			return edges.find({e2.second, e1.second}) != edges.end();
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
	int tot = s->getTriangleCount();
	int triangles = tot / 3;
	
	std::cout << "Total: " << tot << " Triangles: " << triangles << std::endl;

	double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;
	std::cout << "Time Elapsed: " << elapsed << std::endl;
	return 0;
}