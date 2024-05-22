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

#ifndef TEST_MACRO_HH
#define TEST_MACRO_HH

#include <type_traits>

#include <test/util/reg_class.hh>

#define TXX_DO_CONCAT_3_(x, y, z) x ## y ## z
#define TXX_CONCAT_3_(x, y, z) TXX_DO_CONCAT_3_(x, y, z)
#define TXX_GEN_ID(pref) TXX_CONCAT_3_(pref, __COUNTER__, __LINE__)

#define TXX_SECTION() TXX_SECTION_0_(TXX_GEN_ID(txx_sect_guard_))

#define TXX_TEST_CLASS() \
  void const* TXX_GEN_ID(txx_reg_test_)() const noexcept \
  { \
    using This = typename ::std::decay<decltype(*this)>::type; \
    return ::test::util::Reg_class<This>::reg(); \
  }

#endif // TEST_MACRO_HH
