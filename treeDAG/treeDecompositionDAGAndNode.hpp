/*******************************************************************
* Copyright (c) 2010-2013 MiGraNT - DTAI.cs.kuleuven.be
* License details in the root directory of this distribution in
* the LICENSE file
********************************************************************/


#ifndef TREEDAG_TREEDECOMPOSITIONDAGANDNODE_HPP
#define TREEDAG_TREEDECOMPOSITIONDAGANDNODE_HPP

#include "treeDecompositionSubgraph.hpp"
#include "treeDecompositionDAG.hpp"


namespace treeDAG {

class TreeDecompositionDAG;

class TreeDecompositionDAGAndNode
{
public:
    TreeDecompositionDAGAndNode();
    TreeDecompositionDAGAndNode(const TreeDecompositionDAG * dag, TreeDecompositionDAG::Node node_);

    TreeDecompositionDAGLabel::NodeType type() const;
    const TreeDecompositionSubgraph & parent() const;

    const std::vector<std::size_t> & listVertices() const;
    std::size_t extendVertex() const;
    const std::vector<std::size_t> & projectAwayVertices() const;
    const std::vector<std::size_t> & joinVertices() const;

    bool operator==(const TreeDecompositionDAGAndNode & rhs) const;
    bool operator!=(const TreeDecompositionDAGAndNode & rhs) const;

    std::size_t numberOfChildren() const;
    const TreeDecompositionSubgraph & childAt(std::size_t position) const;

private:
    void setComponents(const TreeDecompositionDAG::Graph & structure, TreeDecompositionDAG::Node andNode);
    bool setExtendVertices(const TreeDecompositionDAG & dag);
    bool setListVertices(const TreeDecompositionDAG & dag);
    bool setProjectVertices(const TreeDecompositionDAG & dag);
    bool setJoinVertices(const TreeDecompositionDAG &dag);

    std::vector<std::size_t> vertices;
    const TreeDecompositionSubgraph * parentComponent;
    std::vector<const TreeDecompositionSubgraph *> children_;
    TreeDecompositionDAGLabel::NodeType type_;
};

std::size_t hash_value(const TreeDecompositionDAGAndNode & andNode);


} // namespace mips

#endif // TREEDAG_TREEDECOMPOSITIONDAGANDNODE_HPP
