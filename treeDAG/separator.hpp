#ifndef TREEDAG_SEPARATOR_HPP
#define TREEDAG_SEPARATOR_HPP

#include "separatorConfig.hpp"
#include "separation.hpp"

namespace treeDAG {

class Separator : public SeparatorConfig
{
public:
    typedef SeparatorConfig::Graph                          Graph;
    typedef Separation                                      result_type;
    typedef boost::graph_traits<Graph>::vertex_iterator     VertexIterator;

    explicit Separator(const Graph * graph = 0);

    template <typename SeparatorVertexIterator> void separate(SeparatorVertexIterator first, SeparatorVertexIterator last, result_type & separation) const;
    template <typename SeparatorVertexIterator> result_type operator()(SeparatorVertexIterator first, SeparatorVertexIterator last) const;
    template <typename SeparatorVertexIterator> result_type operator()(const std::pair<SeparatorVertexIterator, SeparatorVertexIterator> & separatorRange) const;

private:
    std::size_t separateIntoComponentMap(const VertexSet & separator, ComponentMap & components) const;
    VertexIterator findFirstNonSeparatorVertex(VertexIterator current, VertexIterator last, const ComponentMap & componentMap) const;
    void fillCurrentComponent(VertexIndexType source, ComponentMap & componentMap, std::size_t componentNumber) const;
    void fillComponents(const ComponentMap & componentMap, ComponentSet & components) const;


    const Graph * graph_;
};

} // namespace treeDAG

#include "separator.hxx"

#endif // TREEDAG_SEPARATOR_HPP
