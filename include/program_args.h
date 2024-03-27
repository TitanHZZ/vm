#pragma once
#include <string_view>
#include <vector>

// used for argument parsing
namespace program_args {
    bool has_option(const std::vector<std::string_view>& args, const char* option_name) {
        for (const std::string_view& arg: args) {
            if (arg == option_name)
                return true;
        }

        return false;
    }

    std::string_view get_option(const std::vector<std::string_view>& args, const char* option_name) {
        for (auto it = args.begin(); it < args.end(); it++) {
            // check if we found the options and the option has a value provided by the user
            if (*it == option_name && it + 1 < args.end())
                return *(it + 1);
        }

        return "";
    }
}
