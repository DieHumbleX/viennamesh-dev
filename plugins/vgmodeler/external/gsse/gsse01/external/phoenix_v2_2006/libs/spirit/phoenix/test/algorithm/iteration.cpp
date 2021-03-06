/*=============================================================================
    Copyright (c) 2005-2006 Dan Marsden
    Copyright (c) 2005-2006 Joel de Guzman

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <boost/spirit/phoenix/stl/algorithm/iteration.hpp>
#include <boost/spirit/phoenix/core.hpp>
#include <boost/detail/lightweight_test.hpp>

#include <functional>

namespace
{
    struct for_each_tester
    {
        int value_;
        for_each_tester() : value_(0) { }
        void operator()(
            int& i)
        {
            value_ += i++;
            return;
        }
    };

    void for_each_test()
    {
        using namespace boost::phoenix;
        int array[] = {1,2,3};
        BOOST_TEST(for_each(arg1, for_each_tester())(array).value_ == 6);
        BOOST_TEST(array[0] == 2);
        BOOST_TEST(array[1] == 3);
        BOOST_TEST(array[2] == 4);
        return;
    }

    void accumulate_test()
    {
        using namespace boost::phoenix;
        int array[] = {1,2,3};
        BOOST_TEST(accumulate(arg1, 0)(array) == 6);
        BOOST_TEST(boost::phoenix::accumulate(arg1, 0, std::minus<int>())(array) == -6);
        return;
    }
}

int main()
{
    for_each_test();
    accumulate_test();
    boost::report_errors();
}
