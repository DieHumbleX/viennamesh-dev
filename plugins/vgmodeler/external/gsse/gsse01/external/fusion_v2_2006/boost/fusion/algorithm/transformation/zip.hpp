/*=============================================================================
    Copyright (c) 2001-2006 Joel de Guzman
    Copyright (c) 2006 Dan Marsden

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#ifndef BOOST_PP_IS_ITERATING
#if !defined(FUSION_ZIP_HPP_20060125_2058)
#define FUSION_ZIP_HPP_20060125_2058

#include <boost/fusion/sequence/view/zip_view.hpp>
#include <boost/fusion/sequence/container/vector.hpp>
#include <boost/fusion/sequence/conversion/as_vector.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/placeholders.hpp>

#if !defined(FUSION_MAX_ZIP_SEQUENCES)
#define FUSION_MAX_ZIP_SEQUENCES 10
#endif

namespace boost { namespace fusion {

    struct void_;
    
    namespace result_of
    {
        template<BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(BOOST_PP_INC(FUSION_MAX_ZIP_SEQUENCES), typename T, fusion::void_)>
        struct zip;
    }

#define BOOST_PP_FILENAME_1 \
    <boost/fusion/algorithm/transformation/zip.hpp>
#define BOOST_PP_ITERATION_LIMITS (2, FUSION_MAX_ZIP_SEQUENCES)
#include BOOST_PP_ITERATE()

}}

#endif

#else

#define ZIP_ITERATION BOOST_PP_ITERATION()

    namespace result_of
    {
        template< BOOST_PP_ENUM_PARAMS(ZIP_ITERATION, typename T) >
        struct zip< BOOST_PP_ENUM_PARAMS(ZIP_ITERATION, T) >
        {
            typedef mpl::vector< BOOST_PP_ENUM_PARAMS(ZIP_ITERATION, T) > sequences;
            typedef typename mpl::transform<sequences, add_reference<mpl::_> >::type ref_params;
            typedef zip_view<typename result_of::as_vector<ref_params>::type> type;
        };
    }

#define FUSION_REF_PARAM(z, n, data) const T ## n&

    template<BOOST_PP_ENUM_PARAMS(ZIP_ITERATION, typename T)>
    inline typename result_of::zip<BOOST_PP_ENUM_PARAMS(ZIP_ITERATION, const T)>::type
    zip(BOOST_PP_ENUM_BINARY_PARAMS(ZIP_ITERATION, T, const& t))
    {
        fusion::vector<BOOST_PP_ENUM(ZIP_ITERATION, FUSION_REF_PARAM, _)> seqs(
            BOOST_PP_ENUM_PARAMS(ZIP_ITERATION, t));
        return typename result_of::zip<BOOST_PP_ENUM_PARAMS(ZIP_ITERATION, const T)>::type(
            seqs);
    }

#undef FUSION_REF_PARAM
#undef ZIP_ITERATION

#endif
