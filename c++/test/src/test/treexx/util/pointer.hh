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

#ifndef TEST_TREEXX_UTIL_POINTER_HH
#define TEST_TREEXX_UTIL_POINTER_HH

#include <cstddef>
#include <limits>
#include <type_traits>

namespace test::treexx::util
{

template<class T>
struct Pointer
{
  using Nullptr = ::std::nullptr_t;

private:
  using Size_ = ::std::size_t;
  static_assert(sizeof (T*) == sizeof(Size_));

  template<class U>
  using Numeric_limits_ = ::std::numeric_limits<U>;

  template<class From, class To>
  using Is_convertible_ = typename ::std::is_convertible<From, To>::type;

  template<bool, class = void>
  struct Enable_if_
  {};

  template<class U>
  struct Enable_if_<true, U>
  {
    using Type = U;
  };

public:
  Pointer() noexcept :
    addr_(reinterpret_cast<T*>(Numeric_limits_<Size_>::max()))
  {}

  Pointer(Nullptr const) noexcept :
    addr_(nullptr)
  {}

  template<
    class U,
    bool e = Is_convertible_<U*, T*>::value,
    class = typename Enable_if_<e>::Type>
  Pointer(Pointer<U> const& x) noexcept :
    addr_(x.xyz_address())
  {}

  [[nodiscard]] explicit operator bool() const noexcept
  {
    return addr_ ? true : false;
  }

  [[nodiscard]] T* xyz_address() const noexcept
  {
    return addr_;
  }

  [[nodiscard]] static Pointer from_xyz_address(T* const addr) noexcept
  {
    return Pointer(addr);
  }

  [[nodiscard]] friend bool operator ==(
    Pointer const& x,
    Pointer const& y) noexcept
  {
    return x.addr_ == y.addr_;
  }

private:
  explicit Pointer(T* const addr) noexcept :
    addr_(addr)
  {}

  T* addr_;
};

} // namespace test::treexx::util

#endif // TEST_TREEXX_UTIL_POINTER_HH
