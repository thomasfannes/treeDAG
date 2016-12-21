#ifndef TREEDAG_TEST_UTIL_HPP
#define TREEDAG_TEST_UTIL_HPP

#include <treeDAG/separatorConfig.hpp>

treeDAG::SeparatorConfig::Graph make_path(std::size_t size);
treeDAG::SeparatorConfig::Graph make_cycle(std::size_t size);

#endif // TREEDAG_TEST_UTIL_HPP
