#include "a_star.h"
#include <iostream>
#include <string>
#include <vector>
#include <stack>                             
#include <set>
#include <utility>
#include <float.h>
#include <unordered_map>
#include <unordered_set>

using namespace std;

// The speed of planes could vary
const int SPEED = 850;

struct cell {
	std::string parent;
	double f, g, h;
};

struct graphNode {
  std::string node;
  std::string start;
  std::string depart;
  double distance;
};

bool isValid(std::string& node, std::unordered_set<std::string>& nodes) {
	return nodes.find(node) != nodes.end();
}

inline bool isUnBlocked(std::string& node, std::unordered_map<std::string, std::vector<graphNode>>& graph, string& currTime) {
  for (auto& flight : graph[node]) {
    if (flight.depart >= currTime) {
      return true;
    }
  }
	return false;
}

inline bool isDestination(std::string& node, std::string& goal) {
	return node == goal;
}

inline double calculateHValue(std::string& node, std::string& goal, string& currTime) {
	// Enter the heuristic here
	//return (double)sqrt((row - dest.first) * (row - dest.first)+ (col - dest.second) * (col - dest.second));
	return 0;
}

inline double timeDifference(std::string& a, std::string& b) {
  int start = (10 * (a[0] - '0') + (a[1] - '0')) * 60 + 10 * (a[2] - '0') + (a[3] - '0');
  int end = (10 * (b[0] - '0') + (b[1] - '0')) * 60 + 10 * (b[2] - '0') + (b[3] - '0');
  return (end - start) * 1.0 / 60;
}

std::string tracePath(std::unordered_map<std::string, cell>& cellDetails, std::string& goal) {

  std::string optimalPath = "";

	std::cout << '\n' << "The Path is ";
	std::string curr = goal;

	std::stack<std::string> Path;

	while (cellDetails[curr].parent != curr) {
		Path.push(curr);
		curr = cellDetails[curr].parent;
	}

	Path.push(curr);
	while (!Path.empty()) {
		std::string p = Path.top();
		Path.pop();
		std::cout << "-> " << p << " ";
    optimalPath += p;
    optimalPath += '-';
	}
  optimalPath.pop_back();

  return optimalPath;
}

std::string astar(std::unordered_map<std::string, std::vector<graphNode>>& graph, std::string& start, std::string& goal, std::string& currTime) {

	if (isDestination(start, goal)) {
		std::cout << "We are already at the destination\n";
		return "At destination";
	}

	std::unordered_map<std::string, bool> closedList;

	std::unordered_map<std::string, cell> cellDetails;


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

	std::set<std::pair<double, std::string>> openList;

	openList.insert(std::make_pair(0.0, start));

	while (!openList.empty()) {
		std::pair<double, std::string> p = *openList.begin();

		openList.erase(openList.begin());

		std::string i = p.second;
		closedList[i] = true;

		double gNew, hNew, fNew;


    for (auto& d : graph[i]) {
      if (d.depart < currTime) {
        continue;
      }

			if (isDestination(d.node, goal)) {
				cellDetails[d.node].parent= i;
				std::cout << "\nThe shortest path has been found\n";
				return tracePath(cellDetails, goal);
			}

			else if (!closedList[d.node] && isUnBlocked(d.node, graph, currTime)) {
				gNew = cellDetails[i].g + d.distance + SPEED * timeDifference(currTime, d.depart);
				hNew = calculateHValue(d.node, goal, currTime);
				fNew = gNew + hNew;

				if (cellDetails[d.node].f == FLT_MAX || cellDetails[d.node].f > fNew) {
					openList.insert(make_pair(fNew, d.node));

					cellDetails[d.node].f = fNew;
					cellDetails[d.node].g = gNew;
					cellDetails[d.node].h = hNew;
					cellDetails[d.node].parent = i;
				}
			}
    }
	}

  std::cout << "Path not found\n";
  return "Path not found";
}
