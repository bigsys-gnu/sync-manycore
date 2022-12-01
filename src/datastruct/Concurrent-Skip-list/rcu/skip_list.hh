#pragma once

#include <vector>
#include "node.hh"

class SkipList
{
private:
  // Head and Tail of the Skiplist
  node_ptr head_{nullptr};
  node_ptr tail_{nullptr};

  int find(int key, vector<node_ptr> &predecessors, vector<node_ptr> &successors);
  node_ptr remove_impl(int key);

public:
  SkipList();
  ~SkipList();

  // Supported operations
  bool add(int key);
  bool search(int key);
  bool remove(int key);
  vector<int> range(int start_key, int end_key);
  void display();
};
