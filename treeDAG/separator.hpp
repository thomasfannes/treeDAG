#ifndef TREEDAG_SEPARATOR_HPP
#define TREEDAG_SEPARATOR_HPP

#include "separatorConfig.hpp"
#include "separation.hpp"

namespace treeDAG {

template <typename _VertexIndexType = SeparatorConfig::VertexIndexType>
class Separator
{
public:
    typedef _VertexIndexType VertexIndexType;
    typedef SeparatorConfig::Graph Graph;
    typedef Separation<VertexIndexType> result_type;
    typedef boost::graph_traits<Graph>::vertex_iterator VertexIterator;
    typedef std::vector<VertexIndexType> VertexSet;

    explicit Separator(const Graph * graph = 0);

    template <typename SeparatorVertexIterator> void separate(SeparatorVertexIterator first, SeparatorVertexIterator last, result_type & separation) const;
    template <typename SeparatorVertexIterator> result_type operator()(SeparatorVertexIterator first, SeparatorVertexIterator last) const;
    template <typename SeparatorVertexIterator> result_type operator()(const std::pair<SeparatorVertexIterator, SeparatorVertexIterator> & separatorRange) const;

    template <typename SeparatorVertexIterator> std::list<VertexSet> findMaximalComponents(SeparatorVertexIterator first, SeparatorVertexIterator last) const;

private:
    VertexIterator findFirstNonSeparatorVertex(VertexIterator current, VertexIterator last, const result_type & result) const;
    void fillCurrentComponent(VertexIndexType source, result_type & result) const;


    const Graph * graph_;
};

} // namespace treeDAG

#include "separator.hxx"

#endif // TREEDAG_SEPARATOR_HPP
