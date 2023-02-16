/**
   Implements the Concurrent Skip list data structure with insert, delete,
   search and range operations
*/

#include "skip_list.hh"
#include "mvrlu_api.hh"
#include "debug.hh"
#include <iostream>
#include <limits>
#include <cstdio>
#include <random>
#include <cassert>

constexpr const auto INT_MINI = std::numeric_limits<int>::min();
constexpr const auto INT_MAXI = std::numeric_limits<int>::max();

SkipList::SkipList()
{
  head_ = new node_t(INT_MINI, MAX_LEVEL);
  tail_ = new node_t(INT_MAXI, MAX_LEVEL);

  for (int i = 0; i <= MAX_LEVEL; i++)
    {
      head_->set_next(tail_, i);
    }
}

/*
 * Must call this function within MV-RLU Session
 */
int SkipList::find(int key, std::vector<deref_ptr> &predecessors,
                   std::vector<deref_ptr> &successors)
{
  int found = -1;
  deref_ptr prev = head_;

  for(int level = MAX_LEVEL; level >= 0; level--)
    {
      auto curr = prev->get_next(level);

      mvrlu_debug::has_zero_value(prev.get());

      while(key > curr->get_key())
        {
          prev = curr;
          curr = prev->get_next(level);
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
int get_random_level()
{
  static thread_local std::mt19937 gen(std::random_device{}());
  std::bernoulli_distribution dis(0.5);

  int level = 0;
  while (dis(gen))
    {
      level++;
    }
  return std::min(int(MAX_LEVEL), level);
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
  std::vector<deref_ptr> preds(MAX_LEVEL + 1);
  std::vector<deref_ptr> succs(MAX_LEVEL + 1);

 restart:
  mvrlu_api::session session; // reader lock and unlock

  // Find the predecessors and successors of where the key must be inserted
  int found = find(key, preds, succs);

  if(found != -1)
    {
      return false;  // already exist
    }

  // create copy every preds and succs
  deref_ptr pre_pred;
  for (int level = top_level; level >= 0; level--)
    {
      if (pre_pred != preds[level])
        {
          pre_pred = preds[level];
          if (!preds[level].try_lock())
            {
              session.abort();
              goto restart;
            }
        }
      else
        {
          preds[level] = preds[level + 1];
        }
    }

  // All conditions satisfied, create the node_t and insert it
  node_ptr new_node = new node_t(key, top_level);

  // Insert
  for(int level = 0; level <= top_level; level++)
    {
      new_node->set_next(succs[level].get(), level);
      preds[level]->set_next(new_node, level);
    }

  return true;
}

/**
   Performs search to find if a node exists.
   Return value if the key found, else return empty
*/
bool SkipList::search(int key)
{
  // Finds the predecessor and successors
  mvrlu_api::session session;

  deref_ptr prev = head_;
  for (int level = MAX_LEVEL; level >= 0; level--)
    {
      auto curr = prev->get_next(level);
      while (key > curr->get_key())
        {
          prev = curr;
          curr = prev->get_next(level);
        }

      if (key == curr->get_key())
        {
          return true;
        }
    }
  return false;
}

/**
   Deletes from the Skip list at the appropriate place using locks.
   Return if key doesn’t exist in the list.
*/
bool SkipList::remove(int key)
{
  // Initialization of references of the predecessors and successors
  std::vector<deref_ptr> preds(MAX_LEVEL + 1);
  std::vector<deref_ptr> succs(MAX_LEVEL + 1);

  deref_ptr victim;
  int top_level = -1;

 restart:
  mvrlu_api::session session;

  // Find the predecessors and successors of where the key to be deleted
  int found = find(key, preds, succs);

  // If found, select the node to delete. else return
  if(found != -1)
    {
      victim = succs[found];
      top_level = victim->get_level();
    }
  else
    return false;

  // create copy for every preds and victim
  if (!victim.try_lock_const())
    {
      session.abort();
      goto restart;
    }

  deref_ptr pre_pred;
  for (int level = top_level; level >= 0; level--)
    {
      if (pre_pred != preds[level])
        {
          pre_pred = preds[level];
          if (!preds[level].try_lock())
            {
              session.abort();
              goto restart;
            }
        }
      else
        {
          preds[level] = preds[level + 1];
        }
    }

  for(int level = top_level; level >= 0; level--)
    {
      preds[level]->set_next(victim->get_next(level).get(), level);
    }

  victim.free();

  return true;
}

SkipList::~SkipList()
{
  // std::size_t node_num = 0;
  // for (auto iter = head_; iter != nullptr;)
  //   {
  //     auto tmp = iter;
  //     iter = iter->next[0];
  //     delete tmp;
  //     node_num++;
  //   }
  // std::cout << "The Number Of Nodes: " << node_num << '\n';
}
