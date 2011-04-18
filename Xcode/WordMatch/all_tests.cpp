//Copyright (c) 2011 Sebastian Böhm sebastian@sometimesfood.org
//                   Heinrich Fink hf@hfink.eu
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in
//all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.

#define BOOST_TEST_MODULE SIMOD1

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include "dtw.hpp"

namespace
{
  template <typename T>
  bool almost_equal(T a, T b)
  {
    const boost::test_tools::fraction_tolerance_t<T> tolerance(1.e-5);
    return boost::test_tools::check_is_close(a, b, tolerance);
  }
}

BOOST_AUTO_TEST_SUITE(vector_distance)

BOOST_AUTO_TEST_CASE(equal_vectors)
{
  typedef simod1::DTW<double, 3> DtwType;
  DtwType::FeatureVector a = { {1, 2, 3} };
  DtwType::FeatureVector b = a;
  BOOST_CHECK_EQUAL(simod1::vector_distance(a, b), 0.0);
}

BOOST_AUTO_TEST_CASE(different_vectors)
{
  // sqrt(-3^2+4^2) == sqrt(25) == 5
  typedef simod1::DTW<double, 2> DtwType;
  DtwType::FeatureVector a = { {-3, 0} };
  DtwType::FeatureVector b = { { 0, 4} };
  BOOST_CHECK_EQUAL(simod1::vector_distance(a, b), 5.0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(dtw)

BOOST_AUTO_TEST_CASE(empty_features)
{
  // dtw needs features (which cannot be empty)
  typedef simod1::DTW<double, 0> DtwType;
  DtwType::Features a, b;
  BOOST_CHECK_THROW(DtwType(a, b, 42), std::logic_error);
}

BOOST_AUTO_TEST_CASE(zero_adjustment_window)
{
  // adjustment window size must not be zero
  typedef simod1::DTW<double, 3> DtwType;
  DtwType::FeatureVector fv = { {1, 2, 3} };
  DtwType::Features a;
  a.push_back(fv);
  DtwType::Features b;
  b.push_back(fv);
  b.push_back(fv);
  BOOST_CHECK_THROW(DtwType(a, b, 0), std::logic_error);
}

BOOST_AUTO_TEST_CASE(equal_features)
{
  // dtw distance between equal features should always be 0
  typedef simod1::DTW<double, 3> DtwType;
  DtwType::FeatureVector fv = { {1, 2, 3} };
  DtwType::Features a;
  a.push_back(fv);
  DtwType::Features b = a;
  DtwType d(a, b, 1);
  BOOST_CHECK_EQUAL(d.minimum_distance(), 0.0);
}

BOOST_AUTO_TEST_CASE(same_distance_as_matlab)
{
  // ugly test: check for the same results as in Matlab
  /*
    features_a=[0 1 2 3 4 5 6 7];
    features_b=[7 6 5 4 3 2 1 0];
    dtw(features_a, features_b, 2)

    ans =

      4.6250
  */
  typedef simod1::DTW<double, 1> DtwType;
  DtwType::Features a, b;
  for (int i=0; i<8; ++i)
  {
    DtwType::FeatureVector fv = {{i}};
    a.push_back(fv);
  }
  b.resize(a.size());
  std::copy(a.rbegin(), a.rend(), b.begin());
  DtwType d(a, b, 2);
  BOOST_CHECK(almost_equal(d.minimum_distance(), 4.625));
}

BOOST_AUTO_TEST_CASE(same_distance_as_matlab_multidim)
{
  // ugly test: check for the same results as in Matlab
  /*
    features_a=[0 1 2 3; 4 5 6 7];
    features_b=[3 2 1 0; 7 6 5 4];
    dtw(features_a, features_b, 2)

    ans =

      3.5355
  */
  typedef simod1::DTW<double, 2> DtwType;
  DtwType::Features a, b;
  for (int i=0; i<4; ++i)
  {
    DtwType::FeatureVector fv = {{i, i+4}};
    a.push_back(fv);
  }
  b.resize(a.size());
  std::copy(a.rbegin(), a.rend(), b.begin());
  DtwType d(a, b, 2);
  BOOST_CHECK(almost_equal(d.minimum_distance(), 3.5355));
}

BOOST_AUTO_TEST_CASE(same_path_as_matlab)
{
  // ugly test: check for the same results as in Matlab
  /*
    features_b=[7 6 5 4 3 2 1 0];
    features_a=[0 1 2 3 4 5 6 7];
    [~, ~, ~, path] = dtw(features_a, features_b, 2)

    path =

         2     1
         3     2
         4     3
         5     4
         6     4
         6     5
         7     5
         7     6
         8     6
         8     7
  */
  typedef simod1::DTW<double, 1> DtwType;
  DtwType::Path matlab_path;
  matlab_path.push_back(std::make_pair(1, 0));
  matlab_path.push_back(std::make_pair(2, 1));
  matlab_path.push_back(std::make_pair(3, 2));
  matlab_path.push_back(std::make_pair(4, 3));
  matlab_path.push_back(std::make_pair(5, 3));
  matlab_path.push_back(std::make_pair(5, 4));
  matlab_path.push_back(std::make_pair(6, 4));
  matlab_path.push_back(std::make_pair(6, 5));
  matlab_path.push_back(std::make_pair(7, 5));
  matlab_path.push_back(std::make_pair(7, 6));
  DtwType::Features a, b;

  for (int i=0; i<8; ++i)
  {
    DtwType::FeatureVector fv = {{i}};
    a.push_back(fv);
  }
  b.resize(a.size());
  std::copy(a.rbegin(), a.rend(), b.begin());
  DtwType d(a, b, 2);

  DtwType::Path p =  d.minimal_path();
  BOOST_CHECK_EQUAL(p.size(), matlab_path.size());
  for (DtwType::Path::size_type i=0; i<p.size(); ++i)
  {
    BOOST_CHECK_EQUAL(p[i].first, matlab_path[i].first);
    BOOST_CHECK_EQUAL(p[i].second, matlab_path[i].second);
  }
}

BOOST_AUTO_TEST_SUITE_END()
