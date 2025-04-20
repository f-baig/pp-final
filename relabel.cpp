#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>

using std::string;

void relabel(const string &input, const string &output) {
	std::ifstream in(input);
    std::ofstream out(output);
    if (!in || !out) {
        std::cerr << "failed to open " << input << " or " << output << '\n';
        return;
    }

    std::unordered_map<string, int> map;
    int idx = 0;

    string u, v;
    while (in >> u >> v) {
        auto get_id = [&](const string& s) -> int {
            if (map.count(s)) {
                return map[s];
            }
            map[s] = idx;
            idx++;
            return map[s]; 
        };
        out << get_id(u) << " " << get_id(v) << "\n";
    }
}

int main(int argc, char** argv) {
    relabel(argv[1], argv[2]);
}
