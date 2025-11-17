#ifndef JSONPARSER_HPP
#define JSONPARSER_HPP

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <cctype>

// Forward declaration
class JSONValue;

// JSON value types
enum class JSONType {
    Null,
    Boolean,
    Number,
    String,
    Object,
    Array
};

// JSON value class
class JSONValue {
public:
    JSONType type;
    
    // Value storage
    bool boolValue;
    double numberValue;
    std::string stringValue;
    std::map<std::string, std::shared_ptr<JSONValue>> objectValue;
    std::vector<std::shared_ptr<JSONValue>> arrayValue;
    
    JSONValue() : type(JSONType::Null), boolValue(false), numberValue(0.0) {}
    
    explicit JSONValue(bool b) : type(JSONType::Boolean), boolValue(b), numberValue(0.0) {}
    explicit JSONValue(double n) : type(JSONType::Number), boolValue(false), numberValue(n) {}
    explicit JSONValue(const std::string& s) : type(JSONType::String), boolValue(false), numberValue(0.0), stringValue(s) {}
    
    // Type checking
    bool isNull() const { return type == JSONType::Null; }
    bool isBool() const { return type == JSONType::Boolean; }
    bool isNumber() const { return type == JSONType::Number; }
    bool isString() const { return type == JSONType::String; }
    bool isObject() const { return type == JSONType::Object; }
    bool isArray() const { return type == JSONType::Array; }
    
    // Getters with type checking
    bool asBool() const {
        if (type != JSONType::Boolean) {
            throw std::runtime_error("JSON value is not a boolean");
        }
        return boolValue;
    }
    
    double asNumber() const {
        if (type != JSONType::Number) {
            throw std::runtime_error("JSON value is not a number");
        }
        return numberValue;
    }
    
    int asInt() const {
        return static_cast<int>(asNumber());
    }
    
    const std::string& asString() const {
        if (type != JSONType::String) {
            throw std::runtime_error("JSON value is not a string");
        }
        return stringValue;
    }
    
    const std::map<std::string, std::shared_ptr<JSONValue>>& asObject() const {
        if (type != JSONType::Object) {
            throw std::runtime_error("JSON value is not an object");
        }
        return objectValue;
    }
    
    const std::vector<std::shared_ptr<JSONValue>>& asArray() const {
        if (type != JSONType::Array) {
            throw std::runtime_error("JSON value is not an array");
        }
        return arrayValue;
    }
    
    // Object access
    bool hasKey(const std::string& key) const {
        if (type != JSONType::Object) return false;
        return objectValue.find(key) != objectValue.end();
    }
    
    std::shared_ptr<JSONValue> get(const std::string& key) const {
        if (type != JSONType::Object) {
            throw std::runtime_error("JSON value is not an object");
        }
        auto it = objectValue.find(key);
        if (it == objectValue.end()) {
            throw std::runtime_error("JSON object does not have key: " + key);
        }
        return it->second;
    }
    
    // Hex string conversion for addresses
    uint64_t asHexAddress() const {
        const std::string& str = asString();
        uint64_t addr;
        std::stringstream ss;
        
        // Handle "0x" prefix
        if (str.length() > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
            ss << std::hex << str.substr(2);
        } else {
            ss << std::hex << str;
        }
        
        ss >> addr;
        if (ss.fail()) {
            throw std::runtime_error("Failed to parse hex address: " + str);
        }
        
        return addr;
    }
};

// JSON Parser class
class JSONParser {
public:
    static std::shared_ptr<JSONValue> parseFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open JSON file: " + filepath);
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        std::string content = buffer.str();
        size_t pos = 0;
        return parseValue(content, pos);
    }
    
    static std::shared_ptr<JSONValue> parseString(const std::string& jsonStr) {
        size_t pos = 0;
        return parseValue(jsonStr, pos);
    }

private:
    static void skipWhitespace(const std::string& str, size_t& pos) {
        while (pos < str.length() && std::isspace(static_cast<unsigned char>(str[pos]))) {
            pos++;
        }
    }
    
    static std::shared_ptr<JSONValue> parseValue(const std::string& str, size_t& pos) {
        skipWhitespace(str, pos);
        
        if (pos >= str.length()) {
            throw std::runtime_error("Unexpected end of JSON input");
        }
        
        char c = str[pos];
        
        if (c == '{') {
            return parseObject(str, pos);
        } else if (c == '[') {
            return parseArray(str, pos);
        } else if (c == '"') {
            return parseStringValue(str, pos);
        } else if (c == 't' || c == 'f') {
            return parseBool(str, pos);
        } else if (c == 'n') {
            return parseNull(str, pos);
        } else if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
            return parseNumber(str, pos);
        } else {
            throw std::runtime_error("Unexpected character in JSON: " + std::string(1, c));
        }
    }
    
    static std::shared_ptr<JSONValue> parseObject(const std::string& str, size_t& pos) {
        auto obj = std::make_shared<JSONValue>();
        obj->type = JSONType::Object;
        
        pos++; // Skip '{'
        skipWhitespace(str, pos);
        
        if (pos < str.length() && str[pos] == '}') {
            pos++; // Empty object
            return obj;
        }
        
        while (pos < str.length()) {
            skipWhitespace(str, pos);
            
            // Parse key
            if (pos >= str.length() || str[pos] != '"') {
                throw std::runtime_error("Expected string key in JSON object");
            }
            
            auto keyValue = parseStringValue(str, pos);
            std::string key = keyValue->asString();
            
            skipWhitespace(str, pos);
            
            // Expect ':'
            if (pos >= str.length() || str[pos] != ':') {
                throw std::runtime_error("Expected ':' after key in JSON object");
            }
            pos++;
            
            // Parse value
            auto value = parseValue(str, pos);
            obj->objectValue[key] = value;
            
            skipWhitespace(str, pos);
            
            if (pos >= str.length()) {
                throw std::runtime_error("Unexpected end of JSON object");
            }
            
            if (str[pos] == '}') {
                pos++;
                break;
            } else if (str[pos] == ',') {
                pos++;
            } else {
                throw std::runtime_error("Expected ',' or '}' in JSON object");
            }
        }
        
        return obj;
    }
    
    static std::shared_ptr<JSONValue> parseArray(const std::string& str, size_t& pos) {
        auto arr = std::make_shared<JSONValue>();
        arr->type = JSONType::Array;
        
        pos++; // Skip '['
        skipWhitespace(str, pos);
        
        if (pos < str.length() && str[pos] == ']') {
            pos++; // Empty array
            return arr;
        }
        
        while (pos < str.length()) {
            auto value = parseValue(str, pos);
            arr->arrayValue.push_back(value);
            
            skipWhitespace(str, pos);
            
            if (pos >= str.length()) {
                throw std::runtime_error("Unexpected end of JSON array");
            }
            
            if (str[pos] == ']') {
                pos++;
                break;
            } else if (str[pos] == ',') {
                pos++;
            } else {
                throw std::runtime_error("Expected ',' or ']' in JSON array");
            }
        }
        
        return arr;
    }
    
    static std::shared_ptr<JSONValue> parseStringValue(const std::string& str, size_t& pos) {
        pos++; // Skip opening '"'
        std::string result;
        
        while (pos < str.length()) {
            char c = str[pos];
            
            if (c == '"') {
                pos++; // Skip closing '"'
                return std::make_shared<JSONValue>(result);
            } else if (c == '\\') {
                pos++;
                if (pos >= str.length()) {
                    throw std::runtime_error("Unexpected end of string escape sequence");
                }
                
                char escaped = str[pos];
                switch (escaped) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default:
                        throw std::runtime_error("Invalid escape sequence in string");
                }
                pos++;
            } else {
                result += c;
                pos++;
            }
        }
        
        throw std::runtime_error("Unterminated string in JSON");
    }
    
    static std::shared_ptr<JSONValue> parseNumber(const std::string& str, size_t& pos) {
        size_t start = pos;
        
        // Optional minus
        if (pos < str.length() && str[pos] == '-') {
            pos++;
        }
        
        // Integer part
        if (pos >= str.length() || !std::isdigit(static_cast<unsigned char>(str[pos]))) {
            throw std::runtime_error("Invalid number in JSON");
        }
        
        while (pos < str.length() && std::isdigit(static_cast<unsigned char>(str[pos]))) {
            pos++;
        }
        
        // Optional decimal part
        if (pos < str.length() && str[pos] == '.') {
            pos++;
            if (pos >= str.length() || !std::isdigit(static_cast<unsigned char>(str[pos]))) {
                throw std::runtime_error("Invalid number in JSON");
            }
            while (pos < str.length() && std::isdigit(static_cast<unsigned char>(str[pos]))) {
                pos++;
            }
        }
        
        // Optional exponent
        if (pos < str.length() && (str[pos] == 'e' || str[pos] == 'E')) {
            pos++;
            if (pos < str.length() && (str[pos] == '+' || str[pos] == '-')) {
                pos++;
            }
            if (pos >= str.length() || !std::isdigit(static_cast<unsigned char>(str[pos]))) {
                throw std::runtime_error("Invalid number in JSON");
            }
            while (pos < str.length() && std::isdigit(static_cast<unsigned char>(str[pos]))) {
                pos++;
            }
        }
        
        std::string numStr = str.substr(start, pos - start);
        double value = std::stod(numStr);
        
        return std::make_shared<JSONValue>(value);
    }
    
    static std::shared_ptr<JSONValue> parseBool(const std::string& str, size_t& pos) {
        if (str.substr(pos, 4) == "true") {
            pos += 4;
            return std::make_shared<JSONValue>(true);
        } else if (str.substr(pos, 5) == "false") {
            pos += 5;
            return std::make_shared<JSONValue>(false);
        } else {
            throw std::runtime_error("Invalid boolean value in JSON");
        }
    }
    
    static std::shared_ptr<JSONValue> parseNull(const std::string& str, size_t& pos) {
        if (str.substr(pos, 4) == "null") {
            pos += 4;
            return std::make_shared<JSONValue>();
        } else {
            throw std::runtime_error("Invalid null value in JSON");
        }
    }
};

#endif // JSONPARSER_HPP
