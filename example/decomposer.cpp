#include <treeDAG/decomposer.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <boost/timer/timer.hpp>

typedef treeDAG::SeparatorConfig::Graph Graph;

Graph create_cyle(std::size_t size)
{
    Graph g(size);
    for(std::size_t i = 0; i < size; ++i)
    {
        std::size_t src = i;
        std::size_t dst = (i+1)%size;
        boost::add_edge(src, dst, g);
    }

    return g;
}

Graph create_path(std::size_t size)
{
    Graph g(size);
    for(std::size_t i = 1; i < size; ++i)
        boost::add_edge(i-1, i, g);

    return g;
}

Graph create_test_graph()
{
    Graph g(7);
    boost::add_edge(0, 1, g);
    boost::add_edge(2, 1, g);
    boost::add_edge(5, 1, g);
    boost::add_edge(2, 3, g);
    boost::add_edge(5, 3, g);
    boost::add_edge(3, 4, g);
    boost::add_edge(3, 6, g);
    boost::add_edge(0, 4, g);
    boost::add_edge(0, 6, g);

    return g;
}

Graph create_test_graph2()
{
    Graph g(7);
    boost::add_edge(0, 1, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(2, 3, g);
    boost::add_edge(3, 0, g);
    boost::add_edge(3, 4, g);
    boost::add_edge(4, 5, g);
    boost::add_edge(5, 6, g);
    boost::add_edge(6, 4, g);


    return g;
}

Graph create_test_graph3()
{
    Graph g(8);
    boost::add_edge(0, 1, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(2, 3, g);
    boost::add_edge(3, 0, g);
    boost::add_edge(2, 4, g);
    boost::add_edge(4, 5, g);
    boost::add_edge(5, 6, g);
    boost::add_edge(6, 7, g);
    boost::add_edge(7, 5, g);


    return g;
}

int main(int argc, char ** argv)
{
    if(argc != 2)
    {
        std::cerr << "usage: " << argv[0] << " cycle_size" << std::endl;
        return -1;
    }

    std::size_t sz = boost::lexical_cast<std::size_t>(argv[1]);
//    Graph g = create_cyle(sz);
    Graph g = create_test_graph3();
//    Graph g = create_path(sz);



    std::stringstream times;

    for(std::size_t i = 0; i < 1; ++i)
    {
        boost::timer::cpu_timer t;
        t.start();

        treeDAG::Decomposer decomposer(&g, 2);
        decomposer.initialize();

        std::vector<unsigned char> roots;
        roots.push_back(0);
        roots.push_back(1);

        decomposer.process(roots.begin(), roots.end());
        t.stop();

        times << t.elapsed().wall << ";";

        std::stringstream filename;
        filename << "test_" << sz << ".dot";

        std::ofstream ofs;
        ofs.open(filename.str().c_str());
        decomposer.writeDot(ofs);
    }


//    std::cout << sz << ";" << times.str() << std::endl;

}

