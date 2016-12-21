#include "util.hpp"

treeDAG::SeparatorConfig::Graph make_path(std::size_t size)
{
    treeDAG::SeparatorConfig::Graph g(size);

    for(std::size_t i = 1; i < size; ++i)
        boost::add_edge(i-1, i, g);

    return g;
}

treeDAG::SeparatorConfig::Graph make_cycle(std::size_t size)
{
    treeDAG::SeparatorConfig::Graph g = make_path(size);
    boost::add_edge(0, size-1, g);

    return g;
}
