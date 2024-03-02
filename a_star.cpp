#include "a_star.h"
#include <iostream>
#include <string>
#include <vector>
// #include <stack>                             
#include <set>
#include <utility>
#include <float.h>
#include <unordered_map>
#include <unordered_set>

// complete heuristic
// add filter options  -> done
// update weights in main.cpp itself  -> done
// add distance and price to final output
// update the algorithm to include other stuff
// change logic for gNew  -> done
// update code for changed json structure for better heuristic
// remove currTime


// using namespace std;

struct cell {
	std::string parent;
	double f, g, h;
};

struct graphNode {
  std::string node;
  std::string mode;
  std::string name;
  std::string start;
  std::string depart;
  std::string reach;
  double distance;
  double price;
  double weight;
};

bool isValid(std::string& node, std::unordered_set<std::string>& nodes) {
	return nodes.find(node) != nodes.end();
}

inline bool isUnBlocked(std::string& node, std::unordered_map<std::string, std::vector<graphNode>>& graph, std::string& currTime) {
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

inline double calculateHValue(std::string& node, std::string& goal, std::string& currTime, std::string& filter) {
	// Enter the heuristic here
	//return (double)sqrt((row - dest.first) * (row - dest.first)+ (col - dest.second) * (col - dest.second));
	return 0;
}

inline double timeDifference(std::string& a, std::string& b) {
  int start = (10 * (a[0] - '0') + (a[1] - '0')) * 60 + 10 * (a[2] - '0') + (a[3] - '0');
  int end = (10 * (b[0] - '0') + (b[1] - '0')) * 60 + 10 * (b[2] - '0') + (b[3] - '0');
  return (end - start) * 1.0 / 60;
}


// std::string tracePath(std::unordered_map<std::string, cell>& cellDetails, std::string& goal) {
//
//   std::string optimalPath = "";
//
// 	std::cout << '\n' << "The Path is ";
// 	std::string curr = goal;
//
// 	std::stack<std::string> Path;
//
// 	while (cellDetails[curr].parent != curr) {
// 		Path.push(curr);
// 		curr = cellDetails[curr].parent;
// 	}
//
// 	Path.push(curr);
// 	while (!Path.empty()) {
// 		std::string p = Path.top();
// 		Path.pop();
// 		std::cout << "-> " << p << " ";
//     optimalPath += p;
//     optimalPath += '-';
// 	}
//   optimalPath.pop_back();
//
//   return optimalPath;
// }

std::pair<std::pair<double, double>, std::vector<std::vector<std::string>>> astar(std::unordered_map<std::string, std::vector<graphNode>>& graph, std::string& start, std::string& goal, std::string& currTime, std::string& filter) {

	if (isDestination(start, goal)) {
		std::cout << "We are already at the destination\n";
		return {};
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

  std::vector<std::vector<std::string>> routes;

	while (!openList.empty()) {
		std::pair<double, std::string> p = *openList.begin();

		openList.erase(openList.begin());

		std::string i = p.second;
		closedList[i] = true;

		double gNew, hNew, fNew;

    for (auto& d : graph[i]) {
      // if (d.depart < currTime) {
      //   continue;
      // }

			if (isDestination(d.node, goal)) {
				cellDetails[d.node].parent= i;
				std::cout << "\nThe shortest path has been found\n";

		    std::vector<std::string> route;
        route.push_back(d.node);
        route.push_back(d.name);
        route.push_back(d.mode);

        std::string currentNode = d.node;
        double totalDistance = 0.0;
        double totalPrice = 0.0;
        
        while (currentNode != start) {
            cell c = cellDetails[currentNode];
            route.insert(route.begin(), c.parent);
            for (const auto& edge : graph[currentNode]) {
              if (edge.node == c.parent) {
                route.push_back(std::to_string(edge.distance));
                route.push_back(std::to_string(edge.price));                 
                totalDistance += edge.distance;
                totalPrice += edge.price;
                break;
              }
            }
            currentNode = c.parent;
        }
        

        routes.push_back(route);

        return {{totalDistance, totalPrice}, routes};	
			}

			else if (!closedList[d.node] && isUnBlocked(d.node, graph, currTime)) {
				gNew = cellDetails[i].g + d.weight;
				hNew = calculateHValue(d.node, goal, currTime, filter);
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
  return {};
}
