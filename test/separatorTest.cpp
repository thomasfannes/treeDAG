#define BOOST_TEST_MODULE SeparatorTest
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <treeDAG/separatorCache.hpp>

#include "util.hpp"


typedef treeDAG::SeparatorConfig::Graph Graph;



BOOST_AUTO_TEST_CASE( path_test )
{
    static const std::size_t MAX_SIZE = 10;

    for(std::size_t size = 2; size < MAX_SIZE; ++size)
    {
        Graph g = make_path(size);
        treeDAG::SeparatorCache<1> cache(1, &g);
        cache.initialize();

        for(std::size_t i = 0; i < size; ++i)
        {
            std::vector<std::size_t> sep;
            sep.push_back(i);

            if(i == 0 || i == size-1)
                BOOST_CHECK(cache.findSeparator(sep) == 0);
            else
                BOOST_CHECK(cache.findSeparator(sep) != 0);
        }
    }
}



BOOST_AUTO_TEST_CASE( cycle_test )
{
    static const std::size_t MAX_SIZE = 10;

    for(std::size_t size = 2; size < MAX_SIZE; ++size)
    {
        Graph g = make_cycle(size);
        treeDAG::SeparatorCache<2> cache(2, &g);
        cache.initialize();

        for(std::size_t i = 0; i < size; ++i)
        {
            for(std::size_t j = i+1; j < size; ++j)
            {
                std::vector<std::size_t> sep;
                sep.push_back(i);
                sep.push_back(j);

                if((j == i+1) || (i == 0 && j == size-1))
                    BOOST_CHECK(cache.findSeparator(sep) == 0);
                else
                    BOOST_CHECK(cache.findSeparator(sep) != 0);
            }
        }
    }


}

