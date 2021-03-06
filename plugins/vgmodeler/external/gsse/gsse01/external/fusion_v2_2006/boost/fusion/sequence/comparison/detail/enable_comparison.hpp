/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_ENABLE_COMPARISON_09232005_1958)
#define FUSION_ENABLE_COMPARISON_09232005_1958

#include <boost/mpl/or.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <boost/fusion/support/detail/is_mpl_sequence.hpp>

namespace boost { namespace fusion { namespace detail
{
    template <typename Seq1, typename Seq2>
    struct enable_equality
        : mpl::or_<fusion::traits::is_sequence<Seq1>, fusion::traits::is_sequence<Seq2> >
    {};

    template <typename Seq1, typename Seq2>
    struct enable_comparison
        : mpl::and_<
        mpl::or_<fusion::traits::is_sequence<Seq1>, fusion::traits::is_sequence<Seq2> >,
        mpl::equal_to<result_of::size<Seq1>, result_of::size<Seq2> > >
    {};

}}}

#endif
