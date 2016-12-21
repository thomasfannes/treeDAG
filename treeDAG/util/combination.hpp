#ifndef TREEDAG_UTIL_COMBINATION_HPP
#define TREEDAG_UTIL_COMBINATION_HPP

#include <vector>
#include <boost/iterator/filter_iterator.hpp>

namespace treeDAG {
namespace util {

template <typename Iterator>
struct CombinationGenerator
{
public:
    CombinationGenerator();
    CombinationGenerator(Iterator first, Iterator last);

    void reset();
    template <typename OutIterator>
    void current(OutIterator it) const;
    std::vector<typename std::iterator_traits<Iterator>::value_type> current() const;
    bool next();

private:
    Iterator first_;
    Iterator last_;
    std::vector<bool> indices_;
};

} // namespace util
} // namespace treeDAG

#include "combination.hxx"

#endif // TREEDAG_UTIL_COMBINATION_HPP
