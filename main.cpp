#include <iostream>
#include <curl/curl.h>
#include <string>
#include "a_star.h"
#include "simdjson.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// g++ -std=c++17 -I/rapidjson simdjson.cpp main.cpp -lcurl

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
	std::unordered_map<std::string, std::vector<std::pair<std::string, double>>> graph;
	std::unordered_set<std::string> nodes;

  simdjson::dom::element startElement = receivedJson["start"];
  simdjson::dom::element goalElement = receivedJson["destination"];
  simdjson::dom::element graphElement = receivedJson["graph"];

  if (startElement.is_string() && goalElement.is_string() && graphElement.is_array()) {
    start = startElement.get_string().value();
    goal = goalElement.get_string().value();

    for (simdjson::dom::element innerArrayElement : graphElement) {
      if (innerArrayElement.is_array()) {
        double distance;
        std::string node1, node2;

        int currNode = 1;
        for (simdjson::dom::element value : innerArrayElement) {
          if (value.is_string()) {
            if (currNode++ == 1) {
              node1 = value.get_string().value();
            }
            else {
              node2 = value.get_string().value();
            }
          }
          else if (value.is_double()) {
            distance = value.get_double().value();
          }
          else {
            std::cerr << "Error: Unexpected value type in the inner array." << '\n';
            return 1;
          }
        }

        nodes.insert(node1);
        nodes.insert(node2);
        graph[node1].push_back({node2, distance});
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

  std::string optimalPath = astar(graph, start, goal);

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
    sendJson.AddMember("optimalPath", optimalPath, allocator);
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
