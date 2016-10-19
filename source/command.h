#pragma once
#include "util.h"

std::string GetCommand();

template <typename T>
bool ParseParams(const std::vector<std::string>& params, T parser) {
    for (unsigned i = 1; i < params.size(); ++i) {
        if (params[i].size() < 1) continue;
        if (!parser(params[i][0], params[i].substr(1))) {
            Log("ParseParams: unknown param \"%s\"\n", params[i].data());
            return false;
        }
    }
    return true;
}
