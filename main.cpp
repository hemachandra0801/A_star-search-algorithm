#include <cstdio>
#include <iostream>
#include <curl/curl.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "a_star.h"
#include "simdjson.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

struct graphNode {
  std::string node;
  std::string start;
  std::string depart;
  double distance;
};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}
// assuming input is sorted according to time
// g++ -std=c++17 -I/rapidjson simdjson.cpp main.cpp a_star.cpp -lcurl


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

  std::cout << "\nconnected to backend\n";

  curl_easy_cleanup(curl);

  simdjson::dom::parser parser;
  // simdjson::dom::element receivedJson = parser.parse(receivedData);


	std::string start = "";
	std::string goal = "";
  std::string currTime = "";
	std::unordered_map<std::string, std::vector<graphNode>> graph;
	std::unordered_set<std::string> nodes;

  std::cout << "\nEnter start: ";
  std::cin >> start;
  std::cout << "\nEnter destination: ";
  std::cin >> goal;
  std::cout << "\nEnter current time: ";
  std::cin >> currTime;
  int entries;
  std::cout << "\nNumber of entries: ";
  std::cin >> entries;
  std::cout << "\nEnter the graph in format {node1 node2 startTime departTime dist}\n";
  for (int i = 0; i < entries; ++i) {
    std::string node1, node2, startTime, departTime; 
    double dist;
    std::cin >> node1 >> node2 >> startTime >> departTime >> dist;
    nodes.insert(node1);
    nodes.insert(node2);
    graph[node1].push_back({node2, startTime, departTime, dist});
  }

  // simdjson::dom::element startElement = receivedJson["start"];
  // simdjson::dom::element goalElement = receivedJson["destination"];
  // simdjson::dom::element timeElement = receivedJson["currTime"];
  // simdjson::dom::element graphElement = receivedJson["graph"];


  // if (startElement.is_string() && goalElement.is_string() && graphElement.is_array()) {
  //   start = startElement.get_string().value();
  //   goal = goalElement.get_string().value();
  //   currTime = timeElement.get_string().value();
  //
  //   for (simdjson::dom::element innerArrayElement : graphElement) {
  //     if (innerArrayElement.is_array()) {
  //       double dist;
  //       std::string node1, node2, startTime, departTime;
  //
  //       int currNode = 1;
  //       for (simdjson::dom::element value : innerArrayElement) {
  //         if (value.is_string()) {
  //           switch (currNode) {
  //             case 1:
  //               node1 = value.get_string().value();
  //               break;
  //             case 2:
  //               node2 = value.get_string().value();
  //               break;
  //             case 3:
  //               startTime = value.get_string().value();
  //               break;
  //             case 4:
  //               departTime = value.get_string().value();
  //               break;
  //             default:
  //               std::cerr << "Json format is wrong" << '\n';
  //           }
  //           ++currNode;
  //         }
  //         else if (value.is_double()) {
  //           dist = value.get_double().value();
  //         }
  //         else {
  //           std::cerr << "Error: Unexpected value type in the inner array." << '\n';
  //           return 1;
  //         }
  //       }
  //
  //       nodes.insert(node1);
  //       nodes.insert(node2);
  //       graph[node1].push_back({node2, startTime, departTime, dist});
  //     }
  //   }
  // }
  // else {
  //   std::cerr << "Error: Invalid JSON structure or missing keys." << '\n';
  // }
	

  if (!isValid(start, nodes)) {
		std::cout << "Source is invalid\n";
    return 1;
  }

  if (!isValid(goal, nodes)) {
		std::cout << "Destination is invalid\n";
    return 1;
  }

  std::string optimalPath = astar(graph, start, goal, currTime);

  rapidjson::Document sendJson;
  sendJson.SetObject();
  rapidjson::Document::AllocatorType& allocator = sendJson.GetAllocator();
  // sendJson.AddMember("key", "new_value", allocator);

  if (optimalPath == "Path not found") {
    sendJson.AddMember("optimalPath", "Path not found", allocator);
  }
  else if (optimalPath == "At destination") {
    sendJson.AddMember("optimalPath", "At destination", allocator);
  }
  else {
    rapidjson::Value pathValue;
    pathValue.SetString(optimalPath.c_str(), optimalPath.length(), allocator);
    sendJson.AddMember("optimalPath", pathValue, allocator);
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
  // else {
  //     simdjson::dom::element responseJson = parser.parse(receivedData);
  //     std::cout << "Received data from server: " << responseJson << '\n';
  // }
  
  std::cout << "\nconnected and sent data to backend\n";

  curl_easy_cleanup(curl);
  curl_global_cleanup();


  return 0;
}
