#ifndef CSVPARSER_HPP
#define CSVPARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>

class CSVParser {
public:
    // Parse CSV file and return rows as vector of maps (column_name -> value)
    static std::vector<std::map<std::string, std::string>> parseCSV(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open CSV file: " + filepath);
        }
        
        std::vector<std::map<std::string, std::string>> rows;
        std::string line;
        std::vector<std::string> headers;
        
        // Read header row
        if (std::getline(file, line)) {
            headers = parseLine(line);
            if (headers.empty()) {
                throw std::runtime_error("CSV file has empty header row: " + filepath);
            }
        } else {
            throw std::runtime_error("CSV file is empty: " + filepath);
        }
        
        // Read data rows
        int lineNumber = 1;
        while (std::getline(file, line)) {
            lineNumber++;
            
            // Skip empty lines
            if (line.empty() || isWhitespace(line)) {
                continue;
            }
            
            std::vector<std::string> values = parseLine(line);
            
            if (values.size() != headers.size()) {
                throw std::runtime_error(
                    "CSV line " + std::to_string(lineNumber) + 
                    " has " + std::to_string(values.size()) + 
                    " fields but header has " + std::to_string(headers.size()) + 
                    " fields in file: " + filepath
                );
            }
            
            // Create map for this row
            std::map<std::string, std::string> row;
            for (size_t i = 0; i < headers.size(); i++) {
                row[headers[i]] = values[i];
            }
            rows.push_back(row);
        }
        
        file.close();
        return rows;
    }

private:
    // Parse a single CSV line, handling quoted fields and empty values
    static std::vector<std::string> parseLine(const std::string& line) {
        std::vector<std::string> fields;
        std::string field;
        bool inQuotes = false;
        
        for (size_t i = 0; i < line.length(); i++) {
            char c = line[i];
            
            if (c == '"') {
                // Toggle quote state
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                // End of field
                fields.push_back(trim(field));
                field.clear();
            } else {
                // Regular character
                field += c;
            }
        }
        
        // Add last field
        fields.push_back(trim(field));
        
        return fields;
    }
    
    // Trim whitespace from both ends of string
    static std::string trim(const std::string& str) {
        size_t start = 0;
        size_t end = str.length();
        
        // Find first non-whitespace
        while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
            start++;
        }
        
        // Find last non-whitespace
        while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
            end--;
        }
        
        return str.substr(start, end - start);
    }
    
    // Check if string is all whitespace
    static bool isWhitespace(const std::string& str) {
        for (char c : str) {
            if (!std::isspace(static_cast<unsigned char>(c))) {
                return false;
            }
        }
        return true;
    }
};

#endif // CSVPARSER_HPP
