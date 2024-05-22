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

#include <iostream>
#include <memory>

#include <treexx/bin/side.hh>
#include <treexx/bin/tree_algo.hh>
#include <treexx/bin/avl/balance.hh>
#include <treexx/bin/avl/tree_algo.hh>

using My_offset = double;

template<class Value>
struct My_node
{
  using Side = ::treexx::bin::Side;
  using Balance = ::treexx::bin::avl::Balance;

  My_node* parent = nullptr;
  My_node* left_child = nullptr;
  My_node* right_child = nullptr;
  My_offset offset = -1.0;
  Balance balance = Balance::poised;
  Side side = Side::left;
  Value value;
};

template<class Value>
using My_node_pointer = My_node<Value>*; // Might be a user-defined class.

template<class Value>
struct My_tree
{
  using Offset = My_offset;
  using Side = ::treexx::bin::Side;
  using Balance = ::treexx::bin::avl::Balance;

  My_tree() = default;
  My_tree(My_tree&&) = delete;
  My_tree(My_tree const&) = delete;

  ~My_tree()
  {
    clear();
  }

  My_tree& operator=(My_tree&&) = delete;
  My_tree& operator=(My_tree const&) = delete;

  void clear() noexcept
  {
    ::treexx::bin::Tree_algo::clear(
      *this,
      [](auto const& node_ptr)
      {
        delete node_ptr;
      });
    root_node_ptr_ = nullptr;
    leftmost_node_ptr_ = nullptr;
    rightmost_node_ptr_ = nullptr;
  }

  My_node_pointer<Value> const& root() noexcept
  {
    return root_node_ptr_;
  }

  void set_root(My_node_pointer<Value> const& node_ptr) noexcept
  {
    root_node_ptr_ = node_ptr;
  }

  template<Side side>
  My_node_pointer<Value> const& extreme() noexcept
  {
    if constexpr(Side::left == side)
    {
      return leftmost_node_ptr_;
    }
    else if constexpr(Side::right == side)
    {
      return rightmost_node_ptr_;
    }
  }

  template<Side side>
  void set_extreme(My_node_pointer<Value> const& node_ptr) noexcept
  {
    if constexpr(Side::left == side)
    {
      leftmost_node_ptr_ = node_ptr;
    }
    else if constexpr(Side::right == side)
    {
      rightmost_node_ptr_ = node_ptr;
    }
  }

  // This method does not have to be static.
  static My_node<Value>* address(
    My_node_pointer<Value> const& node_ptr) noexcept
  {
    return node_ptr;
  }

  // This method does not have to be static.
  static My_node_pointer<Value> const& parent(
    My_node<Value> const& node) noexcept
  {
    return node.parent;
  }

  // This method does not have to be static.
  static void set_parent(
    My_node<Value>& node,
    My_node_pointer<Value> const& parent_ptr) noexcept
  {
    node.parent = parent_ptr;
  }

  // This method does not have to be static.
  template<Side side>
  static My_node_pointer<Value> const& child(
    My_node<Value> const& node) noexcept
  {
    if constexpr(Side::left == side)
    {
      return node.left_child;
    }
    else if constexpr(Side::right == side)
    {
      return node.right_child;
    }
  }

  // This method does not have to be static.
  template<Side side>
  static void set_child(
    My_node<Value>& node,
    My_node_pointer<Value> const& child_ptr) noexcept
  {
    if constexpr(Side::left == side)
    {
      node.left_child = child_ptr;
    }
    else if constexpr(Side::right == side)
    {
      node.right_child = child_ptr;
    }
  }

  // This method does not have to be static.
  static Balance balance(My_node<Value> const& node) noexcept
  {
    return node.balance;
  }

  // This method does not have to be static.
  static void set_balance(My_node<Value>& node, Balance const balance) noexcept
  {
    node.balance = balance;
  }

  // This method does not have to be static.
  static Side side(My_node<Value> const& node) noexcept
  {
    return node.side;
  }

  // This method does not have to be static.
  static void set_side(My_node<Value>& node, Side const side) noexcept
  {
    node.side = side;
  }

  // This method does not have to be static.
  static Offset const& offset(My_node<Value> const& node) noexcept
  {
    return node.offset;
  }

  // This method does not have to be static.
  static void set_offset(My_node<Value>& node, Offset const& offset) noexcept
  {
    node.offset = offset;
  }

  // This method does not have to be static.
  static void add_to_offset(
    My_node<Value>& node,
    Offset const& increment) noexcept
  {
    node.offset += increment;
  }

  // This method does not have to be static.
  static void subtract_from_offset(
    My_node<Value>& node,
    Offset const& decrement) noexcept
  {
    node.offset -= decrement;
  }

  // This method must be static.
  template<unsigned offset>
  static Offset make_offset() noexcept
  {
    return static_cast<Offset>(offset);
  }

private:
  My_node_pointer<Value> root_node_ptr_ = nullptr;
  My_node_pointer<Value> leftmost_node_ptr_ = nullptr;
  My_node_pointer<Value> rightmost_node_ptr_ = nullptr;
};

template<class Tree, class T>
static auto lower_bound_by_offset(Tree& tree, T const& offset)
{
  using ::treexx::Compare_result;

  // The `unique` hint is set to true because each node has a unique offset.
  return ::treexx::bin::avl::Tree_algo::lower_bound<false, false, true, true>(
    tree,
    [&offset](auto const& node_offset) -> Compare_result
    {
      if(node_offset < offset)
      {
        return Compare_result::less;
      }
      return offset < node_offset ?
        Compare_result::greater :
        Compare_result::equal;
    });
}

int main()
{
  using Value = int;
  using Offset = My_offset;
  using Node = My_node<Value>;
  using ::treexx::bin::avl::Tree_algo;

  My_tree<Value> tree;
  auto const offsets = {3.6, 5.4, 10.3, 15.98, -98.1, 1.4, 33.7, 11.9};

  for(auto const& offset: offsets)
  {
    auto node_ptr = ::std::make_unique<Node>();
    node_ptr->value = static_cast<Value>(offset * 4.5);
    Tree_algo::insert_at_offset(tree, node_ptr.release(), offset);
  }

  Tree_algo::for_each(
    tree,
    [&tree](Node& node)
    {
      Offset const node_offset = Tree_algo::node_offset(tree, node);
      ::std::cout <<
        "Node at offset " << node_offset <<
        " has value " << node.value << ::std::endl;
    });

  ::std::cout << ::std::endl;
  for(auto const& offset: offsets)
  {
    auto const node_ptr = lower_bound_by_offset(tree, offset - 0.001);
    if(node_ptr)
    {
      ::std::cout <<
        "Node at offset " << offset <<
        " has value " << node_ptr->value << ::std::endl;
    }
    else
    {
      ::std::cerr << "Failed to find node with offset " << offset;
    }
  }

  tree.clear();
  auto const push_back_offsets = {-10.5, 5.0, 8.5};

  for(auto const& offset: push_back_offsets)
  {
    auto node_ptr = ::std::make_unique<Node>();
    node_ptr->value = static_cast<Value>(offset * 2.0);
    Tree_algo::push_back(tree, node_ptr.release(), offset);
  }

  ::std::cout << ::std::endl;
  Tree_algo::for_each(
    tree,
    [&tree](Node& node)
    {
      Offset const node_offset = Tree_algo::node_offset(tree, node);
      ::std::cout <<
        "Node at offset " << node_offset <<
        " has value " << node.value << ::std::endl;
    });

  return 0;
}
