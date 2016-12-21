#ifndef TREEDAG_DECOMPOSER_HPP
#define TREEDAG_DECOMPOSER_HPP

#include "separatorCache.hpp"
#include "treeDecompositionSubgraph.hpp"
#include "decompositionDAG.hpp"

namespace treeDAG {

template <int SIZE, typename VertexIndexType>
class Decomposer
{
public:
    typedef SeparatorConfig::Graph Graph;

    explicit Decomposer(const Graph * graph, std::size_t k = static_cast<std::size_t>(SIZE));
    Decomposer();

    void initialize();
    template <typename VertexIterator> std::size_t process(VertexIterator firstRoot, VertexIterator lastRoot);

    void writeDot(std::ostream & stream) const;


private:
    typedef std::vector<VertexIndexType> VertexVector;
    typedef typename SeparatorCache<SIZE, VertexIndexType>::ComponentList ComponentList;

    void processSubgraph(const TreeDecompositionSubgraph & subgraph);
    void processClique(const TreeDecompositionSubgraph & subgraph, const VertexVector & oldActives, const VertexVector & newActives);
    void processLeafNode(const TreeDecompositionSubgraph & subgraph, const VertexVector & activeVertices, const VertexVector & projectedVertices);
    void processSeparator(const TreeDecompositionSubgraph & subgraph, const VertexVector & separatorVertices, const ComponentList & separationResult);

    VertexVector extractAllBut(const VertexVector & source, std::size_t elementToForget) const;
    TreeDecompositionSubgraph constructComponent(const TreeDecompositionSubgraph & subgraph, const VertexVector & separatorVertices, const VertexVector & component) const;
    bool hasIntersection(const VertexVector & lhs, const VertexVector & rhs) const;




    SeparatorCache<SIZE, VertexIndexType> cache_;
    std::size_t k_;
    const Graph * graph_;
    std::stack<TreeDecompositionSubgraph> todo_;
    boost::unordered_set<VertexVector> processedSeparators_;
    boost::unordered_set<TreeDecompositionSubgraph> processed_;
    VertexVector roots_;
    DecompositionDAG<VertexIndexType> dag_;
};

} // namespace treeDAG

#include "decomposer.hxx"

#endif // TREEDAG_DECOMPOSER_HPP
