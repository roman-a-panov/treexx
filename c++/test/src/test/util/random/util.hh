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

#include <cstdint>
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

    auto count = 7548u;
    for(T val = static_cast<T>(3);;)
    {
      T const& val_const = val;
      if constexpr(Is_negatable_ret_val_<Fun, T const&>::value)
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

  template<class Fun>
  static void gen_56972304(Fun&& fun)
  {
    using Is_negatable = Is_negatable_ret_val_<Fun, Unt_64_ const&>;

    Gen_56972304_data_<Is_negatable::value> data;
    auto const on_lo_word = [&data, &fun](Unt_32_ const lo_word) -> bool
    {
      auto val = static_cast<Unt_64_>(data.hi_word);
      val <<= 32u;
      val &= 0xffffffff00000000u;
      val += lo_word;
      auto const& val_const = val;
      if constexpr(Is_negatable::value)
      {
        if(!static_cast<Fun&&>(fun)(val_const))
        {
          data.proceed = false;
          return false;
        }
      }
      else
      {
        static_cast<Fun&&>(fun)(val_const);
      }

      return true;
    };
    auto const on_hi_word = [&data, &on_lo_word](Unt_32_ const hw) -> bool
    {
      data.hi_word = hw;
      gen_7548<Unt_32_>(on_lo_word);
      if constexpr(Is_negatable::value)
      {
        return data.proceed;
      }
      else
      {
        return true;
      }
    };
    gen_7548<Unt_32_>(on_hi_word);
  }

private:
  using Unt_32_ = ::std::uint32_t;
  using Unt_64_ = ::std::uint64_t;

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

  template<class Fun, class... Args>
  using Is_negatable_ret_val_ =
    Is_negatable_<decltype(ref_<Fun>()(ref_<Args>()...))>;

  struct Gen_56972304_data_base_
  {
    Unt_32_ hi_word;
  };

  template<bool negatable, int = 0>
  struct Gen_56972304_data_ : Gen_56972304_data_base_
  {};

  template<int z>
  struct Gen_56972304_data_<true, z> : Gen_56972304_data_base_
  {
    Gen_56972304_data_() noexcept :
      proceed(true)
    {}

    bool proceed;
  };
};

} // namespace test::util::random

#endif // TEST_UTIL_RANDOM_UTIL_HH
