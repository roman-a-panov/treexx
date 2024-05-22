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

#ifndef TEST_UTIL_RANDOM_UTIL_HH
#define TEST_UTIL_RANDOM_UTIL_HH

#include <type_traits>

namespace test::util::random
{

struct Util
{
  template<class T, class Fun>
  static void gen_7548(Fun&& fun)
  {
    auto constexpr a = static_cast<T>(7);
    auto constexpr c = static_cast<T>(771);
    auto constexpr m = static_cast<T>(15098);
    bool constexpr is_negatable =
      Is_negatable_<decltype(ref_<Fun>()(ref_<T const&>()))>::value;

    auto count = 7548u;
    for(T val = static_cast<T>(3);;)
    {
      T const& val_const = val;
      if constexpr(is_negatable)
      {
        if(!static_cast<Fun&&>(fun)(val_const))
        {
          break;
        }
      }
      else
      {
        static_cast<Fun&&>(fun)(val_const);
      }

      if(0u < --count)
      {
        val = (val * a + c) % m;
      }
      else
      {
        break;
      }
    }
  }

private:
  template<class... T>
  using Void_ = ::std::void_t<T...>;

  template<class T>
  static T constexpr&& ref_() noexcept;

  template<class, class = Void_<>>
  struct Is_negatable_
  {
    static bool constexpr value = false;
  };

  template<class T>
  struct Is_negatable_<T, Void_<decltype(!ref_<T>() ? 0 : 1)>>
  {
    static bool constexpr value = true;
  };
};

} // namespace test::util::random

#endif // TEST_UTIL_RANDOM_UTIL_HH
