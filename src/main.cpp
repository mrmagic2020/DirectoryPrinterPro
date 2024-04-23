#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_set>

#include "include/CLI11.hpp"
#include "include/termstyle.hpp"

namespace fs = std::__fs::filesystem;
namespace ts = termstyle;

const std::string outFile = "dir_tree.txt";
std::ofstream file(outFile, std::ios::app);

bool no_ignore = false, to_file = false, use_prev_cmd = false;
int maxDepth = -1;
std::string dirName = "";
std::unordered_set<std::string> ignoreFiles;

bool checkIgnore(const fs::path &path)
{
    if (no_ignore)
    {
        return false;
    }
    return ignoreFiles.find(path.filename().string()) != ignoreFiles.end() ||
           path.filename().string() == outFile;
}

std::string subtractPath(const fs::path &path, const fs::path &base)
{
    return path.string().substr(base.string().length() + 1);
}

std::string addSpaces(int depth, bool isEnd)
{
    static const std::string levelConnector = "│   ";
    static const std::string lastBranch = "└─ ";
    static const std::string middleBranch = "├─ ";

    if (depth == 0) return "";

    std::string spaces;
    spaces.reserve(depth * 4);

    for (int i = 1; i < depth; ++i) {
        spaces += levelConnector;
    }

    spaces += (isEnd ? lastBranch : middleBranch);

    return spaces;
}

void print(std::string str)
{
    std::cout << str << std::endl;
    if (to_file)
    {
        file << str << std::endl;
    }
}

void printDir(const fs::path &path, const std::unordered_set<std::string> &ignoreFiles, int maxDepth, int depth)
{
    if (maxDepth != -1 && depth > maxDepth) {
        return;
    }
    std::vector<fs::directory_entry> entries;
    for (const auto &entry : fs::directory_iterator(path))
    {
        if (entry.is_directory())
        {
            entries.insert(entries.begin(), entry);
        }
        else
        {
            entries.emplace_back(entry);
        }
    }
    for (size_t i = 0; i < entries.size(); i++)
    {
        auto &entry = entries[i];
        if (checkIgnore(entry.path()))
        {
            continue;
        }
        bool isEnd = (i == entries.size() - 1);
        print(addSpaces(depth, isEnd) + subtractPath(entry.path(), path));

        if (entry.is_directory())
        {
            printDir(entry.path(), ignoreFiles, maxDepth, depth + 1);
        }
    }
}

void initFile()
{
    if (!to_file)
    {
        return;
    }
    std::ofstream file(outFile);
    file.close();
}

void termstyle_init()
{
    ts::addPreset("Info", {
        .prefix = {
            .text = "[INFO] ",
            .prestyles = {ts::Color(ts::Codes::BRIGHT), ts::Color(ts::Codes::FOREGROUND_CYAN)},
            .poststyles = {ts::Color(ts::Codes::BRIGHT_RESET)}
        },
        .suffix = {
            .text = "\n----------",
            .prestyles = {ts::Color(ts::Codes::RESTORE)}
        }
    });

    ts::addPreset("Error", {
        .prefix = {
            .text = "[ERROR] ",
            .prestyles = {ts::Color(ts::Codes::BRIGHT), ts::Color(ts::Codes::FOREGROUND_RED)},
            .poststyles = {ts::Color(ts::Codes::BRIGHT_RESET)}
        }
    });
}

int main(int argc, char *argv[])
{
    termstyle_init();

    CLI::App app;

    app.add_flag("--no-ignore", no_ignore, "Don't ignore files.");
    app.add_flag("--to-file", to_file, "Output to a text file under the working directory.");
    app.add_flag("--use-prev-cmd", use_prev_cmd, "Use the same command executed previously. Commands without this flag and with the --to-file flag will be stored in the file. This will ignore all other flags and options.");

    app.add_option("-d,--depth", maxDepth, "Set recursion depth. A negative value means infinite depth.");
    app.add_option("-n,--name", dirName, "Set the directory name to start from. Only affects the output.");
    app.add_option("--ignore", ignoreFiles, "Ignore files.");

    CLI11_PARSE(app, argc, argv);

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

    if (dirName == "")
    {
        dirName = fs::current_path().filename().string();
    }

    print(dirName);
    printDir(fs::current_path(), ignoreFiles, maxDepth, 1);

    file.close();

    return 0;
}