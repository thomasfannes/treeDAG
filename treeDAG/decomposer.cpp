
#include "decomposer.hpp"


namespace treeDAG {

Decomposer::Decomposer(const Graph * graph, std::size_t k)
    : cache_(k, graph),
      k_(k),
      graph_(graph)
{
}

Decomposer::Decomposer()
    : k_(0),
      graph_(0)
{
}

void  Decomposer::initialize()
{
    cache_.initialize();
}


void Decomposer::writeDot(std::ostream & stream) const
{
    dag_.write_dot(stream);
}

} // namespace treeDAG

