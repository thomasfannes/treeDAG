/*******************************************************************
* Copyright (c) 2010-2013 MiGraNT - DTAI.cs.kuleuven.be
* License details in the root directory of this distribution in
* the LICENSE file
********************************************************************/


#include "treeDecompositionDAGAndNode.hpp"

namespace treeDAG {

TreeDecompositionDAGAndNode::TreeDecompositionDAGAndNode()
    : type_(TreeDecompositionDAGLabel::NODE_Undefined)
{
}



TreeDecompositionDAGAndNode::TreeDecompositionDAGAndNode(const TreeDecompositionDAG * dag, TreeDecompositionDAG::Node node)
    : type_(dag->structure_[node].type)
{
    setComponents(dag->structure_, node);
    bool success = true;

    switch(type())
    {
    case TreeDecompositionDAGLabel::NODE_ChildrenList:
        success = setListVertices(*dag);
        break;

    case TreeDecompositionDAGLabel::NODE_ChildrenExtend:
        success = setExtendVertices(*dag);

        break;

    case TreeDecompositionDAGLabel::NODE_ChildrenProject:
        success = setProjectVertices(*dag);
        break;

    case TreeDecompositionDAGLabel::NODE_ChildrenJoin:
        success = setJoinVertices(*dag);
        break;

    default:
        throw std::logic_error("TreeDecompositionDAGAndNode: Badly structured dag");
    }

    if(!success)
        throw std::logic_error("TreeDecompositionDAGAndNode: Badly structured dag");
}

bool TreeDecompositionDAGAndNode::setExtendVertices(const TreeDecompositionDAG &dag)
{
    const TreeDecompositionSubgraph & childComponent = childAt(0);
    const TreeDecompositionSubgraph & parentComponent = parent();

    for(std::size_t i = 0; i < dag.patternSize(); ++i)
        if(parentComponent[i] == TreeDecompositionSubgraph::ActiveVertex && childComponent[i] == TreeDecompositionSubgraph::UnseenVertex)
            vertices.push_back(i);

    return vertices.size() == 1;
}

bool TreeDecompositionDAGAndNode::setListVertices(const TreeDecompositionDAG & dag)
{
    for(std::size_t i = 0; i < dag.patternSize(); ++i)
        if(parent()[i] == TreeDecompositionSubgraph::ActiveVertex)
            vertices.push_back(i);

    return !vertices.empty();
}

bool TreeDecompositionDAGAndNode::setProjectVertices(const TreeDecompositionDAG & dag)
{
    const TreeDecompositionSubgraph & childComponent = childAt(0);
    const TreeDecompositionSubgraph & parentComponent = parent();

    for(std::size_t i = 0; i < dag.patternSize(); ++i)
        if(parentComponent[i] == TreeDecompositionSubgraph::ProjectedAwayVertex && childComponent[i] == TreeDecompositionSubgraph::ActiveVertex)
            vertices.push_back(i);

    return !vertices.empty();
}

bool TreeDecompositionDAGAndNode::setJoinVertices(const TreeDecompositionDAG & dag)
{
    // find the join nodes
    if(numberOfChildren() < 2)
        return false;

    std::set<std::size_t> activeVertices;
    for(std::size_t i = 0; i < dag.patternSize(); ++i)
    {
        std::size_t activeCount = 0;
        for(std::size_t j = 0; j < numberOfChildren(); ++j)
            if(childAt(j)[i] == TreeDecompositionSubgraph::ActiveVertex)
                ++activeCount;

        if(activeCount > 2)
            activeVertices.insert(activeVertices.end(), i);
    }

    // assign the vertices
    vertices.assign(activeVertices.begin(), activeVertices.end());

    return true;
}

std::size_t TreeDecompositionDAGAndNode::numberOfChildren() const
{
    return children_.size();
}

const TreeDecompositionSubgraph & TreeDecompositionDAGAndNode::childAt(std::size_t position) const
{
    if(position >= numberOfChildren())
        throw std::out_of_range("TreeDecompositionDAGAndNode::childAt: The position is out of range");

    return *children_[position];
}

bool TreeDecompositionDAGAndNode::operator==(const TreeDecompositionDAGAndNode & rhs) const
{
    if(type() != rhs.type() || parent() != rhs.parent() || children_ != rhs.children_)
        return false;

    return true;
}

std::size_t hash_value(const TreeDecompositionDAGAndNode & andNode)
{
    std::size_t seed = hash_value(andNode.parent());
    for(std::size_t i = 0; i < andNode.numberOfChildren(); ++i)
        boost::hash_combine(seed, andNode.childAt(i));

    return seed;
}

bool TreeDecompositionDAGAndNode::operator!=(const TreeDecompositionDAGAndNode & rhs) const
{
    return !operator==(rhs);
}

TreeDecompositionDAGLabel::NodeType TreeDecompositionDAGAndNode::type() const
{
    return type_;
}

const TreeDecompositionSubgraph & TreeDecompositionDAGAndNode::parent() const
{
    return *parentComponent;
}

const std::vector<std::size_t> &TreeDecompositionDAGAndNode::listVertices() const
{
    if(type_ != TreeDecompositionDAGLabel::NODE_ChildrenList)
        throw std::logic_error("TreeDecompositionDAGAndNode: Not a list node");

    return vertices;
}

std::size_t TreeDecompositionDAGAndNode::extendVertex() const
{
    if(type_ != TreeDecompositionDAGLabel::NODE_ChildrenExtend)
        throw std::logic_error("TreeDecompositionDAGAndNode: Not an extend node");

    return vertices.front();
}
const std::vector<std::size_t> & TreeDecompositionDAGAndNode::projectAwayVertices() const
{
    if(type_ != TreeDecompositionDAGLabel::NODE_ChildrenProject)
        throw std::logic_error("TreeDecompositionDAGAndNode: Not a project node");

    return vertices;
}

const std::vector<std::size_t> & TreeDecompositionDAGAndNode::joinVertices() const
{
    if(type_ != TreeDecompositionDAGLabel::NODE_ChildrenJoin)
        throw std::logic_error("TreeDecompositionDAGAndNode: Not a join node");

    return vertices;
}

void TreeDecompositionDAGAndNode::setComponents(const TreeDecompositionDAG::Graph & dag, TreeDecompositionDAG::Node andNode)
{
    type_ = dag[andNode].type;

    {
        typedef boost::graph_traits<TreeDecompositionDAG::Graph>::in_edge_iterator iet;
        std::size_t pos = 0;
        for(std::pair<iet, iet> p = boost::in_edges(andNode, dag); p.first != p.second; ++p.first, ++pos)
        {
            if(pos != 0)
                throw std::logic_error("TreeDecompositionDAGAndNode: Badly structured dag");

            TreeDecompositionDAG::Node parent = boost::source(*p.first, dag);
            parentComponent = &dag[parent].vertices;
        }
    }

    {
        typedef boost::graph_traits<TreeDecompositionDAG::Graph>::out_edge_iterator oei;
        for(std::pair<oei, oei> p = boost::out_edges(andNode, dag); p.first != p.second; ++p.first)
        {
            TreeDecompositionDAG::Node child = boost::target(*p.first, dag);
            children_.push_back(&dag[child].vertices);
        }

        switch(type_)
        {
        case TreeDecompositionDAGLabel::NODE_ChildrenList:
        case TreeDecompositionDAGLabel::NODE_ChildrenExtend:
        case TreeDecompositionDAGLabel::NODE_ChildrenProject:
            if(children_.size() != 1)
                throw std::logic_error("TreeDecompositionDAGAndNode: Badly structured dag");
            break;

        case TreeDecompositionDAGLabel::NODE_ChildrenJoin:
            if(children_.size() <= 1)
                throw std::logic_error("TreeDecompositionDAGAndNode: Badly structured dag");
            break;

        default:
            break;
        }
    }
}

} // treeDAG namespace
