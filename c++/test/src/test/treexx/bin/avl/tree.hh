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

#ifndef TEST_TREEXX_BIN_AVL_TREE_HH
#define TEST_TREEXX_BIN_AVL_TREE_HH

#include <cstddef>

#include <test/treexx/util/pointer.hh>
#include <treexx/bin/side.hh>
#include <treexx/bin/tree_algo.hh>
#include <treexx/bin/avl/balance.hh>

namespace test::treexx::bin::avl
{

template<class Node>
class Tree
{
  template<class T>
  using Pointer_ = ::test::treexx::util::Pointer<T>;

  template<bool, class = void>
  struct Enable_if_
  {};

  template<class T>
  struct Enable_if_<true, T>
  {
    using Type = T;
  };

public:
  using Size = ::std::size_t;
  using Side = ::treexx::bin::Side;
  using Balance = ::treexx::bin::avl::Balance;

  Tree() noexcept :
    root_(nullptr),
    leftmost_(nullptr),
    rightmost_(nullptr),
    size_(0u)
  {}

  Tree(Tree&&) = delete;
  Tree(Tree const&) = delete;

  ~Tree() noexcept
  {
    ::treexx::bin::Tree_algo::clear(
      *this,
      [](Pointer_<Node> const& p) noexcept
      {
        delete p.xyz_address();
      });
  }

  Tree& operator =(Tree&&) = delete;
  Tree& operator =(Tree const&) = delete;

  [[nodiscard]] bool xyz_empty() const noexcept
  {
    return 1u > size_;
  }

  [[nodiscard]] Size xyz_size() const noexcept
  {
    return size_;
  }

  [[nodiscard]] Pointer_<Node> const& root() noexcept
  {
    return root_;
  }

  [[nodiscard]] Pointer_<Node const> root() const noexcept
  {
    return root_;
  }

  template<Side side>
  [[nodiscard]] Pointer_<Node> const& extreme() noexcept
  {
    return extreme<side, Pointer_<Node> const&>(*this);
  }

  template<Side side>
  [[nodiscard]] Pointer_<Node const> extreme() const noexcept
  {
    return extreme<side, Pointer_<Node const>>(*this);
  }

  [[nodiscard]] static Node* address(
    Pointer_<Node> const& p) noexcept
  {
    return p.xyz_address();
  }

  [[nodiscard]] static Node const* address(
    Pointer_<Node const> const& p) noexcept
  {
    return p.xyz_address();
  }

  [[nodiscard]] static decltype(auto) balance(Node const& n) noexcept
  {
    return n.xyz_balance();
  }

  [[nodiscard]] static decltype(auto) side(Node const& n) noexcept
  {
    return n.xyz_side();
  }

  [[nodiscard]] static decltype(auto) parent(Node& n) noexcept
  {
    return n.xyz_parent();
  }

  [[nodiscard]] static decltype(auto) parent(Node const& n) noexcept
  {
    return n.xyz_parent();
  }

  template<Side side>
  [[nodiscard]] static decltype(auto) child(Node& n) noexcept
  {
    return n.template xyz_child<side>();
  }

  template<Side side>
  [[nodiscard]] static decltype(auto) child(Node const& n) noexcept
  {
    return n.template xyz_child<side>();
  }

  void set_root(Pointer_<Node> const& p) noexcept
  {
    root_ = p;
  }

  template<Side side>
  auto set_extreme(
    Pointer_<Node> const& p) noexcept ->
      typename Enable_if_<Side::left == side>::Type
  {
    leftmost_ = p;
  }

  template<Side side>
  auto set_extreme(
    Pointer_<Node> const& p) noexcept ->
      typename Enable_if_<Side::right == side>::Type
  {
    rightmost_ = p;
  }

  void increment_xyz_size() noexcept
  {
    ++size_;
  }

  void decrement_xyz_size() noexcept
  {
    if(0u < size_)
    {
      --size_;
    }
  }

  void xyz_reset() noexcept
  {
    root_ = nullptr;
    leftmost_ = nullptr;
    rightmost_ = nullptr;
    size_ = 0u;
  }

  static void set_balance(Node& n, Balance const b) noexcept
  {
    return n.set_xyz_balance(b);
  }

  static void set_side(Node& n, Side const s) noexcept
  {
    return n.set_xyz_side(s);
  }

  static void set_parent(Node& n, Pointer_<Node> const& p) noexcept
  {
    n.set_xyz_parent(p);
  }

  template<Side side>
  static void set_child(Node& n, Pointer_<Node> const& c) noexcept
  {
    n.template set_xyz_child<side>(c);
  }

  static void swap_aux(Node& x, Node& y) noexcept
  {
    x.swap_xyz_aux(y);
  }

private:
  template<Side side, class Ret, class Self>
  [[nodiscard]] static auto extreme(
    Self& self) noexcept ->typename Enable_if_<Side::left == side, Ret>::Type
  {
    return self.leftmost_;
  }

  template<Side side, class Ret, class Self>
  [[nodiscard]] static auto extreme(
    Self& self) noexcept ->typename Enable_if_<Side::right == side, Ret>::Type
  {
    return self.rightmost_;
  }

  Pointer_<Node> root_;
  Pointer_<Node> leftmost_;
  Pointer_<Node> rightmost_;
  Size size_;
};

} // namespace test::treexx::bin::avl

#endif // TEST_TREEXX_BIN_AVL_TREE_HH
