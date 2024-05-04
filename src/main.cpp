/**
 * @file main.cpp
 */

#include <chrono>         /// For timing
#include <filesystem>     /// For directory iteration
#include <fstream>        /// For file I/O
#include <iostream>       /// For I/O Operations
#include <unordered_set>  /// For ignoring files
#include <vector>         /// For storing directory entries

#include "include/CLI11.hpp"      /// For command line parsing
#include "include/termstyle.hpp"  /// For colored output

namespace fs = std::__fs::filesystem;
namespace ts = termstyle;

const std::string outFile = "dir_tree.txt";  ///< Default output file
std::ofstream file;                          ///< Output file stream

bool no_ignore = false;     ///< Flag to ignore files
bool to_file = false;       ///< Flag to output to a file
bool use_prev_cmd = false;  ///< Flag to use the previous command
int maxDepth = -1;          ///< Default recursion depth (-1 means infinite)
std::string dirName = "";   ///< Default directory name
std::unordered_set<std::string> ignoreFiles;  ///< Files to ignore
std::unordered_set<std::string> noContent;  ///< Directories to ignore contents

/**
 * @brief Check if the file should be ignored.
 * @param path The path to the file.
 * @return True if the file should be ignored, false otherwise.
 */
bool checkIgnore(const fs::path &path)
{
    if (no_ignore)
    {
        return false;
    }
    return ignoreFiles.find(path.filename().string()) != ignoreFiles.end() ||
           path.filename().string() == outFile;
}

/**
 * @brief Subtract the base path from the given path.
 * @details subtractPath("/home/user/file.txt", "/home/user") -> "file.txt"
 * @param path The path to subtract from.
 * @param base The base path.
 */
std::string subtractPath(const fs::path &path, const fs::path &base)
{
    return path.string().substr(base.string().length() + 1);
}

/**
 * @brief Add spaces to the output based on the depth and if it's the end of the
 * branch.
 * @param depth The depth of the current directory.
 * @param isEnd True if it's the end of the branch, false otherwise.
 * @return The string with the spaces.
 */
std::string addSpaces(int depth, bool isEnd)
{
    static const std::string levelConnector = "│   ";  ///< Level connector
    static const std::string lastBranch = "└─ ";       ///< Last branch
    static const std::string middleBranch = "├─ ";     ///< Middle branch

    if (depth == 0) return "";

    std::string spaces;
    spaces.reserve(depth * 4);

    for (int i = 1; i < depth; ++i)
    {
        spaces += levelConnector;
    }

    spaces += (isEnd ? lastBranch : middleBranch);

    return spaces;
}

/**
 * @brief Print the string to the console and the file if the flag is set.
 * @param str The string to print.
 * @return void
 */
void print(std::string str)
{
    std::cout << str << std::endl;
    if (to_file)
    {
        file << str << std::endl;
    }
}

/**
 * @brief Print the directory tree.
 * @param path The path to the directory.
 * @param ignoreFiles The files to ignore.
 * @param maxDepth The maximum depth of recursion.
 * @param depth The current depth of recursion. Default should be 1.
 * @return void
 */
void printDir(const fs::path &path,
              const std::unordered_set<std::string> &ignoreFiles, int maxDepth,
              int depth)
{
    // Check if the maximum depth is reached
    if (maxDepth != -1 && depth > maxDepth)
    {
        return;
    }
    std::vector<fs::directory_entry> entries;  ///< Directory entries
    // Iterate over the directory entries using filesystem
    for (const auto &entry : fs::directory_iterator(path))
    {
        // Directories should be printed first for better readability
        if (entry.is_directory())
        {
            entries.insert(entries.begin(), entry);
        }
        else
        {
            entries.emplace_back(entry);
        }
    }
    // Iterate over the sorted directory entries
    for (size_t i = 0; i < entries.size(); i++)
    {
        auto &entry = entries[i];
        if (checkIgnore(entry.path()))
        {
            continue;
        }
        bool isEnd =
            (i == entries.size() - 1);  ///< Check if it's the end of the branch
        print(addSpaces(depth, isEnd) + subtractPath(entry.path(), path));

        // Recursively call the function if the entry is a directory, excluding
        // the directories in noContent
        if (entry.is_directory() &&
            noContent.find(entry.path().filename().string()) == noContent.end())
        {
            printDir(entry.path(), ignoreFiles, maxDepth, depth + 1);
        }
    }
}

/**
 * @brief Initialize the output file.
 * @details If the flag is set, the file will be created or truncated if it is
 * already present.
 * @return void
 */
void initFile()
{
    if (!to_file)
    {
        return;
    }
    std::ofstream file(outFile);
    file.close();
}

/**
 * @brief Initialize the termstyle presets.
 * @return void
 */
void termstyle_init()
{
    ts::addPreset(
        "Info",
        {.prefix = {.text = "[INFO] ",
                    .prestyles = {ts::Color(ts::Codes::BRIGHT),
                                  ts::Color(ts::Codes::FOREGROUND_CYAN)},
                    .poststyles = {ts::Color(ts::Codes::BRIGHT_RESET)}}});

    ts::addPreset(
        "Error",
        {.prefix = {.text = "[ERROR] ",
                    .prestyles = {ts::Color(ts::Codes::BRIGHT),
                                  ts::Color(ts::Codes::FOREGROUND_RED)},
                    .poststyles = {ts::Color(ts::Codes::BRIGHT_RESET)}}});
}

/**
 * @brief Main function.
 * @param argc The number of command line arguments.
 * @param argv The command line arguments.
 * @return 0 if the program executed successfully, 1 otherwise.
 */
int main(int argc, char *argv[])
{
    const auto start{
        std::chrono::high_resolution_clock::now()};  ///< Start timer
    termstyle_init();

    CLI::App app;  ///< CLI11 App

    // Add command line options

    app.add_flag("--no-ignore", no_ignore, "Don't ignore files.");
    app.add_flag("--to-file", to_file,
                 "Output to a text file under the working directory.");
    app.add_flag("--use-prev-cmd", use_prev_cmd,
                 "Use the same command executed previously. Commands without "
                 "this flag and with the --to-file flag will be stored in the "
                 "file. This will ignore all other flags and options.");

    app.add_option(
        "-d,--depth", maxDepth,
        "Set recursion depth. A negative value means infinite depth.");
    app.add_option(
        "-n,--name", dirName,
        "Set the directory name to start from. Only affects the output.");
    app.add_option("--ignore", ignoreFiles, "Ignore files.");
    app.add_option("--no-content", noContent,
                   "Ignore the contents of specific directories.");

    CLI11_PARSE(app, argc, argv);

    // Check if the previous command should be executed
    if (use_prev_cmd)
    {
        std::ifstream file(outFile);
        std::string line;
        std::getline(file, line);
        if (line.substr(0, 8) == "printdir")
        {
            // std::cout << "Executing previous command: " << line << std::endl;
            ts::print("Info", "Executing previous command: " + line);
            system(line.c_str());
            return 0;
        }
        else
        {
            ts::print("Error", "No previous command found.");
            return 1;
        }
    }

    initFile();
    // Open the file for appending
    if (to_file) file = std::ofstream(outFile, std::ios::app);

    // Print the command to the file if the flag is set and the previous command
    // is not being used
    if (!use_prev_cmd)
    {
        std::string cmd = "printdir";
        for (int i = 1; i < argc; i++)
        {
            cmd += " ";
            cmd += argv[i];
        }
        print(cmd);
    }

    // If the directory name is not specified, use the current directory
    if (dirName == "")
    {
        dirName = fs::current_path().filename().string();
    }

    // Print the directory tree
    print(dirName);
    printDir(fs::current_path(), ignoreFiles, maxDepth, 1);

    file.close();

    const auto end{std::chrono::high_resolution_clock::now()};  ///< End timer
    const std::chrono::duration<double> elapsed{end - start};  ///< Elapsed time
    ts::print("Info",
              "Execution time: " + std::to_string(elapsed.count()) + "s");

    return 0;
}