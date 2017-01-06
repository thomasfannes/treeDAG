#ifndef TREEDAG_DECOMPOSER_HPP
#define TREEDAG_DECOMPOSER_HPP

#include "separatorCache.hpp"
#include "treeDecompositionSubgraph.hpp"
#include "decompositionDAG.hpp"

namespace treeDAG {

class Decomposer : public SeparatorConfig
{
public:
    Decomposer();
    Decomposer(const Graph * graph, std::size_t k);


    void initialize();
    template <typename VertexIterator> std::size_t process(VertexIterator firstRoot, VertexIterator lastRoot);

    void writeDot(std::ostream & stream) const;


private:
    SeparatorCache cache_;
    std::size_t k_;
    const Graph * graph_;
    VertexSet roots_;
    DecompositionDAG dag_;
};

} // namespace treeDAG

#include "decomposer.hxx"

#endif // TREEDAG_DECOMPOSER_HPP
