#include <iostream>
#include <string>
#include <map>
#include <stdexcept>
#include "src/Simulator.hpp"

/**
 * Parse command-line arguments
 * @param argc Argument count
 * @param argv Argument values
 * @return Map of argument names to values
 */
std::map<std::string, std::string> parseArgs(int argc, char* argv[]) {
    std::map<std::string, std::string> args;
    
    // Parse arguments in the form --key value
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        // Check if this is a flag (starts with --)
        if (arg.substr(0, 2) == "--" && i + 1 < argc) {
            std::string key = arg.substr(2);  // Remove "--" prefix
            std::string value = argv[i + 1];
            args[key] = value;
            ++i;  // Skip the next argument since we used it as the value
        }
    }
    
    return args;
}

/**
 * Display usage message
 */
void displayUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " --config <config.json> --tasks <tasks.csv> --ops <ops.csv>" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Required arguments:" << std::endl;
    std::cerr << "  --config <file>  Path to configuration JSON file" << std::endl;
    std::cerr << "  --tasks <file>   Path to tasks CSV file" << std::endl;
    std::cerr << "  --ops <file>     Path to operations CSV file" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Example:" << std::endl;
    std::cerr << "  " << programName << " --config example_config.json --tasks test_tasks.csv --ops test_ops.csv" << std::endl;
}

/**
 * Main entry point
 */
int main(int argc, char* argv[]) {
    try {
        // Parse command-line arguments
        std::map<std::string, std::string> args = parseArgs(argc, argv);
        
        // Validate all required arguments are provided
        if (args.find("config") == args.end() || 
            args.find("tasks") == args.end() || 
            args.find("ops") == args.end()) {
            std::cerr << "Error: Missing required arguments" << std::endl << std::endl;
            displayUsage(argv[0]);
            return 1;
        }
        
        // Extract file paths
        std::string configPath = args["config"];
        std::string tasksPath = args["tasks"];
        std::string opsPath = args["ops"];
        
        // Create Simulator instance
        Simulator simulator;
        
        // Initialize simulator with file paths
        simulator.initialize(configPath, tasksPath, opsPath);
        
        // Run simulation
        simulator.run();
        
        // Success
        return 0;
        
    } catch (const std::exception& e) {
        // Handle exceptions and display error messages
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        // Handle unknown exceptions
        std::cerr << "Error: Unknown exception occurred" << std::endl;
        return 1;
    }
}
