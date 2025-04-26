#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <omp.h>
#include <utility>

// Include ParlayLib (adjust the path if needed)
#include <parlay/primitives.h>
#include <parlay/parallel.h>
#include <parlay/sequence.h>

parlay::sequence<std::pair<int,int>> parseEdges(const std::string &filename, size_t &vertices) {
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
	vertices = num_vertices;

	std::vector<int> offsets(num_vertices);
	for (size_t i = 0; i < num_vertices; i++) {
		in >> offsets[i];
	}

	std::vector<int> target_vertices(num_edges);
	for (size_t i = 0; i < num_edges; i++) {
		in >> target_vertices[i];
	}

	edges = parlay::sequence<std::pair<int,int>>(num_edges/2);
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
			if (i < target_vertices[j]) {
				edges[idx] = {i, target_vertices[j]};
				idx++;
			}
		}
	}
	
	return edges;
}

// class
// stores mapping
// map from vertexID to sequences 
class Graph {

public:
	parlay::sequence<parlay::sequence<int>> adjList;

	Graph(size_t &num_vertices, parlay::sequence<std::pair<int,int>> &edges) {
		vertices = num_vertices;
		createAdjList(edges);
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
	size_t vertices;

	void createAdjList(const parlay::sequence<std::pair<int,int>>& edges) {
		if (edges.empty()) return;

		adjList = parlay::sequence<parlay::sequence<int>>(vertices);

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

	long long getTriangleCount() { return triangle_count; }

	void computeTriangles() {
		parlay::sequence<long long> counts(edges.size());

		parlay::parallel_for(0, edges.size(), [&](int i) {
			auto e = edges[i];
			counts[i] = countSharedVertices(graph->adjList[e.first], graph->adjList[e.second], e.first, e.second);
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

	long long countSharedVertices(const parlay::sequence<int>& w_seq, 
								  const parlay::sequence<int>& u_seq,
								  size_t w, size_t u) {
		long long count = 0;
		int binary_search_factor = 100;
		size_t w_seq_size = w_seq.size();
		size_t u_seq_size = u_seq.size();
		if (w_seq_size * binary_search_factor < u_seq_size) {
			for (auto &v : w_seq) {
				count += queryEdge(u, v);
			}
		} else if (w_seq_size > u_seq_size * binary_search_factor) {
			for (auto &v : u_seq) {
				count += queryEdge(w, v);
			}
		} else {
			size_t w_idx = 0;
			size_t u_idx = 0;
			 
			while (true) {
				if (w_seq[w_idx] < u_seq[u_idx]) {
					w_idx++;
					if (w_idx == w_seq_size) {
						break;
					}
				} else if (w_seq[w_idx] > u_seq[u_idx]) {
					u_idx++;
					if (u_idx == u_seq_size) {
						break;
					}
				} else { // w[w_idx] = u[u_udx]
					count++; 
					w_idx++;
					u_idx++;
					if (w_idx == w_seq_size) {
						break;
					}
					if (u_idx == u_seq_size) {
						break;
					}
				}
			}
		}
		return count;
	}

	int queryEdge(int u, int v) {
		return std::binary_search(graph->adjList[u].begin(), graph->adjList[u].end(), v);
	};
};

int main(int argc, char** argv) {
	double start_time = omp_get_wtime();

	std::string data_file  = argv[1];
	size_t num_vertices;
	parlay::sequence<std::pair<int,int>> edges = parseEdges(data_file, num_vertices);
	
	double parsing_marker = omp_get_wtime();
	double parsing_time = parsing_marker - start_time;
	std::cout << "Parsing Time: " << parsing_time << std::endl;

	Graph *g = new Graph(num_vertices, edges);

	double construction_marker = omp_get_wtime();
	double construction_time = construction_marker - parsing_marker;
	std::cout << "Adjacency List Construction Time: " << construction_time << std::endl;

	Solver* s = new Solver(g, edges);
	s->computeTriangles();
	long long triangles = s->getTriangleCount();
	
	double end_time = omp_get_wtime();
	double solving_time = end_time - construction_marker;
	std::cout << "Computing Triangles Time: " << solving_time << std::endl;
	
	double elapsed_exclude_parser = end_time - parsing_marker;
	double elapsed = end_time - start_time;
	std::cout << "Total Time (excluding parser): " << elapsed_exclude_parser << std::endl;
	std::cout << "Total Time Elapsed: " << elapsed << std::endl;

	std::cout << "Total: " << triangles << " Triangles: " << triangles << std::endl;
	return 0;
}