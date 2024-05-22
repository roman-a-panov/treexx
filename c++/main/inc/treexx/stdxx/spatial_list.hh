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

#ifndef TREEXX_STDXX_SPATIALLIST_HH
#define TREEXX_STDXX_SPATIALLIST_HH

#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>

#include <treexx/assert.hh>
#include <treexx/bin/side.hh>
#include <treexx/bin/avl/balance.hh>
#include <treexx/bin/avl/tree_algo.hh>

namespace treexx::stdxx
{

template<class T, class S>
struct spatial_list_element
{
  using data_type = T;
  using spatial_size_type = S;

private:
  template<class U, class... Args>
  using Is_constructible_ = typename ::std::is_constructible<U, Args...>::type;

  template<class U, class... Args>
  using Is_nothrow_constructible_ =
    typename ::std::is_nothrow_constructible<U, Args...>::type;

  template<class U>
  using Is_nothrow_copy_constructible_ =
    typename ::std::is_nothrow_copy_constructible<U>::type;

  template<class U>
  using Is_nothrow_move_constructible_ =
    typename ::std::is_nothrow_move_constructible<U>::type;

  template<bool, class = void>
  struct Enable_if_
  {};

  template<class U>
  struct Enable_if_<true, U>
  {
    using Type = U;
  };

public:
  spatial_list_element(spatial_list_element&& x)
    noexcept(
      Is_nothrow_copy_constructible_<spatial_size_type>::value &&
      Is_nothrow_move_constructible_<data_type>::value) :
    size_(x.size_),
    data_(static_cast<data_type&&>(x.data_))
  {}

  spatial_list_element(spatial_list_element const&) = default;

  template<
    class Spatial_size,
    class... Data,
    bool e_0 = Is_constructible_<spatial_size_type, Spatial_size>::value,
    bool e_1 = Is_constructible_<data_type, Data...>::value,
    class = typename Enable_if_<e_0 && e_1>::Type>
  explicit spatial_list_element(Spatial_size&& size, Data&&... data) noexcept(
    Is_nothrow_constructible_<spatial_size_type, Spatial_size>::value &&
    Is_nothrow_constructible_<data_type, Data...>::value) :
    size_(static_cast<Spatial_size&&>(size)),
    data_(static_cast<Data&&>(data)...)
  {}

  spatial_list_element& operator =(spatial_list_element&&) = delete;
  spatial_list_element& operator =(spatial_list_element const&) = delete;

  [[nodiscard]] spatial_size_type const& size() const noexcept
  {
    return size_;
  }

  [[nodiscard]] data_type& data() noexcept
  {
    return data_;
  }

  [[nodiscard]] data_type const& data() const noexcept
  {
    return data_;
  }

private:
  spatial_size_type size_;
  data_type data_;
};

template<
  class T,
  class S = ::std::size_t,
  class A = ::std::allocator<spatial_list_element<T, S>>,
  bool indexed = false>
struct spatial_list
{
  using data_type = T;
  using allocator_type = A;
  using spatial_size_type = S;
  using is_indexed = ::std::integral_constant<bool, indexed>;
  using value_type = spatial_list_element<data_type, spatial_size_type>;
  using reference = value_type&;
  using const_reference = value_type const&;
  using difference_type = ::std::ptrdiff_t;
  using size_type = ::std::size_t;

  [[nodiscard]] bool empty() const noexcept
  {
    return tree_and_alloc_.tree.empty();
  }

  [[nodiscard]] size_type size() const noexcept
  {
    return tree_and_alloc_.tree.size();
  }

  template<class... Args>
  reference emplace_back(Args&&... args)
  {
    Tree_& tree = tree_and_alloc_.tree;
    Unique_node_ node(tree_and_alloc_.allocator());
    node.construct(static_cast<Args&&>(args)...);
    auto const last_node = tree.template extreme<Side_::right>();
    Spatial_size_ offset;
    if(last_node)
    {
      offset = last_node->value.size();
    }
    else
    {
      offset = static_cast<Spatial_size_>(0u);
    }

    auto const node_ptr = node.get();
    TREEXX_ASSERT(node_ptr);
    TREEXX_ASSERT(static_cast<Spatial_size_>(0u) < node_ptr->value.size());

    Tree_algo_::push_back(tree, node_ptr, offset);
    node.release();
    tree.increment_size();
    return node_ptr->value;
  }

  template<class... Args>
  reference emplace_front(Args&&... args)
  {
    Tree_& tree = tree_and_alloc_.tree;
    Unique_node_ node(tree_and_alloc_.allocator());
    node.construct(static_cast<Args&&>(args)...);
    auto const node_ptr = node.get();
    TREEXX_ASSERT(node_ptr);
    TREEXX_ASSERT(static_cast<Spatial_size_>(0u) < node_ptr->value.size());

    Tree_algo_::insert_at_offset(
      tree, node_ptr,
      static_cast<Spatial_size_>(0u),
      node_ptr->value.size());

    node.release();
    tree.increment_size();
    return node_ptr->value;
  }

  reference push_back(value_type&& val)
  {
    return emplace_back(static_cast<value_type&&>(val));
  }

  reference push_back(value_type const& val)
  {
    return emplace_back(val);
  }

  reference push_front(value_type&& val)
  {
    return emplace_front(static_cast<value_type&&>(val));
  }

  reference push_front(value_type const& val)
  {
    return emplace_front(val);
  }

private:
  using Side_ = ::treexx::bin::Side;
  using Balance_ = ::treexx::bin::avl::Balance;
  using Tree_algo_ = ::treexx::bin::avl::Tree_algo;
  using Spatial_size_ = spatial_size_type;
  using Value_ = value_type;
  using Size_ = size_type;

  template<bool c, class U, class V>
  using Conditional_ = typename ::std::conditional<c, U, V>::type;

  template<class U>
  using Add_const_ = typename ::std::add_const<U>::type;

  template<class U>
  using Remove_cv_ = typename ::std::remove_cv<U>::type;

  template<class U>
  using Remove_reference_ = typename ::std::remove_reference<U>::type;

  template<class U>
  using Remove_cv_ref_ = Remove_cv_<Remove_reference_<Remove_cv_<U>>>;

  template<class U, class... Args>
  using Is_constructible_ = typename ::std::is_constructible<U, Args...>::type;

  template<class, class...>
  struct Is_same_
  {
    static bool constexpr value = false;
  };

  template<class U>
  struct Is_same_<U, U>
  {
    static bool constexpr value = true;
  };

  template<bool, class = void>
  struct Enable_if_
  {};

  template<class U>
  struct Enable_if_<true, U>
  {
    using Type = U;
  };

  template<bool = indexed, int = 0>
  struct Node_base_
  {};

  template<int z>
  struct Node_base_<true, z>
  {
    Size_ index;
  };

  struct Node_ : Node_base_<>
  {
    template<
      class... Val_args,
      bool e = Is_constructible_<Value_, Val_args...>::value,
      bool d = Is_same_<Node_, Remove_cv_ref_<Val_args>...>::value,
      class = typename Enable_if_<e && !d>::Type>
    explicit Node_(Val_args&&... val_args) :
      value(static_cast<Val_args&&>(val_args)...)
    {}

    Spatial_size_ offset;
    Node_* parent;
    Node_* left_child;
    Node_* right_child;
    Value_ value;
    Balance_ balance;
    Side_ side;
  };

  template<bool is_const, bool is_reverse>
  struct Iterator_
  {
    using iterator_category = ::std::bidirectional_iterator_tag;
    using value_type = Value_;
    using reference = Conditional_<
      is_const, Add_const_<value_type>&, value_type&>;
    using pointer = Conditional_<
      is_const, Add_const_<value_type>*, value_type*>;

    Iterator_() = default;

    template<bool e = is_const, class = typename Enable_if_<e>::Type>
    Iterator_(Iterator_<false, is_reverse> const& x) noexcept :
      node_(x.node_)
    {}

    [[nodiscard]] spatial_size_type offset() const noexcept
    {
      TREEXX_ASSERT(node_);
      return Tree_algo_::node_offset(Tree_static_(), *node_);
    }

    template<bool e = indexed>
    [[nodiscard]] auto index() const noexcept ->
      typename Enable_if_<e && indexed == e, size_type>::Type
    {
      TREEXX_ASSERT(node_);
      return Tree_algo_::node_index(Tree_static_(), *node_);
    }

    [[nodiscard]] reference operator *() const noexcept
    {
      TREEXX_ASSERT(node_);
      return node_->value;
    }

    [[nodiscard]] pointer operator ->() const noexcept
    {
      TREEXX_ASSERT(node_);
      return ::std::addressof(node_->value);
    }

    Iterator_& operator ++() noexcept
    {
      TREEXX_ASSERT(node_);
      Side_ constexpr side = is_reverse ? Side_::left : Side_::right;
      node_ = Tree_algo_::adjacent_node<side>(Tree_static_(), *node_);
      return *this;
    }

    [[nodiscard]] friend bool operator ==(
      Iterator_ const& x,
      Iterator_ const& y) noexcept
    {
      return x.node_ == y.node_;
    }

    [[nodiscard]] friend bool operator !=(
      Iterator_ const& x,
      Iterator_ const& y) noexcept
    {
      return x.node_ != y.node_;
    }

  private:
    friend struct spatial_list;

    Iterator_(Node_* const n) noexcept :
      node_(n)
    {}

    Node_* node_;
  };

  using Allocator_ = allocator_type;
  using Allocator_traits_ = ::std::allocator_traits<Allocator_>;
  using Node_allocator_ =
    typename Allocator_traits_::template rebind_alloc<Node_>;
  using Node_allocator_traits_ = ::std::allocator_traits<Node_allocator_>;

  static_assert(Is_same_<data_type, Remove_cv_ref_<data_type>>::value);
  static_assert(Is_same_<Allocator_ , Remove_cv_ref_<Allocator_>>::value);
  static_assert(Is_same_<Spatial_size_ , std::decay_t<Spatial_size_>>::value);

public:
  using iterator = Iterator_<false, false>;
  using const_iterator = Iterator_<true, false>;
  using reverse_iterator = Iterator_<false, true>;
  using const_reverse_iterator = Iterator_<true, true>;

  [[nodiscard]] iterator begin() noexcept
  {
    return tree_and_alloc_.tree.template extreme<Side_::left>();
  }

  [[nodiscard]] const_iterator begin() const noexcept
  {
    return tree_and_alloc_.tree.template extreme<Side_::left>();
  }

  [[nodiscard]] iterator end() noexcept
  {
    return nullptr;
  }

  [[nodiscard]] const_iterator end() const noexcept
  {
    return nullptr;
  }

  [[nodiscard]] const_iterator cbegin() const noexcept
  {
    return begin();
  }

  [[nodiscard]] const_iterator cend() const noexcept
  {
    return nullptr;
  }

  template<bool e = indexed>
  [[nodiscard]] auto find(size_type const& idx) noexcept ->
    typename Enable_if_<e && indexed == e, iterator>::Type
  {
    return Tree_algo_::at_index(tree_and_alloc_.tree, idx);
  }

  template<bool e = indexed>
  [[nodiscard]] auto find(size_type const& idx) const noexcept ->
    typename Enable_if_<e && indexed == e, const_iterator>::Type
  {
    return Tree_algo_::at_index(tree_and_alloc_.tree, idx);
  }

private:
  struct Unique_node_
  {
    using Ptr = typename Node_allocator_traits_::pointer;

    explicit Unique_node_(Node_allocator_& alloc) :
      alloc_(::std::addressof(alloc)),
      ptr_(Node_allocator_traits_::allocate(alloc, 1u)),
      constructed(false)
    {}

    Unique_node_(Unique_node_&&) = delete;
    Unique_node_(Unique_node_ const&) = delete;

    ~Unique_node_()
    {
      if(alloc_ && ptr_)
      {
        if(constructed)
        {
          Node_allocator_traits_::destroy(*alloc_, ptr_);
        }

        Node_allocator_traits_::deallocate(*alloc_, ptr_, 1u);
      }
    }

    Unique_node_& operator =(Unique_node_&&) = delete;
    Unique_node_& operator =(Unique_node_ const&) = delete;

    [[nodiscard]] Ptr const& get() const noexcept
    {
      return ptr_;
    }

    void release() noexcept
    {
      alloc_ = nullptr;
    }

    template<class... Args>
    void construct(Args&&... args)
    {
      TREEXX_ASSERT(alloc_);
      TREEXX_ASSERT(!constructed);
      Node_allocator_traits_::construct(
        *alloc_, ptr_, static_cast<Args&&>(args)...);
      constructed = true;
    }

  private:
    Node_allocator_* alloc_;
    Ptr ptr_;
    bool constructed;
  };

  struct Allocator_base_ : Node_allocator_
  {
    [[nodiscard]] Node_allocator_& allocator() noexcept
    {
      return *this;
    }
  };

  template<bool = indexed, int = 0>
  struct Tree_static_base_
  {};

  template<int z>
  struct Tree_static_base_<true, z>
  {
    using Index = Size_;

    [[nodiscard]] static Index const& index(Node_ const& n) noexcept
    {
      return n.index;
    }

    static void increment_index(Node_& n) noexcept
    {
      ++n.index;
    }

    static void add_to_index(Node_& n, Index const& i) noexcept
    {
      n.index += i;
    }

    static void subtract_from_index(Node_& n, Index const& i) noexcept
    {
      n.index -= i;
    }

    template<unsigned i>
    static void set_index(Node_& n) noexcept
    {
      n.index = static_cast<Index>(i);
    }

    template<unsigned i>
    [[nodiscard]] static Index make_index() noexcept
    {
      return static_cast<Index>(i);
    }
  };

  struct Tree_static_ : Tree_static_base_<>
  {
    using Offset = Spatial_size_;

    static Node_* root() noexcept;

    [[nodiscard]] static Node_* address(Node_* const n) noexcept
    {
      return n;
    }

    [[nodiscard]] static Node_* parent(Node_ const& n) noexcept
    {
      return n.parent;
    }

    template<Side_ side>
    static Node_* child(Node_ const& n) noexcept
    {
      static_assert(Side_::left == side || Side_::right == side);
      if constexpr(Side_::left == side)
      {
        return n.left_child;
      }
      else if constexpr(Side_::right == side)
      {
        return n.right_child;
      }
    }

    [[nodiscard]] static Balance_ balance(Node_ const& n) noexcept
    {
      return n.balance;
    }

    [[nodiscard]] static Side_ side(Node_ const& n) noexcept
    {
      return n.side;
    }

    [[nodiscard]] static Spatial_size_ const& offset(Node_ const& n) noexcept
    {
      return n.offset;
    }

    static void set_parent(Node_& n, Node_* const p) noexcept
    {
      n.parent = p;
    }

    template<Side_ side>
    static void set_child(Node_& n, Node_* const c) noexcept
    {
      static_assert(Side_::left == side || Side_::right == side);
      if constexpr(Side_::left == side)
      {
        n.left_child = c;
      }
      else if constexpr(Side_::right == side)
      {
        n.right_child = c;
      }
    }

    static void set_balance(Node_& n, Balance_ const b) noexcept
    {
      n.balance = b;
    }

    static void set_side(Node_& n, Side_ const s) noexcept
    {
      n.side = s;
    }

    static void add_to_offset(Node_& n, Spatial_size_ const& o) noexcept
    {
      n.offset += o;
    }

    static void subtract_from_offset(Node_& n, Spatial_size_ const& o) noexcept
    {
      n.offset -= o;
    }

    static void set_offset(Node_& n, Spatial_size_ const& o) noexcept
    {
      n.offset = o;
    }

    template<unsigned o>
    [[nodiscard]] static Spatial_size_ make_offset() noexcept
    {
      return static_cast<Spatial_size_>(o);
    }
  };

  struct Tree_ : Tree_static_
  {
    Tree_() noexcept :
      root_(nullptr),
      leftmost_(nullptr),
      rightmost_(nullptr),
      size_(static_cast<size_type>(0u))
    {}

    [[nodiscard]] Node_* root() const noexcept
    {
      return root_;
    }

    template<Side_ side>
    [[nodiscard]] Node_* extreme() const noexcept
    {
      static_assert(Side_::left == side || Side_::right == side);
      if constexpr(Side_::left == side)
      {
        return leftmost_;
      }
      else if constexpr(Side_::right == side)
      {
        return rightmost_;
      }
    }

    void set_root(Node_* const r) noexcept
    {
      root_ = r;
    }

    template<Side_ side>
    void set_extreme(Node_* const x) noexcept
    {
      static_assert(Side_::left == side || Side_::right == side);
      if constexpr(Side_::left == side)
      {
        leftmost_ = x;
      }
      else if constexpr(Side_::right == side)
      {
        rightmost_ = x;
      }
    }

    [[nodiscard]] Size_ size() const noexcept
    {
      return size_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
      return 1u > size_;
    }

    void increment_size() noexcept
    {
      ++size_;
    }

  private:
    Node_* root_;
    Node_* leftmost_;
    Node_* rightmost_;
    Size_ size_;
  };

  struct Tree_and_alloc_ : Allocator_base_
  {
    Tree_ tree;
  };

  Tree_and_alloc_ tree_and_alloc_;
};

} // namespace treexx::stdxx

#endif // TREEXX_STDXX_SPATIALLIST_HH
