#ifndef TREEDAG_DECOMPOSER_HPP
#define TREEDAG_DECOMPOSER_HPP

#include "separatorCache.hpp"
#include "decompositionDAG.hpp"
#include <stack>

namespace treeDAG {

class Decomposer : public SeparatorConfig
{
public:
    Decomposer();
    Decomposer(const Graph * graph, std::size_t k);


    void initialize();
    template <typename VertexIterator> void process(VertexIterator firstRoot, VertexIterator lastRoot);

    void writeDot(std::ostream & stream) const;

    const DecompositionDAG & decompositionDAG() const { return dag_; }


private:
    typedef std::set<DecompositionDAG::NodeDescriptor> UsedSeparatorNodeSet;

    void process(DecompositionDAG::NodeDescriptor node);
    void tryClique(DecompositionDAG::NodeDescriptor subgraphNode, const VertexSet & oldVertices, const VertexSet & newVertices, boost::unordered_set<UsedSeparatorNodeSet> & cache);
    void trySeparator(const VertexSet & possibleSeparator, const VertexSet & clique, UsedSeparatorNodeSet & usedSeparators);

    void processRoot(DecompositionDAG::NodeDescriptor node);

    DecompositionDAG::NodeDescriptor addSeparatorNode(const Separation & separation, const VertexSet & inactiveIndex);
    SubgraphNodeData createSubgraphNodeData(const VertexSet & separator, const VertexSet & component);



    SeparatorCache cache_;
    std::size_t k_;
    const Graph * graph_;
    VertexSet roots_;
    DecompositionDAG dag_;

    boost::unordered_set<DecompositionDAG::NodeDescriptor> processed_;
    std::stack<DecompositionDAG::NodeDescriptor> todo_;
};

} // namespace treeDAG

#include "decomposer.hxx"

#endif // TREEDAG_DECOMPOSER_HPP
