#include <cassert>
#include <memory>
#include <ostream>
#include <set>
#include <vector>

#include "set/kdtree.hh"
#include "vector/vector.hh"
vector_vector vtov (const std::vector<int>& v) {
  vector_vector out (v.size ());

  for (size_t i = 0; i < v.size (); ++i)
    out[i] = v[i];
  return out;
}

int main() {
  const size_t dim = 3;
  vector_vector v1(dim);
  v1[0] = 1; v1[1] = 2; v1[2] = 3;
  vector_vector v2(dim);
  v2[0] = 2; v2[1] = 5; v2[2] = 1;
  vector_vector v3(dim);
  v3[0] = 4; v3[1] = 1; v3[2] = 1;

  // std::cout << "We have individual vectors!" << std::endl;

  std::vector<vector_vector> elements;
  elements.emplace_back(v1);
  elements.emplace_back(v2);
  elements.emplace_back(v3);

  /*
   std::cout << "We have a vector of vectors now! ";
   for (auto e : elements)
   std::cout << e << " ";
   std::cout << std::endl;
   */

  set::kdtree<vector_vector> set_one_elt ({v1});
  set_one_elt.union_with (set_one_elt);
  assert (set_one_elt.contains (v1));
  assert (not set_one_elt.contains (v2));
  assert (set_one_elt.size () == 1);
  set_one_elt.intersect_with (set_one_elt);
  assert (set_one_elt.size () == 1);
  set_one_elt = set_one_elt.apply ([] (const vector_vector& v) { return v; });
  assert (set_one_elt.size () == 1);
  assert (set_one_elt.contains (v1));
  assert (not set_one_elt.contains (v2));

  set::kdtree<vector_vector> set (std::move (elements));
  set.union_with (set);
  set.intersect_with (set);
  set = set.apply ([] (const vector_vector& v) { return v; });
  assert (set.size () == 3);

  // std::cout << "We built the kdtree!" << std::endl;

  vector_vector v4(dim);
  v4[0] = 0; v4[1] = 1; v4[2] = 2;
  vector_vector v5(dim);
  v5[0] = 1; v5[1] = 1; v5[2] = 10;

  /*
   std::cout << "Is " << v2 << " in the closure? " << set.contains(v2) << std::endl;
   std::cout << "Is " << v4 << " in the closure? " << set.contains(v4) << std::endl;
   std::cout << "Is " << v5 << " in the closure? " << set.contains(v5) << std::endl;
   */


  assert(set.contains(v2));
  assert(set.contains(v4));
  assert(!set.contains(v5));


  std::vector<vector_vector> others;
  others.emplace_back(v4);
  others.emplace_back(v5);

  set::kdtree<vector_vector> other_set (std::move (others));

  /*
   std::cout << "We built another kdtree! for ";
   for (auto e : others)
   std::cout << e << " ";
   std::cout << std::endl;
   */

  set.union_with (other_set);

  /*
   std::cout << "We built a union kdtree of size " << set.size () << std::endl;
   std::cout << "Is " << v2 << " in the union? " << set.contains (v2) << std::endl;
   std::cout << "Is " << v4 << " in the union? " << set.contains (v4) << std::endl;
   std::cout << "Is " << v5 << " in the union? " << set.contains (v5) << std::endl;
   */
  assert(set.contains(v2));
  assert(set.contains(v4));
  assert(set.contains(v5));

  other_set.intersect_with(set);

  /*
   std::cout << "We built the intersection of last two sets, size = "
   << other_set.size () << std::endl;

   std::cout << "Is " << v2 << " in the intersection? " << other_set.contains (v2) << std::endl;
   std::cout << "Is " << v4 << " in the intersection? " << other_set.contains (v4) << std::endl;
   std::cout << "Is " << v5 << " in the intersection? " << other_set.contains (v5) << std::endl;
   */
  assert(!other_set.contains(v2));
  assert(other_set.contains(v4));
  assert(other_set.contains(v5));


  {
    auto tree = utils::kdtree<vector_vector> (std::vector<vector_vector> ({
      vtov ({7, 0, 9, 9, 7}),
      vtov ({8, 0, 7, 7, 8}),
      vtov ({8, 0, 9, 9, 8}),
      vtov ({9, 0, 7, 7, 9})
        }), 5);
    assert (tree.dominates (vtov ({7, 0, 9, 9, 7}), true));
  }


  auto F1i = set::kdtree<vector_vector> (std::vector<vector_vector> ({
        vtov ({7, 0, 9, 9, 7}),
        vtov ({8, 0, 9, 9, 8}),
        vtov ({9, 0, 7, 7, 9})
      }));
  auto F = set::kdtree<vector_vector> (std::vector<vector_vector> ({
        vtov ({7, 0, 9, 9, 7}),
        vtov ({8, 0, 9, 9, 8}),
        vtov ({9, 0, 7, 7, 9})
      }));

  F.intersect_with (F1i);
  assert (F.size () == 2);

  return 0;
}
