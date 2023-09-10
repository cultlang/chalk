#pragma once
#include <memory>
#include <vector>

#include "chalk.hpp"

namespace chalk
{
    class Repository
    {
        std::shared_ptr<Repository> _meta;

        struct Entry {

        };

        std::vector<Entry> _primary;
        std::vector<Entry> _secondary;
        
    };
}
