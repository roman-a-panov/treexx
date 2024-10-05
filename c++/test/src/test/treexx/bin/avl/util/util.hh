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

#ifndef TEST_TREEXX_AVL_UTIL_UTIL_HH
#define TEST_TREEXX_AVL_UTIL_UTIL_HH

#include <cstdint>
#include <optional>
#include <type_traits>

#include <catch.hpp>

#include <test/treexx/util/util.hh>
#include <treexx/bin/side.hh>
#include <treexx/bin/avl/balance.hh>
#include <treexx/bin/avl/tree_algo.hh>

namespace test::treexx::bin::avl::util
{

struct Util : ::test::treexx::util::Util
{
  template<bool with_index, bool with_offset, class Tree>
  static bool verify_tree(Tree const& tree)
  {
    using Tree_size = Decay_<decltype(tree.xyz_size())>;

    bool const subtrees_ok = 0 <= verify_subtree_(
      tree, tree.root(), Node_pointer<Tree const&>(nullptr), ::std::nullopt);
    Tree_size size = 0u;
    Tree_algo_::for_each(
      tree,
      [&](auto const&) noexcept
      {
        ++size;
      });
    bool const size_ok = tree.xyz_size() == size;
    CHECK(subtrees_ok);
    CHECK(size_ok);
    bool ok = subtrees_ok && size_ok;
    if constexpr(with_index)
    {
      bool const indices_ok = verify_indices(tree);
      ok = ok && indices_ok;
      CHECK(indices_ok);
    }
    if constexpr(with_offset)
    {
      bool const offsets_ok = verify_offsets(tree);
      ok = ok && offsets_ok;
      CHECK(offsets_ok);
    }

    return ok;
  }

private:
  using Int_16_ = ::std::int16_t;
  using Side_ = ::treexx::bin::Side;
  using Balance_ = ::treexx::bin::avl::Balance;
  using Tree_algo_ = ::treexx::bin::avl::Tree_algo;

  template<class T>
  using Optional_ = ::std::optional<T>;

  template<class T>
  using Decay_ = typename ::std::decay<T>::type;

  template<class Tree>
  using Node_pointer = Tree_algo_::Node_pointer<Tree>;

  template<class Tree, class Node_ptr>
  static Int_16_ verify_subtree_(
    Tree const& tree,
    Node_ptr const& node_ptr,
    Node_ptr const& parent_ptr,
    Optional_<Side_> const side)
  {
    auto* const node = tree.address(node_ptr);
    if(!node)
    {
      return 0;
    }

    CHECK(parent_ptr == tree.parent(*node));
    if(side)
    {
      CHECK(*side == tree.side(*node));
    }

    auto const left_subtree_height = verify_subtree_(
      tree, tree.template child<Side_::left>(*node), node_ptr, Side_::left);
    if(0 > left_subtree_height)
    {
      return left_subtree_height;
    }

    auto const right_subtree_height = verify_subtree_(
      tree, tree.template child<Side_::right>(*node), node_ptr, Side_::right);
    if(0 > right_subtree_height)
    {
      return right_subtree_height;
    }

    auto height = left_subtree_height;
    Balance_ const balance = tree.balance(*node);

    if(left_subtree_height < right_subtree_height)
    {
      auto const imbalance = right_subtree_height - left_subtree_height;
      CHECK(1 == imbalance);
      if(1 != imbalance)
      {
        return -1;
      }

      CHECK(Balance_::overright == balance);
      if(Balance_::overright != balance)
      {
        return -1;
      }

      height = right_subtree_height;
    }
    else if(right_subtree_height < left_subtree_height)
    {
      auto const imbalance = left_subtree_height - right_subtree_height;
      CHECK(1 == imbalance);
      if(1 != imbalance)
      {
        return -1;
      }

      CHECK(Balance_::overleft == balance);
      if(Balance_::overleft != balance)
      {
        return -1;
      }
    }
    else
    {
      // Perfectly balanced.
      CHECK(Balance_::poised == balance);
      if(Balance_::poised != balance)
      {
        return -1;
      }
    }

    auto constexpr max_height = 35;
    CHECK(max_height > height);
    if(max_height > height)
    {
      ++height;
      return height;
    }

    return -1;
  }

  template<class Tree>
  [[nodiscard]] static bool verify_indices(Tree const& tree)
  {
    using Index = typename Decay_<Tree>::Index;

    auto node_ptr = tree.template extreme<Side_::left>();
    Index idx = Decay_<Tree>::template make_index<0u>();
    bool ok = true;

    while(node_ptr)
    {
      auto* const node = tree.address(node_ptr);
      if(!node)
      {
        CHECK(false);
        return false;
      }

      Index const node_idx = Tree_algo_::node_index(tree, *node);
      CHECK(node_idx == idx);
      ok = ok && node_idx == idx;

      auto const found_node_ptr = Tree_algo_::at_index(tree, idx);
      CHECK(found_node_ptr == node_ptr);
      ok = ok && found_node_ptr == node_ptr;

      node_ptr = Tree_algo_::next_node(tree, *node);
      ++idx;
    }

    auto const tree_size = tree.xyz_size();
    CHECK(tree_size == idx);
    ok = ok && tree_size == idx;
    return ok;
  }

  template<class Tree>
  [[nodiscard]] static bool verify_offsets(Tree const& tree)
  {
    using Offset = typename Decay_<Tree>::Offset;

    auto node_ptr = tree.template extreme<Side_::left>();
    Optional_<Offset> offset;
    bool ok = true;

    while(node_ptr)
    {
      auto* const node = tree.address(node_ptr);
      if(!node)
      {
        CHECK(false);
        return false;
      }

      Offset const node_offset = Tree_algo_::node_offset(tree, *node);
      CHECK(node_offset > offset);
      ok = ok && node_offset > offset;

      auto const found_node_ptr = Tree_algo_::binary_search<false, false, true>(
        tree,
        [&node_offset](auto const& o) noexcept -> Compare_result
        {
          return compare(o, node_offset);
        });
      CHECK(found_node_ptr == node_ptr);
      ok = ok && found_node_ptr == node_ptr;

      node_ptr = Tree_algo_::next_node(tree, *node);
      offset = node_offset;
    }

    return ok;
  }
};

} // namespace test::treexx::bin::avl::util

#endif // TEST_TREEXX_AVL_UTIL_UTIL_HH
