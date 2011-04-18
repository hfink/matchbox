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

#include <boost/test/unit_test.hpp>
#include <boost/scoped_array.hpp>

#include <algorithm>
#include <stdexcept>

#include "TriangleFilter.hpp"

#include <iostream>

typedef boost::scoped_array<float> FloatScopedArray;

BOOST_AUTO_TEST_SUITE( TriangleFilterTest )

using namespace WM;

/**
 * Set up a signal with a dirac impulse at a known location. A triangle filter
 * is centered at this impulse with a height of 1, and therefore should 
 * preserve the impulse exactly.
 */
BOOST_AUTO_TEST_CASE(ImpulseTest) {
    
    // define an impulse and apply triangle filter
    FloatScopedArray arr(new float[100]);
    vDSP_vclr(arr.get(), 1, 100);
    
    // set one dirac impulse at idx  = 10
    arr[10] = 1.0f;
    
    // Create a triangle filter with center at idx = 10
    
    TriangleFilter filter(7, 13, 1.0f);
    
    float result = filter.apply(arr.get());
    
    //std::cout << "Filter: " << filter << std::endl;
    
    BOOST_REQUIRE_CLOSE(result, 1.0f, 0.1);
    
    
}

/**
 * We define a constant signal, and define a normalized triangle filter. The
 * resulting value must be equal to one.
 */
BOOST_AUTO_TEST_CASE(ConstantInputTest) {
    
    // define a constant and apply triangle filter
    
    FloatScopedArray arr(new float[100]);
    std::fill(&arr[0], &arr[100], 1);
    
    int right = 60;
    int left = 10;
    
    float height = 2.0f / (right - left);
    
    TriangleFilter filter(left, right, height);
    
    float result = filter.apply(arr.get());
    
    //std::cout << "Filter: " << filter << std::endl;
    
    BOOST_REQUIRE_CLOSE(result, 1.0f, 0.1);
    
}

/**
 * Assymetric case.
 */
BOOST_AUTO_TEST_CASE(AssymetricFilter) {
    
    FloatScopedArray arr(new float[6]);
    std::fill(&arr[0], &arr[6], 1);
    
    int right = 5;
    int left = 0;
    
    float height = 2.0f / (right - left);
    
    TriangleFilter filter(left, right, height);
    
    float result = filter.apply(arr.get());
    
    //std::cout << "Filter: " << filter << std::endl;
    
    BOOST_REQUIRE_CLOSE(result, 1.0f, 0.1);
    
}

/**
 * The same as last test, but with larger numbers.
 */
BOOST_AUTO_TEST_CASE(ConstantInputTestLarge) {
    
    // define a constant and apply triangle filter
    static const int big_number = 60000;
    FloatScopedArray arr(new float[big_number]);
    std::fill(&arr[0], &arr[big_number], 1);
    
    int right = big_number-1;
    int left = 0;
    
    float height = 2.0f / (right - left);
    
    TriangleFilter filter(left, right, height);
    
    float result = filter.apply(arr.get());
    
    BOOST_REQUIRE_CLOSE(result, 1.0f, 0.1);
    
}

/**
 * We test junk input (on construction).
 */
BOOST_AUTO_TEST_CASE(JunkInput) {
    
    BOOST_CHECK_THROW(TriangleFilter t(-100, 0, 1), std::invalid_argument);
    BOOST_CHECK_THROW(TriangleFilter t(20, 0, 1), std::invalid_argument);    
    BOOST_CHECK_THROW(TriangleFilter t(0, 0, 0), std::invalid_argument);
    
}

BOOST_AUTO_TEST_SUITE_END()