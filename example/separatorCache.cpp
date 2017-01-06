#include <treeDAG/separatorCache.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

typedef treeDAG::SeparatorConfig::Graph Graph;

Graph create_cyle(std::size_t size)
{
    Graph g(size);
    for(std::size_t i = 0; i < size; ++i)
        boost::add_edge(i, (i+1)%size, g);

    return g;
}

int main(int argc, char ** argv)
{
    if(argc != 2)
    {
        std::cerr << "usage: " << argv[0] << " cycle_size" << std::endl;
        return -1;
    }

    std::size_t sz = boost::lexical_cast<std::size_t>(argv[1]);
    Graph g(sz);

    treeDAG::SeparatorCache cache(2, &g);
    cache.initialize();
}

