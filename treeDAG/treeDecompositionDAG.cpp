/*******************************************************************
* Copyright (c) 2010-2013 MiGraNT - DTAI.cs.kuleuven.be
* License details in the root directory of this distribution in
* the LICENSE file
********************************************************************/

#include "treeDecompositionDAG.hpp"
#include <iostream>
#include <stack>
#include <queue>
#include <set>
#include <algorithm>
#include <fstream>
#include <iostream>

#include <boost/graph/topological_sort.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "treeDecompositionDAGAndNode.hpp"

#include <boost/graph/graphviz.hpp>

namespace treeDAG {

TreeDecompositionDAG::AndNodeTransformer::AndNodeTransformer()
    : dag_(0)
{
}

TreeDecompositionDAG::AndNodeTransformer::AndNodeTransformer(bool isSource, const TreeDecompositionDAG * dag)
    : dag_(dag),
      isSource_(isSource)
{
}

TreeDecompositionDAGAndNode TreeDecompositionDAG::AndNodeTransformer::operator()(boost::graph_traits<Graph>::edge_descriptor edge) const
{
    return TreeDecompositionDAGAndNode(dag_, isSource_ ? boost::source(edge, dag_->structure_) : boost::target(edge, dag_->structure_) );
}

TreeDecompositionDAG::ComponentTransformer::ComponentTransformer(const Graph * graph)
    : graph_(graph)
{
}

const TreeDecompositionSubgraph & TreeDecompositionDAG::ComponentTransformer::operator()(boost::graph_traits<Graph>::vertex_descriptor vd) const
{
    return graph_->operator [](vd).vertices;
}


TreeDecompositionDAG::ComponentFilter::ComponentFilter(const Graph * graph)
    : graph_(graph)
{
}

bool TreeDecompositionDAG::ComponentFilter::operator()(const TreeDecompositionSubgraph & component) const
{
    return component.size() != 0;
}


TreeDecompositionDAG::TreeDecompositionDAG(std::size_t patternSize)
    : patternSize_(patternSize)
{

}

TreeDecompositionDAG::TreeDecompositionDAG(const TreeDecompositionDAG & rhs)
    : structure_(rhs.structure_),
      patternSize_(rhs.patternSize_)
{
    refillMap();
}

void TreeDecompositionDAG::reset(std::size_t patternSize)
{
    structure_.clear();
    map_.clear();
    patternSize_ = patternSize;
}

TreeDecompositionDAG & TreeDecompositionDAG::operator=(const TreeDecompositionDAG & rhs)
{
    if(&rhs == this)
        return *this;

    patternSize_ = rhs.patternSize();
    structure_ = rhs.structure_;

    refillMap();

    return *this;
}

std::size_t TreeDecompositionDAG::patternSize() const
{
    return patternSize_;
}

std::size_t TreeDecompositionDAG::numComponents() const
{
    return map_.size();
}

bool TreeDecompositionDAG::hasComponent(const TreeDecompositionSubgraph & component) const
{
    return map_.find(component) != map_.end();
}

void TreeDecompositionDAG::addComponent(const TreeDecompositionSubgraph & component)
{
    addOrFindNode(component);
}

std::pair<TreeDecompositionDAG::ChildIterator, TreeDecompositionDAG::ChildIterator> TreeDecompositionDAG::children(const TreeDecompositionSubgraph & component) const
{
    Node n = findExistingNode(component);
    if(n == invalidDescriptor())
        return std::make_pair(ChildIterator(), ChildIterator());

    typedef boost::graph_traits<Graph>::out_edge_iterator oei;
    std::pair<oei, oei> p = boost::out_edges(n, structure_);

    return std::make_pair(
                boost::make_transform_iterator(p.first, AndNodeTransformer(false, this)),
                boost::make_transform_iterator(p.second, AndNodeTransformer(false, this))
                );
}

std::pair<TreeDecompositionDAG::ParentIterator, TreeDecompositionDAG::ParentIterator> TreeDecompositionDAG::parents(const TreeDecompositionSubgraph &component) const
{
    Node n = findExistingNode(component);
    if(n == invalidDescriptor())
        return std::make_pair(ParentIterator(), ParentIterator());

    typedef boost::graph_traits<Graph>::in_edge_iterator iet;
    std::pair<iet, iet> p = boost::in_edges(n, structure_);

    return std::make_pair(
                boost::make_transform_iterator(p.first, AndNodeTransformer(true, this)),
                boost::make_transform_iterator(p.second, AndNodeTransformer(true, this))
                );
}

std::pair<TreeDecompositionDAG::ComponentIterator, TreeDecompositionDAG::ComponentIterator> TreeDecompositionDAG::components() const
{
    typedef boost::graph_traits<Graph>::vertex_iterator vit;
    typedef boost::transform_iterator<ComponentTransformer, vit> transform_it;

    ComponentTransformer transformer(&structure_);
    ComponentFilter filter(&structure_);

    std::pair<vit, vit> p = boost::vertices(structure_);
    std::pair<transform_it, transform_it> p2(
                boost::make_transform_iterator(p.first, transformer),
                boost::make_transform_iterator(p.second, transformer)
                );

    return std::make_pair(
        boost::make_filter_iterator(filter, p2.first, p2.second),
        boost::make_filter_iterator(filter, p2.second, p2.second)
                );
}

TreeDecompositionDAG::Node TreeDecompositionDAG::findExistingNode(const TreeDecompositionSubgraph & component) const
{
    boost::unordered_map<TreeDecompositionSubgraph, Node>::const_iterator it = map_.find(component);
    if(it == map_.end())
        return invalidDescriptor();

    return it->second;
}


const std::pair<TreeDecompositionSubgraph, TreeDecompositionDAG::Node> TreeDecompositionDAG::addOrFindNode(const TreeDecompositionSubgraph & component)
{
    boost::unordered_map<TreeDecompositionSubgraph, Node>::const_iterator it = map_.find(component);

    // a new node
    if(it == map_.end())
    {
        TreeDecompositionDAGLabel lbl;
        lbl.vertices = component;
        lbl.type = TreeDecompositionDAGLabel::NODE_ChildrenOR;
        Node n = boost::add_vertex(lbl, structure_);

        it = map_.insert(std::make_pair(component, n)).first;
    }

    return *it;
}
void TreeDecompositionDAG::remapIndices(const std::vector<std::size_t> &newIndices)

{
    if(newIndices.size() != patternSize())
        throw std::logic_error("Not a valid indices map");

    // count them all
    bool valid = true;
    std::vector<std::size_t> cnt(patternSize());

    // set the new indices
    for(std::size_t i =0; i < newIndices.size() && valid; ++i)
        if(newIndices[i] >= patternSize())
            valid = false;
        else
            ++cnt[newIndices[i]];

    for(std::size_t i = 0; i < cnt.size(); ++i)
        if(cnt[i] != 1)
            valid = false;

    if(!valid)
        throw std::logic_error("Not a valid indices map");

    typedef boost::graph_traits<Graph>::vertex_iterator vit;
    for(std::pair<vit, vit> p = boost::vertices(structure_); p.first != p.second; ++p.first)
    {
        TreeDecompositionDAGLabel & lbl = structure_[*p.first];
        if(lbl.type == TreeDecompositionDAGLabel::NODE_ChildrenOR)
            remapComponent(newIndices, lbl.vertices);
    }

    map_.clear();
    refillMap();
}

void TreeDecompositionDAG::remapComponent(const std::vector<std::size_t> & newIndices, TreeDecompositionSubgraph & subgraph)
{
    TreeDecompositionSubgraph newComp(subgraph.size());

    for(std::size_t i = 0; i < subgraph.size(); ++i)
        newComp[newIndices[i]] = subgraph[i];

    std::swap(subgraph, newComp);
}

TreeDecompositionSubgraph TreeDecompositionDAG::addUnaryAnd(const TreeDecompositionSubgraph & child, const TreeDecompositionSubgraph & parent, TreeDecompositionDAGLabel::NodeType type)
{
    const std::pair<TreeDecompositionSubgraph, Node> & c = addOrFindNode(child);
    const std::pair<TreeDecompositionSubgraph, Node> & p = addOrFindNode(parent);

    // check whether this link already exists
    for(std::pair<ChildIterator, ChildIterator> p1 = children(parent); p1.first != p1.second; ++p1.first)
    {
        TreeDecompositionDAGAndNode andNode = *p1.first;
        if(andNode.type() == type && andNode.childAt(0) == child)
            return p.first;
    }


    // add the link
    TreeDecompositionDAGLabel lbl;
    lbl.type = type;
    Node andNode = boost::add_vertex(lbl, structure_);

    boost::add_edge(p.second, andNode, structure_);
    boost::add_edge(andNode, c.second, structure_);

    return p.first;
}

TreeDecompositionSubgraph TreeDecompositionDAG::addNaryAnd(std::vector<TreeDecompositionSubgraph> childComponents, const TreeDecompositionSubgraph & parent, TreeDecompositionDAGLabel::NodeType type)
{
    // sort the children
    std::sort(childComponents.begin(), childComponents.end());

    // find the parent node
    const std::pair<TreeDecompositionSubgraph, Node> & p = addOrFindNode(parent);


    // check whether this link already exists
    for(std::pair<ChildIterator, ChildIterator> p1 = children(parent); p1.first != p1.second; ++p1.first)
    {
        const TreeDecompositionDAGAndNode & andNode = *p1.first;
        if(andNode.type() != type || andNode.numberOfChildren() != childComponents.size())
            continue;

        bool allSame = true;
        for(std::size_t i = 0; i < andNode.numberOfChildren() && allSame; ++i)
            if(andNode.childAt(i) != childComponents[i])
                allSame = false;

        if(allSame)
            return p.first;
    }

    // now find the child nodes
    std::vector<Node> childNodes(childComponents.size());
    for(std::size_t i = 0; i < childNodes.size(); ++i)
        childNodes[i] = addOrFindNode(childComponents[i]).second;

    // add the link
    TreeDecompositionDAGLabel lbl;
    lbl.type = type;
    Node andNode = boost::add_vertex(lbl, structure_);

    boost::add_edge(p.second, andNode, structure_);
    for(std::size_t i = 0; i < childNodes.size(); ++i)
        boost::add_edge(andNode, childNodes[i], structure_);

    return p.first;
}

TreeDecompositionSubgraph TreeDecompositionDAG::addBinaryAnd(const TreeDecompositionSubgraph & childA, const TreeDecompositionSubgraph & childB, const TreeDecompositionSubgraph & parent, TreeDecompositionDAGLabel::NodeType type)
{
    std::vector<TreeDecompositionSubgraph> nodes(2);
    nodes[0] = childA;
    nodes[1] = childB;

    return addNaryAnd(nodes, parent, type);
}

// addition methods
TreeDecompositionSubgraph TreeDecompositionDAG::addList(std::size_t vertex)
{
    if(vertex > patternSize())
        throw errOutOfBounds();

    TreeDecompositionSubgraph child(patternSize());
    TreeDecompositionSubgraph parent(patternSize());
    parent[vertex] = TreeDecompositionSubgraph::ActiveVertex;

    return addUnaryAnd(child, parent, TreeDecompositionDAGLabel::NODE_ChildrenList);
}

TreeDecompositionSubgraph TreeDecompositionDAG::addExtend(const TreeDecompositionSubgraph & child, std::size_t vertexToExtend)
{
    if(vertexToExtend > patternSize())
        throw errOutOfBounds();

    TreeDecompositionSubgraph parent(child);

    if(parent[vertexToExtend] != TreeDecompositionSubgraph::UnseenVertex)
        throw errNotUnseen();

    parent[vertexToExtend] = TreeDecompositionSubgraph::ActiveVertex;

    return addUnaryAnd(child, parent, TreeDecompositionDAGLabel::NODE_ChildrenExtend);
}

TreeDecompositionSubgraph TreeDecompositionDAG::addProject(const TreeDecompositionSubgraph & child, std::size_t vertexToProjectAway)
{
    if(vertexToProjectAway > patternSize())
        throw errOutOfBounds();

    TreeDecompositionSubgraph parent(child);
    if(parent[vertexToProjectAway] != TreeDecompositionSubgraph::ActiveVertex)
        throw errNotActive();

    parent[vertexToProjectAway] = TreeDecompositionSubgraph::ProjectedAwayVertex;

    return addUnaryAnd(child, parent, TreeDecompositionDAGLabel::NODE_ChildrenProject);
}

TreeDecompositionSubgraph TreeDecompositionDAG::addJoin(const TreeDecompositionSubgraph & lhsComponent, const TreeDecompositionSubgraph & rhsComponent)
{   
    TreeDecompositionSubgraph parent(lhsComponent);
    parent += rhsComponent;

    return addBinaryAnd(lhsComponent, rhsComponent, parent, TreeDecompositionDAGLabel::NODE_ChildrenJoin);
}


// removal method
void TreeDecompositionDAG::removeNode(const TreeDecompositionSubgraph & component)
{
    boost::unordered_map<TreeDecompositionSubgraph, Node>::iterator it = map_.find(component);
    if(it == map_.end())
        return;

    Node n = it->second;

    typedef boost::graph_traits<Graph>::in_edge_iterator iet;
    typedef boost::graph_traits<Graph>::out_edge_iterator oei;

    std::set<Node> toRemove;
    for(std::pair<iet, iet> p = boost::in_edges(n, structure_); p.first != p.second; ++p.first)
        toRemove.insert(boost::source(*p.first, structure_));
    for(std::pair<oei, oei> p = boost::out_edges(n, structure_); p.first != p.second; ++p.first)
        toRemove.insert(boost::target(*p.first, structure_));


    for(std::set<Node>::const_iterator it = toRemove.begin(); it != toRemove.end(); ++it)
    {
        Node n2 = *it;
        boost::clear_vertex(n2, structure_);
        boost::remove_vertex(n2, structure_);
    }

    boost::remove_vertex(n, structure_);
    map_.erase(it);
}


void TreeDecompositionDAG::removeSubtree(const TreeDecompositionSubgraph & component)
{
    // find the root node
    Node root = findExistingNode(component);
    if(root == invalidDescriptor())
        return;



    // visit the subtree which belongs uniquely to this node
    boost::unordered_map<Node, std::size_t> degreeMap;
    boost::unordered_set<Node> toRemove;
    std::queue<Node> todo;

    degreeMap.insert(std::make_pair(root, boost::in_degree(root, structure_)));
    todo.push(root);

    while(!todo.empty())
    {
        Node cur = todo.front();
        todo.pop();

        toRemove.insert(cur);

        // check the children of the children
        typedef boost::graph_traits<Graph>::out_edge_iterator oei;
        for(std::pair<oei, oei> pAnd = boost::out_edges(cur, structure_); pAnd.first != pAnd.second; ++pAnd.first)
        {
            // add the and node
            Node andNode = boost::target(*pAnd.first, structure_);
            toRemove.insert(andNode);

            // and add the children which reside only in the subtree
            for(std::pair<oei, oei> pChild = boost::out_edges(andNode, structure_); pChild.first != pChild.second; ++pChild.first)
            {
                Node childNode = boost::target(*pChild.first, structure_);
                std::size_t & curD = degreeMap[childNode];
                ++curD;

                // have we seen al the in edges?
                if(boost::in_degree(childNode, structure_) == curD)
                    todo.push(childNode);
            }
        }
    }

    // add the and node parent of the component
    {
        typedef boost::graph_traits<Graph>::in_edge_iterator iet;
        for(std::pair<iet, iet> p = boost::in_edges(root, structure_); p.first != p.second; ++p.first)
            toRemove.insert(boost::source(*p.first, structure_));

    }

    // first clear all the nodes (edges)
    for(boost::unordered_set<Node>::const_iterator it = toRemove.begin(); it != toRemove.end(); ++it)
    {
        Node node = *it;
        boost::clear_vertex(node, structure_);

        // remove from the map if it is an a or node
        if(structure_[node].type == TreeDecompositionDAGLabel::NODE_ChildrenOR)
            map_.erase(structure_[node].vertices);

        // and remove from the graph
        boost::remove_vertex(node, structure_);
    }
}

std::logic_error TreeDecompositionDAG::errOutOfBounds() const
{
    return std::logic_error("TreeDecompositionDAG: The supplied vertex is out of bound");
}
std::logic_error TreeDecompositionDAG::errNotActive() const
{
    return std::logic_error("TreeDecompositionDAG: The supplied vertex is not active");
}
std::logic_error TreeDecompositionDAG::errAlreadyActive() const
{
    return std::logic_error("TreeDecompositionDAG: The supplied vertex is active");
}

std::logic_error TreeDecompositionDAG::errBadRange() const
{
    return std::logic_error("TreeDecompositionDAG: The supplied range is invalid");
}

std::logic_error TreeDecompositionDAG::errNotUnseen() const
{
    return std::logic_error("TreeDecompositionDAG: The supplied vertex is not unseen");
}

TreeDecompositionDAG::Node TreeDecompositionDAG::invalidDescriptor()
{
    return boost::graph_traits<Graph>::null_vertex();
}

TreeDecompositionDAG::InsertResult TreeDecompositionDAG::checkForInsertion(const TreeDecompositionSubgraph & component, const TreeDecompositionSubgraph & toInsert) const
{
    bool thisPossible = true;
    for(std::size_t i = 0; i < patternSize(); ++i)
    {
        if(toInsert[i] == TreeDecompositionSubgraph::ActiveVertex)
        {
            if(component[i] == TreeDecompositionSubgraph::UnseenVertex)
                return NotPossible;
            else if(component[i] == TreeDecompositionSubgraph::ProjectedAwayVertex)
                thisPossible = false;
        }
        else if(toInsert[i] == TreeDecompositionSubgraph::ProjectedAwayVertex)
        {
            if(component[i] != TreeDecompositionSubgraph::UnseenVertex)
                return NotPossible;
        }
    }

    return thisPossible ? CanInsert : MaybeIntoChildren;
}

bool TreeDecompositionDAG::constructInsertColorMap(Node rootNode, std::set<TreeDecompositionSubgraph> insertNodes, std::map<Node, InsertResult> & map) const
{
    map.clear();

    std::stack<Node> todo;
    todo.push(rootNode);
    std::set<Node> visitedNodes;
    std::stack<Node> toVisit;

    // depth first visiting of all nodes
    while(todo.empty())
    {
        Node curNode = todo.top();
        todo.pop();

        // bookkeeping
        if(visitedNodes.count(curNode) > 0)
            continue;
        visitedNodes.insert(curNode);

        // get the label
        const TreeDecompositionSubgraph & comp = structure_[curNode].vertices;

        // is this one of the nodes we are looking for?
        std::set<TreeDecompositionSubgraph>::iterator it = insertNodes.find(comp);
        if(it != insertNodes.end())
        {
            map.insert(std::make_pair(curNode, CanInsert));
            toVisit.push(curNode);
        }

        // and continue the of the children
        typedef boost::graph_traits<Graph>::in_edge_iterator iet;
        for(std::pair<iet, iet> p1 = boost::in_edges(curNode, structure_); p1.first != p1.second; ++p1.first)
        {
            Node andNode = boost::source(*p1.first, structure_);
            for(std::pair<iet, iet> p2 = boost::in_edges(andNode, structure_); p2.first != p2.second; ++p2.first)
            {
                Node childNode = boost::source(*p2.first, structure_);
                todo.push(childNode);
            }
        }
    }

    // check if all nodes found
    if(insertNodes.size() != map.size())
        return false;

    // no go bottom up, and tag all nodes (till rootnode)
    std::set<Node> visitedParents;
    while(!toVisit.empty())
    {
        Node curNode = toVisit.top();
        toVisit.pop();

        // bookkeeping
        if(!visitedParents.insert(curNode).second)
            continue;

        // if no color, assign intoChild color
        if(map.count(curNode) == 0)
            map.insert(std::make_pair(curNode, MaybeIntoChildren));

        // and go to the parents
        typedef boost::graph_traits<Graph>::out_edge_iterator oei;
        for(std::pair<oei, oei> p1 = boost::out_edges(curNode, structure_); p1.first != p1.second; ++p1.first)
        {
            Node andNode = boost::target(*p1.first, structure_);
            Node parentNode = boost::target(*boost::out_edges(andNode, structure_).first, structure_);

            if(visitedNodes.count(parentNode) > 0)
            {
                map.insert(std::make_pair(andNode, MaybeIntoChildren));
                toVisit.push(parentNode);
            }
        }
    }

    return true;
}

TreeDecompositionDAG::InsertResult TreeDecompositionDAG::findInMap(const TreeDecompositionSubgraph & component, const InsertionMap & map) const
{
    InsertionMap::const_iterator it = map.find(component);
    return it == map.end() ? NotPossible : it->second;
}

TreeDecompositionSubgraph TreeDecompositionDAG::insert(const TreeDecompositionSubgraph & rootComponent, const TreeDecompositionSubgraph & toInsertComponent, const InsertionMap & insertionMap)
{
    // check for validity
    if(!hasComponent(rootComponent))
        throw errBadRange();

    if(findInMap(rootComponent, insertionMap) == NotPossible)
        throw errBadRange();


    std::stack<TreeDecompositionSubgraph> todo;
    std::set<TreeDecompositionSubgraph> handledNodes;
    todo.push(rootComponent);

    while(!todo.empty())
    {
        TreeDecompositionSubgraph curComp = todo.top();
        todo.pop();

        // already handled?
        if(!handledNodes.insert(curComp).second)
            continue;

        InsertResult result = findInMap(curComp, insertionMap);
        if(result == NotPossible)
            continue;

        if(result == CanInsert)
        {
            if(!partial_order_compare(curComp, toInsertComponent))
                addJoin(curComp, toInsertComponent);
        }

        // loop over the children
        for(std::pair<ChildIterator, ChildIterator> p = children(curComp); p.first != p.second; ++p.first)
        {
            const TreeDecompositionDAGAndNode & andNode = *p.first;

            switch(andNode.type())
            {
            case TreeDecompositionDAGLabel::NODE_ChildrenExtend:
            {
                const TreeDecompositionSubgraph & child = andNode.childAt(0);
                if(findInMap(child, insertionMap) != NotPossible)
                {
                    todo.push(child);
                    addExtend(createInsertComponent(child, toInsertComponent), andNode.extendVertex());
                }

                break;
            }

            case TreeDecompositionDAGLabel::NODE_ChildrenProject:
            {
                const TreeDecompositionSubgraph & child = andNode.childAt(0);
                if(findInMap(child, insertionMap) != NotPossible)
                {
                    todo.push(child);
                    addProject(createInsertComponent(child, toInsertComponent), andNode.projectAwayVertices().begin(), andNode.projectAwayVertices().end());
                }

                break;
            }


            case TreeDecompositionDAGLabel::NODE_ChildrenJoin:
            {
                std::vector<TreeDecompositionSubgraph> children(andNode.numberOfChildren());

                for(std::size_t i = 0; i < andNode.numberOfChildren(); ++i)
                    children[i] = andNode.childAt(i);

                for(std::size_t i = 0; i < andNode.numberOfChildren(); ++i)
                {
                    if(findInMap(children[i], insertionMap) != NotPossible)
                    {
                        todo.push(children[i]);
                        children[i] = createInsertComponent(children[i], toInsertComponent);
                        addJoin(children.begin(), children.end());

                        children[i] = andNode.childAt(i);
                    }
                }
                break;
            }

            default:
                break;
            }

        }
    }

    return createInsertComponent(rootComponent, toInsertComponent);
}

TreeDecompositionSubgraph TreeDecompositionDAG::createInsertComponent(const TreeDecompositionSubgraph & node, const TreeDecompositionSubgraph & toInsertComponent) const
{
    TreeDecompositionSubgraph result(patternSize());

    for(std::size_t i = 0 ; i < patternSize(); ++i)
        result[i] = (node[i] == TreeDecompositionSubgraph::UnseenVertex ? toInsertComponent[i] : node[i]);

    return result;
}



namespace {

template <typename LabelMap>
struct TreeDecompositionDAGLabel_DOTWriter
{
    typedef typename boost::property_traits<LabelMap>::key_type KeyType;
    typedef std::set<TreeDecompositionSubgraph> Keys;

    TreeDecompositionDAGLabel_DOTWriter(const LabelMap & map, const Keys & selectedKeys = Keys())
        : map_(map),
          selectedKeys_(selectedKeys)
    {
    }

    template <typename VertexDescriptor>
    void operator()(std::ostream & str, const VertexDescriptor & vd) const
    {
        typedef typename boost::property_traits<LabelMap>::value_type LabelType;
        const LabelType & label = boost::get(map_, vd);

        str << "[label=\"";

        if((label.type & TreeDecompositionDAGLabel::NODE_ORANDMASK) == TreeDecompositionDAGLabel::NODE_ChildrenAND)
        {
            switch(label.type)
            {
            case TreeDecompositionDAGLabel::NODE_ChildrenExtend:
                str << "Extend";
                break;
            case TreeDecompositionDAGLabel::NODE_ChildrenJoin:
                str << "Join";
                break;
            case TreeDecompositionDAGLabel::NODE_ChildrenList:
                str << "List";
                break;
            case TreeDecompositionDAGLabel::NODE_ChildrenProject:
                str << "Project";
                break;
            default:
                str << "Unknown";
                break;
            }
        }
        else
        {
            str << "{";
            unsigned int written = 0;
            for(unsigned int i = 0; i < label.vertices.size(); ++i)
                if(label.vertices[i] == TreeDecompositionSubgraph::ActiveVertex)
                {
                    if(written++ != 0)
                        str << ", ";
                    str << "v" << i;
                }
            str << "}, *{";
            written = 0;
            for(unsigned int i = 0; i < label.vertices.size(); ++i)
                if(label.vertices[i] == TreeDecompositionSubgraph::ProjectedAwayVertex)
                {
                    if(written++ != 0)
                        str << ", ";
                    str << "v" << i;
                }
            str << "}";
        }

        str << "\"";

        if(selectedKeys_.count(label.vertices) > 0)
            str << "color=red; style=filled;";


        str << "]";
    }

    LabelMap map_;
    Keys selectedKeys_;

};



template <typename VertexLabelWriter, typename DAGType>
void write_dot_format_helper(
        const DAGType & dag,
        std::ostream & stream,
        const std::set<TreeDecompositionSubgraph> & chosenNodes = std::set<TreeDecompositionSubgraph>())
{
    typedef typename boost::property_map<DAGType, boost::vertex_bundle_t>::const_type DAG_Label;
    typedef typename boost::graph_traits<DAGType>::vertex_iterator vertexIterator;
    typedef typename boost::graph_traits<DAGType>::vertex_descriptor vertexDescriptor;
    typedef std::map<vertexDescriptor, std::size_t> VertexIDAssocContainer;
    typedef VertexLabelWriter LabelWriter;

    // create vertexID MAP
    VertexIDAssocContainer map;
    boost::associative_property_map<VertexIDAssocContainer> vertexIDMap(map);

    // fill the map
    unsigned int count = 0;
    for(std::pair<vertexIterator, vertexIterator> p = boost::vertices(dag); p.first != p.second; ++p.first)
        map[*p.first] = count++;


    DAG_Label lmap = boost::get(boost::vertex_bundle, dag);
    VertexLabelWriter w(lmap, chosenNodes);

    boost::write_graphviz(stream, dag, w, boost::default_writer(), boost::default_writer(), vertexIDMap);
}

} //  namespace

void TreeDecompositionDAG::writeGraphviz(std::ostream & stream, const std::set<TreeDecompositionSubgraph> & activeComponents) const
{
    typedef boost::property_map<Graph, boost::vertex_bundle_t>::const_type DAG_Label;
    typedef TreeDecompositionDAGLabel_DOTWriter<DAG_Label> LabelWriter;

    write_dot_format_helper<LabelWriter>(structure_, stream, activeComponents);
}

void TreeDecompositionDAG::cleanUp(const TreeDecompositionSubgraph & rootComponent)
{
    Node node = findExistingNode(rootComponent);
    if(node == invalidDescriptor())
    {
        structure_.clear();
        map_.clear();
    }

    // visit all child nodes
    std::set<Node> visitedNodes;
    std::stack<Node> todo;
    todo.push(node);

    while(!todo.empty())
    {
        Node curNode = todo.top();
        todo.pop();

        // already seen
        if(!visitedNodes.insert(curNode).second)
            continue;

        typedef boost::graph_traits<Graph>::out_edge_iterator oei;
        for(std::pair<oei, oei> p1 = boost::out_edges(curNode, structure_); p1.first != p1.second; ++p1.first)
        {
            Node andNode = boost::target(*p1.first, structure_);
            visitedNodes.insert(andNode);

            for(std::pair<oei, oei> p2 = boost::out_edges(andNode, structure_); p2.first != p2.second; ++p2.first)
            {
                todo.push(boost::target(*p2.first, structure_));
            }
        }
    }

    // list all nodes to remove
    typedef boost::graph_traits<Graph>::vertex_iterator vit;
    std::list<TreeDecompositionSubgraph> toRemove;
    for(std::pair<vit, vit> p = boost::vertices(structure_); p.first != p.second; ++p.first)
    {
        const TreeDecompositionDAGLabel & lbl = structure_[*p.first];
        if(lbl.type == TreeDecompositionDAGLabel::NODE_ChildrenOR && visitedNodes.count(*p.first) == 0)
            toRemove.push_back(lbl.vertices);
    }

    for(std::list<TreeDecompositionSubgraph>::iterator it = toRemove.begin(); it != toRemove.end(); ++it)
        removeNode(*it);
}

namespace {

std::size_t convertToInt(const std::string & str)
{
    return boost::lexical_cast<std::size_t>(str);
}



std::string convertToString(const TreeDecompositionDAGLabel & label)
{
    std::stringstream str;
    str << static_cast<int>(label.type) << ',' << label.vertices.size() << ',';
    for(std::size_t i = 0; i < label.vertices.size(); ++i)
    {
        if(label.vertices[i] == TreeDecompositionSubgraph::ActiveVertex)
            str << "a";
        else if(label.vertices[i] == TreeDecompositionSubgraph::ProjectedAwayVertex)
            str << "p";
        else if(label.vertices[i] == TreeDecompositionSubgraph::UnseenVertex)
            str << "u";
        else
            throw std::logic_error("Bad formatted TreeDecompositionSubgraph");
    }

    return str.str();
}

TreeDecompositionDAGLabel convertFromString(const std::string & str)
{
    std::vector<std::string> vct;
    boost::split(vct, str, boost::is_any_of(","));

    if(vct.size() < 2)
        throw std::logic_error("Bad formatted TreeDecompositionSubgraph");

    TreeDecompositionDAGLabel lbl;
    lbl.type = static_cast<TreeDecompositionDAGLabel::NodeType>(convertToInt(vct[0]));
    lbl.vertices = TreeDecompositionSubgraph(convertToInt(vct[1]));

    if(lbl.vertices.size() != 0)
    {
        if(vct.size() < 3 || vct[2].size() != lbl.vertices.size())
            throw std::logic_error("Bad formatted TreeDecompositionSubgraph");

        for(std::size_t i = 0; i < lbl.vertices.size(); ++i)
            if(vct[2][i] == 'a')
                lbl.vertices[i] = TreeDecompositionSubgraph::ActiveVertex;
            else if(vct[2][i] == 'p')
                lbl.vertices[i] = TreeDecompositionSubgraph::ProjectedAwayVertex;
            else if(vct[2][i] == 'u')
                lbl.vertices[i] = TreeDecompositionSubgraph::UnseenVertex;
            else
                throw std::logic_error("Bad formatted TreeDecompositionSubgraph");

    }

    return lbl;
}

} //  namespace


void TreeDecompositionDAG::serialize(std::ostream & stream) const
{
    typedef boost::graph_traits<Graph>::vertex_iterator vit;

    stream << "t # " << patternSize() << std::endl;

    // write the vertices
    std::map<Node, std::size_t> positionMap;
    std::size_t count = 0;
    for(std::pair<vit, vit> p = vertices(structure_); p.first != p.second; ++p.first, ++count)
    {
        Node v = *p.first;
        stream << "v " << count << " " << convertToString(structure_[v]) << std::endl;
        positionMap.insert(std::make_pair(v, count));
    }

    // write the edges
    typedef boost::graph_traits<Graph>::edge_iterator eit;
    for(std::pair<eit, eit> p = boost::edges(structure_); p.first != p.second; ++p.first)
    {
        Node src = boost::source(*p.first, structure_);
        Node tgt = boost::target(*p.first, structure_);

        stream << "e " << positionMap.find(src)->second << " " << positionMap.find(tgt)->second << "  " << std::endl;
    }
}

bool TreeDecompositionDAG::deserialize(std::istream & stream)
{
    std::string line;
    std::size_t pos = 0;

    std::vector<Node> nodevct;

    try
    {
        while(std::getline(stream, line))
        {
            std::vector<std::string> vct;
            boost::split(vct, line, boost::is_any_of(" "));

            if(vct.empty())
                continue;

            if(pos == 0)
            {
                if(vct.size() != 3 && vct[0] != "t" && vct[1] != "#")
                    return false;

                patternSize_ = convertToInt(vct[2]);

                // initialise the graph
                structure_.clear();
                pos = 1;
            }
            else if(pos == 1)
            {
                if(vct[0]  != "v")
                {
                    pos = 2;
                }
                else
                {
                    nodevct.push_back(boost::add_vertex(convertFromString(vct[2]), structure_));
                }
            }
            if(pos == 2)
            {
                if(vct[0] != "e")
                    return false;

                std::size_t src = convertToInt(vct[1]);
                std::size_t dst = convertToInt(vct[2]);
                boost::add_edge(nodevct[src], nodevct[dst], structure_);
            }
        }
    }
    catch(...)
    {
        structure_.clear();
        patternSize_ = 0;
        map_.clear();
    }

    refillMap();

    return true;
}

void TreeDecompositionDAG::refillMap()
{
    map_.clear();

    typedef boost::graph_traits<Graph>::vertex_iterator vit;
    for(std::pair<vit, vit> p = boost::vertices(structure_); p.first != p.second; ++p.first)
    {
        const TreeDecompositionDAGLabel & lbl = structure_[*p.first];
        if(lbl.type == TreeDecompositionDAGLabel::NODE_ChildrenOR)
            map_.insert(std::make_pair(lbl.vertices, *p.first));
    }
}

void TreeDecompositionDAG::writeGraphviz(std::ostream & stream) const
{
    writeGraphviz(stream, std::set<TreeDecompositionSubgraph>());
}

namespace {

typedef std::map<TreeDecompositionSubgraph, std::size_t> DegreeMap;
typedef std::set<TreeDecompositionSubgraph> VisitedNodeSet;

VisitedNodeSet visitSubdag(const TreeDecompositionDAG & dag, const TreeDecompositionSubgraph & rootComponent)
{
    VisitedNodeSet visitedSubgraph;
    std::stack<TreeDecompositionSubgraph> todo;
    todo.push(rootComponent);

    // loop over the children of the root component
    while(!todo.empty())
    {
        TreeDecompositionSubgraph comp = todo.top();
        todo.pop();

        // already seen?
        if(!visitedSubgraph.insert(comp).second)
            continue;

        // visit all the children
        for(std::pair<TreeDecompositionDAG::ChildIterator, TreeDecompositionDAG::ChildIterator> p = dag.children(comp); p.first != p.second; ++p.first)
        {
            const TreeDecompositionDAGAndNode & andNode = *p.first;
            for(std::size_t i = 0; i < andNode.numberOfChildren(); ++i)
                todo.push(andNode.childAt(i));
        }
    }

    return visitedSubgraph;
}

DegreeMap fillDegreeMap(const VisitedNodeSet & visitedNodes, const TreeDecompositionDAG & dag)
{
    DegreeMap map;
    for(VisitedNodeSet::const_iterator it = visitedNodes.begin(); it != visitedNodes.end(); ++it)
    {
        const TreeDecompositionSubgraph & comp = *it;
        std::size_t count = 0;
        for(std::pair<TreeDecompositionDAG::ParentIterator, TreeDecompositionDAG::ParentIterator> p = dag.parents(comp); p.first != p.second; ++p.first)
        {
            const TreeDecompositionDAGAndNode & andNode = *p.first;
            count += visitedNodes.count(andNode.parent());
        }

        map.insert(std::make_pair(comp, count));
    }

    return map;
}

std::list<TreeDecompositionSubgraph> extractAndRemoveZeros(DegreeMap & map)
{
    std::list<TreeDecompositionSubgraph> zeros;

    for(DegreeMap::const_iterator it = map.begin(); it != map.end(); ++it)
        if(it->second == 0)
            zeros.push_back(it->first);

    for(std::list<TreeDecompositionSubgraph>::const_iterator it = zeros.begin(); it != zeros.end(); ++it)
        map.erase(*it);

    return zeros;
}




} //  namespace

std::vector<TreeDecompositionSubgraph> TreeDecompositionDAG::topologicalOrder(const TreeDecompositionSubgraph & rootComponent) const
{
    VisitedNodeSet nodes = visitSubdag(*this, rootComponent);
    DegreeMap degreeMap = fillDegreeMap(nodes, *this);

    std::vector<TreeDecompositionSubgraph> result;

    // perform the sorting
    while(!degreeMap.empty())
    {
        // extract the non-zero's
        const std::list<TreeDecompositionSubgraph> & zeros = extractAndRemoveZeros(degreeMap);
        if(zeros.empty())
            throw std::logic_error("TreeDecompositionDAG: Not in DAG order");

        // add them to the list
        result.insert(result.end(), zeros.begin(), zeros.end());

        // and process the children
        for(std::list<TreeDecompositionSubgraph>::const_iterator it = zeros.begin(); it != zeros.end(); ++it)
        {
            const TreeDecompositionSubgraph & comp = *it;
            for(std::pair<ChildIterator, ChildIterator> p = children(comp); p.first != p.second; ++p.first)
            {
                const TreeDecompositionDAGAndNode & andNode = *p.first;
                for(std::size_t i = 0; i < andNode.numberOfChildren(); ++i)
                    --degreeMap[andNode.childAt(i)];
            }
        }
    }

    return result;
}

std::vector<TreeDecompositionSubgraph> TreeDecompositionDAG::topologicalOrder() const
{
    VisitedNodeSet nodes;

    typedef boost::graph_traits<Graph>::vertex_iterator vit;
    for(std::pair<vit, vit> p = boost::vertices(structure_); p.first != p.second; ++p.first)
    {
        const TreeDecompositionDAGLabel & lbl = structure_[*p.first];
        if(lbl.type == TreeDecompositionDAGLabel::NODE_ChildrenOR)
            nodes.insert(lbl.vertices);
    }

    DegreeMap degreeMap = fillDegreeMap(nodes, *this);

    std::vector<TreeDecompositionSubgraph> result;

    // perform the sorting
    while(!degreeMap.empty())
    {
        // extract the non-zero's
        const std::list<TreeDecompositionSubgraph> & zeros = extractAndRemoveZeros(degreeMap);
        if(zeros.empty())
            throw std::logic_error("TreeDecompositionDAG: Not in DAG order");

        // add them to the list
        result.insert(result.end(), zeros.begin(), zeros.end());

        // and process the children
        for(std::list<TreeDecompositionSubgraph>::const_iterator it = zeros.begin(); it != zeros.end(); ++it)
        {
            const TreeDecompositionSubgraph & comp = *it;
            for(std::pair<ChildIterator, ChildIterator> p = children(comp); p.first != p.second; ++p.first)
            {
                const TreeDecompositionDAGAndNode & andNode = *p.first;
                for(std::size_t i = 0; i < andNode.numberOfChildren(); ++i)
                    --degreeMap[andNode.childAt(i)];
            }
        }
    }

    return result;
}

namespace {

std::size_t childrenTreewidth(const TreeDecompositionDAGAndNode & andNode, const std::map<TreeDecompositionSubgraph, std::size_t> & sizeMap)
{
    std::size_t curMax = sizeMap.find(andNode.childAt(0))->second;
    if(andNode.numberOfChildren() == 2)
        curMax = std::max(curMax, sizeMap.find(andNode.childAt(1))->second);

    return curMax;
}


} //  namespace

void TreeDecompositionDAG::removeAndNode(const TreeDecompositionDAGAndNode & andNode)
{
    Node n = findExistingNode(andNode.parent());
    if(n == invalidDescriptor())
        return;

    // find the node
    typedef boost::graph_traits<Graph>::out_edge_iterator oei;

    for(std::pair<oei, oei> p = boost::out_edges(n, structure_); p.first != p.second; ++p.first)
    {
        Node an = boost::target(*p.first, structure_);

        if(TreeDecompositionDAGAndNode(this, an) == andNode)
        {
            boost::clear_vertex(an, structure_);
            boost::remove_vertex(an, structure_);

            return;
        }
    }
}

std::size_t TreeDecompositionDAG::calculateTreewidth(const TreeDecompositionSubgraph & rootComponent) const
{
    const std::vector<TreeDecompositionSubgraph> & order = topologicalOrder(rootComponent);
    std::map<TreeDecompositionSubgraph, std::size_t> sizeMap;

    std::size_t result = 0;


    for(std::vector<TreeDecompositionSubgraph>::const_reverse_iterator it = order.rbegin(); it != order.rend(); ++it)
    {
        const TreeDecompositionSubgraph & component = *it;
        std::size_t curSize = component.count(TreeDecompositionSubgraph::ActiveVertex);

        // find the minimum value over the children
        std::size_t minValue = std::numeric_limits<std::size_t>::max();
        for(std::pair<ChildIterator, ChildIterator> p = children(component); p.first != p.second; ++p.first)
            minValue = std::min(minValue, childrenTreewidth(*p.first, sizeMap));

        // update the value
        curSize = minValue == std::numeric_limits<std::size_t>::max() ? curSize : std::max(curSize, minValue);

        // insert in the map
        sizeMap.insert(std::make_pair(component, curSize));

        result = std::max(curSize, result);
    }

    return result - 1;
}

void TreeDecompositionDAG::makeMinimalTreewidth(const TreeDecompositionSubgraph & rootComponent)
{
    std::list<TreeDecompositionDAGAndNode> toRemove;

    const std::vector<TreeDecompositionSubgraph> & order = topologicalOrder(rootComponent);
    std::map<TreeDecompositionSubgraph, std::size_t> sizeMap;


    for(std::vector<TreeDecompositionSubgraph>::const_reverse_iterator it = order.rbegin(); it != order.rend(); ++it)
    {
        const TreeDecompositionSubgraph & component = *it;
        std::size_t curSize = component.count(TreeDecompositionSubgraph::ActiveVertex);

        // find the minimum value over the children
        std::size_t minValue = std::numeric_limits<std::size_t>::max();
        for(std::pair<ChildIterator, ChildIterator> p = children(component); p.first != p.second; ++p.first)
            minValue = std::min(minValue, childrenTreewidth(*p.first, sizeMap));

        // find the children which should be removed
        for(std::pair<ChildIterator, ChildIterator> p = children(component); p.first != p.second; ++p.first)
        {
            const TreeDecompositionDAGAndNode & andNode = *p.first;
            if(childrenTreewidth(andNode, sizeMap) > minValue)
                toRemove.push_back(andNode);
        }

        // update the value
        curSize = minValue == std::numeric_limits<std::size_t>::max() ? curSize : std::max(curSize, minValue);

        // insert in the map
        sizeMap.insert(std::make_pair(component, curSize));
    }

    // and now remove these links
    for(std::list<TreeDecompositionDAGAndNode>::const_iterator it = toRemove.begin(); it != toRemove.end(); ++it)
        removeAndNode(*it);
}

bool TreeDecompositionDAG::isBinary() const
{
    typedef boost::graph_traits<Graph>::vertex_iterator vit;
    for(std::pair<vit, vit> p = boost::vertices(structure_); p.first != p.second; ++p.first)
        if(structure_[*p.first].type != TreeDecompositionDAGLabel::NODE_ChildrenOR && boost::out_degree(*p.first, structure_) > 2)
            return false;

    return true;
}


TreeDecompositionSubgraph TreeDecompositionDAG::getRootNode() const
{
    TreeDecompositionSubgraph result;

    for(std::pair<ComponentIterator, ComponentIterator> p = components(); p.first != p.second; ++p.first)
    {
        std::pair<ParentIterator, ParentIterator> pit = parents(*p.first);
        if(pit.first == pit.second)
        {
            if(result.size() != 0)
                throw std::logic_error("TreeDecompositionDAG: The treedecomposition dag has multiple roots");

            result = *p.first;
        }
    }

    return result;
}

std::vector<TreeDecompositionSubgraph> TreeDecompositionDAG::getRootNodes() const
{
    std::vector<TreeDecompositionSubgraph> roots;
    for(std::pair<ComponentIterator, ComponentIterator> p = components(); p.first != p.second; ++p.first)
    {
        std::pair<ParentIterator, ParentIterator> pit = parents(*p.first);
        if(pit.first == pit.second)
            roots.push_back(*p.first);
    }

    return roots;
}

namespace {

void findCompleteJoinVertices(const TreeDecompositionDAGAndNode & andNode, std::vector<TreeDecompositionSubgraph> & completeNodes, std::vector<TreeDecompositionSubgraph> & nonCompleteNodes)
{
    assert(andNode.type() == TreeDecompositionDAGLabel::NODE_ChildrenJoin);
    const std::vector<std::size_t> & joinVertices = andNode.joinVertices();

    for(std::size_t i = 0; i < andNode.numberOfChildren(); ++i)
    {
        const TreeDecompositionSubgraph & cur = andNode.childAt(i);

        bool allSet = true;
        for(std::vector<std::size_t>::const_iterator it = joinVertices.begin(); it != joinVertices.end() && allSet; ++it)
            if(cur[*it] != TreeDecompositionSubgraph::ActiveVertex)
                allSet = false;

        if(allSet)
            completeNodes.push_back(cur);
        else
            nonCompleteNodes.push_back(cur);
    }
}

void performAllPermutationsJoin(std::vector<TreeDecompositionSubgraph> & completeChildren, std::vector<TreeDecompositionSubgraph> & nonCompleteChildren, TreeDecompositionDAG & dag)
{
    std::sort(completeChildren.begin(), completeChildren.end());
    std::sort(nonCompleteChildren.begin(), nonCompleteChildren.end());

    TreeDecompositionSubgraph result;
    do
    {
        TreeDecompositionSubgraph curResult = completeChildren[0];
        for(std::size_t i = 1; i < completeChildren.size(); ++i)
            curResult = dag.addJoin(curResult, completeChildren[i]);

        result = curResult;
    }
    while(std::next_permutation(completeChildren.begin(), completeChildren.end()));

    do
    {
        TreeDecompositionSubgraph curResult = result;
        for(std::size_t i = 0; i < nonCompleteChildren.size(); ++i)
            curResult = dag.addJoin(curResult, nonCompleteChildren[i]);
    }
    while(std::next_permutation(nonCompleteChildren.begin(), nonCompleteChildren.end()));
}

void performSimpleJoin(std::vector<TreeDecompositionSubgraph> & completeChildren, std::vector<TreeDecompositionSubgraph> & nonCompleteChildren, TreeDecompositionDAG & dag)
{
    TreeDecompositionSubgraph result = completeChildren[0];
    for(std::size_t i = 1; i < completeChildren.size(); ++i)
        result = dag.addJoin(result, completeChildren[i]);

    for(std::size_t i = 0; i < nonCompleteChildren.size(); ++i)
        result = dag.addJoin(result, nonCompleteChildren[i]);
}

void make_binary_node(const TreeDecompositionDAGAndNode & andNode, TreeDecompositionDAG & dag, bool usePermutations)
{
    TreeDecompositionSubgraph parent = andNode.parent();
    std::vector<TreeDecompositionSubgraph> completeChildren, nonCompleteChildren;

    // find the complete and the non-complete children
    findCompleteJoinVertices(andNode, completeChildren, nonCompleteChildren);
    assert(!completeChildren.empty());

    // remove the and node
    dag.removeAndNode(andNode);

    // and now join the children
    if(usePermutations)
        performAllPermutationsJoin(completeChildren, nonCompleteChildren, dag);
    else
        performSimpleJoin(completeChildren, nonCompleteChildren, dag);
}

} //  namespace

void make_binary_treedecomposition(TreeDecompositionDAG & dag, bool useAllPermutations)
{
    std::stack<TreeDecompositionDAGAndNode> nonBinaryAndNodes;

    typedef TreeDecompositionDAG::ComponentIterator cit;
    typedef TreeDecompositionDAG::ChildIterator chit;

    // find out all the non-binary and nodes
    for(std::pair<cit, cit> p = dag.components(); p.first != p.second; ++p.first)
    {
        for(std::pair<chit, chit> p2 = dag.children(*p.first); p2.first != p2.second; ++p2.first)
        {
            const TreeDecompositionDAGAndNode & andNode = *p2.first;
            if(andNode.type() == TreeDecompositionDAGLabel::NODE_ChildrenJoin && andNode.numberOfChildren() > 2)
                nonBinaryAndNodes.push(andNode);
        }
    }

    // and make them binary
    while(!nonBinaryAndNodes.empty())
    {
        TreeDecompositionDAGAndNode andNode = nonBinaryAndNodes.top();
        nonBinaryAndNodes.pop();
        make_binary_node(andNode, dag, useAllPermutations);
    }
}

void swap(TreeDecompositionDAG & lhs, TreeDecompositionDAG & rhs)
{
    using std::swap;

    swap(lhs.structure_, rhs.structure_);
    swap(lhs.map_, rhs.map_);
    swap(lhs.patternSize_, rhs.patternSize_);
}

} // treeDAG namespace
