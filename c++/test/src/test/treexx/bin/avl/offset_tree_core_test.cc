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
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <catch.hpp>

#include <test/macro.hh>
#include <test/treexx/bin/avl/index_tree.hh>
#include <test/treexx/bin/avl/node.hh>
#include <test/treexx/bin/avl/offset_tree.hh>
#include <test/treexx/bin/avl/tree.hh>
#include <test/treexx/bin/avl/util/util.hh>
#include <test/treexx/util/pointer.hh>
#include <test/util/random/uniform_gen.hh>
#include <test/util/random/util.hh>
#include <treexx/compare_result.hh>
#include <treexx/bin/side.hh>
#include <treexx/bin/avl/tree_algo.hh>

namespace test::treexx::bin::avl
{

class Offset_tree_core_test
{
  using Size_ = ::std::size_t;
  using Ptrdiff_ = ::std::ptrdiff_t;
  using Int_32_ = ::std::int32_t;
  using Unt_32_ = ::std::uint32_t;
  using Int_64_ = ::std::int64_t;
  using Unt_64_ = ::std::uint64_t;
  using String_ = ::std::string;
  using Side_ = ::treexx::bin::Side;
  using Compare_result_ = ::treexx::Compare_result;
  using Util_ = ::test::treexx::bin::avl::util::Util;
  using Tree_algo_ = ::treexx::bin::avl::Tree_algo;

  template<class... T>
  using Set_ = ::std::set<T...>;

  template<class... T>
  using Deque_ = ::std::deque<T...>;

  template<class... T>
  using Tuple_ = ::std::tuple<T...>;

  template<class... T>
  using Vector_ = ::std::vector<T...>;

  template<class T>
  using Numeric_limits_ = ::std::numeric_limits<T>;

  template<class... T>
  using Unique_ptr_ = ::std::unique_ptr<T...>;

  template<bool c, class... T>
  using Conditional_ = typename ::std::conditional<c, T...>::type;

  template<class T>
  using Pointer_ = ::test::treexx::util::Pointer<T>;

  template<class T>
  using Uniform_gen_ = ::test::util::random::Uniform_gen<T>;

  template<class Value, class Offset, bool indexed>
  using Node_ = ::test::treexx::bin::avl::Node<
    Value,
    Conditional_<indexed, Size_, void>,
    Offset>;

  template<class Value, class Offset, bool indexed>
  struct Tree_core_base_type_
  {
    using Type = ::test::treexx::bin::avl::Tree<Node_<Value, Offset, indexed>>;
  };

  template<class Value, class Offset>
  struct Tree_core_base_type_<Value, Offset, true>
  {
    using Type = ::test::treexx::bin::avl::Index_tree<
      ::test::treexx::bin::avl::Tree<Node_<Value,  Offset, true>>,
      Size_>;
  };

  template<class Value, class Offset, bool indexed>
  using Tree_core_base_ =
    typename Tree_core_base_type_<Value, Offset, indexed>::Type;

  template<class Value, class Offset, bool indexed>
  using Tree_core_ = ::test::treexx::bin::avl::Offset_tree<
    Tree_core_base_<Value, Offset, indexed>, Offset>;

  template<class T>
  [[nodiscard]] static decltype(auto) comparator(T const& x) noexcept
  {
    return [&x](auto const& y) noexcept -> Compare_result_
    {
      return Util_::compare(y, x);
    };
  }

  template<class Value, class Offset, bool indexed>
  struct Tree_
  {
    using Node = Node_<Value, Offset, indexed>;
    using Node_const_pointer = Pointer_<Node const>;
    using Node_pointer = Pointer_<Node>;

    [[nodiscard]] decltype(auto) size() const noexcept
    {
      return core_.xyz_size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
      return 1u > size();
    }

    [[nodiscard]] Node_const_pointer binary_search(
      Offset const& offset) const noexcept
    {
      return Tree_algo_::binary_search<false, false, true>(
        core_, comparator(offset));
    }

    [[nodiscard]] Node_const_pointer lower_bound(
      Offset const& offset) const noexcept
    {
      auto const node_ptr_0 = lower_bound_<false>(offset);
      auto const node_ptr_1 = lower_bound_<true>(offset);
      CHECK(node_ptr_0 == node_ptr_1);
      return node_ptr_0;
    }

    [[nodiscard]] Node_const_pointer upper_bound(
      Offset const& offset) const noexcept
    {
      return Tree_algo_::upper_bound<false, false, true>(
        core_, comparator(offset));
    }

    void verify() const
    {
      Util_::verify_tree<indexed, true>(core_);
    }

    template<class Entries>
    void expect_match(Entries const& entries) const
    {
      CHECK(entries.size() == size());
      auto node_ptr = core_.template extreme<Side_::left>();
      for(auto const& e: entries)
      {
        if(node_ptr)
        {
          Node const* const node = Core_::address(node_ptr);
          if(node)
          {
            auto const found_node_ptr = binary_search(e.offset);
            CHECK(node_ptr == found_node_ptr);
            CHECK(e.value == node->value());
            CHECK(e.offset == Tree_algo_::node_offset(core_, *node));
            node_ptr = Tree_algo_::next_node(core_, *node);
          }
          else
          {
            CHECK(false);
            break;
          }
        }
        else
        {
          CHECK(false);
          break;
        }
      }
    }

    template<Side_ side>
    [[nodiscard]] Node_pointer extreme() noexcept
    {
      return core_.template extreme<side>();
    }

    template<Side_ side>
    [[nodiscard]] Node_const_pointer extreme() const noexcept
    {
      return core_.template extreme<side>();
    }

    [[nodiscard]] Node_pointer extreme(Side_ const side)
    {
      REQUIRE((Side_::left == side || Side_::right == side));
      return Side_::left == side ?
        core_.template extreme<Side_::left>() :
        core_.template extreme<Side_::right>();
    }

    [[nodiscard]] Node_pointer next_node(Node& n) noexcept
    {
      return Tree_algo_::next_node(core_, n);
    }

    [[nodiscard]] Node_const_pointer next_node(Node const& n) const noexcept
    {
      return Tree_algo_::next_node(core_, n);
    }

    [[nodiscard]] Node_pointer at(Size_ const& idx)
    {
      if(size() <= idx)
      {
        CHECK(false);
        return nullptr;
      }

      if constexpr(indexed)
      {
        return Tree_algo_::at_index(core_, idx);
      }
      else
      {
        Node_pointer node_ptr = core_.template extreme<Side_::left>();
        for(Size_ i = 0u;; ++i)
        {
          if(node_ptr)
          {
            if(idx == i)
            {
              return node_ptr;
            }

            Node* const node = Core_::address(node_ptr);
            if(node)
            {
              node_ptr = Tree_algo_::next_node(core_, *node);
            }
            else
            {
              CHECK(false);
              break;
            }
          }
          else
          {
            CHECK(false);
            break;
          }
        }

        return nullptr;
      }
    }

    void shift_suffix(
      Node& node,
      Offset const& shift,
      Side_ const side) noexcept
    {
      Tree_algo_::shift_suffix(core_, node, shift, side);
    }

    template<class... Val>
    Value& emplace_back(Offset const& offset, Val&&... val)
    {
      auto n = ::std::make_unique<Node>(static_cast<Val&&>(val)...);
      Tree_algo_::push_back(
        core_, Node_pointer::from_xyz_address(n.get()), offset);
      core_.increment_xyz_size();
      return n.release()->value();
    }

    template<class... Val>
    Value& emplace(Offset const& offset, Val&&... val)
    {
      return emplace_(offset, Tuple_<>(), static_cast<Val&&>(val)...);
    }

    template<class... Val>
    Value& emplace_and_shift(
      Offset const& offset, Offset const& shift, Val&&... val)
    {
      return emplace_(
        offset,
        ::std::forward_as_tuple(shift),
        static_cast<Val&&>(val)...);
    }

    void pop(Side_ const side)
    {
      REQUIRE((Side_::left == side || Side_::right == side));
      if(empty())
      {
        CHECK(false);
        return;
      }

      Node_pointer const node_ptr = Side_::left == side ?
        Tree_algo_::pop_front(core_) :
        Tree_algo_::pop_back(core_);
      if(node_ptr)
      {
        Unique_ptr_<Node> const node(Core_::address(node_ptr));
        CHECK(node);
      }
      else
      {
        CHECK(false);
      }

      core_.decrement_xyz_size();
    }

    void erase(Node_pointer const& node_ptr)
    {
      REQUIRE(node_ptr);
      if(empty())
      {
        CHECK(false);
        return;
      }

      Tree_algo_::erase(core_, node_ptr);
      Unique_ptr_<Node> const node(Core_::address(node_ptr));
      core_.decrement_xyz_size();
      CHECK(node);
    }

    [[nodiscard]] static Node* address(
      Node_pointer const& node_ptr) noexcept
    {
      return Core_::address(node_ptr);
    }

    [[nodiscard]] static Node const* address(
      Node_const_pointer const& node_ptr) noexcept
    {
      return Core_::address(node_ptr);
    }

  private:
    using Core_ = Tree_core_<Value, Offset, indexed>;

    template<bool unique>
    [[nodiscard]] Node_const_pointer lower_bound_(
      Offset const& offset) const noexcept
    {
      return Tree_algo_::lower_bound<false, false, true, unique>(
        core_, comparator(offset));
    }

    template<class... Val, class Shift_tpl>
    Value& emplace_(
      Offset const& offset,
      Shift_tpl const& shift_tpl,
      Val&&... val)
    {
      return ::std::apply(
        [&](auto const&... shift) -> Value&
        {
          auto n = ::std::make_unique<Node>(static_cast<Val&&>(val)...);
          Tree_algo_::insert_at_offset(
            core_, Node_pointer::from_xyz_address(n.get()), offset, shift...);
          core_.increment_xyz_size();
          return n.release()->value();
        },
        shift_tpl);
    }

    Core_ core_;
  };

  template<class Value, class Offset>
  struct Node_data_
  {
    template<class... Val_args>
    explicit Node_data_(Offset const& off, Val_args&&... val_args) :
      offset(off),
      value(static_cast<Val_args&&>(val_args)...)
    {}

    [[nodiscard]] friend bool operator <(
      Node_data_ const& x,
      Node_data_ const& y) noexcept
    {
      return x.offset < y.offset;
    }

    Offset offset;
    Value value;
  };

  template<class Nodes, class Iterator, class Shift>
  static void shift_suffix_(
    Nodes& nodes,
    Iterator it,
    Shift const& shift,
    Side_ const side) noexcept
  {
    auto const end = nodes.end();
    bool first = true;
    while(end != it)
    {
      if(Side_::left == side)
      {
        auto& offset = it->offset;
        offset -= shift;
        if(first)
        {
          if(nodes.begin() != it)
          {
            auto prev_it = it;
            --prev_it;
            REQUIRE(offset > prev_it->offset);
          }
        }
        else
        {
          first = false;
        }
      }
      else
      {
        it->offset += shift;
      }

      ++it;
    }
  }

  template<class T, class Fun>
  static void gen_7548(Fun&& fun)
  {
    ::test::util::random::Util::gen_7548<T>(static_cast<Fun&&>(fun));
  }

  template<class Test_case>
  struct Test_case_base_
  {
    static void register_test_cases()
    {
      register_test_case_<false>();
      register_test_case_<true>();
    }

  private:
    template<bool indexed>
    static void register_test_case_()
    {
      String_ name(Test_case::name());
      name.append(" (");
      if constexpr(!indexed)
      {
        name.append("not ");
      }
      name.append("indexed)");
      REGISTER_TEST_CASE(
        Test_case::template run<indexed>,
        static_cast<String_&&>(name), Test_case::tags());
    }
  };

  struct Test_case_insert_ : Test_case_base_<Test_case_insert_>
  {
    TXX_TEST_CLASS()

    static char const* name() noexcept
    {
      return "Offset AVL tree core: insert";
    }

    static char const* tags() noexcept
    {
      return "[tree++][treexx][bin][avl][algo][offset][insert]";
    }

    template<bool>
    static void run();
  };

  struct Test_case_insert_back_ : Test_case_base_<Test_case_insert_back_>
  {
    TXX_TEST_CLASS()

    static char const* name() noexcept
    {
      return "Offset AVL tree core: push_back";
    }

    static char const* tags() noexcept
    {
      return "[tree++][treexx][bin][avl][algo][offset][insert][push_back]";
    }

    template<bool>
    static void run();
  };

  struct Test_case_0_ : Test_case_base_<Test_case_0_>
  {
    TXX_TEST_CLASS()

    static char const* name() noexcept
    {
      return "Offset AVL tree core: erase, shift, binary search";
    }

    static char const* tags() noexcept
    {
      return
        "[tree++][treexx][bin][avl][algo][offset]"
        "[erase][pop_back][pop_front][shift_suffix]"
        "[binary_search][lower_bound][upper_bound]";
    }

    template<bool>
    static void run();
  };
};

template<bool indexed>
void Offset_tree_core_test::Test_case_insert_::run()
{
  using Size = Size_;
  using Int_32 = Int_32_;
  using Unt_32 = Unt_32_;
  using Int_64 = Int_64_;
  using Unt_64 = Unt_64_;
  using Value = Int_32;
  using Offset = Int_64;
  using Tree = Tree_<Value, Offset, indexed>;
  using Node_data = Node_data_<Value, Offset>;

  Tree tree;
  Uniform_gen_<Value> gen_val(-9187, 716211);

  SECTION("Normal size")
  {
    Deque_<Node_data> deq;
    auto const emplace = [&deq, &tree](
      Offset const& offset,
      Offset const& shift,
      auto const&... val)
    {
      auto const vec_end = deq.end();
      auto const vec_it = ::std::lower_bound(
        deq.begin(), vec_end, offset,
        [](Node_data const& node, Offset const& off) noexcept
        {
          return node.offset < off;
        });

      if(0 < shift)
      {
        for(auto it = vec_it; vec_end != it; ++it)
        {
          it->offset += shift;
        }
      }
      else
      {
        if(vec_end != vec_it)
        {
          REQUIRE(offset < vec_it->offset);
        }
      }

      deq.emplace(vec_it, offset, val...);
      if(0 < shift)
      {
        tree.emplace_and_shift(offset, shift, val...);
      }
      else
      {
        tree.emplace(offset, val...);
      }

      tree.expect_match(deq);
      tree.verify();
    };

    Offset const shift = GENERATE(0, 176);
    gen_7548<Offset>(
      [&emplace, &gen_val, &shift](Offset const& offset)
      {
        emplace(offset, shift, gen_val());
      });

    Size count = 5785u;
    for(auto c = count; 0u < c; --c)
    {
      if(deq.empty())
      {
        CHECK(false);
        break;
      }

      Offset front_offset = deq.front().offset;
      Offset const back_offset = deq.back().offset + 10;
      if(1 > shift)
      {
        front_offset -= 71;
      }

      emplace(front_offset, shift, gen_val());
      emplace(back_offset, shift, gen_val());
    }
  }

  SECTION("Big size")
  {
    Size constexpr count = 7548u * 7548u;
    Set_<Node_data> set;

    gen_7548<Unt_32>(
      [&](Unt_32 const hi_word)
      {
        gen_7548<Unt_32>(
          [&, hi_word](Unt_32 const lo_word)
          {
            auto offset_u64 = static_cast<Unt_64>(hi_word);
            offset_u64 <<= 32u;
            offset_u64 |= static_cast<Unt_64>(lo_word);
            REQUIRE(
              static_cast<Unt_64>(Numeric_limits_<Offset>::max()) > offset_u64);
            auto const offset = static_cast<Offset>(offset_u64);
            auto const val = gen_val();
            CHECK(set.emplace(offset, val).second);
            tree.emplace(offset, val);
          });
      });

    CHECK(count == tree.size());
    CHECK(count == set.size());
    tree.expect_match(set);
    tree.verify();
  }
}

template<bool indexed>
void Offset_tree_core_test::Test_case_insert_back_::run()
{
  using Value = Int_32_;
  using Offset = Int_64_;
  using Tree = Tree_<Value, Offset, indexed>;
  using Vector = Vector_<Node_data_<Value, Offset>>;

  Tree tree;
  Vector vec;
  Size_ constexpr count = 10000u;
  Uniform_gen_<Value> gen_val(-9187, 716211);
  Uniform_gen_<Offset> gen_rel_offset(1, 36512322);
  bool const with_insert = GENERATE(false, true);
  auto const insert_back = [&tree, &vec, &gen_rel_offset, &gen_val, with_insert]
  {
    Offset offset = -1762;
    Offset rel_offset = offset;
    if(!vec.empty())
    {
      rel_offset = gen_rel_offset();
      offset = vec.back().offset + rel_offset;
    }

    Value const val = gen_val();
    CHECK(val == vec.emplace_back(offset, val).value);
    if(with_insert)
    {
      CHECK(val == tree.emplace(offset, val));
    }
    else
    {
      CHECK(val == tree.emplace_back(rel_offset, val));
    }

    tree.expect_match(vec);
    tree.verify();
  };

  for(auto c = count; 0u < c; --c)
  {
    insert_back();
  }

  CHECK(count == tree.size());
  CHECK(count == vec.size());
}

template<bool indexed>
void Offset_tree_core_test::Test_case_0_::run()
{
  using Side = Side_;
  using Size = Size_;
  using Value = Int_32_;
  using Offset = Int_64_;
  using Tree = Tree_<Value, Offset, indexed>;
  using Deque = Deque_<Node_data_<Value, Offset>>;

  Tree tree;
  Deque deq;
  Size_ constexpr count = 10000u;
  Uniform_gen_<Value> gen_val(-675411, 28716111);
  Uniform_gen_<Offset> gen_rel_offset(1, 1876229);
  Uniform_gen_<double> gen_0_1(0.0, 1.0);
  auto const gen_index = [&deq, &gen_0_1]() -> Size
  {
    Size const sz = deq.size();
    auto idx = static_cast<Size>(static_cast<double>(sz) * gen_0_1());
    if(sz <= idx)
    {
      REQUIRE(0u < sz);
      idx = sz;
      --idx;
    }

    return idx;
  };

  auto const verify = [&tree, &deq]
  {
    tree.verify();
    tree.expect_match(deq);
  };

  for(auto c = count; 0u < c; --c)
  {
    Offset offset = -81765;
    Offset rel_offset = offset;
    if(!deq.empty())
    {
      rel_offset = gen_rel_offset();
      offset = deq.back().offset + rel_offset;
    }

    Value const val = gen_val();
    CHECK(val == deq.emplace_back(offset, val).value);
    CHECK(val == tree.emplace_back(rel_offset, val));
  }

  CHECK(count == tree.size());
  CHECK(count == deq.size());

  SECTION("Erase")
  {
    SECTION("Erase extreme")
    {
      Side const side = GENERATE(Side::left, Side::right);
      bool const with_erase = GENERATE(false, true);

      while(!deq.empty())
      {
        switch(side)
        {
        case Side::left:
          deq.pop_front();
          break;
        case Side::right:
          deq.pop_back();
          break;
        default:
          REQUIRE(false);
          break;
        }

        if(with_erase)
        {
          tree.erase(tree.extreme(side));
        }
        else
        {
          tree.pop(side);
        }

        verify();
      }
    }

    SECTION("Erase random node")
    {
      while(!deq.empty())
      {
        auto idx = gen_index();
        REQUIRE(deq.size() > idx);
        auto const deq_it = deq.cbegin() + static_cast<Ptrdiff_>(idx);
        auto const node_ptr = tree.at(idx);
        REQUIRE(node_ptr);
        deq.erase(deq_it);
        tree.erase(node_ptr);
        verify();
      }
    }

    CHECK(tree.empty());
    CHECK(deq.empty());
  }

  SECTION("Shift suffix")
  {
    auto const sides = {Side::right, Side::left};
    for(Side const side: sides)
    {
      auto deq_it = deq.begin();
      auto const dec_end = deq.end();
      auto node_ptr = tree.template extreme<Side::left>();
      Offset const shift = Side::left == side ? 7 : 10;
      while(dec_end != deq_it)
      {
        if(node_ptr)
        {
          auto* const node = Tree::address(node_ptr);
          if(node)
          {
            shift_suffix_(deq, deq_it, shift, side);
            tree.shift_suffix(*node, shift, side);
            verify();
            node_ptr = tree.next_node(*node);
            ++deq_it;
          }
          else
          {
            CHECK(false);
            break;
          }
        }
        else
        {
          CHECK(false);
          break;
        }
      }
    }
  }

  SECTION("Binary search")
  {
    auto const& ctree = tree;
    Size node_count = 0u;
    auto node_ptr = ctree.template extreme<Side::left>();
    auto const deq_begin = deq.cbegin();
    auto const deq_end = deq.cend();
    auto deq_it = deq_begin;
    while(node_ptr)
    {
      auto const* const node = Tree::address(node_ptr);
      if(!node)
      {
        CHECK(false);
        break;
      }
      if(deq_end == deq_it)
      {
        CHECK(false);
        break;
      }

      Offset const& offset = deq_it->offset;
      auto found_node_ptr = ctree.binary_search(offset);
      CHECK(found_node_ptr == node_ptr);

      found_node_ptr = ctree.lower_bound(offset);
      CHECK(found_node_ptr == node_ptr);

      Offset less_offset = offset;
      bool search_less = true;
      --less_offset;
      if(deq_begin != deq_it)
      {
        auto deq_prev_it = deq_it;
        --deq_prev_it;
        if(less_offset == deq_prev_it->offset)
        {
          search_less = false;
        }
      }
      if(search_less)
      {
        found_node_ptr = ctree.lower_bound(less_offset);
        CHECK(found_node_ptr == node_ptr);
        found_node_ptr = ctree.upper_bound(less_offset);
        CHECK(found_node_ptr == node_ptr);
      }

      auto const next_node_ptr = ctree.next_node(*node);
      found_node_ptr = ctree.lower_bound(offset + static_cast<Offset>(1));
      CHECK(found_node_ptr == next_node_ptr);
      found_node_ptr = ctree.upper_bound(offset);
      CHECK(found_node_ptr == next_node_ptr);

      node_ptr = next_node_ptr;
      ++node_count;
      ++deq_it;
    }

    CHECK(count == node_count);
    CHECK(count == tree.size());
  }
}

} // test::treexx::bin::avl
