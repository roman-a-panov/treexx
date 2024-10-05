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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <set>
#include <vector>

#include <catch.hpp>

#include <test/treexx/bin/avl/node.hh>
#include <test/treexx/bin/avl/tree.hh>
#include <test/treexx/bin/avl/util/util.hh>
#include <test/treexx/util/pointer.hh>
#include <test/util/random/uniform_gen.hh>
#include <test/util/random/util.hh>
#include <treexx/bin/side.hh>
#include <treexx/compare_result.hh>
#include <treexx/bin/avl/tree_algo.hh>

namespace test::treexx::bin::avl
{

class Simple_tree_core_test
{
  using Compare_result_ = ::treexx::Compare_result;
  using Tree_algo_ = ::treexx::bin::avl::Tree_algo;
  using Util_ = ::test::treexx::bin::avl::util::Util;
  using Random_util_ = ::test::util::random::Util;
  using Side_ = ::treexx::bin::Side;

  template<class... T>
  using Unique_ptr_ = ::std::unique_ptr<T...>;

  template<class T>
  using Pointer_ = ::test::treexx::util::Pointer<T>;

  template<class Value>
  using Node_ = ::test::treexx::bin::avl::Node<Value>;

  template<class Value>
  using Tree_core_ = ::test::treexx::bin::avl::Tree<Node_<Value>>;

protected:
  using Size = ::std::size_t;
  using Ptrdiff = ::std::ptrdiff_t;
  using Int_32 = ::std::int32_t;
  using Unt_32 = ::std::uint32_t;
  using Int_64 = ::std::int64_t;
  using Unt_64 = ::std::uint64_t;

  template<class... T>
  using Set = ::std::set<T...>;

  template<class... T>
  using Deque = ::std::deque<T...>;

  template<class... T>
  using Vector = ::std::vector<T...>;

  template<class T>
  using Uniform_gen = ::test::util::random::Uniform_gen<T>;

  template<class Value>
  struct Tree
  {
    using Node = Node_<Value>;
    using Node_pointer = Pointer_<Node>;
    using Node_const_pointer = Pointer_<Node const>;

    struct Try_insert_result
    {
      Try_insert_result(Node_pointer const& p, bool const i) noexcept :
        node_pointer(p),
        inserted(i)
      {}

      Node_pointer node_pointer;
      bool inserted;
    };

    [[nodiscard]] bool empty() const noexcept
    {
      return core_.xyz_empty();
    }

    [[nodiscard]] decltype(auto) size() const noexcept
    {
      return core_.xyz_size();
    }

    [[nodiscard]] Node_const_pointer leftmost() const noexcept
    {
      return core_.template extreme<Side_::left>();
    }

    [[nodiscard]] Node_const_pointer rightmost() const noexcept
    {
      return core_.template extreme<Side_::right>();
    }

    [[nodiscard]] Node_const_pointer next_node(Node const& n) const noexcept
    {
      return Tree_algo_::next_node(core_, n);
    }

    [[nodiscard]] Node_const_pointer previous_node(Node const& n) const noexcept
    {
      return Tree_algo_::previous_node(core_, n);
    }

    template<class T>
    [[nodiscard]] bool contains(T const& x) const noexcept
    {
      auto const node_ptr = Tree_algo_::lower_bound(
        core_,
        [&x](auto const& node) noexcept -> Compare_result_
        {
          return Util_::compare(node.value(), x);
        });

      if(node_ptr)
      {
        auto const* const node = address(node_ptr);
        if(node)
        {
          return x == node->value();
        }
        else
        {
          CHECK(false);
        }
      }

      return false;
    }

    template<class T>
    [[nodiscard]] Node_pointer binary_search(T const& x) noexcept
    {
      return Tree_algo_::binary_search(
        core_,
        [&x](Node const& node) noexcept -> Compare_result_
        {
          return Util_::compare(node.value(), x);
        });
    }

    template<class T>
    [[nodiscard]] Node_pointer lower_bound(T const& x) noexcept
    {
      return Tree_algo_::lower_bound(
        core_,
        [&x](Node const& node) noexcept -> Compare_result_
        {
          return Util_::compare(node.value(), x);
        });
    }

    template<class T>
    [[nodiscard]] Node_pointer upper_bound(T const& x) noexcept
    {
      return Tree_algo_::upper_bound(
        core_,
        [&x](Node const& node) noexcept -> Compare_result_
        {
          return Util_::compare(node.value(), x);
        });
    }

    template<class Fun>
    void for_each(Fun&& fun) const
    {
      Tree_algo_::for_each(core_, static_cast<Fun&&>(fun));
    }

    template<class Fun>
    void for_each_backward(Fun&& fun) const
    {
      Tree_algo_::for_each_backward(core_, static_cast<Fun&&>(fun));
    }

    void verify() const
    {
      Util_::verify_tree<false, false>(core_);
    }

    template<class... T>
    Value& emplace_back(T&&... x) noexcept
    {
      return emplace_<Side_::right>(static_cast<T&&>(x)...);
    }

    template<class... T>
    Value& emplace_front(T&&... x) noexcept
    {
      return emplace_<Side_::left>(static_cast<T&&>(x)...);
    }

    template<class... T>
    Value& emplace(Node_pointer const& spot, T&&... x) noexcept
    {
      auto n = ::std::make_unique<Node>(static_cast<T&&>(x)...);
      Tree_algo_::insert(core_, spot, Node_pointer::from_xyz_address(n.get()));
      core_.increment_xyz_size();
      return n.release()->value();
    }

    template<class T>
    Try_insert_result try_insert(T&& x)
    {
      auto& core = core_;
      bool inserted = false;
      Node_pointer const node_ptr = Tree_algo_::try_insert(
        core_,
        [&x](Node const& node) noexcept -> Compare_result_
        {
          return Util_::compare(node.value(), x);
        },
        [&](Node_pointer const& parent, Side_ const side) -> Node_pointer
        {
          inserted = true;
          auto n = ::std::make_unique<Node>(parent, side, static_cast<T&&>(x));
          core.increment_xyz_size();
          return Node_pointer::from_xyz_address(n.release());
        });
      return {node_ptr, inserted};
    }

    void pop_back()
    {
      pop_<Side_::right>();
    }

    void pop_front()
    {
      pop_<Side_::left>();
    }

    bool erase(Value const& x) noexcept
    {
      Node_pointer const node_ptr = Tree_algo_::lower_bound(
        core_,
        [&x](auto const& node) noexcept -> Compare_result_
        {
          return Util_::compare(node.value(), x);
        });

      if(node_ptr)
      {
        Unique_ptr_<Node> const node(Core_::address(node_ptr));
        if(node)
        {
          if(x == node->value())
          {
            Tree_algo_::erase(core_, node_ptr);
            core_.decrement_xyz_size();
            return true;
          }
        }
        else
        {
          CHECK(false);
        }
      }

      return false;
    }

    template<class Fun>
    void clear(Fun&& fun) noexcept
    {
      Tree_algo_::clear(
        core_,
        [&fun](Node_pointer const& node_ptr)
        {
          Unique_ptr_<Node> node(Core_::address(node_ptr));
          static_cast<Fun&&>(fun)(node_ptr);
          node.reset();
        });

      core_.xyz_reset();
    }

    [[nodiscard]] static Node const* address(
      Node_const_pointer const& node_ptr) noexcept
    {
      return Core_::address(node_ptr);
    }

  private:
    using Core_ = Tree_core_<Value>;

    template<Side_ side, class... T>
    Value& emplace_(T&&... x) noexcept
    {
      static_assert(Side_::left == side || Side_::right == side);
      auto n = ::std::make_unique<Node>(static_cast<T&&>(x)...);
      auto const p = Node_pointer::from_xyz_address(n.get());
      if constexpr(Side_::left == side)
      {
        Tree_algo_::push_front(core_, p);
      }
      else if constexpr(Side_::right == side)
      {
        Tree_algo_::push_back(core_, p);
      }

      core_.increment_xyz_size();
      return n.release()->value();
    }

    template<Side_ side>
    void pop_() noexcept
    {
      static_assert(Side_::left == side || Side_::right == side);
      if(empty())
      {
        CHECK(false);
        return;
      }

      Node_pointer node_ptr;
      if constexpr(Side_::left == side)
      {
        node_ptr = Tree_algo_::pop_front(core_);
      }
      else if constexpr(Side_::right == side)
      {
        node_ptr = Tree_algo_::pop_back(core_);
      }

      Unique_ptr_<Node> const node(Core_::address(node_ptr));
      core_.decrement_xyz_size();
      CHECK(node);
    }

    Core_ core_;
  };

  template<class For_each, class Iterator, class End>
  static void expect_match(For_each&& for_each, Iterator it, End const& end)
  {
    for_each(
      [&it, &end](const auto& node)
      {
        CHECK(end != it);
        if(end != it)
        {
          CHECK(*it == node.value());
          ++it;
        }
      });
    CHECK(end == it);
  }

  template<class Values, class Tree>
  static void expect_match(Values const& values, Tree& tree)
  {
    expect_match(
      [&tree](auto&& fun)
      {
        tree.for_each(static_cast<decltype(fun)&&>(fun));
      },
      values.begin(),
      values.end());
    expect_match(
      [&tree](auto&& fun)
      {
        tree.for_each_backward(static_cast<decltype(fun)&&>(fun));
      },
      values.rbegin(),
      values.rend());
  }

  template<class T, class Fun>
  static void gen_7548(Fun&& fun)
  {
    Random_util_::gen_7548<T>(static_cast<Fun&&>(fun));
  }

  template<class Fun>
  static void gen_56972304(Fun&& fun)
  {
    Random_util_::gen_56972304(static_cast<Fun&&>(fun));
  }
};

TEST_CASE_METHOD(
  Simple_tree_core_test,
  "Simple AVL tree core: binary_search, lower_bound, upper_bound",
  "[tree++][treexx][bin][avl][algo][simple]"
  "[binary_search][lower_bound][upper_bound]")
{
  using Value = Int_64;
  using Vector = Vector<Value>;
  using Tree = Tree<Value>;

  Size constexpr count = 7548u;
  Vector vec;
  Tree tree;

  vec.reserve(count);
  gen_7548<Int_32>(
    [&vec, &tree](Int_32 const val_32)
    {
      Value const val =
        static_cast<Value>(val_32) * static_cast<Value>(10);
      vec.emplace_back(val);
      CHECK(tree.try_insert(val).inserted);
    });

  ::std::sort(vec.begin(), vec.end());
  tree.verify();
  expect_match(vec, tree);
  CHECK(count == vec.size());
  CHECK(count == tree.size());

  auto const vec_end = vec.cend();
  auto vec_it = vec.cbegin();
  for(;;)
  {
    auto const& val = *vec_it;
    bool binary_search = true;
    for(Value sub = 0; 8u > sub;)
    {
      Tree::Node_pointer node_ptr;
      if(binary_search)
      {
        node_ptr = tree.binary_search(val - sub);
        binary_search = false;
      }
      else
      {
        node_ptr = tree.lower_bound(val - sub);
        ++sub;
      }

      CHECK(node_ptr);
      if(node_ptr)
      {
        auto const* const node = Tree::address(node_ptr);
        REQUIRE(node);
        CHECK(val == node->value());
      }
    }

    auto const node_ptr = tree.upper_bound(val);
    ++vec_it;
    if(vec_end == vec_it)
    {
      CHECK(!node_ptr);
      break;
    }

    CHECK(node_ptr);
    if(node_ptr)
    {
      auto const* const node = Tree::address(node_ptr);
      REQUIRE(node);
      CHECK(*vec_it == node->value());
    }
  }

  // Insert duplicates.
  for(auto const& val: vec)
  {
    for(auto c = 195u;;)
    {
      tree.emplace(tree.lower_bound(val), val);
      if(1u > --c)
      {
        break;
      }
    }
  }

  tree.verify();
  for(auto const& val: vec)
  {
    auto const node_ptr = tree.lower_bound(val);
    CHECK(node_ptr);
    if(node_ptr)
    {
      auto const* const node = Tree::address(node_ptr);
      REQUIRE(node);
      CHECK(val == node->value());

      auto const prev_node_ptr = tree.previous_node(*node);
      if(prev_node_ptr)
      {
        auto const* const prev_node = Tree::address(prev_node_ptr);
        REQUIRE(prev_node);
        CHECK(prev_node->value() < node->value());
      }
    }
  }

  auto const leftmost_ptr = tree.leftmost();
  auto const rightmost_ptr = tree.rightmost();
  auto const* const leftmost = Tree::address(leftmost_ptr);
  auto const* const rightmost = Tree::address(rightmost_ptr);
  REQUIRE(rightmost_ptr);
  REQUIRE(leftmost_ptr);
  REQUIRE(rightmost);
  REQUIRE(leftmost);

  Value const& min_val = leftmost->value();
  Value const& max_val = rightmost->value();
  auto second_val_node_ptr = tree.next_node(*leftmost);
  for(;;)
  {
    auto const* const second_val_node = Tree::address(second_val_node_ptr);
    REQUIRE(second_val_node_ptr);
    REQUIRE(second_val_node);
    if(second_val_node->value() > min_val)
    {
      break;
    }
    second_val_node_ptr = tree.next_node(*second_val_node);
  }

  CHECK(leftmost_ptr == tree.lower_bound(min_val));
  CHECK(second_val_node_ptr == tree.upper_bound(min_val));
  CHECK(!tree.upper_bound(max_val));
}

TEST_CASE_METHOD(
  Simple_tree_core_test,
  "Simple AVL tree core: push_back, push_front",
  "[tree++][treexx][bin][avl][algo][simple][push_back][push_front]")
{
  using Value = Int_32;

  Deque<Value> deq;
  Tree<Value> tree;
  bool const front = GENERATE(false, true);

  auto const push = [&deq, &tree, front](Value const& x)
  {
    if(front)
    {
      deq.emplace_front(x);
      tree.emplace_front(x);
    }
    else
    {
      deq.emplace_back(x);
      tree.emplace_back(x);
    }

    tree.verify();
    expect_match(deq, tree);
  };

  gen_7548<Value>(
    [&push](Value const& x)
    {
      push(x);
    });
}

TEST_CASE_METHOD(
  Simple_tree_core_test,
  "Simple AVL tree core: insert",
  "[tree++][treexx][bin][avl][algo][simple][insert]")
{
  using Value = Int_32;
  using Tree = Tree<Value>;
  using Set = Set<Value>;

  Set set;
  Tree tree;

  auto const insert = [&set, &tree](auto const& x)
  {
    bool const inserted = set.insert(x).second;
    auto const spot_ptr = tree.lower_bound(x);
    if(spot_ptr)
    {
      auto const* const spot = Tree::address(spot_ptr);
      REQUIRE(spot);
      if(x == spot->value())
      {
        // Already there.
        CHECK(!inserted);
        return;
      }
    }

    tree.emplace(spot_ptr, x);
    tree.verify();
    expect_match(set, tree);
    CHECK(inserted);
  };

  SECTION("Sequence 0")
  {
    insert(767);
    insert(828);
    insert(829);
    insert(888);
    insert(333);
    insert(331);
    insert(329);
    insert(320);
    insert(300);
    insert(200);
    insert(332);
  }

  SECTION("Sequence 1")
  {
    gen_7548<Value>(
      [&insert](Value const & x)
      {
        insert(x);
      });
  }
}

TEST_CASE_METHOD(
  Simple_tree_core_test,
  "Simple AVL tree core: try insert 0",
  "[tree++][treexx][bin][avl][algo][simple][insert][try_insert]")
{
  using Value = Int_32;
  using Tree = Tree<Value>;
  using Vector = Vector<Value>;

  Tree tree;
  Vector vec;
  auto const& ctree = tree;
  Size count = 0u;

  SECTION("Sequence 0")
  {
    vec = {3, 5, 9, 29, 33, 39, 67, 365, 408, 507, 604, 728, 888, 999, 1089};
  }

  SECTION("Sequence 1")
  {
    vec = {1024, 905, 904, 853, 709, 643, 508, 435, 399, 208, 106, 9, 0, -2};
  }

  SECTION("Sequence 2")
  {
    vec = {0, 10, 20, 15, 14, 13};
  }

  SECTION("Sequence 3")
  {
    vec = {0, 10, 20, 15, 12, 14};
  }

  SECTION("Sequence 4")
  {
    vec = {80, 70, 60, 65, 69, 67};
  }

  SECTION("Sequence 5")
  {
    vec = {80, 70, 60, 65, 67, 69};
  }

  SECTION("Sequence 7")
  {
    vec = {
      123, 90, 32, 1234, 1092822, 78, -987, 17, 38, 30872, -32768, 21114,
      820, 8270, 15716, -3800, 555113, 1898, 1904, 1893, 1776, 1147, 1221
    };
  }

  SECTION("Sequence 8")
  {
    vec = {10, 5, 8};
  }

  SECTION("Sequence 9")
  {
    vec = {10, 20, 15};
  }

  SECTION("Sequence 10")
  {
    vec = {20, 10, 30, 5, 35};
  }

  SECTION("Sequence 11")
  {
    gen_7548<Value>(
      [&vec](auto const& x)
      {
        vec.emplace_back(x);
      });
  }

  for(const auto& x: vec)
  {
    auto const ins = tree.try_insert(x);
    auto const& node_ptr = ins.node_pointer;
    auto const* const node = Tree::address(node_ptr);
    ++count;
    ctree.verify();
    CHECK(count == ctree.size());
    CHECK(ins.inserted);
    REQUIRE(node_ptr);
    REQUIRE(node);
    CHECK(x == node->value());

    for(Size i = 0u; count > i; ++i)
    {
      auto const& val = vec[i];
      CHECK(ctree.contains(val));
      CHECK(tree.contains(val));
    }
  }

  ::std::sort(vec.begin(), vec.end());
  expect_match(vec, ctree);
  expect_match(vec, tree);
}

TEST_CASE_METHOD(
  Simple_tree_core_test,
  "Simple AVL tree core: try insert 1",
  "[tree++][treexx][bin][avl][algo][simple][insert][try_insert]")
{
  Vector<Unt_32> val_32_vec;
  Tree<Unt_64> tree;
  Size constexpr val_32_count = 7548u;
  Size constexpr val_64_count = val_32_count * val_32_count;

  gen_7548<Unt_32>(
    [&val_32_vec](auto const& x)
    {
      val_32_vec.emplace_back(x);
    });

  for(const Unt_32 hi_word: val_32_vec)
  {
    for(const Unt_32 lo_word: val_32_vec)
    {
      auto val_64 = static_cast<Unt_64>(hi_word);
      val_64 <<= 32u;
      val_64 |= static_cast<Unt_64>(lo_word);
      auto const ins = tree.try_insert(val_64);
      auto const& node_ptr = ins.node_pointer;
      auto const* const node = Tree<Unt_64>::address(node_ptr);
      CHECK(ins.inserted);
      REQUIRE(node_ptr);
      REQUIRE(node);
      CHECK(val_64 == node->value());
    }
  }

  CHECK(val_32_count == val_32_vec.size());
  CHECK(val_64_count == tree.size());
  tree.verify();
}

TEST_CASE_METHOD(
  Simple_tree_core_test,
  "Simple AVL tree core: pop_back, pop_front",
  "[tree++][treexx][bin][avl][algo][simple][pop_back][pop_front]")
{
  using Value = Int_32;

  Deque<Value> deq;
  Tree<Value> tree;
  bool const front = GENERATE(false, true);

  gen_7548<Value>(
    [&deq, &tree](Value const& x)
    {
      deq.emplace_back(x);
      tree.emplace_back(x);
    });

  auto const pop = [&deq, &tree, front]() -> bool
  {
    if(deq.empty())
    {
      return false;
    }

    if(front)
    {
      deq.pop_front();
      tree.pop_front();
    }
    else
    {
      deq.pop_back();
      tree.pop_back();
    }

    tree.verify();
    expect_match(deq, tree);
    return true;
  };

  tree.verify();
  while(pop())
  {}

  CHECK(deq.empty());
  CHECK(tree.empty());
}

TEST_CASE_METHOD(
  Simple_tree_core_test,
  "Simple AVL tree core: erase all nodes",
  "[tree++][treexx][bin][avl][algo][simple][erase]")
{
  Vector<Int_32> vec;
  Tree<Int_32> tree;
  Uniform_gen<double> gen_0_1(0.0, 1.0);

  auto const verify = [&vec, &tree]
  {
    tree.verify();
    expect_match(vec, tree);
    CHECK(vec.size() == tree.size());
  };

  auto const insert_vec = [&vec, &tree, &verify]
  {
    for(auto const& x: vec)
    {
      CHECK(tree.try_insert(x).inserted);
    }
    ::std::sort(vec.begin(), vec.end());
    verify();
  };

  auto const erase = [&vec, &tree, verify](auto const& x) noexcept
  {
    auto const vec_end = vec.cend();
    auto const vec_it = ::std::lower_bound(vec.cbegin(), vec_end, x);
    bool const exists = vec_end != vec_it && x == *vec_it;
    bool const erased = tree.erase(x);
    if(exists)
    {
      vec.erase(vec_it);
    }

    CHECK(exists);
    CHECK(erased);
    verify();
  };

  auto const erase_at = [&vec, &tree, verify](auto const& idx) noexcept
  {
    REQUIRE(vec.size() > idx);
    auto const vec_it = vec.cbegin() + static_cast<Ptrdiff>(idx);
    bool const erased = tree.erase(*vec_it);
    vec.erase(vec_it);

    CHECK(erased);
    verify();
  };

  auto const erase_at_random = [&vec, &gen_0_1, &erase_at]() -> bool
  {
    auto const vec_size = vec.size();
    if(0u < vec_size)
    {
      auto idx = static_cast<Size>(static_cast<double>(vec_size) * gen_0_1());
      if(vec_size <= idx)
      {
        idx = vec_size;
        --idx;
      }

      erase_at(idx);
      return true;
    }

    return false;
  };

  auto const erase_all_randomly = [&erase_at_random]
  {
    while(erase_at_random())
    {}
  };

  SECTION("Sequence 0")
  {
    vec = {87, 50, 95, 25, 62, 90, 99};
    insert_vec();

    SECTION("Erasure order 0")
    {
      erase(90);
      erase(99);
      erase(95);
      erase(25);
      erase(87);
      erase(50);
      erase(62);
    }

    SECTION("Erasure order 1")
    {
      erase(99);
      erase(95);
      erase(25);
      erase(50);
      erase(87);
      erase(90);
      erase(62);
    }

    SECTION("Erasure order 2")
    {
      erase(87);
      erase(50);
      erase(25);
      erase(62);
      erase(95);
      erase(99);
      erase(90);
    }

    SECTION("Random erasure order")
    {
      erase_all_randomly();
    }
  }

  SECTION("Sequence 1")
  {
    SECTION("Subsequence 0")
    {
      vec = {33, 67};
    }

    SECTION("Subsequence 1")
    {
      vec = {299};
    }

    SECTION("Subsequence 2")
    {
      vec = {0, 10, 20, 15, 12, 14};
    }

    SECTION("Subsequence 3")
    {
      vec = {80, 70, 60, 65, 69, 67};
    }

    SECTION("Subsequence 4")
    {
      vec = {80, 70, 60, 65, 67, 69};
    }

    SECTION("Subsequence 5")
    {
      vec = {
        123, 90, 32, 1234, 1092822, 78, -987, 17, 38, 30872, -32768, 21114,
        820, 8270, 15716, -3800, 555113, 1898, 1904, 1893, 1776, 1147, 1221
      };
    }

    SECTION("Subsequence 6")
    {
      gen_7548<Int_32>(
        [&vec](auto const& x)
        {
          vec.emplace_back(x);
        });
    }

    insert_vec();
    erase_all_randomly();
  }

  CHECK(vec.empty());
  CHECK(0u == tree.size());
}

TEST_CASE_METHOD(
  Simple_tree_core_test,
  "Simple AVL tree core: erase one node",
  "[tree++][treexx][bin][avl][algo][simple][erase]")
{
  Size constexpr count = 7548u;
  Vector<Int_32> vec;
  vec.reserve(count);

  for(Size idx = 0u; count > idx; ++idx)
  {
    vec.clear();
    gen_7548<Int_32>(
      [&vec](auto const& x)
      {
        vec.emplace_back(x);
      });

    Tree<Int_32> tree;
    REQUIRE(count == vec.size());

    for(auto const& x: vec)
    {
      bool const inserted = tree.try_insert(x).inserted;
      CHECK(inserted);
    }

    tree.verify();
    CHECK(vec.size() == tree.size());

    auto const vec_it = vec.cbegin() + static_cast<Ptrdiff>(idx);
    bool const erased = tree.erase(*vec_it);
    vec.erase(vec_it);
    ::std::sort(vec.begin(), vec.end());
    tree.verify();
    expect_match(vec, tree);
    CHECK(vec.size() == tree.size());
    CHECK(erased);
  }
}

TEST_CASE_METHOD(
  Simple_tree_core_test,
  "Simple AVL tree core: clear",
  "[tree++][treexx][bin][avl][algo][simple][clear]")
{
  using Value = Unt_64;
  using Tree = Tree<Value>;

  Size const count(GENERATE(
    1u, 2u, 3u, 4u, 10u, 16u, 32u, 37u, 100u, 119u, 256u, 333u, 334u, 512u,
    1000u, 1024u, 2048u, 2539u, 7548u, 10000u, 0xffffu, 0x10000u, 1000000u,
    0x100000u, 3987379u, 5286432, 0x1000000u, 25901556u, 56972304u));

  Tree tree;
  Set<void const*> nodes;

  gen_56972304(
    [&tree, &nodes, count](Value const& x) -> bool
    {
      auto const ins_res(tree.try_insert(x));
      auto const* const node_addr(Tree::address(ins_res.node_pointer));
      bool const is_new_node = nodes.emplace(node_addr).second;

      CHECK(node_addr);
      CHECK(is_new_node);
      CHECK(ins_res.inserted);
      return tree.size() < count;
    });

  CHECK(count == tree.size());
  tree.verify();

  tree.clear(
    [&nodes](auto const& node_ptr)
    {
      auto const nodes_erased(nodes.erase(Tree::address(node_ptr)));
      CHECK(1u == nodes_erased);
    });

  CHECK(0u == tree.size());
  CHECK(nodes.empty());
  CHECK(tree.empty());
  tree.verify();
}

} // namespace test::treexx::bin::avl
