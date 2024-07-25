# Content
* [What is Tree++?](#what-is-tree)
* [What is AVL Tree?](#what-is-avl-tree)
* [What is indexed AVL Tree](#what-is-indexed-avl-tree)
* [What is Offset Tree?](#what-is-offset-tree)
* [How to use Tree++?](#how-to-use-tree)
* [How to implement a Tree](#how-to-implement-a-tree)
* [Algorithms](#algorithms)

# What is Tree++?
Tree++ is a C++17 template library that implements algorithms for working with:

* [AVL Tree](#what-is-avl-tree)
* [Indexed AVL Tree](#what-is-indexed-avl-tree)
* [Offset Tree built atop of AVL Tree](#what-is-offset-tree)

# What is AVL Tree?
AVL tree is a self-balancing binary search tree that supports insertion,
erasure and lookup of an element with _O_(log(_n_)) complexity, where _n_ is
the total number of elements stored in the tree. It is described in detail on
[Wikipedia](https://en.wikipedia.org/wiki/AVL_tree) and many other internet
resources.

# What is indexed AVL Tree?
An indexed AVL tree is an AVL tree that in addition to all the operations
supported by simple AVL trees supports lookup of an element by its index with
_O_(log(_n_)) complexity. By index here we mean the ordinal number of an
element (node) when traversing the tree in ascending order, that is from left
to right. For a tree with _n_ nodes, the leftmost node has index 0, whereas the
rightmost node's index is _n-1_, exactly as is customary in the C++ standard
library containers, e.g. `std::vector`.

# What is Offset Tree?
An offset tree is a slightly modified binary search tree whose elements
(nodes) are ordered by their offsets. Offsets from what point? Does not matter
indeed. Offset is just a numeric spatial characteristic that describes how
nodes are stored one after another. Like any balanced binary search tree an
offset tree supports insertion of a node at the specified offset, erasure of a
node and lookup by offset. All these operations have _O_(log(_n_)) complexity.
But besides these, it can shift all nodes or a butch of nodes also in
_O_(log(_n_)). By shifting nodes we mean incrementing or decrementing their
offsets. Basically you can shift all nodes staring from some node to the right
or to the left by a specified value. It is called [shifting the suffix](#shift).
A suffix starting with the leftmost node represents the entire tree.

## Where does Offset Tree help?
Here is an example illustrating a problem solvable with offset tree. Consider
there is a long list of items we have to display. The items might be,
for example, e-mail messages, chat messages, images or something like that. Let
us call them widgets. Widgets are placed one after another back-to-back. That
is, where one widget ends, the next one begins. The list is too long and does
not fit our screen, so we have to display only the part of the list that
intersects the viewport.

**List of widgets and viewport**
```


                              +----------------------------------------------------+
                              |                         Viewport                   |
--------------+---------------|--+--------+------------------+---------------------|--
              |               |  |        |                  |                     |
  Widget i-2  |    Widget i-1 |  |Widget i|    Widget i+1    |       Widget i+2    |
              |               |  |        |                  |                     |
--------------+---------------|--+--------+------------------+---------------------|--
                              |                                                    |
                              +----------------------------------------------------+
```
We have to find the leftmost widget that hits the viewport, render it and then
iterate over the list rendering each subsequent widget until we find a widget,
which is completely out of the viewport, or the list ends. In the picture above
the widgets to be rendered are widgets from `i-1` to `i+2` inclusively. Widgets
have different sizes.

How to solve this problem? Well, we can store the widgets (or pointers to them)
in, for instance, `std::vector` in ascending order. Then we'll employ
`std::lower_bound` to find the leftmost widget that hits the viewport and begin
rendering from it. Alternatively we can instantiate `std::map` that will map
offsets to widgets.

The proposed solution works perfectly well if the content of the list is
static. But what if it is not? Consider, for example, insertion of a new widget
somewhere in the middle of the list, or erasure of an arbitrary widget. Since
we need the widgets to be placed back-to-back, we have to shift the suffix -
all elements to the right of the newly inserted of erased one. The same applies
to the resize operation. If a widget changes its size, all the following
widgets will have to be shifted. Inserting a new widget at the front of the
list requires the entire list to be shifted. With `std::vector` shifting a
suffix has linear complexity in the size of the suffix being shifted.
Does `std::map` solve the problem better? No, with it things get even worser,
since it does not allow modifying keys of the stored elements. With map all the
elements representing the suffix will have to be erased and then inserted back
with the new offsets.

Offset tree solves this task in the logarithmic complexity, it can
[shift a suffix](#shift) without updating all the nodes comprising it.

## How does Offset Tree work?
First off, let us consider the solution with a vanilla binary search tree
ordered by offsets of the items. It is by no means better than the solution
with `std::vector` or something similar, because to shift all nodes starting
from some of them we still need to visit all of them and update their offsets.

But offset tree is a little different. It does not store offsets explicitly.
Instead, it stores so-called relative (or implicit) offsets. The relative
offset of a node represents the offset of the node from the ancestry node
in whose right subtree the node resides. If no such ancestry node exists
(for example, the root and the leftmost nodes do not have such ancestors) the
absolute offset is stored within the node.

**Binary search Tree and Offset Tree corresponding to it**
```
                                +-----+
                                |  55 |
                                +-----+
                               /       \
                              /         \
                    +--------+           +---------+
                   /                                \
                  /                                  \
              +--+--+                              +--+--+
              |  30 |                              |  75 |
              +-----+                              +-----+
             /       \                            /       \
            /         \                          /         \
           /           \                        /           \ 
          /             \                      /             \
         /               \                    /               \
     +--+--+           +--+--+            +--+--+           +--+--+
     |  20 |           |  45 |            |  65 |           |  85 |
     +-----+           +-----+            +-----+           +-----+
    /       \         /       \          /       \         /       \
+--+--+   +--+--+ +--+--+   +--+--+  +--+--+   +--+--+ +--+--+   +--+--+
|  10 |   |  25 | |  40 |   |  50 |  |  60 |   |  70 | |  80 |   |  90 |
+-----+   +-----+ +-----+   +-----+  +-----+   +-----+ +-----+   +-----+

                                +-----+
                                |  55 |
                                +-----+
                               /       \
                              /         \
                    +--------+           +---------+
                   /                                \
                  /                                  \
              +--+--+                              +--+--+
              |  30 |                              | +20 |
              +-----+                              +-----+
             /       \                            /       \
            /         \                          /         \
           /           \                        /           \ 
          /             \                      /             \
         /               \                    /               \
     +--+--+           +--+--+            +--+--+           +--+--+
     |  20 |           | +15 |            | +10 |           | +10 |
     +-----+           +-----+            +-----+           +-----+
    /       \         /       \          /       \         /       \
+--+--+   +--+--+ +--+--+   +--+--+  +--+--+   +--+--+ +--+--+   +--+--+
|  10 |   |  +5 | | +10 |   |  +5 |  |  +5 |   |  +5 | |  +5 |   |  +5 |
+-----+   +-----+ +-----+   +-----+  +-----+   +-----+ +-----+   +-----+
```
Nodes that store relative offsets are marked with `+`.The picture exhibits that
only nodes at the left edge contain their real (absolute) offsets.

This trick makes the deal. Now to shift all nodes starting from some node we do
not need to update offsets stored in all these nodes. We only have to update
the offset of the first node and then walk to the root node. During the walk if
we come to a node from its left subtree, the offset of the node is to be
updated. The amount of the required computations is proportional to the height
of the tree. For balanced trees it yields the complexity logarithmic in the
size of the tree.

# How to use Tree++?
Tree++ is a template library, all its functionality resides in header files.
All the functional headers are placed at `c++/main/inc`, just add this folder
to the list of include paths of your project. No need to build Tree++ separately.

Tree++ does not implement a tree or any data structure at all. Instead, it
implements algorithms - template functions that work with trees. You need to
define a [node class](#node-class) and a [tree class](#tree-class) that meet
the specific requirements. This is essentially the same as concepts, but in
C++17. This documentation contains a very detailed step-by-step instruction on
how to define the [node class](#node-class) and the [tree class](#tree-class).

By implementing algorithms instead of data structures Tree++ provides high
degree of flexibility. It is you to decide how to organize nodes and trees.
Meantime this flexibility entails the necessity to write quite a bit of
boilerplate code. This code is simple, typically does not contain any logic and
represents a bunch of accessor and setter methods, but still needs to be
written. This is OK, because the primary intention of Tree++ is to be a
building block for higher level classes. For example, with Tree++ you can
easily implement `std::map` and `std::set`, but not only them.

The documentation is quite detailed, I tried to write it well, with details and
examples, and hope you will enjoy the reading. The folder `c++/examples`
contains several ready-to-use examples. They do not cover all the functionality
of the library, whereas this documentation does. There is also a bunch of unit
tests that cover the entire functionality of Tree++. They can help understand
how to use the library. But reading the test code is not a piece of cake, I'd
better read the documentation.

# How to implement a Tree
To implement a tree you have to define a class for the tree node and a class
for the tree itself. The next sections contain a step-by-step instruction on
how to define such classes for a simple AVL tree. To make your AVL tree
indexed read [here](#making-the-tree-indexed). To make offset tree atop of your
AVL tree read [here](#turning-the-tree-into-offset-tree).

* [Common enums](#common-enums)
* [Node class](#node-class)
* [Node pointer](#node-pointer)
* [Tree class](#tree-class)
* [Making the Tree indexed](#making-the-tree-indexed)
* [Turning the Tree into Offset Tree](#turning-the-tree-into-offset-tree)
* [Custom Node pointer class](#custom-node-pointer-class)

## Common enums
These enums are used throughout the library.

### `Compare_result`
Declared in header `<treexx/compare_result.hh>`
```c++
namespace treexx
{
  enum class Compare_result : char unsigned
  {
    equal = 0u,
    greater,
    less
  };
}
```

### `Side`
Declared in header `<treexx/bin/side.hh>`
```c++
namespace treexx::bin
{
  enum class Side : char unsigned
  {
    left = 0u,
    right
  };
}
```

### `Balance`
Declared in header `<treexx/bin/avl/balance.hh>`
```c++
namespace treexx::bin::avl
{
  enum class Balance : char unsigned
  {
    poised = 0u, // The subtree is balanced.
    overright,   // The subtree rooted at the right child is higher.
    overleft     // The subtree rooted at the left child is higher.
  };
}
```

## Node class
Node class is to store per node data:

* Pointer to parent
* Pointers to children
* Balance of the subtree rooted at this node
* Side that indicates how the node relates to its parent, whether it is the
left or the right child of the parent
* Whatever user-defined value per node

**Example**
```cpp
#include <treexx/bin/side.hh>
#include <treexx/bin/avl/balance.hh>

template<class Value>
struct My_node
{
  using Side = ::treexx::bin::Side;
  using Balance = ::treexx::bin::avl::Balance;

  My_node* parent = nullptr;
  My_node* left_child = nullptr;
  My_node* right_child = nullptr;
  Balance balance = Balance::poised;
  Side side = Side::left;
  Value value;
};
```
> **Please note** that this is only an example. The node fields are **not**
> accessed directly by the Tree++ algorithms. Instead Tree++ accesses or
> modifies them through the appropriate methods defined in the
> [Tree class](#tree-class). It basically means that it is you to decide how to
> store the information in the node class. For example, `side` and `balance`
> can be stored in bit fields, or in the least significant bits of pointers,
> which are always zeros because of the alignment. Nothing is imposed!

## Node pointer
Node pointer does not have to be a C++ pointer, though it typically is.

**Example**
```cpp
template<class Value>
using My_node_pointer = My_node<Value>*; // Might be a user-defined class.
```
To implement a user-defined node pointer read [here](#custom-node-pointer-class).

## Tree class
Tree class is to store per tree data, like pointers to the root node, the
leftmost and the rightmost nodes. The stored data is never accessed by Tree++
directly. The tree class has to implement methods for accessing and modifying
per tree data as wel as per node data.

**Example of Tree data**
```c++
template<class Value>
struct Tree
{
private:
  My_node_pointer<Value> root_node_ptr_ = nullptr;
  My_node_pointer<Value> leftmost_node_ptr_ = nullptr;
  My_node_pointer<Value> rightmost_node_ptr_ = nullptr;
};
```

### Methods to access and modify the root Node
**Example**
```c++
template<class Value>
struct Tree
{
  My_node_pointer<Value> const& root() noexcept
  {
    return root_node_ptr_;
  }

  void set_root(My_node_pointer<Value> const& node_ptr) noexcept
  {
    root_node_ptr_ = node_ptr;
  }
};
```
Return type of the `root()` method is used to infer the `Node_pointer`
type. It is basically a decay of this return type:
```c++
template<class Tree>
using Node_pointer = std::decay_t<decltype(std::declval<Tree>().root())>;
```

### Method to convert Node pointer to Node address
Node pointer type does not have to be a C++ pointer. Alternatively it,
for example, can store index of the node in a pool of nodes or an offset in a
movable memory block. To get address of a node from its pointer Tree++ invokes
the method `address` of the [tree class](#tree-class).

**Example**
```c++
template<class Value>
struct Tree
{
  My_node<Value>* address(My_node_pointer<Value> const& node_ptr) noexcept
  {
    return node_ptr;
  }
};
```
For a user-defined pointer class this method might have more complex implementation.  
This method might be `static`, but this is not mandated.

Return type of the `address` method is used to infer the `Node` type:
```c++
template<class Tree>
using Node = std::remove_reference_t<std::remove_pointer_t<std::decay_t<
  decltype(std::declval<Tree>().address(std::declval<Node_pointer<Tree> const&>()))>>>;
```


### Methods to access and modify the leftmost and the rightmost Nodes
**Example**
```c++
template<class Value>
struct Tree
{
  template<Side side>
  My_node_pointer<Value> const& extreme() noexcept
  {
    if constexpr(Side::left == side)
    {
      return leftmost_node_ptr_;
    }
    else if constexpr(Side::right == side)
    {
      return rightmost_node_ptr_;
    }
  }

  template<Side side>
  void set_extreme(My_node_pointer<Value> const& node_ptr) noexcept
  {
    if constexpr(Side::left == side)
    {
      leftmost_node_ptr_ = node_ptr;
    }
    else if constexpr(Side::right == side)
    {
      rightmost_node_ptr_ = node_ptr;
    }
  }
};
```

### Methods to access and modify Node parent
**Example**
```c++
template<class Value>
struct Tree
{
  My_node_pointer<Value> const& parent(My_node<Value> const& node) noexcept
  {
    return node.parent;
  }

  void set_parent(
    My_node<Value>& node,
    My_node_pointer<Value> const& parent_ptr) noexcept
  {
    node.parent = parent_ptr;
  }
};
```
These methods might be `static`, but this is not mandated.

### Methods to access and modify Node children
**Example**
```c++
template<class Value>
struct Tree
{
  template<Side side>
  My_node_pointer<Value> const& child(My_node<Value> const& node) noexcept
  {
    if constexpr(Side::left == side)
    {
      return node.left_child;
    }
    else if constexpr(Side::right == side)
    {
      return node.right_child;
    }
  }

  template<Side side>
  void set_child(
    My_node<Value>& node,
    My_node_pointer<Value> const& child_ptr) noexcept
  {
    if constexpr(Side::left == side)
    {
      node.left_child = child_ptr;
    }
    else if constexpr(Side::right == side)
    {
      node.right_child = child_ptr;
    }
  }
};
```
These methods might be `static`, but this is not mandated.

### Methods to access and modify Node balance
**Example**
```c++
#include <treexx/bin/avl/balance.hh>

template<class Value>
struct Tree
{
  using Balance = ::treexx::bin::avl::Balance;

  Balance balance(My_node<Value> const& node) noexcept
  {
    return node.balance;
  }

  void set_balance(My_node<Value>& node, Balance const balance) noexcept
  {
    node.balance = balance;
  }
};
```
These methods might be `static`, but this is not mandated.

### Methods to access and modify Node side
**Example**
```c++
#include <treexx/bin/side.hh>

template<class Value>
struct Tree
{
  using Side = ::treexx::bin::Side;

  Side side(My_node<Value> const& node) noexcept
  {
    return node.side;
  }

  void set_side(My_node<Value>& node, Side const side) noexcept
  {
    node.side = side;
  }
};
```
These methods might be `static`, but this is not mandated.

## Making the Tree indexed
### Defining `Index` type
To make the tree indexed you have to define `Index` type inside your tree
class.

**Example**
```c++
#include <cstddef>

using My_index = std::size_t;

template<class Value>
struct My_tree
{
  using Index = My_index;
};
```
Your can use any integral type for the index, or even a user-defined class that
behaves like an integral type.

### Storing index per node
Each node of an indexed tree has to store a value of type `Index`.

**Example**
```c++
template<class Value>
struct My_node
{
  My_index index = 0u;
};
```
> **Note!** The value stored inside the node is not the node's real index.
> It is so-called implicit index that is used for computing the real index. 

### Methods to access and modify indices stored in nodes
An indexed tree class has to implement methods for accessing and modifying
index values stored within nodes.

**Example**
```c++
template<class Value>
struct My_tree
{
  Index const& index(My_node<Value> const& node) noexcept
  {
    return node.index;
  }

  template<unsigned index>
  void set_index(My_node<Value>& node) noexcept
  {
    node.index = static_cast<Index>(index);
  }

  void set_index(My_node<Value>& node, Index const& index) noexcept
  {
    node.index = index;
  }

  void increment_index(My_node<Value>& node) noexcept
  {
    ++node.index;
  }

  void decrement_index(My_node<Value>& node) noexcept
  {
    --node.index;
  }

  void add_to_index(My_node<Value>& node, Index const& increment) noexcept
  {
    node.index += increment;
  }

  void subtract_from_index(My_node<Value>& node, Index const& decrement) noexcept
  {
    node.index -= decrement;
  }
};
```
These methods might be `static`, but this is not mandated.

### Method to instantiate `Index`
An indexed tree class has to implement a `static` method that instantiates `Index`
with a compiled-time unsigned integral value.

**Example**
```c++
template<class Value>
struct My_tree
{
  template<unsigned index>
  static Index make_index() noexcept
  {
    return static_cast<Index>(index);
  }
};
```
This method **must** be `static`.

## Turning the Tree into Offset Tree
### Defining `Offset` type
To make an offset tree you have to define `Offset` type inside your tree
class.

**Example**
```c++
using My_offset = double;

template<class Value>
struct My_tree
{
  using Offset = My_offset;
};
```
Your can use any arithmetic type for the offset, or even a user-defined class that
behaves like an arithmetic type.

### Storing offset per node
Each node of an offset tree has to store a value of type `Offset`.

**Example**
```c++
template<class Value>
struct My_node
{
  My_offset offset = -1.0;
};
```
> **Note!** The value stored inside the node is not the node's real offset.
> It is so-called implicit offset that is used for computing the real offset. 

### Methods to access and modify offsets stored in nodes
An offset tree class has to implement methods for accessing and modifying
offset values stored within nodes.

**Example**
```c++
template<class Value>
struct My_tree
{
  Offset const& offset(My_node<Value> const& node) noexcept
  {
    return node.offset;
  }

  void set_offset(My_node<Value>& node, Offset const& offset) noexcept
  {
    node.offset = offset;
  }

  void add_to_offset(My_node<Value>& node, Offset const& increment) noexcept
  {
    node.offset += increment;
  }

  void subtract_from_offset(My_node<Value>& node, Offset const& decrement) noexcept
  {
    node.offset -= decrement;
  }
};
```
These methods might be `static`, but this is not mandated.

### Method to instantiate `Offset`
An offset tree class has to implement a `static` method that instantiates `Offset`
with a compiled-time unsigned integral value.

**Example**
```c++
template<class Value>
struct My_tree
{
  template<unsigned offset>
  static Offset make_offset() noexcept
  {
    return static_cast<Offset>(offset);
  }
};
```
This method **must** be `static`.  
In fact Tree++ instantiates this template method with only `0` value.

> **Please note** that an offset tree **can** be indexed. There is no problem
> combining these two pieces of functionality in ono tree class. To make an
> indexed offset tree you have to define both `Index` and `Offset` types within
> your tree class and implement all the methods described
> [here](#making-the-tree-indexed) and [here](#turning-the-tree-into-offset-tree).

## Custom Node pointer class
As has already been stated node pointer type can be a user defined class.
Such a class must be:

* Default constructible
* Copy constructible
* Constructible with `nullptr`
* Destructible
* Copy assignable
* Assignable from `nullptr`
* Explicitly convertible to `bool`; used to detect whether a pointer is null
* Equally comparable

**Custom Node pointer Example**
```c++
template<class T>
struct Pointer
{
  Pointer() = default;

  Pointer(std::nullptr_t const) noexcept :
    real_address_blah_blah(nullptr)
  {}

  // Must be const!
  explicit operator bool() const noexcept
  {
    return real_address_blah_blah ? true : false;
  }

  Pointer& operator =(std::nullptr_t const) noexcept
  {
    real_address_blah_blah = nullptr;
    return *this;
  }

  // Must be capable of comparing const values!
  friend bool operator ==(Pointer const& x, Pointer const& y) noexcept
  {
    return x.real_address_blah_blah == y.real_address_blah_blah;
  }

  // This field may be named however you want,
  // the library  will never try to access it.
  T* real_address_blah_blah;
};
```
As you see the pointer class does not even need to be dereferencable. Tree++
never tries to dereference a pointer. Instead, it invokes the method `address`
of the [tree class](#tree-class) to obtain addresses of nodes.

> **Note!** User-defined node pointers allow the nodes to reside in movable
> memory. That is, two invocations of the `address` methods with the same node
> pointer can return different addresses. But please exercise caution when
> working with such trees. A library function may, for example, obtain the
> address of the root node by invoking the `address` method and then use the
> obtained address multiple times until the function returns. That is, nodes of
> a tree must not be moved in memory while an invocation of a Tree++ function
> is being carried out on that tree. Moving the nodes between calls to library
> functions is safe, because each call invokes the `address` method and obtains
> the actual address.

# Algorithms
* [Insertion](#insertion)
* [Erasure](#erasure)
* [Cleanup](#cleanup)
* [Shift](#shift)
* [Lookup](#lookup)
* [Node Queries](#node-queries)
* [Navigation](#navigation)

## Insertion
* [Lookup and insert](#lookup-and-insert)
* [Insert before another Node](#insert-before-another-node)
* [Insert at index](#insert-at-index)
* [Insert at offset](#insert-at-offset)
* [Push back](#push-back)
* [Push front](#push-front)

### Lookup and insert
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<class Tree, class Compare, class Create_node>
auto treexx::bin::avl::Tree_algo::try_insert(
  Tree&& tree,
  Compare&& compare,
  Create_node&& create_node) -> Node_pointer<Tree>;
```
> Applicable only if the `tree` is **not** an offset tree.

Searches for a matching node using the provided `compare` function object.
If the node is found in the `tree`, returns a pointer to that node.
If no matching node is found, creates the node by invoking the `create_node`
function object, inserts the newly created node at the spot where it should
reside and returns a pointer to that node.

`compare` is a unary comparator. It is invoked with a reference to the node
being examined for matching. It must return something that is implicitly
convertible to [`treexx::Compare_result`](#compare_result).

`create_node` gets invoked if and only if the matching node is **not** found.
The invocation parameters are a pointer to the parent node and the side of
the node being created. `create_node` must return a pointer to the created
node, or something that is implicitly convertible to it.

**Complexity**  
Logarithmic in the size of the `tree`.

**Example**
```c++
using Value = int;
using Node = My_node<Value>;
using Node_pointer = My_node_pointer<Value>;
using treexx::Compare_result;
using treexx::bin::Side;
using treexx::bin::avl::Tree_algo;

My_tree<Value> tree;
Value const value = 8377;

auto const compare = [&value](Node const& node) noexcept -> Compare_result
{
  if(node.value < value)
  {
    return Compare_result::less;
  }
  return value < node.value ?
    Compare_result::greater : Compare_result::equal;
};

bool inserted = false;
auto const create_node = [&value, &inserted](
  Node_pointer const& parent, Side const side) -> Node_pointer
{
  auto* const node = new Node;
  node->parent = parent;
  node->value = value;
  node->side = side;
  inserted = true;
  return node;
};

Tree_algo::try_insert(tree, compare, create_node);

if(inserted)
{
  std::cout << "The value " << value << " has just been inserted";
}
else
{
  std::cout << "The value " << value << " already exists";
}
```
### Insert before another Node
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<class Tree>
void treexx::bin::avl::Tree_algo::insert(
  Tree&& tree,
  Node_pointer<Tree> const& spot_ptr,
  Node_pointer<Tree> const& node_ptr) noexcept;
```
> Applicable only if the `tree` is **not** an offset tree.

Inserts the node pointed to by `node_ptr` right before the node pointed to
by `spot_ptr`. The later one must be present in the `tree`, otherwise the
behavior is undefined. The node being inserted must have been allocated
beforehand. If `node_ptr` points to a node that is already present in the
`tree` the behavior is undefined.

**Complexity**  
Logarithmic in the size of the `tree`.

### Insert at index
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<class Tree>
void treexx::bin::avl::Tree_algo::insert_at_index(
  Tree&& tree,
  Node_pointer<Tree> const& node_ptr,
  Index<Tree>::Type const& index) noexcept;
```
> Applicable only if the `tree` is indexed and is **not** an offset tree.

Inserts the node pointed to by `node_ptr` at the specified `index`. The `index`
must be less than the size of the `tree`. The node being inserted must have
been allocated beforehand. If `node_ptr` points to a node that is already
present in the `tree` the behavior is undefined.

**Complexity**  
Logarithmic in the size of the `tree`.

### Insert at offset
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<class Tree>
void treexx::bin::avl::Tree_algo::insert_at_offset(
  Tree&& tree,
  Node_pointer<Tree> const& node_ptr,
  Offset<Tree> const& offset) noexcept;

template<class Tree>
void treexx::bin::avl::Tree_algo::insert_at_offset(
  Tree&& tree,
  Node_pointer<Tree> const& node_ptr,
  Offset<Tree> const& offset,
  Offset<Tree> const& shift) noexcept;
```
> Applicable only if the `tree` is an offset tree.

Inserts the node pointed to by `node_ptr` at the specified `offset`. The second
version of this function accepts an additional parameter `shift` that instructs
to right-shift all the nodes to the right of the `offset` by the specified
value. The node being inserted must have been allocated beforehand. If
`node_ptr` points to a node that is already present in the `tree` the behavior
is undefined. If a node with the specified `offset` is already present in the
`tree`, a positive `shift` must be provided. In such a case the node that
previously resided at the `offset` (as well as all the nodes to the right of
it) will be right-shifted by the value of `shift`. Otherwise, the behavior is
undefined.

### Push back
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<class Tree>
void treexx::bin::avl::Tree_algo::push_back(
  Tree&& tree,
  Node_pointer<Tree> const& node_ptr) noexcept;
```
> Applicable only if the `tree` is **not** an offset tree.

```c++
template<class Tree>
void treexx::bin::avl::Tree_algo::push_back(
  Tree&& tree,
  Node_pointer<Tree> const& node_ptr
  Offset<Tree> const& offset) noexcept;
```
> Applicable only if the `tree` is an offset tree.

Inserts the node pointed to by `node_ptr` at the rightmost position in the
`tree`. If the `tree` is an offset tree, an `offset` must be provided
that specifies the offset of the node being inserted **relative** to the offset
of the current rightmost node. If the `tree` is empty the `offset` is treated
as the absolute offset of the first node being inserted into the tree. If the
`tree` is not empty and the `offset` is not positive the behavior is undefined.  
The node being inserted must have been allocated beforehand. If `node_ptr`
points to a node that is already present in the `tree` the behavior is
undefined.

> **Note!** Draw attention at the fact that the specified `offset` is not an
> absolute offset. This delivers us from the need to compute the offset and
> decreases the computation complexity.

**Complexity**  
Amortized constant. Logarithmic in the size of the `tree` in the worst case.

**Example with offset tree**
```c++
using Value = int;
using Offset = My_offset;
using Node = My_node<Value>;
using treexx::bin::avl::Tree_algo;

auto const push_back_offsets = {-10.5, 5.0, 8.5};

for(auto const& offset: push_back_offsets)
{
  auto node_ptr = std::make_unique<Node>();
  node_ptr->value = static_cast<Value>(offset * 2.0);
  Tree_algo::push_back(tree, node_ptr.release(), offset);
}

Tree_algo::for_each(
  tree,
  [&tree](Node& node)
  {
    Offset const node_offset = Tree_algo::node_offset(tree, node);
    std::cout <<
      "Node at offset " << node_offset <<
      " has value " << node.value << std::endl;
  });
```
**Output**
```
Node at offset -10.5 has value -21
Node at offset -5.5 has value 10
Node at offset 3 has value 17
```

### Push front
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<class Tree>
void treexx::bin::avl::Tree_algo::push_front(
  Tree&& tree,
  Node_pointer<Tree> const& node_ptr) noexcept;
```
> Applicable only if the `tree` is **not** an offset tree.

Inserts the node pointed to by `node_ptr` at the leftmost position in the
`tree`. The node being inserted must have been allocated beforehand.
If `node_ptr` points to a node that is already present in the `tree` the behavior
is undefined.

**Complexity**  
Amortized constant. Logarithmic in the size of the `tree` in the worst case.

## Erasure
* [Erase](#erase)
* [Pop back](#pop-back)
* [Pop front](#pop-front)

### Erase
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<class Tree>
void treexx::bin::avl::Tree_algo::erase(
  Tree&& tree,
  Node_pointer<Tree> const& node_ptr) noexcept;
```
Erases the node pointed to by `node_ptr`. The node must be present in the
`tree`, otherwise the behavior is undefined.

**Complexity**  
Amortized constant. Logarithmic in the size of the `tree` in the worst case.

### Pop back
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<class Tree>
auto treexx::bin::avl::Tree_algo::pop_back(
  Tree&& tree) noexcept -> Node_pointer<Tree>;
```
Erases the rightmost node from the `tree` and returns a pointer to that node.
If the `tree` is empty the behavior is undefined.

**Complexity**  
Amortized constant. Logarithmic in the size of the `tree` in the worst case.

### Pop front
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<class Tree>
auto treexx::bin::avl::Tree_algo::pop_front(
  Tree&& tree) noexcept -> Node_pointer<Tree>;
```
Erases the leftmost node from the `tree` and returns a pointer to that node.
If the `tree` is empty the behavior is undefined.

**Complexity**  
Amortized constant. Logarithmic in the size of the `tree` in the worst case.

## Cleanup
Defined in header `<treexx/bin/tree_algo.hh>`
```c++
template<class Tree, class Destroy>
void treexx::bin::Tree_algo::clear(
  Tree&& tree,
  Destroy&& destroy);
```
Traverses the `tree` visiting each node and invoking the `destroy` function
object with pointers to the visited nodes. After the invocation of `destroy`
with a pointer to a node, that node is never accessed during this call,
therefore it is safe to deallocate the node inside the call to `destroy`.

Since this function can be applied not only to an AVL, but to any binary tree,
it is placed inside the class `treexx::bin::Tree_algo`.

**Complexity**  
Linear in the size of the `tree`.

**Example**
```c++
#include <treexx/bin/tree_algo.hh>

template<class Value>
struct My_tree
{
  void clear() noexcept
  {
    treexx::bin::Tree_algo::clear(
      *this,
      [](auto const& node_ptr)
      {
        delete node_ptr;
      });
    root_node_ptr_ = nullptr;
    leftmost_node_ptr_ = nullptr;
    rightmost_node_ptr_ = nullptr;
  }
};
```

## Shift
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<Side side, class Tree>
void treexx::bin::avl::Tree_algo::shift_suffix(
  Tree&& tree,
  Node<Tree>& node,
  Offset<Tree> const& shift) noexcept;
  
template<class Tree>
void treexx::bin::avl::Tree_algo::shift_suffix(
  Tree&& tree,
  Node<Tree>& node,
  Offset<Tree> const& shift,
  Side side) noexcept;
```
> Applicable only if the `tree` is an offset tree.

Shifts all nodes starting from the `node` in the direction specified by `side`,
either to the left or to the right. If the `node` is not present in the
`tree` the behavior is undefined.

**Complexity**  
Logarithmic in the size of the `tree`.

> **Note!** This function can break your offset tree if used incorrectly. When
> moving nodes too far to the left (or to the right by a negative `shift` value,
> which effectively means to the left) the ordering of the tree can get broken.
> If the shift results in a reordering of the nodes or two nodes at the same
> offset, the behavior is undefined.

## Lookup
* [Lookup by index](#lookup-by-index)
* [Comparator](#comparator)
* [Binary search](#binary-search)
* [Lower bound](#lower-bound)
* [Upper bound](#upper-bound)

### Lookup by index
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<class Tree>
auto treexx::bin::avl::Tree_algo::at_index(
  Tree&& tree,
  Index<Tree> const& index) -> Node_pointer<Tree>;
```
> Applicable only if the `tree` is indexed.

Searches for a node with the specified `index`. Returns a pointer to the node
if it is found, otherwise returns null pointer.

**Complexity**  
Logarithmic in the size of the `tree`.

### Comparator
The lookup functions accept comparators. A Comparator is a function object that
returns an object of type [treexx::Compare_result](#compare_result) or something
that is implicitly convertible to such object. The comparator is invoked with
one, two or three arguments depending on the compile-time flags provided to the
lookup function. These flags are:

* `with_node` - The comparator's first argument is a reference to the node
being examined.
* `with_index` - The comparator's next argument is the node's index. This is
the real (computed) index of the node, not just the value stored within it.
* `with_offset` - The comparator's last argument is the node's offset. This is
the real (computed) offset of the node, not just the value stored within it.

Of course `with_index` and `with_offset` flags apply only to indexed and offset
trees respectively. Setting `with_index` to `true` when searching in a not
indexed tree results in a compile-time error. The similar rule applies to the
`with_offset` flag.

### Binary search
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<
  bool with_node = true,
  bool with_index = false,
  bool with_offset = false,
  class Tree,
  class Compare>
auto treexx::bin::avl::Tree_algo::binary_search(
  Tree&& tree,
  Compare&& compare) -> Node_pointer<Tree>;
```
Searches for a node that compares equally, according to the `compare`
comparator. If such a node is found, returns a pointer to it, otherwise returns
null pointer. If the `tree` contains multiple nodes that fulfil the search
criterion, returns a pointer to any of them. Meaning of the parameters
`with_node`, `with_index` and `with_offset` is as described in the
[Comparator](#comparator) subsection.

**Complexity**  
Logarithmic in the size of the `tree`.

**Search by value example**
```c++
#include <treexx/bin/avl/tree_algo.hh>

template<class Tree, class T>
auto find_by_value(Tree& tree, T const& x)
{
  using treexx::Compare_result;

  return treexx::bin::avl::Tree_algo::binary_search(
    tree,
    [&x](auto const& node) -> Compare_result
    {
      if(node.value < x)
      {
        return Compare_result::less;
      }
      return x < node.value ?
        Compare_result::greater :
        Compare_result::equal;
    });
}
```
**Search by offset example**
```c++
#include <treexx/bin/avl/tree_algo.hh>

template<class Tree, class T>
auto find_by_offset(Tree& tree, T const& offset)
{
  using treexx::Compare_result;

  return treexx::bin::avl::Tree_algo::binary_search<false, false, true>(
    tree,
    [&offset](auto const& node_offset) -> Compare_result
    {
      if(node_offset < offset)
      {
        return Compare_result::less;
      }
      return offset < node_offset ?
        Compare_result::greater :
        Compare_result::equal;
    });
}
```
> **Note!** Do not use the exact search by offset if the offset has a floating
> point type. The computed real offset might slightly differ from the correct
> value because of the rounding error. Prefer [Lower bound](#lower-bound) or
> employ an approximate comparison instead of the equality operator.

### Lower bound
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<
  bool with_node = true,
  bool with_index = false,
  bool with_offset = false,
  bool unique = false,
  class Tree,
  class Compare>
auto treexx::bin::avl::Tree_algo::lower_bound(
  Tree&& tree,
  Compare&& compare) -> Node_pointer<Tree>;
```
Searches for the leftmost node that compares equally or greater, according to
the `compare` comparator. If such a node is found, returns a pointer to it,
otherwise returns null pointer. Meaning of the parameters
`with_node`, `with_index` and `with_offset` is as described in the
[Comparator](#comparator) subsection. The `unique` hint if set to `true`
indicates that there are no duplicates in the `tree`. This is a performance
hint. If `unique` is `true` and `lower_bound` finds a node that compares
equally, the search terminates and a pointer to that node is returned. Be
careful with this hint. If it is set and the `tree` contains multiple nodes
that fulfil the search criterion, a pointer to any of these nodes may be
returned, whereas the correct behavior would be to return the leftmost one.

**Complexity**  
Logarithmic in the size of the `tree`.

**Search by offset example**
```c++
#include <treexx/bin/avl/tree_algo.hh>

template<class Tree, class T>
auto lower_bound_by_offset(Tree& tree, T const& offset)
{
  using treexx::Compare_result;

  // The `unique` hint is set to true because each node has a unique offset.
  return treexx::bin::avl::Tree_algo::lower_bound<false, false, true, true>(
    tree,
    [&offset](auto const& node_offset) -> Compare_result
    {
      if(node_offset < offset)
      {
        return Compare_result::less;
      }
      return offset < node_offset ?
        Compare_result::greater :
        Compare_result::equal;
    });
}
```

### Upper bound
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<
  bool with_node = true,
  bool with_index = false,
  bool with_offset = false,
  class Tree,
  class Compare>
auto treexx::bin::avl::Tree_algo::upper_bound(
  Tree&& tree,
  Compare&& compare) -> Node_pointer<Tree>;
```
Searches for the leftmost node that compares greater, according to the
`compare` comparator. If such a node is found, returns a pointer to it,
otherwise returns null pointer. Meaning of the parameters
`with_node`, `with_index` and `with_offset` is as described in the
[Comparator](#comparator) subsection.

**Complexity**  
Logarithmic in the size of the `tree`.

## Node Queries
* [Query node index](#query-node-index)
* [Query node offset](#query-node-offset)

### Query node index
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<class Tree>
auto treexx::bin::avl::Tree_algo::node_index(
    Tree&& tree,
    Node<Tree>& node) noexcept -> Index<Tree>;
```
> Applicable only if the `tree` is indexed.

Computes the index of the specified `node`. If the `node` is not
present in the `tree` the behavior is undefined.

**Complexity**  
Logarithmic in the size of the `tree`.

### Query node offset
Defined in header `<treexx/bin/avl/tree_algo.hh>`
```c++
template<class Tree>
auto treexx::bin::avl::Tree_algo::node_offset(
    Tree&& tree,
    Node<Tree>& node) noexcept -> Offset<Tree>;
```
> Applicable only if the `tree` is an offset tree.

Computes the offset of the specified `node`. If the `node` is not
present in the `tree` the behavior is undefined.

**Complexity**  
Logarithmic in the size of the `tree`.

## Navigation
The functions described in this subsection can be applied not only to an AVL,
but to any binary tree, therefore they are placed inside the class
`treexx::bin::Tree_algo`.

* [Go to next/previous node](#go-to-nextprevious-node)
* [For each](#for-each)

### Go to next/previous node
Defined in header `<treexx/bin/tree_algo.hh>`
```c++
template<class Tree>
auto treexx::bin::Tree_algo::next_node(
  Tree&& tree,
  Node<Tree>& node) noexcept -> Node_pointer<Tree>;

template<class Tree>
auto treexx::bin::Tree_algo::previous_node(
  Tree&& tree,
  Node<Tree>& node) noexcept -> Node_pointer<Tree>;

template<Side side, class Tree>
auto treexx::bin::Tree_algo::adjacent_node(
  Tree&& tree,
  Node<Tree>& node) noexcept -> Node_pointer<Tree>;
```
Returns a pointer to the next or the previous node. If such node does not exist
a null pointer is returned. If the `node` is not present in the `tree` the
behavior is undefined.  
The function `adjacent_node` with `side` parameter set to `Side::left` is
equivalent to `previous_node`. With `side` set to `Side::right` it is
equivalent to `next_node`.

**Complexity**  
Amortized constant. Logarithmic in the size of the `tree` in the worst case.

### For each
Defined in header `<treexx/bin/tree_algo.hh>`
```c++
template<class Tree, class Fun>
void treexx::bin::Tree_algo::for_each(
  Tree&& tree,
  Fun&& fun);
```
Visits each node of the `tree` in ascending order (i.e. from left to right) and
invokes the `fun` function object with a reference to the visited node. 
```c++
template<class Tree, class Fun>
void treexx::bin::Tree_algo::for_each_backward(
  Tree&& tree,
  Fun&& fun);
```
Visits each node of the `tree` in descending order (i.e. from right to left) and
invokes the `fun` function object with a reference to the visited node. 

**Complexity**  
Linear in the size of the `tree`.
