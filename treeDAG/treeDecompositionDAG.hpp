/*******************************************************************
* Copyright (c) 2010-2013 MiGraNT - DTAI.cs.kuleuven.be
* License details in the root directory of this distribution in
* the LICENSE file
********************************************************************/


#ifndef TREEDAG_TREEDECOMPOSITIONDAG_HPP
#define TREEDAG_TREEDECOMPOSITIONDAG_HPP

#include <boost/graph/adjacency_list.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include "treeDecompositionSubgraph.hpp"

namespace treeDAG {


struct TreeDecompositionDAGLabel
{
    enum NodeType
    {
        NODE_ChildrenOR = 0,
        NODE_ChildrenAND = 1,
        NODE_ORANDMASK = 1,
        NODE_ChildrenList = 3,
        NODE_ChildrenExtend = 5,
        NODE_ChildrenProject = 7,
        NODE_ChildrenJoin = 9,
        NODE_Undefined = -1
    };

    TreeDecompositionSubgraph vertices;
    NodeType type;
};

class TreeDecompositionDAGAndNode;

class TreeDecompositionDAG
{
public:
    typedef boost::adjacency_list<boost::listS, boost::listS, boost::bidirectionalS, TreeDecompositionDAGLabel> Graph;

    struct AndNodeTransformer
    {
        typedef TreeDecompositionDAGAndNode result_type;
        AndNodeTransformer();
        AndNodeTransformer(bool isSource, const TreeDecompositionDAG * dag);

        TreeDecompositionDAGAndNode operator()(boost::graph_traits<Graph>::edge_descriptor edge) const;

    private:
        const TreeDecompositionDAG * dag_;
        bool isSource_;
    };

    struct ComponentTransformer
    {
        typedef TreeDecompositionSubgraph result_type;
        explicit ComponentTransformer(const Graph * graph = 0);

        const TreeDecompositionSubgraph & operator()(boost::graph_traits<Graph>::vertex_descriptor vd) const;

    private:
        const Graph * graph_;
    };

    struct ComponentFilter
    {
        explicit ComponentFilter(const Graph * graph = 0);

        bool operator()(const TreeDecompositionSubgraph & component) const;

    private:
        const Graph * graph_;
    };


    typedef boost::transform_iterator<AndNodeTransformer, boost::graph_traits<Graph>::out_edge_iterator> ChildIterator;
    typedef boost::transform_iterator<AndNodeTransformer, boost::graph_traits<Graph>::in_edge_iterator> ParentIterator;
    typedef boost::filter_iterator<ComponentFilter, boost::transform_iterator<ComponentTransformer, boost::graph_traits<Graph>::vertex_iterator> > ComponentIterator;

    explicit TreeDecompositionDAG(std::size_t patternSize = 0);
    TreeDecompositionDAG(const TreeDecompositionDAG & rhs);

    void reset(std::size_t patternSize = 0);
    std::size_t numComponents() const;

    TreeDecompositionDAG & operator=(const TreeDecompositionDAG & rhs);

    std::size_t patternSize() const;
    bool hasComponent(const TreeDecompositionSubgraph & component) const;
    void addComponent(const TreeDecompositionSubgraph & component);
    bool isBinary() const;

    std::pair<ChildIterator, ChildIterator> children(const TreeDecompositionSubgraph &component) const;
    std::pair<ParentIterator, ParentIterator> parents(const TreeDecompositionSubgraph & component) const;
    std::pair<ComponentIterator, ComponentIterator> components() const;

    // addition methods
    TreeDecompositionSubgraph addList(std::size_t vertex);
    TreeDecompositionSubgraph addExtend(const TreeDecompositionSubgraph & component, std::size_t vertexToExtend);
    TreeDecompositionSubgraph addProject(const TreeDecompositionSubgraph & component, std::size_t vertexToProjectAway);
    TreeDecompositionSubgraph addJoin(const TreeDecompositionSubgraph & lhsComponent, const TreeDecompositionSubgraph & rhsComponent);
    template <typename Iterator> TreeDecompositionSubgraph addList(Iterator firstVertex, Iterator lastVertex);
    template <typename Iterator> TreeDecompositionSubgraph addExtend(const TreeDecompositionSubgraph & childComponent, Iterator firstVertexToExtend, Iterator lastVertexToExtend);
    template <typename Iterator> TreeDecompositionSubgraph addProject(const TreeDecompositionSubgraph & childComponent, Iterator firstVertexToProjectAway, Iterator lastVertexToProjectAway);
    template <typename Iterator> TreeDecompositionSubgraph addJoin(Iterator firstComponent, Iterator lastComponent);

    void remapIndices(const std::vector<std::size_t> & newIndices);

    // insertion methods
    enum InsertResult
    {
        CanInsert,
        MaybeIntoChildren,
        NotPossible
    };
    typedef std::map<TreeDecompositionSubgraph, InsertResult> InsertionMap;

    InsertResult checkForInsertion(const TreeDecompositionSubgraph & component, const TreeDecompositionSubgraph & toInsert) const;
    template <typename Iterator> InsertResult checkForInsertion(const TreeDecompositionSubgraph & component, Iterator firstRequiredVertex, Iterator lastRequiredVertex) const;
    TreeDecompositionSubgraph insert(const TreeDecompositionSubgraph & rootComponent, const TreeDecompositionSubgraph & toInsertComponent, const InsertionMap & insertionPositionMap);


    // removal method
    void removeNode(const TreeDecompositionSubgraph & component);
    void removeAndNode(const TreeDecompositionDAGAndNode & andNode);
    template <typename Iterator> void removeNodes(Iterator firstComponentIterator, Iterator lastComponentIterator);
    void cleanUp(const TreeDecompositionSubgraph & rootComponent);
    void makeMinimalTreewidth(const TreeDecompositionSubgraph & rootComponent);
    void removeSubtree(const TreeDecompositionSubgraph & component);

    // ordering methods
    std::vector<TreeDecompositionSubgraph> topologicalOrder(const TreeDecompositionSubgraph & rootComponent) const;
    std::vector<TreeDecompositionSubgraph> topologicalOrder() const;
    TreeDecompositionSubgraph getRootNode() const;
    std::vector<TreeDecompositionSubgraph> getRootNodes() const;

    // writing to stream
    void serialize(std::ostream & stream) const;
    bool deserialize(std::istream & stream);
    void writeGraphviz(std::ostream & stream) const;
    void writeGraphviz(std::ostream & stream, const std::set<TreeDecompositionSubgraph> & activeComponents) const;

    // visiting methods
    template <typename Iterator> void visitComponents(const TreeDecompositionSubgraph & rootComponent, Iterator outputIterator) const;
    std::size_t calculateTreewidth(const TreeDecompositionSubgraph & rootComponent) const;


private:
    typedef boost::graph_traits<Graph>::vertex_descriptor Node;
    static Node invalidDescriptor();

    void remapComponent(const std::vector<std::size_t> & newIndices, TreeDecompositionSubgraph & subgraph);

    // errors to throw
    std::logic_error errOutOfBounds() const;
    std::logic_error errNotActive() const;
    std::logic_error errNotUnseen() const;
    std::logic_error errBadRange() const;
    std::logic_error errAlreadyActive() const;

    // insertion helpers
    bool constructInsertColorMap(Node rootNode, std::set<TreeDecompositionSubgraph> insertNodes, std::map<Node, InsertResult> & map) const;
    void performInsertion(const TreeDecompositionSubgraph & toInsertComponent, Node rootNode, const std::map<Node, InsertResult> & map);
    TreeDecompositionSubgraph createInsertComponent(const TreeDecompositionSubgraph & node, const TreeDecompositionSubgraph & toInsertComponent) const;

    InsertResult findInMap(const TreeDecompositionSubgraph & component, const InsertionMap & map) const;
    void refillMap();


    TreeDecompositionSubgraph addUnaryAnd(const TreeDecompositionSubgraph & child, const TreeDecompositionSubgraph & parent, TreeDecompositionDAGLabel::NodeType type);
    TreeDecompositionSubgraph addBinaryAnd(const TreeDecompositionSubgraph & childA, const TreeDecompositionSubgraph & childB, const TreeDecompositionSubgraph & parent, TreeDecompositionDAGLabel::NodeType type);
    TreeDecompositionSubgraph addNaryAnd(std::vector<TreeDecompositionSubgraph> children, const TreeDecompositionSubgraph & parent, TreeDecompositionDAGLabel::NodeType type);


    const std::pair<TreeDecompositionSubgraph, Node> addOrFindNode(const TreeDecompositionSubgraph & component);
    Node findExistingNode(const TreeDecompositionSubgraph & component) const;

    friend class TreeDecompositionDAGAndNode;
    friend void swap(TreeDecompositionDAG & lhs, TreeDecompositionDAG & rhs);

    Graph structure_;
    boost::unordered_map<TreeDecompositionSubgraph, Node> map_;
    std::size_t patternSize_;
};

void make_binary_treedecomposition(TreeDecompositionDAG & dag, bool useAllPermutations = false);

} // treeDAG namespace

#include "treeDecompositionDAG.hxx"

#endif // TREEDAG_TREEDECOMPOSITIONDAG_HPP

