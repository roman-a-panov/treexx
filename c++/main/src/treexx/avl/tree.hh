// Copyright (C) 2013-2024, Roman Panov (roman.a.panov@gmail.com).

#ifndef TREXX_AVL_TREE_HH
#define TREXX_AVL_TREE_HH

#include <memory>

#include <treexx/avl/balance.hh>
#include <treexx/avl/tree_core.hh>

namespace treexx::avl
{

template<class T, class A = ::std::allocator<T>>
struct Tree
{
  using Value = T;
  using Allocator = A;

private:
  struct Node_
  {
  };

  struct Core_traits_
  {
  };
};

} // namespace treexx::avl

#endif // TREXX_AVL_TREE_HH
