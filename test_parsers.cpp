#include "CSVParser.hpp"
#include "JSONParser.hpp"
#include <iostream>
#include <fstream>
#include <cassert>

void testCSVParser() {
    std::cout << "Testing CSV Parser..." << std::endl;
    
    // Create a test CSV file
    std::ofstream csvFile("test.csv");
    csvFile << "id,name,value,description\n";
    csvFile << "1,Task A,100,\"First task\"\n";
    csvFile << "2,Task B,200,\n";
    csvFile << "3,\"Task C\",300,\"Task with, comma\"\n";
    csvFile.close();
    
    try {
        auto rows = CSVParser::parseCSV("test.csv");
        
        assert(rows.size() == 3);
        assert(rows[0]["id"] == "1");
        assert(rows[0]["name"] == "Task A");
        assert(rows[0]["value"] == "100");
        assert(rows[0]["description"] == "First task");
        
        assert(rows[1]["id"] == "2");
        assert(rows[1]["name"] == "Task B");
        assert(rows[1]["description"] == "");
        
        assert(rows[2]["id"] == "3");
        assert(rows[2]["name"] == "Task C");
        assert(rows[2]["description"] == "Task with, comma");
        
        std::cout << "  ✓ CSV Parser tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "  ✗ CSV Parser test failed: " << e.what() << std::endl;
    }
}

void testJSONParser() {
    std::cout << "Testing JSON Parser..." << std::endl;
    
    // Create a test JSON file
    std::ofstream jsonFile("test.json");
    jsonFile << R"({
        "cores": 16,
        "enabled": true,
        "frequency": 2.5,
        "name": "Test Config",
        "address": "0x80000000",
        "cache": {
            "size": 32768,
            "enabled": false
        },
        "banks": [1, 2, 4, 8]
    })";
    jsonFile.close();
    
    try {
        auto root = JSONParser::parseFile("test.json");
        
        assert(root->isObject());
        assert(root->get("cores")->asInt() == 16);
        assert(root->get("enabled")->asBool() == true);
        assert(root->get("frequency")->asNumber() == 2.5);
        assert(root->get("name")->asString() == "Test Config");
        assert(root->get("address")->asHexAddress() == 0x80000000);
        
        auto cache = root->get("cache");
        assert(cache->isObject());
        assert(cache->get("size")->asInt() == 32768);
        assert(cache->get("enabled")->asBool() == false);
        
        auto banks = root->get("banks");
        assert(banks->isArray());
        assert(banks->asArray().size() == 4);
        assert(banks->asArray()[0]->asInt() == 1);
        assert(banks->asArray()[3]->asInt() == 8);
        
        std::cout << "  ✓ JSON Parser tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "  ✗ JSON Parser test failed: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Running Parser Tests\n" << std::endl;
    
    testCSVParser();
    testJSONParser();
    
    std::cout << "\nAll tests completed!" << std::endl;
    return 0;
}
