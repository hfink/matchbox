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

#include "TriangleFilter.hpp"

#include <Accelerate/Accelerate.h>

#include <sstream>
#include <stdexcept>

using namespace WM;

TriangleFilter::TriangleFilter(int left_edge, int right_edge, float height) :
    left_edge_(left_edge),
    right_edge_(right_edge),
    height_(height),
    size_(right_edge - left_edge + 1),
    filter_data_(new float[size_])
{
    
    if ( (left_edge >= right_edge) || 
         (left_edge < 0) || 
         (right_edge < 0) ) {
        std::ostringstream oss;
        oss << "TriangleFilter: edge values are invalid: left_edge = '"
            << left_edge << "' right_edge = '" 
            << right_edge << "'." << std::endl;
        
        throw std::invalid_argument(oss.str());
    }
    
    if (height == 0) {
        throw std::invalid_argument("Invalid height input: height == 0.");
    }
    
    int center = (int)((left_edge + right_edge) * 0.5f + 0.5f);
    
    // left rising part with positive slope, without setting center
    int left_side_length = (center - left_edge);
    float left_dx = height / left_side_length;
    float zero = 0;
    vDSP_vramp(&zero, 
               &left_dx, 
               filter_data_.get(), 
               1, 
               left_side_length);    
    
    // right falling part with negative slope, also setting center
    int right_side_length = right_edge - center;
    float right_dx = - height / right_side_length;
    vDSP_vramp(&height, 
               &right_dx, 
               &filter_data_.get()[size_-right_side_length-1], 
               1, 
               right_side_length+1);
    
}

TriangleFilter::~TriangleFilter() {}

float TriangleFilter::apply(const float* buffer) {
    
    //we can simply apply the filter as the dot product with the sample buffer
    //within its range
    
    float result = 0;
    vDSP_dotpr(&buffer[left_edge_], 
               1, 
               filter_data_.get(), 
               1, 
               &result, 
               size_);
    
    return result;
    
}

