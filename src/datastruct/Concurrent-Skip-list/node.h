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
public:
  // Stores the key and value for the Node
  pair<int, string> key_value_pair;

  // Stores the reference of the next node until the top level for the node
  array<Node *, L> next {nullptr};

  // Lock to lock the node when modifing it
  mutex node_lock;

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
  string get_value();
  void lock();
  void unlock();
};

template <size_t L>
Node<L>::Node()
{
}

template <size_t L>
Node<L>::Node(int key, size_t top_level):
  top_level(top_level)
{
  key_value_pair = make_pair(key, "");
}

template <size_t L>
Node<L>::Node(int key, string value, size_t top_level):
  top_level(top_level)
{
  key_value_pair = make_pair(key, value);
}

/**
    Returns the key in the node
*/
template <size_t L>
int Node<L>::get_key()
{
  return key_value_pair.first;
}

/**
    Returns the value in the node
*/
template <size_t L>
string Node<L>::get_value()
{
  return key_value_pair.second;
}

/**
    Locks the node
*/
template <size_t L>
void Node<L>::lock()
{
  node_lock.lock();
}

/**
    Unlocks the node
*/
template <size_t L>
void Node<L>::unlock()
{
  node_lock.unlock();
}

template <size_t L>
Node<L>::~Node()
{
}
