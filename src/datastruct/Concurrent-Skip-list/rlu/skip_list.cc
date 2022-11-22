/**
    Implements the Concurrent Skip list data structure with insert, delete,
   search and range operations
*/

#include "skip_list.hh"
#include <iostream>
#include <limits>
#include <map>
#include <cstdio>
#include <cstdlib>

constexpr const auto INT_MINI = numeric_limits<int>::min();
constexpr const auto INT_MAXI = numeric_limits<int>::max();

/**
    Constructor
*/
SkipList::SkipList()
{
  // MAX_LEVEL = (int) round(log(max_elements) / log(1 / prob)) - 1;
  head_ = new node_t(INT_MINI, MAX_LEVEL);
  tail_ = new node_t(INT_MAXI, MAX_LEVEL);

  for (auto iter = head_->next.begin(); iter != head_->next.end(); iter++)
    {
      *iter = tail_;
    }
}

/**
    Finds the predecessors and successors at each level of where a given key
   exists or might exist. Updates the references in the vector using pass by
   reference. Returns -1 if not the key does not exist.
*/
int SkipList::find(int key, vector<node_ptr> &predecessors, vector<node_ptr> &successors)
{
  int found = -1;
  auto prev = head_;

  for(int level = MAX_LEVEL - 1; level >= 0; level--)
    {
      auto curr = prev->next[level];

      while(key > curr->get_key())
        {
          prev = curr;
          curr = prev->next[level];
        }

      if(found == -1 && key == curr->get_key())
        {
          found = level;
        }

      predecessors[level] = prev;
      successors[level] = curr;
    }
  return found;
}

/**
    Randomly generates a number and increments level if number less than or
   equal to 0.5 Once more than 0.5, returns the level or available max level.
    This decides until which level a new node_t is available.
*/
int SkipList::get_random_level()
{
  size_t l = 0;
  while(static_cast<float>(rand()) / static_cast<float>(RAND_MAX) <= 0.5)
    {
      l++;
    }
  return l >= MAX_LEVEL ? MAX_LEVEL - 1 : l;
}

/**
    Inserts into the Skip list at the appropriate place using locks.
    Return if already exists.
*/
bool SkipList::add(int key)
{
  // Get the level until which the new node must be available
  int top_level = get_random_level();

  // Initialization of references of the predecessors and successors
  vector<node_ptr> preds(MAX_LEVEL + 1, nullptr);
  vector<node_ptr> succs(MAX_LEVEL + 1, nullptr);

  // Keep trying to insert the element into the list. In case predecessors and
  // successors are changed, this loop helps to try the insert again
  while(true)
    {
      // Find the predecessors and successors of where the key must be inserted
      int found = find(key, preds, succs);

      if(found != -1)
        {
          return false;  // already exist
        }

      // All conditions satisfied, create the node_t and insert it as we have all
      // the required locks
      node_ptr new_node = new node_t(key, top_level);

      // Insert
      for(int level = 0; level <= top_level; level++)
        {
          new_node->next[level] = succs[level];
          preds[level]->next[level] = new_node;
        }

      return true;
    }
}

/**
    Performs search to find if a node exists.
    Return value if the key found, else return empty
*/
bool SkipList::search(int key)
{
  // Finds the predecessor and successors
  vector<node_ptr> preds(MAX_LEVEL + 1, nullptr);
  vector<node_ptr> succs(MAX_LEVEL + 1, nullptr);

  int found = find(key, preds, succs);

  return found != -1;
}

/**
    Deletes from the Skip list at the appropriate place using locks.
    Return if key doesn’t exist in the list.
*/
bool SkipList::remove(int key)
{
  // Initialization of references of the predecessors and successors
  vector<node_ptr> preds(MAX_LEVEL + 1, nullptr);
  vector<node_ptr> succs(MAX_LEVEL + 1, nullptr);

  node_ptr victim;
  int top_level = -1;

  // Find the predecessors and successors of where the key to be deleted
  int found = find(key, preds, succs);

  // If found, select the node to delete. else return
  if(found != -1)
    {
      victim = succs[found];
      top_level = victim->top_level;
    }
  else
    return false;

  for(int level = top_level; level >= 0; level--)
    {
      preds[level]->next[level] = victim->next[level];
    }

  delete victim;

  return true;
}

/**
    Display the skip list in readable format
*/
void SkipList::display()
{
  for(size_t i = 0; i < MAX_LEVEL; i++)
    {
      auto temp = head_;
      int count = 0;
      if(!(temp->get_key() == INT_MINI && temp->next[i]->get_key() == INT_MAXI))
        {
          printf("Level %ld  ", i);
          while(temp != nullptr)
            {
              printf("%d -> ", temp->get_key());
              temp = temp->next[i];
              count++;
            }
          cout << endl;
        }
      if(count == 3)
        break;
    }
  printf("---------- Display done! ----------\n\n");
}

SkipList::~SkipList()
{
  for (auto iter = head_; iter != nullptr;)
    {
      auto tmp = iter;
      iter = iter->next[0];
      delete tmp;
    }
}
