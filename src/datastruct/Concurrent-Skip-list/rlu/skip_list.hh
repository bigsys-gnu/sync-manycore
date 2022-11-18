#pragma once

#include <map>
#include "node.hh"

class SkipList
{
private:
  // Head and Tail of the Skiplist
  node_ptr head_{nullptr};
  node_ptr tail_{nullptr};

  int find(int key, vector<node_ptr> &predecessors, vector<node_ptr> &successors);
  int get_random_level();

public:
  SkipList();
  ~SkipList();

  // Supported operations
  bool add(int key);
  bool search(int key);
  bool remove(int key);
  void display();
};
