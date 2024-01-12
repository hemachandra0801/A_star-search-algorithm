#ifndef A_STAR_H
#define A_STAR_H

#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <unordered_set>

struct cell; 

bool isValid(std::string&, std::unordered_set<std::string>&); 
std::string astar(std::unordered_map<std::string, std::vector<std::pair<std::string, double>>>&, std::string&, std::string&);

#endif
