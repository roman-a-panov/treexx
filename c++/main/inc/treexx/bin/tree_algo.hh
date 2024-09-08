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

#ifndef TREEXX_BIN_TREEALGO_HH
#define TREEXX_BIN_TREEALGO_HH

#include <memory>
#include <type_traits>

#include <treexx/assert.hh>
#include <treexx/bin/side.hh>
#include <treexx/util/util.hh>

namespace treexx::bin
{

class Tree_algo
{
  using Util_ = ::treexx::util::Util;

  template<class T>
  using Decay_ = typename ::std::decay<T>::type;

  template<class T>
  using Add_const_ = typename ::std::add_const<T>::type;

  template<class T>
  using Remove_pointer_ = typename ::std::remove_pointer<T>::type;

  template<class T>
  using Remove_reference_ = typename ::std::remove_reference<T>::type;

public:
  template<class Tree>
  using Node_pointer = Decay_<decltype(Util_::ref<Tree>().root())>;

  template<class Tree>
  using Node = Remove_reference_<Remove_pointer_<Decay_<decltype(
    Util_::ref<Tree>().address(
      Util_::ref<Add_const_<Node_pointer<Tree>>&>()))>>>;

  template<class Tree>
  [[nodiscard]] static Node_pointer<Tree> next_node(
    Tree&& tree,
    Node<Tree>& node) noexcept
  {
    return adjacent_node<Side::right>(static_cast<Tree&&>(tree), node);
  }

  template<class Tree>
  [[nodiscard]] static Node_pointer<Tree> previous_node(
    Tree&& tree,
    Node<Tree>& node) noexcept
  {
    return adjacent_node<Side::left>(static_cast<Tree&&>(tree), node);
  }

  template<Side side, class Tree>
  [[nodiscard]] static Node_pointer<Tree> adjacent_node(
    Tree&& tree,
    Node<Tree>& node) noexcept
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    static_assert(Side::left == side || Side::right == side);
    Side constexpr opp_side = Side::left == side ? Side::right : Side::left;
    Node_pointer child_ptr(
      static_cast<Tree&&>(tree).template child<side>(node));
    if(child_ptr)
    {
      for(;;)
      {
        Node* const child(static_cast<Tree&&>(tree).address(child_ptr));
        TREEXX_ASSERT(child);
        Node_pointer const grandchild_ptr(
          static_cast<Tree&&>(tree).template child<opp_side>(*child));
        if(grandchild_ptr)
        {
          child_ptr = grandchild_ptr;
        }
        else
        {
          return child_ptr;
        }
      }
    }

    Node_pointer parent_ptr(static_cast<Tree&&>(tree).parent(node));
    if(parent_ptr)
    {
      Side subtree_side(static_cast<Tree&&>(tree).side(node));
      for(;;)
      {
        if(opp_side == subtree_side)
        {
          return parent_ptr;
        }

        Node* const parent(static_cast<Tree&&>(tree).address(parent_ptr));
        TREEXX_ASSERT(parent);
        Node_pointer const grandparent_ptr(
          static_cast<Tree&&>(tree).parent(*parent));
        if(grandparent_ptr)
        {
          parent_ptr = grandparent_ptr;
          subtree_side = static_cast<Tree&&>(tree).side(*parent);
        }
        else
        {
          break;
        }
      }
    }

    return Node_pointer(nullptr);
  }

  template<class Tree, class Fun>
  static void for_each(Tree&& tree, Fun&& fun)
  {
    for_each_<Side::right>(static_cast<Tree&&>(tree), static_cast<Fun&&>(fun));
  }

  template<class Tree, class Fun>
  static void for_each_backward(Tree&& tree, Fun&& fun)
  {
    for_each_<Side::left>(static_cast<Tree&&>(tree), static_cast<Fun&&>(fun));
  }

  template<class Tree, class Destroy>
  static void clear(Tree&& tree, Destroy&& destroy) noexcept(noexcept(
    Util_::ref<Destroy>()(Util_::ref<Add_const_<Node_pointer<Tree>>&>())))
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    enum class Subtree_state : short unsigned
    {
      none = 0u,
      right,
      left_and_right
    };

    Node_pointer node_ptr(
      static_cast<Tree&&>(tree).template extreme<Side::left>());
    if(!node_ptr)
    {
      return; // Empty tree.
    }

    Subtree_state subtree_state = Subtree_state::right;

    for(;;)
    {
      Node* const node(static_cast<Tree&&>(tree).address(node_ptr));
      TREEXX_ASSERT(node);

      switch(subtree_state)
      {
      case Subtree_state::left_and_right:
        {
          Node_pointer const left_child_ptr(
            static_cast<Tree&&>(tree).template child<Side::left>(*node));
          if(left_child_ptr)
          {
            node_ptr = left_child_ptr;
          }
          else
          {
            subtree_state = Subtree_state::right;
          }
        }
        break;

      case Subtree_state::right:
        {
          Node_pointer const right_child_ptr(
            static_cast<Tree&&>(tree).template child<Side::right>(*node));
          if(right_child_ptr)
          {
            node_ptr = right_child_ptr;
            subtree_state = Subtree_state::left_and_right;
          }
          else
          {
            subtree_state = Subtree_state::none;
          }
        }
        break;

      default:
        TREEXX_ASSERT(Subtree_state::none == subtree_state);
        {
          Node_pointer const node_to_destroy_ptr(node_ptr);
          Node_pointer const parent_ptr(
            static_cast<Tree&&>(tree).parent(*node));
          bool const has_parent(parent_ptr ? true : false);
          if(has_parent)
          {
            Side const side(static_cast<Tree&&>(tree).side(*node));
            node_ptr = parent_ptr;
            if(Side::left == side)
            {
              subtree_state = Subtree_state::right;
            }
            else
            {
              TREEXX_ASSERT(Side::right == side);
              subtree_state = Subtree_state::none;
            }
          }

          static_cast<Destroy&&>(destroy)(node_to_destroy_ptr);
          if(!has_parent)
          {
            return;
          }
        }
        break;
      }
    }
  }

  template<class Tree>
  static void swap(
    Tree&& tree,
    Node_pointer<Tree> const& node_x_ptr,
    Node_pointer<Tree> const& node_y_ptr) noexcept
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    TREEXX_ASSERT(node_x_ptr);
    TREEXX_ASSERT(node_y_ptr);
    if(node_x_ptr == node_y_ptr)
    {
      return;
    }

    Node* const node_x(static_cast<Tree&&>(tree).address(node_x_ptr));
    Node* const node_y(static_cast<Tree&&>(tree).address(node_y_ptr));
    TREEXX_ASSERT(node_x);
    TREEXX_ASSERT(node_y);

    Node_pointer const* const new_leftmost_ptr(swapped_extreme_<Side::left>(
      static_cast<Tree&&>(tree), node_x_ptr, node_y_ptr));
    Node_pointer const* const new_rightmost_ptr(swapped_extreme_<Side::right>(
      static_cast<Tree&&>(tree), node_x_ptr, node_y_ptr));
    Node_pointer const* new_root_ptr = nullptr;
    {
      Node_pointer const root_ptr(static_cast<Tree&&>(tree).root());
      if(node_x_ptr == root_ptr)
      {
        new_root_ptr = ::std::addressof(node_y_ptr);
      }
      else if(node_y_ptr == root_ptr)
      {
        new_root_ptr = ::std::addressof(node_x_ptr);
      }
    }

    Node_pointer parent_x_ptr(static_cast<Tree&&>(tree).parent(*node_x));
    Node_pointer parent_y_ptr(static_cast<Tree&&>(tree).parent(*node_y));

    Node* child(nullptr);
    Node* parent(nullptr);
    Node_pointer const* child_ptr(nullptr);
    Node_pointer const* parent_ptr(nullptr);
    Node_pointer const* grandparent_ptr(nullptr);
    if(node_x_ptr == parent_y_ptr)
    {
      child = node_y;
      parent = node_x;
      child_ptr = ::std::addressof(node_y_ptr);
      parent_ptr = ::std::addressof(node_x_ptr);
      grandparent_ptr = ::std::addressof(parent_x_ptr);
    }
    else if (node_y_ptr == parent_x_ptr)
    {
      child = node_x;
      parent = node_y;
      child_ptr = ::std::addressof(node_x_ptr);
      parent_ptr = ::std::addressof(node_y_ptr);
      grandparent_ptr = ::std::addressof(parent_y_ptr);
    }

    if(child)
    {
      TREEXX_ASSERT(parent);
      TREEXX_ASSERT(child_ptr);
      TREEXX_ASSERT(parent_ptr);
      TREEXX_ASSERT(grandparent_ptr);
      swap_parent_child_(
        static_cast<Tree&&>(tree),
        *child, *parent, *child_ptr, *parent_ptr, *grandparent_ptr);
    }
    else
    {
      Node* parent_x(nullptr);
      Node* parent_y(nullptr);
      Node* left_child_x(nullptr);
      Node* left_child_y(nullptr);
      Node* right_child_x(nullptr);
      Node* right_child_y(nullptr);
      Node_pointer const left_child_x_ptr(
        static_cast<Tree&&>(tree).template child<Side::left>(*node_x));
      Node_pointer const left_child_y_ptr(
        static_cast<Tree&&>(tree).template child<Side::left>(*node_y));
      Node_pointer const right_child_x_ptr(
        static_cast<Tree&&>(tree).template child<Side::right>(*node_x));
      Node_pointer const right_child_y_ptr(
        static_cast<Tree&&>(tree).template child<Side::right>(*node_y));
      Side const side_x(static_cast<Tree&&>(tree).side(*node_x));
      Side const side_y(static_cast<Tree&&>(tree).side(*node_y));
      if(parent_x_ptr)
      {
        parent_x = static_cast<Tree&&>(tree).address(parent_x_ptr);
        TREEXX_ASSERT(parent_x);
      }
      if(parent_y_ptr)
      {
        parent_y = static_cast<Tree&&>(tree).address(parent_y_ptr);
        TREEXX_ASSERT(parent_y);
      }
      if(left_child_x_ptr)
      {
        left_child_x = static_cast<Tree&&>(tree).address(left_child_x_ptr);
        TREEXX_ASSERT(left_child_x);
      }
      if(left_child_y_ptr)
      {
        left_child_y = static_cast<Tree&&>(tree).address(left_child_y_ptr);
        TREEXX_ASSERT(left_child_y);
      }
      if(right_child_x_ptr)
      {
        right_child_x = static_cast<Tree&&>(tree).address(right_child_x_ptr);
        TREEXX_ASSERT(right_child_x);
      }
      if(right_child_y_ptr)
      {
        right_child_y = static_cast<Tree&&>(tree).address(right_child_y_ptr);
        TREEXX_ASSERT(right_child_y);
      }

      static_cast<Tree&&>(tree).set_parent(*node_y, parent_x_ptr);
      static_cast<Tree&&>(tree).template set_child<Side::left>(
        *node_y, left_child_x_ptr);
      static_cast<Tree&&>(tree).template set_child<Side::right>(
        *node_y, right_child_x_ptr);
      if(parent_x)
      {
        set_child(static_cast<Tree&&>(tree), *parent_x, node_y_ptr, side_x);
      }
      if(left_child_x)
      {
        static_cast<Tree&&>(tree).set_parent(*left_child_x, node_y_ptr);
      }
      if(right_child_x)
      {
        static_cast<Tree&&>(tree).set_parent(*right_child_x, node_y_ptr);
      }

      static_cast<Tree&&>(tree).set_parent(*node_x, parent_y_ptr);
      static_cast<Tree&&>(tree).template set_child<Side::left>(
        *node_x, left_child_y_ptr);
      static_cast<Tree&&>(tree).template set_child<Side::right>(
        *node_x, right_child_y_ptr);
      if(parent_y)
      {
        set_child(static_cast<Tree&&>(tree), *parent_y, node_x_ptr, side_y);
      }
      if(left_child_y)
      {
        static_cast<Tree&&>(tree).set_parent(*left_child_y, node_x_ptr);
      }
      if(right_child_y)
      {
        static_cast<Tree&&>(tree).set_parent(*right_child_y, node_x_ptr);
      }
    }

    static_cast<Tree&&>(tree).swap_aux(*node_x, *node_y);

    if(new_root_ptr)
    {
      static_cast<Tree&&>(tree).set_root(*new_root_ptr);
    }
    if(new_rightmost_ptr)
    {
      static_cast<Tree&&>(tree).
        template set_extreme<Side::right>(*new_rightmost_ptr);
    }
    if(new_leftmost_ptr)
    {
      static_cast<Tree&&>(tree).
        template set_extreme<Side::left>(*new_leftmost_ptr);
    }
  }

protected:
  template<class Tree>
  static void set_child(
    Tree&& tree,
    Node<Tree>& parent,
    Node_pointer<Tree> const& child_ptr,
    Side const side) noexcept
  {
    if(Side::left == side)
    {
      static_cast<Tree&&>(tree).
        template set_child<Side::left>(parent, child_ptr);
    }
    else
    {
      TREEXX_ASSERT(Side::right == side);
      static_cast<Tree&&>(tree).
        template set_child<Side::right>(parent, child_ptr);
    }
  }

private:
  template<Side direction, class Tree, class Fun>
  static void for_each_(Tree&& tree, Fun&& fun)
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    static_assert(Side::left == direction || Side::right == direction);
    Side constexpr extreme_side =
      Side::left == direction ? Side::right : Side::left;
    Node_pointer node_ptr(
      static_cast<Tree&&>(tree).template extreme<extreme_side>());
    while(node_ptr)
    {
      Node* const node(static_cast<Tree&&>(tree).address(node_ptr));
      TREEXX_ASSERT(node);
      static_cast<Fun&&>(fun)(*node);
      node_ptr = adjacent_node<direction>(static_cast<Tree&&>(tree), *node);
    }
  }

  template<class Tree>
  static void swap_parent_child_(
    Tree&& tree,
    Node<Tree>& child,
    Node<Tree>& parent,
    Node_pointer<Tree> const& child_ptr,
    Node_pointer<Tree> const& parent_ptr,
    Node_pointer<Tree> const& grandparent_ptr)
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    Node* sibling(nullptr);
    Node* grandparent(nullptr);
    Node* left_grandchild(nullptr);
    Node* right_grandchild(nullptr);
    Node_pointer const left_grandchild_ptr(
      static_cast<Tree&&>(tree).template child<Side::left>(child));
    Node_pointer const right_grandchild_ptr(
      static_cast<Tree&&>(tree).template child<Side::right>(child));
    Node_pointer sibling_ptr, new_left_child_ptr, new_right_child_ptr;
    Side const child_side(static_cast<Tree&&>(tree).side(child));
    if(Side::left == child_side)
    {
      sibling_ptr =
        static_cast<Tree&&>(tree).template child<Side::right>(parent);
      new_right_child_ptr = sibling_ptr;
      new_left_child_ptr = parent_ptr;
    }
    else
    {
      TREEXX_ASSERT(Side::right == child_side);
      sibling_ptr =
        static_cast<Tree&&>(tree).template child<Side::left>(parent);
      new_left_child_ptr = sibling_ptr;
      new_right_child_ptr = parent_ptr;
    }
    if(sibling_ptr)
    {
      sibling = static_cast<Tree&&>(tree).address(sibling_ptr);
      TREEXX_ASSERT(sibling);
    }
    if(grandparent_ptr)
    {
      grandparent = static_cast<Tree&&>(tree).address(grandparent_ptr);
      TREEXX_ASSERT(grandparent);
    }
    if(left_grandchild_ptr)
    {
      left_grandchild =
        static_cast<Tree&&>(tree).address(left_grandchild_ptr);
      TREEXX_ASSERT(left_grandchild);
    }
    if(right_grandchild_ptr)
    {
      right_grandchild =
        static_cast<Tree&&>(tree).address(right_grandchild_ptr);
      TREEXX_ASSERT(right_grandchild);
    }

    if(grandparent)
    {
      Side const parent_side(static_cast<Tree&&>(tree).side(parent));
      set_child(
        static_cast<Tree&&>(tree),
        *grandparent, child_ptr, parent_side);
    }

    static_cast<Tree&&>(tree).set_parent(child, grandparent_ptr);
    static_cast<Tree&&>(tree).template set_child<Side::left>(
      child, new_left_child_ptr);
    static_cast<Tree&&>(tree).template set_child<Side::right>(
      child, new_right_child_ptr);
    static_cast<Tree&&>(tree).set_parent(parent, child_ptr);
    static_cast<Tree&&>(tree).template set_child<Side::left>(
      parent, left_grandchild_ptr);
    static_cast<Tree&&>(tree).template set_child<Side::right>(
      parent, right_grandchild_ptr);

    if(sibling)
    {
      static_cast<Tree&&>(tree).set_parent(*sibling, child_ptr);
    }
    if(left_grandchild)
    {
      static_cast<Tree&&>(tree).set_parent(*left_grandchild, parent_ptr);
    }
    if(right_grandchild)
    {
      static_cast<Tree&&>(tree).set_parent(*right_grandchild, parent_ptr);
    }
  }

  template<Side side, class Tree>
  static auto swapped_extreme_(
    Tree&& tree,
    Node_pointer<Tree> const& node_x_ptr,
    Node_pointer<Tree> const& node_y_ptr) noexcept -> Node_pointer<Tree> const*
  {
    Node_pointer<Tree> const* node(nullptr);
    Node_pointer<Tree> const extreme(
      static_cast<Tree&&>(tree).template extreme<side>());
    if(node_x_ptr == extreme)
    {
      node = ::std::addressof(node_y_ptr);
    }
    else if(node_y_ptr == extreme)
    {
      node = ::std::addressof(node_x_ptr);
    }

    return node;
  }
};

} // namespace treexx::bin

#endif // TREEXX_BIN_TREEALGO_HH
