/*=============================================================================
    Copyright (c) 1999-2003 Jaakko J�rvi
    Copyright (c) 1999-2003 Jeremiah Willcock
    Copyright (c) 2001-2006 Joel de Guzman

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_IN_05042005_0120)
#define BOOST_IN_05042005_0120

#include <iostream>
#include <boost/fusion/sequence/io/detail/in.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/or.hpp>

namespace boost { namespace fusion
{
    template <typename IStream, typename Sequence>
    inline typename
        enable_if<
            fusion::traits::is_sequence<Sequence>
          , IStream&
        >::type
    operator>>(IStream& is, Sequence& seq)
    {
        detail::read_sequence(is, seq);
        return is;
    }
}}

#endif
