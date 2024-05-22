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

#include <treexx/compare_result.hh>
#include <treexx/bin/side.hh>
#include <treexx/bin/tree_algo.hh>
#include <treexx/bin/avl/balance.hh>
#include <treexx/bin/avl/tree_algo.hh>

template<class Value>
struct My_node
{
  using Side = ::treexx::bin::Side;
  using Balance = ::treexx::bin::avl::Balance;

  My_node* parent = nullptr;
  My_node* left_child = nullptr;
  My_node* right_child = nullptr;
  Balance balance = Balance::poised;
  Side side = Side::left;
  Value value;
};

template<class Value>
using My_node_pointer = My_node<Value>*; // Might be a user-defined class.

template<class Value>
struct My_tree
{
  using Side = ::treexx::bin::Side;
  using Balance = ::treexx::bin::avl::Balance;

  My_tree() = default;
  My_tree(My_tree&&) = delete;
  My_tree(My_tree const&) = delete;

  ~My_tree()
  {
    ::treexx::bin::Tree_algo::clear(
      *this,
      [](auto const& node_ptr)
      {
        delete node_ptr;
      });
  }

  My_tree& operator=(My_tree&&) = delete;
  My_tree& operator=(My_tree const&) = delete;

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

private:
  My_node_pointer<Value> root_node_ptr_ = nullptr;
  My_node_pointer<Value> leftmost_node_ptr_ = nullptr;
  My_node_pointer<Value> rightmost_node_ptr_ = nullptr;
};

template<class Tree, class T>
static auto find_by_value(Tree& tree, T const& x)
{
  using ::treexx::Compare_result;

  return ::treexx::bin::avl::Tree_algo::binary_search(
    tree,
    [&x](auto const& node) -> Compare_result
    {
      if(node.value < x)
      {
        return Compare_result::less;
      }
      return x < node.value ?
        Compare_result::greater :
        Compare_result::equal;
    });
}

int main()
{
  using Value = int;
  using Node = My_node<Value>;
  using Node_pointer = My_node_pointer<Value>;
  using ::treexx::Compare_result;
  using ::treexx::bin::Side;
  using ::treexx::bin::avl::Tree_algo;

  My_tree<Value> tree;
  auto const values = {3, 5, 10, 15, -98, 1, 33, 11, 15};

  for(auto const& value: values)
  {
    bool inserted = false;
    Tree_algo::try_insert(
      tree,
      [&value](Node const& node) noexcept -> Compare_result
      {
        if(node.value < value)
        {
          return Compare_result::less;
        }
        return value < node.value ?
          Compare_result::greater : Compare_result::equal;
      },
      [&](Node_pointer const& parent, Side const side) -> Node_pointer
      {
        auto* const node = new Node;
        node->parent = parent;
        node->value = value;
        node->side = side;
        inserted = true;
        return node;
      });

    ::std::cout <<
      ::std::boolalpha << "Value " << value << " inserted: " <<
      inserted << ::std::endl;
  }

  Value total_value = 0;
  Tree_algo::for_each(
    tree,
    [&total_value](Node const node) noexcept
    {
      total_value += node.value;
    });

  ::std::cout << "Total value: " << total_value << ::std::endl;
  ::std::cout << ::std::endl;

  for(Value value = 0; 34 > value; ++value)
  {
    Node_pointer const node_ptr = find_by_value(tree, value);
    bool const found = node_ptr ? true : false;
    ::std::cout <<
      ::std::boolalpha << "Is value " << value <<
      " present: " << found << ::std::endl;
  }

  return 0;
}
