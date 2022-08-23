# Concurrent Skip list

# Why
- Skip list is based on sorted linked list which is very concurrency friendly because:
  - Localized: When you modify or update a node in list, you only need to touch 3 nodes, the edited node itself and 2 adjacent nodes. Localized helps a lot in concurrency environment since you don't have to lock and synchronized the whole data when mutate data structure.

# Compare with Binary search tree
- Skip list and binary search tree both provide O(lgn) on search operation but in the concurrency environment, skip list based on linked list have big advantage regarding mutation ( update, delete, add ).

- Most simple example of usable BST is Red Black Tree, we will examine its mutation operation and its concurrency friendliness. The RBTree stays balance why having a node as red or black and applying rules that optionally rebalancing the tree on each insert or erase to avoid having different branches of the tree become too uneven. Rebalancing the tree is done by rotating subtrees, which involves touching an inserted and erased node's parent and/or uncle node, that node's own parent and/or uncle, and so on to the grandparents and granduncles up to the tree, possibly as far as the root.

As you can see, one single mutation can touch many different nodes up to the root, it lose it's localized character in many cases. In the concurrency env, if we apply a lock to the whole tree, it would provide no concurrency. But if we put a lock in each node, it would cause many problem to be solved such as dead lock, data is not synchronized correctly because the complexity nature of its operation.

# History
- Skiplist was invented in 1978 as an alternative for BST. It was implemented successfully 5 years after the research. And it's considered a better version of binary search tree, and it's being interviewed a lot at Google recently ...

# More
- http://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-579.pdf
