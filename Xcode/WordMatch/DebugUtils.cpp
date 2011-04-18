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

#include "DebugUtils.h"
#include <boost/lexical_cast.hpp>
#include <fstream>

std::string distance_matrix_to_matlab(const simod1::DTW<float>::DistanceMatrix& d)
{
    std::string s="[";
    typedef simod1::DTW<float>::DistanceMatrix DistanceMatrix;
    for (DistanceMatrix::size_type y=0; y<d.size(); ++y)
    {
        for (DistanceMatrix::size_type x=0; x<d[y].size(); ++x)
        {
            s+=boost::lexical_cast<std::string>(d[y][x]);
            s+=", ";
        }
        s+="; ";
    }
    s+="]";
    return s;
}

std::string path_to_matlab(const simod1::DTW<float>::Path p)
{
    std::string s="[";
    for (simod1::DTW<float>::Path::size_type i=0; i!=p.size(); ++i)
    {
        s += boost::lexical_cast<std::string>(p[i].first);
        s += ", ";
        s += boost::lexical_cast<std::string>(p[i].second);
        s += "; ";
    }
    s+="]";
    return s;
}

void dump_dtw_to_matlab_file(simod1::DTW<float> dtw,
                             const std::string& filename)
{
    std::ofstream outfile(filename.c_str());
    
    if(outfile)
    {
        outfile << "D = " << distance_matrix_to_matlab(dtw.local_distances())  << ";\n";
        outfile << "G = " << distance_matrix_to_matlab(dtw.global_distances()) << ";\n";
        outfile << "path = " << path_to_matlab(dtw.minimal_path()) << ";\n";
    }
    else
    {
        std::cerr << "Could not open " << filename;
    }
}