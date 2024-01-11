#include <iostream>
#include <curl/curl.h>
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



  // Run the algorithm and do the operations here


 
  rapidjson::Document sendJson;
  sendJson.SetObject();
  rapidjson::Document::AllocatorType& allocator = sendJson.GetAllocator();
  sendJson.AddMember("key", "new_value", allocator);

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
