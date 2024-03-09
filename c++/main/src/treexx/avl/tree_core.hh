// Copyright (C) 2013-2024, Roman Panov (roman.a.panov@gmail.com).

#ifndef TREXX_AVL_TREECORE_HH
#define TREXX_AVL_TREECORE_HH

#include <treexx/compare_result.hh>

namespace treexx::avl
{

template<class T>
struct Tree_core
{
  using Traits = T;
  using Node = typename Traits::Node;
  using Node_pointer = typename Traits::Node_pointer;
  using Node_const_pointer = typename Traits::Node_const_pointer;

  struct Emplace_result
  {
    Emplace_result() = default;

    Emplace_result(Node_pointer const& p, bool const e) noexcept :
      node_pointer(p),
      emplaced(e)
    {}

    Node_pointer node_pointer;
    bool emplaced = false;
  };

  [[nodiscard]] Node_const_pointer root() const noexcept
  {
    return root_;
  }

  template<class Compare>
  [[nodiscard]] Node_const_pointer lower_bound(Compare&& compare) const
  {
    Node const* node;
    Node_const_pointer ptr(nullptr);
    Node_const_pointer p = root_;
    Compare_result_ cmp;
    for(;;)
    {
      node = Traits::address(*this, p);
      if(node)
      {
        cmp = static_cast<Compare&&>(compare)(*node);
        switch(cmp)
        {
          case Compare_result_::equal:
            return p;
          case Compare_result_::greater:
            ptr = p;
            p = node->leftChild();
            break;
          case Compare_result_::less:
            p = node->rightChild();
            break;
        }
      }
      else
      {
        break;
      }
    }

    return ptr;
  }

  template<class Compare, class Create_node>
  Emplace_result emplace(Compare&&, Create_node&&)
  {
    return {};
  }

private:
  using Compare_result_ = ::treexx::Compare_result;

private:
  Node_pointer root_{nullptr};
};

} // namespace treexx::avl

#endif // TREXX_AVL_TREECORE_HH
