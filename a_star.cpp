#include <iostream>
#include <vector>
#include <cmath>
#include <stack>                             
#include <set>
#include <iterator>
#include <utility>
#include <float.h>
#include <unordered_map>
#include <unordered_set>

using namespace std;

struct cell {
	string parent;
	double f, g, h;
};

inline bool isValid(string& node, unordered_set<string>& nodes) {
	return nodes.find(node) != nodes.end();
}

inline bool isUnBlocked(string& node, unordered_map<string, vector<pair<string, double>>>& graph) {
	return graph[node].size() > 0;
}

inline bool isDestination(string& node, string& goal) {
	return node == goal;
}

inline double calculateHValue(string& node, string& goal) {
	// Enter the heuristic here
	//return (double)sqrt((row - dest.first) * (row - dest.first)+ (col - dest.second) * (col - dest.second));
	return 0;
}

void tracePath(unordered_map<string, cell>& cellDetails, string& goal) {

	std::cout << '\n' << "The Path is ";
	string curr = goal;

	stack<string> Path;

	while (cellDetails[curr].parent != curr) {
		Path.push(curr);
		string temp_curr = cellDetails[curr].parent;
		curr = temp_curr;
	}

	Path.push(curr);
	while (!Path.empty()) {
		string p = Path.top();
		Path.pop();
		std::cout << "-> " << p << " ";
	}
}

void astar(unordered_map<string, vector<pair<string, double>>>& graph, string& start, string& goal) {

	if (isDestination(start, goal)) {
		std::cout << "We are already at the destination\n";
		return;
	}

	unordered_map<string, bool> closedList;

	unordered_map<string, cell> cellDetails;


	for (auto& i : graph) {
		cellDetails[i.first].f = FLT_MAX;
		cellDetails[i.first].g = FLT_MAX;
		cellDetails[i.first].h = FLT_MAX;
		cellDetails[i.first].parent = "";

		closedList[i.first] = false;
	}

	cellDetails[start].f = 0.0;
	cellDetails[start].g = 0.0;
	cellDetails[start].h = 0.0;
	cellDetails[start].parent = start;

	set<pair<double, string>> openList;

	openList.insert(make_pair(0.0, start));

	bool foundDest = false;

	while (!openList.empty()) {
		pair<double, string> p = *openList.begin();

		openList.erase(openList.begin());

		string i = p.second;
		closedList[i] = true;

		double gNew, hNew, fNew;


        for (auto& d : graph[i]) {
			if (isDestination(d.first, goal)) {
				cellDetails[d.first].parent= i;
				std::cout << "\nThe shortest path has been found\n";
				tracePath(cellDetails, goal);
				foundDest = true;
				return;
			}

			else if (!closedList[d.first] && isUnBlocked(d.first, graph)) {
				gNew = cellDetails[i].g + d.second;
				hNew = calculateHValue(d.first, goal);
				fNew = gNew + hNew;

				if (cellDetails[d.first].f == FLT_MAX || cellDetails[d.first].f > fNew) {
					openList.insert(make_pair(fNew, d.first));

					cellDetails[d.first].f = fNew;
					cellDetails[d.first].g = gNew;
					cellDetails[d.first].h = hNew;
					cellDetails[d.first].parent = i;
				}
			}
        }
	}

	if (!foundDest)
		std::cout << "Destination cannot be reached\n";

	return;
}

int main() {

	unordered_map<string, vector<pair<string, double>>> graph;
	unordered_set<string> nodes;

	int edges;
	std::cout << "Enter the number of edges: ";
	std::cin >> edges;

	std::cout << "\nEnter the graph below (NODE1 -> NODE2 -> DISTANCE)\n";
	for (int i = 0; i < edges; ++i) {
		double distance;
		string node1, node2;
		std::cin >> node1 >> node2 >> distance;

		nodes.insert(node1);
		nodes.insert(node2);
		graph[node1].push_back({node2, distance});
	}

	string start = "";
	string goal = "";


	std::cout << "\nEnter the name of the starting location: ";
	while (std::cin >> start) {
		if (isValid(start, nodes)) break;
		std::cout << "Source is invalid\n";
	}

	std::cout << "\nEnter the name of the destination: ";
	while (std::cin >> goal) {
		if (isValid(goal, nodes)) break;
		std::cout << "Destination is invalid\n";
	}
	

	astar(graph, start, goal);

	return 0;
}