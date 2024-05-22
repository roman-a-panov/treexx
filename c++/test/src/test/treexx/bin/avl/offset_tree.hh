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

#ifndef TEST_TREEXX_BIN_AVL_OFFSETTREE_HH
#define TEST_TREEXX_BIN_AVL_OFFSETTREE_HH

namespace test::treexx::bin::avl
{

template<class Base, class O>
struct Offset_tree : Base
{
  using Offset = O;

  using Base::Base;

  template<class N>
  [[nodiscard]] static decltype(auto) offset(N const& n) noexcept
  {
    return n.xyz_offset();
  }

  template<class N>
  static void add_to_offset(N& n, Offset const& o) noexcept
  {
    n.add_to_xyz_offset(o);
  }

  template<class N>
  static void subtract_from_offset(N& n, Offset const& o) noexcept
  {
    n.subtract_from_xyz_offset(o);
  }

  template<class N>
  static void set_offset(N& n, Offset const& o) noexcept
  {
    n.set_xyz_offset(o);
  }

  template<unsigned o>
  [[nodiscard]] static Offset make_offset() noexcept
  {
    return static_cast<Offset>(o);
  }
};

} // namespace test::treexx::bin::avl

#endif // TEST_TREEXX_BIN_AVL_OFFSETTREE_HH
