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

#ifndef WORD_MATCH_DTW_HPP
#define WORD_MATCH_DTW_HPP

#include <boost/multi_array.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <cmath>
#include <algorithm>

namespace
{
  template <typename iterator, typename element>
  size_t index_of(iterator first, iterator last, element e);
}

namespace simod1
{
  template <typename T, size_t feature_number>
  T vector_distance(const boost::array<T, feature_number>& v1,
                    const boost::array<T, feature_number>& v2);

  template <typename T = double, size_t feature_number=3>
  class DTW
  {
  public:
    typedef boost::array<T, feature_number> FeatureVector;
    typedef std::vector<FeatureVector> Features;
    typedef boost::multi_array<T, 2> DistanceMatrix;
    typedef std::pair<unsigned, unsigned> Coordinate;
    typedef std::vector<Coordinate> Path;
    static const size_t feature_number_size = feature_number;
    DTW(const Features& a, const Features& b, unsigned adjustment_window_size);
    ~DTW() {}
    T minimum_distance() const { return minimum_distance_; }
    Path minimal_path();
    DistanceMatrix local_distances() const { return d; }
    DistanceMatrix global_distances() const { return g; }
  private:
    const Features features_a;
    const Features features_b;
    const unsigned r;
    DistanceMatrix d;
    DistanceMatrix g;
    T minimum_distance_;
    typedef boost::multi_array<unsigned short, 2> step_matrix;
    step_matrix steps;
    void init_local_distance_matrix();
    void init_global_distance_matrix();
    void calculate_global_distance_matrix();
    void calculate_minimum_distance();
  };

  template <typename T, size_t feature_number>
  DTW<T, feature_number>::DTW(const Features& a,
                              const Features& b,
                              unsigned adjustment_window_size)
    : features_a(a),
      features_b(b),
      r(adjustment_window_size),
      d(boost::extents[a.size()][b.size()]),
      g(boost::extents[a.size()+1][b.size()+1]),
      steps(boost::extents[a.size()][b.size()])
  {
    if ((a.size() == 0) || (b.size() == 0))
      throw std::logic_error("feature argument empty");
    if (adjustment_window_size == 0)
      throw std::logic_error("adjustment window size cannot be zero");

    init_local_distance_matrix();
    init_global_distance_matrix();
    calculate_global_distance_matrix();
    calculate_minimum_distance();
  }

  template <typename T, size_t feature_number>
  void DTW<T, feature_number>::init_global_distance_matrix()
  {
    T max_value = std::numeric_limits<T>::max();
    if (std::numeric_limits<T>::has_infinity)
      max_value = std::numeric_limits<T>::infinity();
    std::fill(g.origin(), g.origin()+g.num_elements(), max_value);
  }

  template <typename T, size_t feature_number>
  void DTW<T, feature_number>::init_local_distance_matrix()
  {
    for (typename DistanceMatrix::size_type i=0; i<features_a.size(); ++i)
      for (typename DistanceMatrix::size_type j=0; j<features_b.size(); ++j)
        d[i][j] = vector_distance<T, feature_number>(features_a[i], features_b[j]);
  }

  template <typename T, size_t feature_number>
  void DTW<T, feature_number>::calculate_global_distance_matrix()
  {
    g[0][0] = 2*d[0][0];
    const T slope = static_cast<T>(features_b.size())/static_cast<T>(features_a.size());
    for (typename DistanceMatrix::size_type i=1; i!=g.size(); ++i)
      for (typename DistanceMatrix::size_type j=1; j!=g[i].size(); ++j)
      {
        if (std::fabs(i-(j/slope)) > r)
          continue;
        const T distances[] = {
            g[i  ][j-1] +   d[i-1][j-1],
            g[i-1][j-1] + 2*d[i-1][j-1],
            g[i-1][j  ] +   d[i-1][j-1]
          };
        const T min_distance = *std::min_element(distances, distances+3);
        steps[i-1][j-1] = index_of(distances, distances+3, min_distance);
        g[i][j] = min_distance;
      }
  }

  template <typename T, size_t feature_number>
  void DTW<T, feature_number>::calculate_minimum_distance()
  {
    typename DistanceMatrix::size_type I = g.size()-1;
    typename DistanceMatrix::size_type J = g[I].size()-1;
    minimum_distance_ = g[I][J]/(I+J);
  }

  template <typename T, size_t feature_number>
  typename DTW<T, feature_number>::Path DTW<T, feature_number>::minimal_path()
  {
    typename DTW::step_matrix::size_type i = steps.shape()[0]-1;
    typename DTW::step_matrix::size_type j = steps.shape()[1]-1;
    typename DTW::Path min_path;

    while ((i>0) && (j>0))
    {
      switch (steps[i][j])
      {
        case 0:
          //we got here from g(i, j-1)
          --j;
          break;
        case 1:
          // we got here from g(i-1, j-1)
          --i;
          --j;
          break;
        case 2:
          // we got here from g(i-1, j)
          --i;
          break;
        default:
          throw std::logic_error("Oh noes!!1 This cannot have happened.");
      }
      min_path.push_back(std::make_pair(i, j));
    }
    typename DTW::Path retval;
    std::copy(min_path.rbegin(), min_path.rend(), std::back_inserter(retval));
    return retval;
  }

    
  //Note: since we are now using this for boost::array types, the size is
  //part of the template arguments and therefore we can be sure that v1 and v2
  //are equally sized.
  template <typename T, size_t feature_number>
  T vector_distance(const boost::array<T, feature_number>& v1,
                    const boost::array<T, feature_number>& v2)
  {

    typename boost::array<T, feature_number> d;
    for (typename boost::array<T, feature_number>::size_type i=0; i<v1.size(); ++i)
      d[i] = (v1[i]-v2[i]);
    std::transform(d.begin(),
                   d.end(),
                   d.begin(),
                   boost::bind<T, T(&)(T, T)>(std::pow,
                                              _1,
                                              2));
    return (T)std::sqrt(std::accumulate(d.begin(), d.end(), 0));
  }
}

namespace
{
  template <typename iterator, typename element>
  size_t index_of(iterator first, iterator last, element e)
  {
    iterator i = std::find(first, last, e);
    return std::distance(first, i);
  }
}

#endif // WORD_MATCH_DTW_HPP
