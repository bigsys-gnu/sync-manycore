/**
    Implements the Concurrent Skip list data structure with insert, delete,
   search and range operations
*/

#include "skip_list.h"
#include <iostream>
#include <limits>
#include <map>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

constexpr const auto INT_MIN = numeric_limits<int>::min();
constexpr const auto INT_MAX = numeric_limits<int>::max();

/**
    Constructor
*/
SkipList::SkipList()
{
  // MAX_LEVEL = (int) round(log(max_elements) / log(1 / prob)) - 1;
  head_ = make_shared<node_t>(INT_MIN, MAX_LEVEL);
  tail_ = make_shared<node_t>(INT_MAX, MAX_LEVEL);

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
bool SkipList::add(int key, string value)
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

      // If found and marked, wait and continue insert
      // If found and unmarked, wait until it is fully_linked and return. No
      // insert needed If not found, go ahead with insert
      if(found != -1)
        {
          node_ptr node_found = succs[found];

          if(!node_found->marked)
            {
              while(!node_found->fully_linked)
                {
                }
              return false;
            }
          continue;
        }

      // Store all the Nodes which lock we acquire in a map
      // Map used so that we don't try to acquire lock to a node_t we have already
      // acquired This may happen when we have the same predecessor at different
      // levels
      vector<unique_lock<recursive_mutex>> locks;

      // Traverse the skip list and try to acquire the lock of predecessor at
      // every level
      node_ptr pred;
      node_ptr succ;
      node_ptr pre_pred;

      // Used to check if the predecessor and successors are same from when we
      // tried to read them before
      bool valid = true;

      for(int level = 0; valid && (level <= top_level); level++)
        {
          pred = preds[level];
          succ = succs[level];

          if (pred != pre_pred)
            {
              locks.emplace_back(pred->acquire_and_get());
              pre_pred = pred;
            }

          // If predecessor marked or if the predecessor and successors change,
          // then abort and try again
          valid = !(pred->marked.load(std::memory_order_seq_cst)) &&
            !(succ->marked.load(std::memory_order_seq_cst)) && pred->next[level] == succ;
        }

      // Conditons are not met, release locks, abort and try again.
      if(!valid)
        {
          continue;
        }

      // All conditions satisfied, create the node_t and insert it as we have all
      // the required locks
      node_ptr new_node = make_shared<node_t>(key, value, top_level);

      // Update the predecessor and successors
      for(int level = 0; level <= top_level; level++)
        {
          new_node->next[level] = succs[level];
          preds[level]->next[level] = new_node;
        }

      // Mark the node as completely linked.
      new_node->fully_linked = true;
      return true;
    }
}

/**
    Performs search to find if a node exists.
    Return value if the key found, else return empty
*/
string SkipList::search(int key)
{

  // Finds the predecessor and successors
  vector<node_ptr> preds(MAX_LEVEL + 1, nullptr);
  vector<node_ptr> succs(MAX_LEVEL + 1, nullptr);

  int found = find(key, preds, succs);

  // If not found return empty.
  if(found == -1)
    {
      return "";
    }

  auto curr = succs[found];

  // If found, unmarked and fully linked, then return value. Else return empty.
  if(curr->fully_linked && !curr->marked)
    {
      return curr->get_value();
    }
  else
    {
      return "";
    }
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
  bool is_marked = false;
  int top_level = -1;

  // Keep trying to delete the element from the list. In case predecessors and
  // successors are changed, this loop helps to try the delete again
  while(true)
    {
      // Find the predecessors and successors of where the key to be deleted
      int found = find(key, preds, succs);

      // If found, select the node to delete. else return
      if(found != -1)
        {
          victim = succs[found];
        }
      else
        return false;

      // If node not found and the node to be deleted is fully linked and not
      // marked return
      if(is_marked | (found != -1 && (victim->fully_linked && victim->top_level == size_t(found) && !(victim->marked))))
        {
          unique_lock<recursive_mutex> victim_lock;
          // If not marked, the we lock the node and mark the node to delete
          if(!is_marked)
            {
              top_level = victim->top_level;
              victim_lock = victim->acquire_and_get();
              if(victim->marked)
                {
                  return false;
                }
              victim->marked = true;
              is_marked = true;
            }

          // Store all the Nodes which lock we acquire in a map
          // Map used so that we don't try to acquire lock to a node_t we have already
          // acquired This may happen when we have the same predecessor at different
          // levels
          vector<unique_lock<recursive_mutex>> locks;

          // Traverse the skip list and try to acquire the lock of predecessor at
          // every level
          node_ptr pred;
          node_ptr succ;
          node_ptr pre_pred;

          // Used to check if the predecessors are not marked for delete and if
          // the predecessor next is the node we are trying to delete or if it is
          // changed.
          bool valid = true;

          for(int level = 0; valid && (level <= top_level); level++)
            {
              pred = preds[level];
              succ = succs[level];
              if (pred != pre_pred)
                {
                  locks.emplace_back(pred->acquire_and_get());
                  pre_pred = pred;
                }

              // If predecessor marked or if the predecessor's next has changed,
              // then abort and try again
              valid = !(pred->marked) && pred->next[level] == succ;
            }

          // Conditons are not met, release locks, abort and try again.
          if(!valid)
            {
              continue;
            }

          // All conditions satisfied, delete the node_t and link them to the
          // successors appropriately
          for(int level = top_level; level >= 0; level--)
            {
              preds[level]->next[level] = victim->next[level];
            }

          // delete victim; -> shared ptr automatically remove victim
          return true;
        }
      else
        {
          return false;
        }
    }
}

/**
    Searches for the start_key in the skip list by traversing once we reach a
   point closer to start_key reaches to level 0 to find all keys between
   start_key and end_key. If search exceeds end, then abort Updates and returns
   the key value pairs in a map.
*/
map<int, string> SkipList::range(int start_key, int end_key)
{

  map<int, string> range_output;

  if(start_key > end_key)
    {
      return range_output;
    }

  auto curr = head_;

  for(int level = MAX_LEVEL - 1; level >= 0; level--)
    {
      while(curr->next[level] != nullptr && start_key > curr->next[level]->get_key())
        {
          if(curr->get_key() >= start_key && curr->get_key() <= end_key)
            {
              range_output.insert(make_pair(curr->get_key(), curr->get_value()));
            }
          curr = curr->next[level];
        }
    }

  while(curr != nullptr && end_key >= curr->get_key())
    {
      if(curr->get_key() >= start_key && curr->get_key() <= end_key)
        {
          range_output.insert(make_pair(curr->get_key(), curr->get_value()));
        }
      curr = curr->next[0];
    }

  return range_output;
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
      if(!(temp->get_key() == INT_MIN && temp->next[i]->get_key() == INT_MAX))
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
}
