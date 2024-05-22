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
          bool const has_parent = parent_ptr ? true : false;
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
};

} // namespace treexx::bin

#endif // TREEXX_BIN_TREEALGO_HH
