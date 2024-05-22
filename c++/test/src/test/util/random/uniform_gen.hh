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

#ifndef TEST_UTIL_RANDOM_UNIFORMGEN_HH
#define TEST_UTIL_RANDOM_UNIFORMGEN_HH

#include <random>
#include <type_traits>

namespace test::util::random
{

template<class T>
class Uniform_gen
{
  template<class U>
  using Remove_cv_ = typename ::std::remove_cv<U>::type;

  template<class U>
  using Remove_reference_ = typename ::std::remove_reference<U>::type;

  template<class U>
  using Remove_cv_ref_ = Remove_cv_<Remove_reference_<Remove_cv_<U>>>;

  template<class U>
  using Is_integral_ = typename ::std::is_integral<U>::type;

  template<class U>
  using Is_floating_point_ = typename ::std::is_floating_point<U>::type;

public:
  using Value = Remove_cv_ref_<T>;

  explicit Uniform_gen(Value const& a, T const& b) :
    dist_(a, b)
  {}

  [[nodiscard]] T operator ()()
  {
    return dist_(engine_);
  }

private:
  using Engine_ = ::std::mt19937;

  static bool constexpr is_int_ = Is_integral_<Value>::value;
  static bool constexpr is_float_ = Is_floating_point_<Value>::value;
  static_assert(is_int_ || is_float_);

  template<class U = Value, bool = is_int_, bool = is_float_>
  struct Dist_type_
  {};

  template<class U>
  struct Dist_type_<U, true, false>
  {
    using Type = ::std::uniform_int_distribution<U>;
  };

  template<class U, bool is_int>
  struct Dist_type_<U, is_int, true>
  {
    using Type = ::std::uniform_real_distribution<U>;
  };

  using Dist_ = typename Dist_type_<>::Type;

  Engine_ engine_;
  Dist_ dist_;
};

} // namespace test::util::random

#endif // TEST_UTIL_RANDOM_UNIFORMGEN_HH
