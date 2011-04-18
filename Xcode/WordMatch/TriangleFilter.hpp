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

#ifndef WORD_MATCH_TRIANGLE_FILTER_HPP
#define WORD_MATCH_TRIANGLE_FILTER_HPP

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>

#include <iostream>
#include <iomanip>

namespace WM {
    
    /**
     * A triangle-shaped filter that can be applied on an array of floats. This
     * filter is windows the array of floats with the triangle and returns the
     * sum of the windowed data, i.e. its the dot-product between the triangle
     * and the data to be filtered. This class operators on array indices, i.e.
     * a left and right border is defined by indices. This is useful when e.g.
     * applying this filter on FFT bins. The filter uses optimized vectorized
     * versions for filter creaetion and execution.
     */
    class TriangleFilter : boost::noncopyable {
        
    public:    
        
        /**
         * Creates a new TriangleFilter. 
         * @param An index defining the left edge of the triangle. (including)
         * @param An index defining the right edge of the triangle. (including)
         * @param The height of the triangle.
         *
         * Note that the array passed to TriangleFilter::apply is required to be
         * valid for index range defined by this constructor, i.e. it has to 
         * accomodate at least right_edge + 1 number of float values.
         */
        TriangleFilter(int left_edge, int right_edge, float height);
        
        ~TriangleFilter();
        
        /**
         * Applies the filter to a float buffer.
         * Note that the array passed to TriangleFilter::apply is required to be
         * valid for index range defined by this constructor, i.e. it has to 
         * accomodate at least right_edge + 1 number of float values.         
         *
         * @param buffer The array to be filter by this TriangleFilter.
         * @return The result of the filtering operation, i.e. the dot-product
         * between the triangle shape and the elements of the buffer as defined
         * by the triangles left and right edge indices.
         */
        float apply(const float* buffer);
        
        /**
         * Used for debugging purposes.
         */
        inline friend std::ostream& operator<< (std::ostream& out, 
                                                const TriangleFilter& f) {
            
            for (int i = 0; i<f.left_edge_; ++i) {
                if (i != 0)
                    out << ", ";
                out << "0 ";
            }
            
            for (int i = 0; i<f.size_; ++i) {
                out << ", " << f.filter_data_[i];        
            }
            
            return out;
        }
        
    private:
        
        const int left_edge_;
        const int right_edge_;
        const float height_;
        
        typedef boost::scoped_array<float> FloatScopedArray;
        
        unsigned short size_;
        
        FloatScopedArray filter_data_;
        
    };
    
    typedef boost::shared_ptr<TriangleFilter> TriangleFilterRef;
}

#endif //WORD_MATCH_TRIANGLE_FILTER_HPP