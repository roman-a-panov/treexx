// Copyright (C) 2013-2024, Roman Panov (roman.a.panov@gmail.com).

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <catch.hpp>

#include <treexx/compare_result.hh>
#include <treexx/avl/balance.hh>
#include <treexx/avl/test_util.hh>
#include <treexx/avl/tree_core.hh>

namespace treexx::avl
{

class Simple_tree_core_test
{
  template<class T, class... Args>
  using Is_constructible_ = typename ::std::is_constructible<T, Args...>::type;

  using Compare_result_ = ::treexx::Compare_result;

  template<class Value>
  struct Node_
  {
    template<class... Args>
    explicit Node_(Node_* const parent, Args&&... args) :
      parent_(parent),
      value_(static_cast<Args&&>(args)...)
    {}

    [[nodiscard]] Value const& value() const noexcept
    {
      return value_;
    }

    [[nodiscard]] Node_ const* leftChild() const noexcept
    {
      return leftChild_;
    }

    [[nodiscard]] Node_ const* rightChild() const noexcept
    {
      return rightChild_;
    }

  private:
    Node_* parent_;
    Node_* leftChild_ = nullptr;
    Node_* rightChild_ = nullptr;
    Value value_;
    Balance balance_ = Balance::poised;
  };

  template<class Value>
  struct Core_traits_
  {
    using Node = Node_<Value>;
    using Node_pointer = Node*;
    using Node_const_pointer = Node const*;

    template<class T>
    static constexpr Node* address(
      const T&,
      const Node_pointer& p) noexcept
    {
      return p;
    }

    template<class T>
    static constexpr Node const* address(
      const T&,
      const Node_const_pointer& p) noexcept
    {
      return p;
    }
  };

protected:
  using Size = ::std::size_t;
  using Int_32 = ::std::int32_t;

  template<class Value>
  struct Tree : Tree_core<Core_traits_<Value>>
  {
    template<class X>
    [[nodiscard]] bool contains(const X& x) const noexcept
    {
      auto const node_ptr = Tree::lower_bound(
        [&x](auto const& node) noexcept -> Compare_result_
        {
          return Test_util::compare(node.value(), x);
        });
      return node_ptr && node_ptr->value() == x;
    }

    template<class X>
    decltype(auto) insert(X&& x)
    {
      using Node = typename Tree::Node;
      using Node_pointer = typename Tree::Node_pointer;

      return Tree::emplace(
        [&x](Node const& node) noexcept -> Compare_result_
        {
          return Test_util::compare(node.value(), x);
        },
        [&x](const Node_pointer& parent) -> typename Tree::Node_pointer
        {
          return new Node(parent, static_cast<X&&>(x));
        });
    }
  };
};

TEST_CASE_METHOD(Simple_tree_core_test, "Simple AVL tree core test 00")
{
  Int_32 values[] = {3, 5, 9, 29, 33};
  Tree<Int_32> tree;
  Size count = 0u;

  for(const auto& x: values)
  {
    tree.insert(x);
    ++count;

    for(Size i = 0u; count > i; ++i)
    {
      CHECK(tree.contains(values[i]));
    }
  }
}

} // namespace treexx::avl
