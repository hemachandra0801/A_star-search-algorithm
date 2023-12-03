#include <iostream>
#include <vector>
#include <cmath>
#include <stack>                             
#include <set>
#include <iterator>
#include <utility>
#include <float.h>

using namespace std;

int ROW = -1, COL = -1;

struct cell {
	int parent_i, parent_j;
	double f, g, h;
};

inline bool isValid(int row, int col) {
	return row >= 0 && row < ROW && col >= 0 && col < COL;
}

inline bool isUnBlocked(vector<vector<int>>& grid, int row, int col) {
	return grid[row][col] == 0;
}

inline bool isDestination(int row, int col, pair<int, int>& dest) {
	return row == dest.first && col == dest.second;
}

inline double calculateHValue(int row, int col, pair<int, int>& dest) {
	return (double)sqrt((row - dest.first) * (row - dest.first)+ (col - dest.second) * (col - dest.second));
}

void tracePath(vector<vector<cell>>& cellDetails, pair<int, int>& dest) {

	std::cout << '\n' << "The Path is ";
	int row = dest.first;
	int col = dest.second;

	stack<pair<int, int>> Path;

	while (!(cellDetails[row][col].parent_i == row && cellDetails[row][col].parent_j == col)) {
		Path.push(make_pair(row, col));
		int temp_row = cellDetails[row][col].parent_i;
		int temp_col = cellDetails[row][col].parent_j;
		row = temp_row;
		col = temp_col;
	}

	Path.push(make_pair(row, col));
	while (!Path.empty()) {
		pair<int, int> p = Path.top();
		Path.pop();
		std::cout << "-> (" << p.first << ", " << p.second << ") ";
	}
}

void astar(vector<vector<int>>& grid, pair<int, int>& src, pair<int, int>& dest) {

	if (!isValid(src.first, src.second)) {
		std::cout << "Source is invalid\n";
		return;
	}

	if (!isValid(dest.first, dest.second)) {
		std::cout << "Destination is invalid\n";
		return;
	}

	if (!isUnBlocked(grid, src.first, src.second) || !isUnBlocked(grid, dest.first, dest.second)) {
		std::cout << "Source or the destination is blocked\n";
		return;
	}

	if (isDestination(src.first, src.second, dest)) {
		std::cout << "We are already at the destination\n";
		return;
	}

	vector<vector<bool>> closedList(ROW, vector<bool>(COL, false));

	vector<vector<cell>> cellDetails(ROW, vector<cell>(COL));

	int i, j;

	for (i = 0; i < ROW; i++) {
		for (j = 0; j < COL; j++) {
			cellDetails[i][j].f = FLT_MAX;
			cellDetails[i][j].g = FLT_MAX;
			cellDetails[i][j].h = FLT_MAX;
			cellDetails[i][j].parent_i = -1;
			cellDetails[i][j].parent_j = -1;
		}
	}

	i = src.first, j = src.second;
	cellDetails[i][j].f = 0.0;
	cellDetails[i][j].g = 0.0;
	cellDetails[i][j].h = 0.0;
	cellDetails[i][j].parent_i = i;
	cellDetails[i][j].parent_j = j;

	set<pair<double, pair<int, int>>> openList;

	openList.insert(make_pair(0.0, make_pair(i, j)));

	bool foundDest = false;

	while (!openList.empty()) {
		pair<double, pair<int, int>> p = *openList.begin();

		openList.erase(openList.begin());

		i = p.second.first;
		j = p.second.second;
		closedList[i][j] = true;

		double gNew, hNew, fNew;

        vector<vector<int>> dir = {{0, 1}, {1, 0}, {-1, 0}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};

        for (auto& d : dir) {
            if (isValid(i + d[0], j + d[1])) {
                if (isDestination(i + d[0], j + d[1], dest)) {
                    cellDetails[i + d[0]][j + d[1]].parent_i = i;
                    cellDetails[i + d[0]][j + d[1]].parent_j = j;
                    std::cout << "\nThe destination cell is found\n";
                    tracePath(cellDetails, dest);
                    foundDest = true;
                    return;
                }

                else if (!closedList[i + d[0]][j + d[1]] && isUnBlocked(grid, i + d[0], j + d[1])) {
                    gNew = cellDetails[i][j].g + (d[0] != 0 && d[1] != 0 ? 1.414 : 1.0);
                    hNew = calculateHValue(i + d[0], j + d[1], dest);
                    fNew = gNew + hNew;

                    if (cellDetails[i + d[0]][j + d[1]].f == FLT_MAX || cellDetails[i + d[0]][j + d[1]].f > fNew) {
                        openList.insert(make_pair(fNew, make_pair(i + d[0], j + d[1])));

                        cellDetails[i + d[0]][j + d[1]].f = fNew;
                        cellDetails[i + d[0]][j + d[1]].g = gNew;
                        cellDetails[i + d[0]][j + d[1]].h = hNew;
                        cellDetails[i + d[0]][j + d[1]].parent_i = i;
                        cellDetails[i + d[0]][j + d[1]].parent_j = j;
                    }
                }
            }
        }
	}

	if (!foundDest)
		std::cout << "Destination cannot be reached\n";

	return;
}

int main() {


	std::cout << "Enter the number of rows and columns in the grid (row, col): ";
	std::cin >> ROW >> COL;
	
	vector<vector<int>> grid(ROW, vector<int>(COL));

	std::cout << "\nEnter the grid below\n";
	for (int i = 0; i < ROW; ++i) {
		string dummy;
		std::cin >> dummy;
		for (int j = 0; j < COL; ++j) {
			grid[i][j] = dummy[j] - '0';
		}
	}

	int start_x = -1, start_y = -1;
	int goal_x = -1, goal_y = -1;

	std::cout << "\nEnter the coordinates of the source (x, y): ";
	std::cin >> start_x >> start_y;

	std::cout << "\nEnter the coordinates of the destination (x, y): ";
	std::cin >> goal_x >> goal_y;

	pair<int, int> src = make_pair(start_x, start_y);
	pair<int, int> dest = make_pair(goal_x, goal_y);

	astar(grid, src, dest);

	return 0;
}
