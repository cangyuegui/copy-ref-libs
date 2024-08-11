#include <iostream>
#include <string>
#include <array>
#include <set>
#include <sstream>

std::string do_exec(const std::string& cmd)
{
    std::array<char, 256> buffer;
    std::string result;
    auto pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    try {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            result += buffer.data();
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

std::set<std::string> get_refs(const std::string& input) {
    std::set<std::string> libraryPaths;
    std::istringstream stream(input);
    std::string line;

    while (std::getline(stream, line)) {
        size_t pos = line.find("=>");
        if (pos != std::string::npos) {
            std::string path = line.substr(pos + 3); // Skip "=> "
            size_t endPos = path.find(" ");
            if (endPos != std::string::npos) {
                path = path.substr(0, endPos); // Remove any trailing information after the path
            }
            if (!path.empty() && path[0] == '/') {
                libraryPaths.insert(path);
            }
        }
    }

    return libraryPaths;
}

std::string get_dir(const std::string& filePath) {
    // Find the last occurrence of the directory separator
    std::size_t pos = filePath.find_last_of("/\\");

    // If found, return the substring up to that position
    if (pos != std::string::npos) {
        return filePath.substr(0, pos);
    }

    // If no separator is found, return an empty string or the original path
    return ""; // or return filePath if you prefer
}

std::string get_file_name(const std::string& filePath) {
    // Find the last occurrence of the directory separator
    std::size_t pos = filePath.find_last_of("/\\");

    // If found, return the substring after that position
    if (pos != std::string::npos) {
        return filePath.substr(pos + 1);
    }

    // If no separator is found, return the original path
    return filePath;
}


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "no input exec";
        return 0;
    }

    std::string dir_path = argc < 3 ? get_dir(argv[1]) : std::string(argv[2]);

    std::string ldd_cmd = std::string("ldd \"") + argv[1] + "\"";
    std::string ldd_res = do_exec(ldd_cmd);
    std::cout << ldd_res << std::endl;

    std::set<std::string> refs = get_refs(ldd_res);
    std::set<std::string> trefs;
    std::set<std::string> crefs;

    while (true)
    {
        if (crefs.empty())
            crefs = refs;

        for (auto str : crefs)
        {
            ldd_cmd = std::string("ldd \"") + str + "\"";
            ldd_res = do_exec(ldd_cmd);

            std::set<std::string> rstrs = get_refs(ldd_res);

            for (auto sub : rstrs)
            {
                if (!refs.count(sub))
                {
                    refs.insert(sub);
                    trefs.insert(sub);
                }
            }
        }

        if (trefs.empty())
            break;

        crefs = trefs;
        trefs.clear();
    }

    for (auto str : refs)
    {
        std::string dst_file = dir_path + "/" + get_file_name(str);
        std::string cp_cmd = std::string("cp -f \"") + str + "\" \"" + dst_file + "\"";
        std::cout << str  << " " << "->" << dst_file << std::endl;
        system(cp_cmd.c_str());
    }

    return 0;
}
