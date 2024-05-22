/*
The MIT License (MIT) https://opensource.org/license/mit

Copyright (c) 2013-2024 Roman Panov roman.a.panov@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef TEST_TREEXX_UTIL_UTIL_HH
#define TEST_TREEXX_UTIL_UTIL_HH

#include <treexx/compare_result.hh>

namespace test::treexx::util
{

struct Util
{
  using Compare_result = ::treexx::Compare_result;

  template<class X, class Y>
  [[nodiscard]] static Compare_result compare(X const& x, Y const& y) noexcept
  {
    if(x < y)
    {
      return Compare_result::less;
    }
    return y < x ? Compare_result::greater : Compare_result::equal;
  }
};

} // namespace test::treexx::util

#endif // TEST_TREEXX_UTIL_UTIL_HH
