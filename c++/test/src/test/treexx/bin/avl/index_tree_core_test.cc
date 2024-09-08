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
#include <deque>
#include <memory>
#include <set>
#include <type_traits>
#include <vector>

#include <catch.hpp>

#include <test/treexx/bin/avl/index_tree.hh>
#include <test/treexx/bin/avl/node.hh>
#include <test/treexx/bin/avl/tree.hh>
#include <test/treexx/bin/avl/util/util.hh>
#include <test/treexx/util/pointer.hh>
#include <test/util/random/uniform_gen.hh>
#include <test/util/random/util.hh>
#include <treexx/compare_result.hh>
#include <treexx/bin/avl/tree_algo.hh>

namespace test::treexx::bin::avl
{

class Index_tree_core_test
{
  using Side_ = ::treexx::bin::Side;
  using Compare_result_ = ::treexx::Compare_result;
  using Tree_algo_ = ::treexx::bin::avl::Tree_algo;
  using Util_ = ::test::treexx::bin::avl::util::Util;
  using Random_util_ = ::test::util::random::Util;

  template<class T>
  using Decay_ = typename ::std::decay<T>::type;

  template<class... T>
  using Unique_ptr_ = ::std::unique_ptr<T...>;

  template<class T>
  using Pointer_ = ::test::treexx::util::Pointer<T>;

  template<class Value, class Index>
  using Node_ = ::test::treexx::bin::avl::Node<Value, Index>;

  template<class Value, class Index>
  using Tree_core_base_ = ::test::treexx::bin::avl::Tree<Node_<Value, Index>>;

  template<class Value, class Index>
  using Tree_core_ = ::test::treexx::bin::avl::Index_tree<
    Tree_core_base_<Value, Index>, Index>;

protected:
  using Size = ::std::size_t;
  using Ptrdiff = ::std::ptrdiff_t;
  using Int_32 = ::std::int32_t;
  using Unt_64 = ::std::uint64_t;

  template<class... T>
  using Set = ::std::set<T...>;

  template<class... T>
  using Deque = ::std::deque<T...>;

  template<class... T>
  using Vector = ::std::vector<T...>;

  template<class T>
  using Uniform_gen = ::test::util::random::Uniform_gen<T>;

  template<class Value, class I = Size>
  struct Tree
  {
    using Index = I;
    using Node = Node_<Value, Index>;
    using Node_pointer = Pointer_<Node>;
    using Node_const_pointer = Pointer_<Node const>;

    [[nodiscard]] bool empty() const noexcept
    {
      return core_.xyz_empty();
    }

    [[nodiscard]] decltype(auto) size() const noexcept
    {
      return core_.xyz_size();
    }

    [[nodiscard]] Node_pointer at(Index const& idx) noexcept
    {
      return Tree_algo_::at_index(core_, idx);
    }

    [[nodiscard]] Node_const_pointer at(Index const& idx) const noexcept
    {
      return Tree_algo_::at_index(core_, idx);
    }

    [[nodiscard]] Value const& back() const noexcept
    {
      return extreme_<Side_::right>();
    }

    [[nodiscard]] Value const& front() const noexcept
    {
      return extreme_<Side_::left>();
    }

    void verify() const
    {
      Util_::verify_tree<true, false>(core_);
    }

    template<class... T>
    Value& emplace_back(T&&... x)
    {
      return emplace_<Side_::right>(static_cast<T&&>(x)...);
    }

    template<class... T>
    Value& emplace_front(T&&... x)
    {
      return emplace_<Side_::left>(static_cast<T&&>(x)...);
    }

    template<class... T>
    Value& emplace(Node_pointer const& spot, T&&... x)
    {
      auto n = ::std::make_unique<Node>(static_cast<T&&>(x)...);
      Tree_algo_::insert(core_, spot, Node_pointer::from_xyz_address(n.get()));
      core_.increment_xyz_size();
      return n.release()->value();
    }

    template<class... T>
    Value& emplace_at(Index const& idx, T&&... x)
    {
      auto n = ::std::make_unique<Node>(static_cast<T&&>(x)...);
      Tree_algo_::insert_at_index(
        core_, Node_pointer::from_xyz_address(n.get()), idx);
      core_.increment_xyz_size();
      return n.release()->value();
    }

    void pop_back()
    {
      pop_<Side_::right>();
    }

    void pop_front()
    {
      pop_<Side_::left>();
    }

    void erase_at(Index const& idx)
    {
      Node_pointer const node_ptr = Tree_algo_::at_index(core_, idx);
      if(node_ptr)
      {
        Tree_algo_::erase(core_, node_ptr);
        Unique_ptr_<Node> const node(Core_::address(node_ptr));
        core_.decrement_xyz_size();
        CHECK(node);
      }
      else
      {
        CHECK(false);
      }
    }

    template<class T>
    bool try_insert(T&& x)
    {
      auto& core = core_;
      bool inserted = false;
      Tree_algo_::try_insert(
        core_,
        [&x](Node const& node) noexcept -> Compare_result_
        {
          return Util_::compare(node.value(), x);
        },
        [&](Node_pointer const& parent, Side_ const side) -> Node_pointer
        {
          auto n = ::std::make_unique<Node>(parent, side, static_cast<T&&>(x));
          core.increment_xyz_size();
          inserted = true;
          return Node_pointer::from_xyz_address(n.release());
        });
      return inserted;
    }

    void swap(Index const& i, Index const& j)
    {
      auto const size = this->size();
      if(size > i)
      {
        if(size > j)
        {
          auto const node_i_ptr = Tree_algo_::at_index(core_, i);
          auto const node_j_ptr = Tree_algo_::at_index(core_, j);
          if(node_i_ptr)
          {
            if(node_j_ptr)
            {
              Tree_algo_::swap(core_, node_i_ptr, node_j_ptr);
            }
            else
            {
              CHECK(false);
            }
          }
          else
          {
            CHECK(false);
          }
        }
        else
        {
          CHECK(false);
        }
      }
      else
      {
        CHECK(false);
      }
    }

    [[nodiscard]] static Node const* address(
      Node_const_pointer const& node_ptr) noexcept
    {
      return Core_::address(node_ptr);
    }

  private:
    using Core_ = Tree_core_<Value, Index>;

    template<Side_ side>
    [[nodiscard]] Value const& extreme_() const noexcept
    {
      auto const ptr(core_.template extreme<side>());
      REQUIRE(ptr);
      auto const* const addr(ptr.xyz_address());
      REQUIRE(addr);
      return addr->value();
    }

    template<Side_ side, class... T>
    Value& emplace_(T&&... x)
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
    void pop_()
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

  template<class Values, class Tree>
  static void expect_match(Values const& values, Tree& tree)
  {
    using Index = typename Decay_<Tree>::Index;

    auto const size = values.size();
    CHECK(size == tree.size());
    Index idx = 0u;
    for(auto const& x: values)
    {
      auto const node_ptr = tree.at(idx++);
      CHECK(node_ptr);
      if(node_ptr)
      {
        auto const* const node = tree.address(node_ptr);
        REQUIRE(node);
        CHECK(x == node->value());
      }
    }

    if(0u < size)
    {
      CHECK(*values.crbegin() == tree.back());
      CHECK(*values.cbegin() == tree.front());
    }
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
  Index_tree_core_test,
  "Index AVL tree core: push_back, push_front",
  "[tree++][treexx][bin][avl][algo][index][push_back][push_front]")
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
  Index_tree_core_test,
  "Index AVL tree core: insert",
  "[tree++][treexx][bin][avl][algo][index][insert][insert_at_index]")
{
  using Value = Int_32;
  using Tree = Tree<Value>;
  using Index = Tree::Index;
  using Vector = Vector<Value>;

  Tree tree;
  Vector vec;
  auto const& ctree = tree;
  bool const by_index = GENERATE(false, true);

  auto const insert_at = [&, by_index](Index const& idx, auto const& x)
  {
    auto tree_size = tree.size();
    REQUIRE(tree_size == vec.size());
    REQUIRE(tree_size >= idx);

    vec.emplace(vec.cbegin() + static_cast<Ptrdiff>(idx), x);
    if(by_index)
    {
      CHECK(x == tree.emplace_at(idx, x));
    }
    else
    {
      Tree::Node_pointer spot = nullptr;
      if(idx < tree_size)
      {
        spot = tree.at(idx);
        REQUIRE(spot);
      }

      CHECK(x == tree.emplace(spot, x));
    }

    ++tree_size;
    ctree.verify();
    CHECK(tree_size == tree.size());
    expect_match(vec, ctree);
    expect_match(vec, tree);
  };

  SECTION("Sequence 0")
  {
    insert_at(0u, 146);
    insert_at(1u, 215);
    insert_at(0u, 318);
    insert_at(0u, 156);
    insert_at(0u, 233);
    insert_at(0u, 919);
    insert_at(5u, 178);
    insert_at(5u, 424);
    insert_at(5u, 333);
  }

  SECTION("Sequence 1")
  {
    Uniform_gen<double> gen_0_1(0.0, 1.0);
    gen_7548<Value>(
      [&](auto const& x)
      {
        auto const idx =
          static_cast<Index>(static_cast<double>(tree.size()) * gen_0_1());
        insert_at(idx, x);
      });
  }
}

TEST_CASE_METHOD(
  Index_tree_core_test,
  "Index AVL tree core: try insert",
  "[tree++][treexx][bin][avl][algo][index][insert][try_insert]")
{
  using Value = Int_32;
  using Tree = Tree<Value>;
  using Set = Set<Value>;

  Set set;
  Tree tree;

  auto const insert = [&set, &tree](auto const& val)
  {
    bool const set_inserted = set.insert(val).second;
    bool const tree_inserted = tree.try_insert(val);
    tree.verify();
    expect_match(set, tree);
    CHECK(set_inserted == tree_inserted);
  };

  SECTION("Sequence 0")
  {
    insert(27);
    insert(32);
    insert(-5);
    insert(87);
    insert(18);
    insert(71);
    insert(45);
    insert(32);
  }

  SECTION("Sequence 0")
  {
    gen_7548<Value>(
      [&insert](Value const& val)
      {
        insert(val);
      });
  }
}

TEST_CASE_METHOD(
  Index_tree_core_test,
  "Index AVL tree core: erase all nodes",
  "[tree++][treexx][bin][avl][algo][index][erase][pop_back][pop_front]")
{
  using Value = Int_32;
  using Deque = Deque<Value>;
  using Tree = Tree<Value>;
  using Index = Tree::Index;

  enum class Action
  {
    pop_back,
    pop_front,
    erase_back,
    erase_front,
    erase_random,
  };

  Deque deq;
  Tree tree;
  Uniform_gen<double> gen_0_1(0.0, 1.0);
  Action const action = GENERATE(
    Action::pop_back,
    Action::pop_front,
    Action::erase_back,
    Action::erase_front,
    Action::erase_random);

  auto const verify = [&deq, &tree]
  {
    tree.verify();
    expect_match(deq, tree);
    CHECK(deq.size() == tree.size());
  };

  auto const insert_vec = [&deq, &tree, &verify]
  {
    Index idx = 0u;
    for(auto const& x: deq)
    {
      tree.emplace_at(idx++, x);
    }
    verify();
  };

  auto const erase_at = [&deq, &tree, &verify](Index const& idx)
  {
    REQUIRE(deq.size() > idx);
    auto const vec_it = deq.cbegin() + static_cast<Ptrdiff>(idx);
    deq.erase(vec_it);
    tree.erase_at(idx);
    verify();
  };

  auto const pop_back = [&deq, &tree, &verify]() -> bool
  {
    if(deq.empty())
    {
      return false;
    }

    deq.pop_back();
    tree.pop_back();
    verify();
    return true;
  };

  auto const pop_front = [&deq, &tree, &verify]() -> bool
  {
    if(deq.empty())
    {
      return false;
    }

    deq.pop_front();
    tree.pop_front();
    verify();
    return true;
  };

  auto const erase_back = [&deq, &erase_at]() -> bool
  {
    if(deq.empty())
    {
      return false;
    }

    auto idx = static_cast<Index>(deq.size());
    --idx;
    erase_at(idx);
    return true;
  };

  auto const erase_front = [&deq, &erase_at]() -> bool
  {
    if(deq.empty())
    {
      return false;
    }

    erase_at(0u);
    return true;
  };

  auto const erase_random = [&deq, &gen_0_1, &erase_at]() -> bool
  {
    if(deq.empty())
    {
      return false;
    }

    auto const vec_size = deq.size();
    auto idx = static_cast<Index>(static_cast<double>(vec_size) * gen_0_1());
    if(vec_size <= idx)
    {
      idx = static_cast<Index>(vec_size);
      --idx;
    }

    erase_at(idx);
    return true;
  };

  SECTION("Sequence 0")
  {
    deq = {83, 45, 12, 34, 56, 345, 67, 198, 227};
  }

  SECTION("Subsequence 1")
  {
    deq = {299};
  }

  SECTION("Subsequence 2")
  {
    deq = {0, 10, 20, 15, 12, 14};
  }

  SECTION("Subsequence 3")
  {
    deq = {80, 70, 60, 65, 69, 67};
  }

  SECTION("Subsequence 4")
  {
    deq = {80, 70, 60, 65, 67, 69};
  }

  SECTION("Subsequence 5")
  {
    deq = {
      123, 90, 32, 1234, 1092822, 78, -987, 17, 38, 30872, -32768, 21114,
      820, 8270, 15716, -3800, 555113, 1898, 1904, 1893, 1776, 1147, 1221
    };
  }

  SECTION("Subsequence 6")
  {
    gen_7548<Value>(
      [&deq](auto const& x)
      {
        deq.emplace_back(x);
      });
  }

  insert_vec();

  for(;;)
  {
    bool erased = false;
    switch(action)
    {
    case Action::pop_back:
      erased = pop_back();
      break;
    case Action::pop_front:
      erased = pop_front();
      break;
    case Action::erase_back:
      erased = erase_back();
      break;
    case Action::erase_front:
      erased = erase_front();
      break;
    case Action::erase_random:
      erased = erase_random();
      break;
    default:
      REQUIRE(false);
      break;
    }

    if(!erased)
    {
      break;
    }
  }

  CHECK(deq.empty());
  CHECK(tree.empty());
}

TEST_CASE_METHOD(
  Index_tree_core_test,
  "Index AVL tree core: swap nodes",
  "[tree++][treexx][bin][avl][algo][index][swap]")
{
  using Value = Unt_64;
  using Tree = Tree<Value>;
  using Vector = Vector<Value>;

  Tree tree;
  Vector vec;
  Size const size = GENERATE(
    2000005u,
    4u, 7u, 18u, 32u, 58u, 128u, 138u, 177u, 201u, 345u, 380u, 401u, 408u);
  auto const verify = [&vec, &tree]
  {
    expect_match(vec, tree);
    tree.verify();
  };
  auto const swap = [&vec, &tree, &verify](auto const& i, auto const& j)
  {
    if(i != j)
    {
      ::std::swap(vec[i], vec[j]);
    }
    tree.swap(i, j);
    verify();
  };

  vec.reserve(size);
  gen_56972304(
    [&vec, &tree, size](Value const& val) -> bool
    {
      vec.emplace_back(val);
      tree.emplace_back(val);
      return vec.size() < size;
    });

  REQUIRE(size == vec.size());
  REQUIRE(size == tree.size());
  verify();

  if(2u < size)
  {
    swap(1u, 2u);
  }

  if(0x200u > size)
  {
    for(Size i = 0u; size > i; ++i)
    {
      for(Size j = 0u; size > j; ++j)
      {
        swap(i, j);
      }
    }
  }
  else
  {
    swap(0u, 0u);
    swap(0u, size - 1u);
    swap(0u, size - 37u);
    swap(0u, size / 2u);
    swap(0u, size / 15u);
    swap(5u, size / 3u);
    swap(5u, size * 2u / 3u);
  }
}

} // namespace test::treexx::bin::avl
