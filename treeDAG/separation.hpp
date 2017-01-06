#ifndef TREEDAG_SEPARATION_HPP
#define TREEDAG_SEPARATION_HPP

#include <limits>
#include <vector>

#include "separatorConfig.hpp"

namespace treeDAG {

struct Separation : public SeparatorConfig
{
    void limitToMaximalComponents();

    VertexSet separator;
    ComponentMap componentMap;
    ComponentSet components;
};

bool operator==(const Separation & lhs, const Separation & rhs);
std::size_t hash_value(const Separation & separation);

struct SeparationEqual : public SeparatorConfig
{
    bool operator()(const Separation & lhs, const Separation & rhs) const;
    bool operator()(const VertexSet & lhs, const Separation & rhs) const;
    bool operator()(const Separation & lhs, const VertexSet & rhs) const;
    bool operator()(const VertexSet & lhs, const VertexSet & rhs) const;
};

struct SeparationHash : public SeparatorConfig
{
    std::size_t operator()(const Separation & separation) const;
    std::size_t operator()(const VertexSet & separator) const;
};





} // namespace treeDAG

#endif // TREEDAG_SEPARATION_HPP
