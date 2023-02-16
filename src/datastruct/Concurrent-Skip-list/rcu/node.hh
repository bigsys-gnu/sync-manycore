#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <array>
#include <utility>

using namespace std;

template <size_t L>
class Node
{
  int key_;

  // Lock to lock the node when modifing it
  recursive_mutex lock_;

public:
  // Stores the reference of the next node until the top level for the node
  array<Node *, L + 1> next {nullptr};


  // Atomic variable to be marked if this Node is being deleted
  atomic<bool> marked = {false};

  // Atomic variable to indicate the Node is completely linked to predecessors and successors
  atomic<bool> fully_linked = {false};

  const size_t top_level{L};

  Node();
  Node(int key, size_t top_level);
  Node(int key, string value, size_t top_level);
  ~Node();
  int get_key();

  recursive_mutex& get_lock()
  {
    return lock_;
  }

};

template <size_t L>
Node<L>::Node()
{
}

template <size_t L>
Node<L>::Node(int key, size_t top_level):
  top_level(top_level)
{
  key_ = key;
}

/**
    Returns the key in the node
*/
template <size_t L>
int Node<L>::get_key()
{
  return key_;
}

template <size_t L>
Node<L>::~Node()
{
}

constexpr const size_t MAX_LEVEL = 20;
using node_t = Node<MAX_LEVEL>;
using node_ptr = node_t *;
