#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <array>
#include <utility>
#include <memory>
#include <iostream>

using namespace std;

template <size_t L>
class Node
{
  // Stores the key and value for the Node
  pair<int, string> key_value_pair_;

  // Lock to lock the node when modifing it
  recursive_mutex lock_;

public:
  // Stores the reference of the next node until the top level for the node
  array<shared_ptr<Node>, L> next {nullptr};


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
  unique_lock<recursive_mutex> acquire_and_get()
  {
    return unique_lock<recursive_mutex>(lock_);
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
  key_value_pair_ = make_pair(key, "");
}

template <size_t L>
Node<L>::Node(int key, string value, size_t top_level):
  top_level(top_level)
{
  key_value_pair_ = make_pair(key, value);
}

/**
    Returns the key in the node
*/
template <size_t L>
int Node<L>::get_key()
{
  return key_value_pair_.first;
}

/**
    Returns the value in the node
*/
template <size_t L>
string Node<L>::get_value()
{
  return key_value_pair_.second;
}

template <size_t L>
Node<L>::~Node()
{
}
