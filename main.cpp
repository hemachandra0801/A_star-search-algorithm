#include <cctype>
#include <cstdio>
#include <iostream>
#include <curl/curl.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include "a_star.h"
#include "simdjson.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

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

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}
// assuming input is sorted according to time
// g++ -std=c++17 -I/rapidjson simdjson.cpp main.cpp a_star.cpp -lcurl

inline double timeDifference(std::string& a, std::string& b) {
  int start = (10 * (a[0] - '0') + (a[1] - '0')) * 60 + 10 * (a[2] - '0') + (a[3] - '0');
  int end = (10 * (b[0] - '0') + (b[1] - '0')) * 60 + 10 * (b[2] - '0') + (b[3] - '0');
  return (end - start) * 1.0 / 60;
}


int main() {
  CURL* curl;
  CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (res != CURLE_OK) {
    std::cerr << "curl_global_init() failed" << curl_easy_strerror(res) << '\n';
    return 1;
  }

  curl = curl_easy_init();
  if (!curl) {
    std::cerr << "curl_easy_init() failed" << '\n';
    return 1;
  }

  const char* getUrl = "http://localhost:8000";

  curl_easy_setopt(curl, CURLOPT_URL, getUrl);

  std::string receivedData;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &receivedData);

  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
      curl_easy_cleanup(curl);
      curl_global_cleanup();
      return 1;
  }

  curl_easy_cleanup(curl);

  simdjson::dom::parser parser;
  simdjson::dom::element receivedJson = parser.parse(receivedData);


	std::string start = "";
	std::string goal = "";
  std::string currTime = "";
  std::string filter = "";
	std::unordered_map<std::string, std::vector<graphNode>> graph;
	std::unordered_set<std::string> nodes;

  simdjson::dom::element startElement = receivedJson["start"];
  simdjson::dom::element goalElement = receivedJson["destination"];
  simdjson::dom::element timeElement = receivedJson["currTime"];
  simdjson::dom::element graphElement = receivedJson["graph"];
  simdjson::dom::element filterElement = receivedJson["filter"];

  if (startElement.is_string() && goalElement.is_string() && timeElement.is_string() && filterElement.is_string() && graphElement.is_array()) {
    start = startElement.get_string().value();
    goal = goalElement.get_string().value();
    currTime = timeElement.get_string().value();
    filter = filterElement.get_string().value();
    
    transform(start.begin(), start.end(),start.begin(), ::tolower);
    transform(goal.begin(), goal.end(), goal.begin(), ::tolower);
    transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

    for (simdjson::dom::element innerArrayElement : graphElement) {
      if (innerArrayElement.is_array()) {
        double dist, price, weight;
        std::string node1, node2, mode, name, arrivalTime, departTime, reachTime;

        int currNode = 1;
        for (simdjson::dom::element value : innerArrayElement) {
          if (value.is_string()) {
            switch (currNode) {
              case 1:
                node1 = value.get_string().value();
                transform(node1.begin(), node1.end(), node1.begin(), ::tolower);
                break;
              case 2:
                node2 = value.get_string().value();
                transform(node2.begin(), node2.end(), node2.begin(), ::tolower);
                break;
              case 3:
                mode = value.get_string().value();
                transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
                break;
              case 4:
                name = value.get_string().value();
                transform(name.begin(), name.end(), name.begin(), ::tolower);
                break;
              case 5:
                arrivalTime = value.get_string().value();
                break;
              case 6:
                departTime = value.get_string().value();
                break;
              case 7:
                reachTime = value.get_string().value();
                break;
              default:
                std::cerr << "Json format is wrong" << '\n';
            }
            ++currNode;
          }
          else if (value.is_double()) {
            dist = value.get_double().value();
            currNode = 1;
            switch (currNode) {
              case 1:
                dist = value.get_double().value();
                break;
              case 2:
                price = value.get_double().value();
                break;
              default:
                std::cerr << "Json format is wrong" << '\n';
            }
            ++currNode;
          }
          else {
            std::cerr << "Error: Unexpected value type in the inner array." << '\n';
            return 1;
          }
        }
        
        if (filter == "cost") {
          weight = price;
        }
        else if (filter == "distance") {
          weight = dist;
        }
        else if (filter == "time") {
          weight = timeDifference(arrivalTime, reachTime);
        }
        else {
          std::cerr << "Error: Unexpected value type in the inner array." << '\n';
        }

        nodes.insert(node1);
        nodes.insert(node2);
        graph[node1].push_back({node2, mode, name, arrivalTime, departTime, reachTime, dist, price, weight});
      }
    }
  }
  else {
    std::cerr << "Error: Invalid JSON structure or missing keys." << '\n';
  }
	

  if (!isValid(start, nodes)) {
		std::cout << "Source is invalid\n";
    return 1;
  }

  if (!isValid(goal, nodes)) {
		std::cout << "Destination is invalid\n";
    return 1;
  }

  std::pair<std::pair<double, double>, std::vector<std::vector<std::string>>> optimalPath = astar(graph, start, goal, currTime, filter);

  rapidjson::Document sendJson;
  sendJson.SetObject();
  rapidjson::Document::AllocatorType& allocator = sendJson.GetAllocator();
  // sendJson.AddMember("key", "new_value", allocator);

  if (optimalPath.second.empty()) {
    sendJson.AddMember("optimalPath", "Path not found", allocator);
  }
  else {
    sendJson.AddMember("totalDistance", optimalPath.first.first, allocator);
    sendJson.AddMember("totalPrice", optimalPath.first.second, allocator);

    rapidjson::Value routes(rapidjson::kArrayType);
    for (const auto& route : optimalPath.second) {
      rapidjson::Value routeObj(rapidjson::kArrayType);
      for (const auto& item : route) {
        routeObj.PushBack(rapidjson::StringRef(item.c_str()), allocator);
      }
      routes.PushBack(routeObj, allocator);
    }
    sendJson.AddMember("routes", routes, allocator);
  }

  rapidjson::StringBuffer sendJsonBuffer;
  rapidjson::Writer<rapidjson::StringBuffer> sendJsonWriter(sendJsonBuffer);
  sendJson.Accept(sendJsonWriter);
  std::string sendJsonString = sendJsonBuffer.GetString();

  curl = curl_easy_init();
  if (!curl) {
      std::cerr << "curl_easy_init() failed" << '\n';
      curl_global_cleanup();
      return 1;
  }

  const char* sendUrl = "http://localhost:8001";

  curl_easy_setopt(curl, CURLOPT_URL, sendUrl);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sendJsonString.c_str());

  receivedData.clear();
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &receivedData);

  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
  }
  else {
      simdjson::dom::element responseJson = parser.parse(receivedData);
      std::cout << "Received data from server: " << responseJson << '\n';
  }

  curl_easy_cleanup(curl);
  curl_global_cleanup();


  return 0;
}
