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

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <catch.hpp>

#include <test/treexx/util/util.hh>
#include <test/util/time_probe.hh>
#include <test/util/random/util.hh>
#include <treexx/compare_result.hh>
#include <treexx/bin/side.hh>
#include <treexx/bin/avl/balance.hh>
#include <treexx/bin/avl/tree_algo.hh>

namespace test::treexx::bin::avl
{

class Benchmark
{
  using Nullptr_ = ::std::nullptr_t;
  using Side_ = ::treexx::bin::Side;
  using Util_ = ::test::treexx::util::Util;
  using Balance_ = ::treexx::bin::avl::Balance;
  using Random_util_ = ::test::util::random::Util;
  using Compare_result_ = ::treexx::Compare_result;
  using Tree_algo_ = ::treexx::bin::avl::Tree_algo;

  template<class T>
  using Numeric_limits_ = ::std::numeric_limits<T>;

  template<class... T>
  using Vector_ = ::std::vector<T...>;

  template<class T>
  using Remove_cv_ = typename ::std::remove_cv<T>::type;

  template<class T>
  using Remove_ref_ = typename ::std::remove_reference<T>::type;

  template<class T>
  using Remove_cv_ref_ = Remove_cv_<Remove_ref_<Remove_cv_<T>>>;

  template<bool, class = void>
  struct Enable_if_
  {};

  template<class T>
  struct Enable_if_<true, T>
  {
    using Type = T;
  };

protected:
  using Size = ::std::size_t;
  using Unt_64 = ::std::uint64_t;
  using Time_probe = ::test::util::Time_probe;

  template<class T>
  using Optional = ::std::optional<T>;

  template<class... T>
  using Map = ::std::map<T...>;

  template<class... T>
  using Unordered_map = ::std::unordered_map<T...>;

  template<class K, class V>
  struct Avl_map
  {
    using Key = K;
    using Value = V;

    [[nodiscard]] Size const& size() const noexcept
    {
      return tree_.size;
    }

    void reserve(Size const n)
    {
      tree_.nodes.reserve(n);
    }

    template<class T>
    [[nodiscard]] Value const* find(T&& key) const
    {
      Node_ptr_ const p(Tree_algo_::binary_search(
        tree_,
        [&key](Node_ const& n) -> Compare_result_
        {
          return Util_::compare(n.key, static_cast<T&&>(key));
        }));
      return p ? ::std::addressof(tree_.nodes.data()[p.idx_].value) : nullptr;
    }

    template<class T, class U>
    bool emplace(T&& key, U&& value)
    {
      struct Ctrl
      {
        explicit Ctrl(T&& k, U&& v, Nodes_& n) noexcept :
          key_(static_cast<T&&>(k)),
          value_(static_cast<U&&>(v)),
          nodes_(n),
          created(false)
        {}

        [[nodiscard]] Compare_result_ operator()(Node_ const& n) const
        {
          return Util_::compare(n.key, static_cast<T&&>(key_));
        }

        [[nodiscard]] Node_ptr_ operator()(
          Node_ptr_ const& parent,
          Side_ const side)
        {
          auto const node_idx = nodes_.size();
          nodes_.emplace_back(
            static_cast<T&&>(key_),
            static_cast<U&&>(value_),
            parent, side);
          created = true;
          return node_idx;
        }

      private:
        T&& key_;
        U&& value_;
        Nodes_& nodes_;

      public:
        bool created;
      };

      Ctrl ctrl(static_cast<T&&>(key), static_cast<U&&>(value), tree_.nodes);
      Tree_algo_::try_insert(tree_, ctrl, ctrl);
      if (ctrl.created)
      {
        ++tree_.size;
      }

      return ctrl.created;
    }

  private:
    static Size constexpr invalid_node_index_ = Numeric_limits_<Size>::max();

    struct Node_ptr_
    {
      Node_ptr_() = default;

      Node_ptr_(Nullptr_ const&) noexcept :
        idx_(invalid_node_index_)
      {}

      [[nodiscard]] explicit operator bool() const noexcept
      {
        return invalid_node_index_ != idx_;
      }

      Node_ptr_& operator=(Nullptr_ const&) noexcept
      {
        idx_ = invalid_node_index_;
        return *this;
      }

    private:
      friend Avl_map;

      Node_ptr_(Size const& idx) :
        idx_(idx)
      {}

      Size idx_;
    };

    struct Node_
    {
      template<class T, class U>
      explicit Node_(T&& k, U&& v, Node_ptr_ const& p, Side_ const s) :
        parent(p),
        key(static_cast<T&&>(k)),
        value(static_cast<U&&>(v)),
        side(s)
      {}

      Node_ptr_ parent;
      Node_ptr_ left_child;
      Node_ptr_ right_child;
      Key const key;
      Value value;
      Balance_ balance;
      Side_ side;
    };

    using Nodes_ = Vector_<Node_>;

    struct Tree_
    {
      Tree_() noexcept :
        size(0u),
        root_(nullptr),
        leftmost_(nullptr),
        rightmost_(nullptr)
      {}

      Tree_(Tree_&&) = delete;
      Tree_(Tree_ const&) = delete;
      Tree_& operator=(Tree_&&) = delete;
      Tree_& operator=(Tree_ const&) = delete;

      [[nodiscard]] Node_ptr_ const& root() const noexcept
      {
        return root_;
      }

      void set_root(Node_ptr_ const& p) noexcept
      {
        root_ = p;
      }

      template<Side_ side>
      auto set_extreme(Node_ptr_ const& p) noexcept ->
        typename Enable_if_<Side_::left == side>::Type
      {
        leftmost_ = p;
      }

      template<Side_ side>
      auto set_extreme(Node_ptr_ const& p) noexcept ->
        typename Enable_if_<Side_::right == side>::Type
      {
        rightmost_ = p;
      }

      [[nodiscard]] Node_* address(Node_ptr_ const& p) noexcept
      {
        return nodes.data() + p.idx_;
      }

      [[nodiscard]] Node_ const* address(Node_ptr_ const& p) const noexcept
      {
        return nodes.data() + p.idx_;
      }

      [[nodiscard]] static Node_ptr_ const& parent(Node_ const& n) noexcept
      {
        return n.parent;
      }

      template<Side_ side>
      [[nodiscard]] static auto child(Node_ const& n) noexcept ->
        typename Enable_if_<Side_::left == side, Node_ptr_ const&>::Type
      {
        return n.left_child;
      }

      template<Side_ side>
      [[nodiscard]] static auto child(Node_ const& n) noexcept ->
        typename Enable_if_<Side_::right == side, Node_ptr_ const&>::Type
      {
        return n.right_child;
      }

      [[nodiscard]] static Balance_ const& balance(Node_ const& n) noexcept
      {
        return n.balance;
      }

      [[nodiscard]] static Side_ const& side(Node_ const& n) noexcept
      {
        return n.side;
      }

      static void set_parent(Node_& n, Node_ptr_ const& p) noexcept
      {
        n.parent = p;
      }

      template<Side_ side>
      static auto set_child(Node_& n, Node_ptr_ const& p) noexcept ->
        typename Enable_if_<Side_::left == side>::Type
      {
        n.left_child = p;
      }

      template<Side_ side>
      static auto set_child(Node_& n, Node_ptr_ const& p) noexcept ->
        typename Enable_if_<Side_::right == side>::Type
      {
        n.right_child = p;
      }

      static void set_balance(Node_& n, Balance_ const b) noexcept
      {
        n.balance = b;
      }

      static void set_side(Node_& n, Side_ const s) noexcept
      {
        n.side = s;
      }

      Size size;
      Nodes_ nodes;

    private:
      Node_ptr_ root_;
      Node_ptr_ leftmost_;
      Node_ptr_ rightmost_;
    };

    Tree_ tree_;
  };

private:
  template<class T, class M = Remove_cv_ref_<T>, class = M>
  struct Map_traits_
  {
    using Mapped = typename M::mapped_type;
    static bool constexpr is_avl = false;
  };

  template<class T, class U, class... V>
  struct Map_traits_<T, Avl_map<V...>, U>
  {
    using Mapped = typename U::Value;
    static bool constexpr is_avl = true;
  };

protected:
  template<
    class M, class S,
    class Map_traits = Map_traits_<M>,
    class Ret = typename Map_traits::Mapped>
  static Ret access(M&& map, S const& size)
  {
    struct Ctrl
    {
      explicit Ctrl(M&& m, S const& s) :
        map_(static_cast<M&&>(m)),
        size_(s),
        ret(static_cast<Ret>(0))
      {}

      [[nodiscard]] bool operator()(Unt_64 const& key)
      {
        if(static_cast<decltype(size_)>(0) < size_)
        {
          --size_;
          auto const fnd(static_cast<M&&>(map_).find(key));
          if constexpr(Map_traits::is_avl)
          {
            if(fnd)
            {
              ret += *fnd;
            }
          }
          else
          {
            if(static_cast<M&&>(map_).end() != fnd)
            {
              ret += fnd->second;
            }
          }
          return true;
        }

        return false;
      }

    private:
      M&& map_;
      Remove_cv_ref_<S> size_;

    public:
      Ret ret;
    };

    Ctrl ctrl(static_cast<M&&>(map), size);
    Random_util_::gen_56972304(ctrl);
    return ctrl.ret;
  }

  template<class M, class S>
  static void insert(M&& map, S const& size)
  {
    Random_util_::gen_56972304(
      [&map, size](auto const& key) -> bool
      {
        static_cast<M&&>(map).emplace(key, make_value(key));
        return static_cast<M&&>(map).size() < size;
      });
  }

  template<class T>
  [[nodiscard]] static T make_value(T const& key) noexcept
  {
    return key % static_cast<T>(17u);
  }
};

TEST_CASE_METHOD(
  Benchmark,
  "Benchmark: Tree++ AVL tree vs std::map vs std::unordered_map",
  "[tree++][treexx][bin][avl][algo][benchmark][std::map][std::unordered_map]")
{
  using Key = Unt_64;
  using Value = Unt_64;

  Size constexpr size(500000u);

  auto& cout = ::std::cout;
  Map<Key, Value> map;
  Unordered_map<Key, Value> unordered_map;
  Avl_map<Key, Value> avl_map;
  Optional<Time_probe> tp;

  tp.emplace();
  insert(map, size);
  auto ns_map(tp->ns());

  tp.emplace();
  insert(unordered_map, size);
  auto ns_unordered_map(tp->ns());

  tp.emplace();
  avl_map.reserve(size);
  insert(avl_map, size);
  auto ns_avl_map(tp->ns());

  CHECK(size == map.size());
  CHECK(size == unordered_map.size());
  CHECK(size == avl_map.size());

  auto const print_time = [&](auto const& op)
  {
    cout << "std::map           " << op << " time: " << ns_map << " ns\n";
    cout <<
      "std::unordered_map " << op <<
      " time: " << ns_unordered_map << " ns\n";
    cout << "Avl_map            " << op << " time: " << ns_avl_map << " ns\n";
  };

  cout << "Element count: " << size << "\n\n";
  print_time("insertion");

  tp.emplace();
  auto const acc_map(access(map, size));
  ns_map = tp->ns();

  tp.emplace();
  auto const acc_unordered_map(access(unordered_map, size));
  ns_unordered_map = tp->ns();

  tp.emplace();
  auto const acc_avl_map(access(avl_map, size));
  ns_avl_map = tp->ns();

  CHECK(acc_avl_map == acc_unordered_map);
  CHECK(acc_map == acc_unordered_map);
  CHECK(acc_map == acc_avl_map);

  cout << "\n";
  print_time("access");
}

} // namespace test::treexx::bin::avl
