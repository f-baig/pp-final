#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <unordered_map>
#include <unordered_set>
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
    if (line.empty()) continue;
    std::istringstream iss(line);
    int u, v;
    if (iss >> u >> v) {
      edges.push_back({u, v});
    } else {
      std::cerr << "Warning: skipping invalid line: " << line << "\n";
    }
  }
  return edges;
}

// class
// stores mapping
// sequence of sequences 

class Graph {
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
public:
  parlay::sequence<parlay::sequence<int>> adjList;
  Graph(parlay::sequence<std::pair<int,int>> &edges) {
    createNodeCellMapping(edges);
    createAdjList(edges);
  }
  void printMapping() {
    for (auto &ele : mapping) {
      if (ele.second  < 5) {std::cout << "Key: " << ele.first << " Index: " << ele.second << "\n";}
    }
  }
  void printAdjList() {
    for (int i = 0; i < 5; i++) {
      std::cout << "Index: " << i << " Adjacent Nodes: ";

      for (auto &e : adjList[i]) {
        std::cout << e << ", ";
      }

      std::cout << std::endl;
    }
  }
};

// class Solver {
// private:
//   int triangle_count = 0;
//   Graph *graph;
//   std::unordered_set<std::pair<int,int>> edges;

//   int queryEdges(std::pair<int,int> e1, std::pair<int,int> e2) {
//     return edges.find({e1.second, e2.second}) != edges.end();
//   }
// public:
//   Solver(const Graph* g, const parlay::sequence<std::pair<int,int>> &edges) : graph(g), edges(edges.begin(), edges.end()) {}
//   int getTriangleCount() { return triangle_count; }

//   void computeTriangles() {
//     for (auto &e : edges) {
//       int w, u;
//       if (graph->adjList[e.first].size() <= graph->adjList[e.second].size()) { 
//         w, u = e.first, e.second;
//       } else {
//         w, u = e.second, e.first;
//       }

//       for (auto &v : graph->adjList[w]) {
//         triangle_count += queryEdges({w,u}, {w, v});
//       }
//     }
//   }
// };

struct PairHash {
    std::size_t operator()(const std::pair<int,int>& p) const noexcept {
        // CHANGE 2
        return ~(std::hash<int>{}(p.first)) ^ (std::hash<int>{}(p.second));
    }
};

class Solver {
private:
  int triangle_count = 0;
  Graph *graph;

  std::unordered_set<std::pair<int,int>, PairHash> edges;

  int queryEdges(std::pair<int,int> e1, std::pair<int,int> e2) {
    return edges.find({e1.second, e2.second}) != edges.end();
  }

public:
  Solver(const Graph* g,
         const parlay::sequence<std::pair<int,int>>& es)
    : graph(const_cast<Graph*>(g)), edges(es.begin(), es.end()) {}

  int getTriangleCount() { return triangle_count; }

  void computeTriangles() {
    for (const auto& e : edges) {
      int w, u;
      if (graph->adjList[e.first].size() <= graph->adjList[e.second].size()) {
        w = e.first;  u = e.second;
      } else {
        w = e.second; u = e.first;
      }
      // CHANGE 1
      for (auto &v : graph->adjList[w]) {
        triangle_count += queryEdges({w,u}, {w,v});
      }
    }
  }
};

int main(int argc, char** argv) {

  std::string data_file  = argv[1];
  parlay::sequence<std::pair<int,int>> edges = parseEdges(data_file);
  Graph *g = new Graph(edges);

  // g->printMapping();
  // g->printAdjList();

  Solver* s = new Solver(g, edges);
  s->computeTriangles();
  int tot = s->getTriangleCount();
  int triangles = tot / 6;

  std::cout << "Total: " << tot << " Triangles: " << triangles << std::endl;

  return 0;
}