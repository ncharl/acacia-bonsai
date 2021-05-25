#pragma once
#include <bitset>

namespace vectors {

  static constexpr auto nbools_to_nbitsets (size_t nbools) {
    return (nbools + sizeof (unsigned long) * 8 - 1) / (sizeof (unsigned long) * 8);
  }

  static constexpr auto nbitsets_to_nbools (size_t nbitsets) {
    return nbitsets * sizeof (unsigned long) * 8;
  }

  template <typename X, size_t NBitsets>
  class X_and_bitset {
      using self = X_and_bitset<X, NBitsets>;

      static constexpr auto Bools = nbitsets_to_nbools (NBitsets);
    public:
      using value_type = X::value_type;

      X_and_bitset (const std::vector<value_type>& v) :
        k {v.size ()},
        local_bool_threshold {std::max (std::min (bool_threshold, k), k - std::min (Bools, k))},
        x {std::span (v.data (), local_bool_threshold)}
      {
        for (size_t i = local_bool_threshold; i < k; ++i) {
          if ((bools[i - local_bool_threshold] = (v[i] + 1)) == true)
            sum++;
        }
      }

      size_t size () const { return k; }

      X_and_bitset (self&& other) = default;

    private:

      X_and_bitset (size_t k, size_t local_bool_threshold, X&& x, std::bitset<Bools>&& bools, size_t sum) :
        k {k},
        local_bool_threshold {local_bool_threshold},
        x {std::move (x)},
        bools {std::move (bools)},
        sum {sum}
      {}

    public:

      // explicit copy operator
      self copy () const {
        std::bitset<Bools> b = bools;
        return X_and_bitset (k, local_bool_threshold, x.copy (), std::move (b), sum);
      }

      self& operator= (self&& other) {
        assert (other.k == k);
        x = std::move (other.x);
        bools = std::move (other.bools);
        return *this;
      }

      self& operator= (const self& other) = delete;

      static size_t capacity_for (size_t elts) {
        if (bool_threshold > elts) [[unlikely]] // No boolean state.
          return X::capacity_for (elts);
        return std::max (elts,
                         X::capacity_for (elts - std::min (Bools, elts - bool_threshold)));
      }

      void to_vector (std::span<value_type> v) const {
        x.to_vector (std::span (v.data (), local_bool_threshold));
        for (size_t i = local_bool_threshold; i < k; ++i)
          v[i] = bools[i - local_bool_threshold] - 1;
      }

      class po_res {
        public:
          po_res (const self& lhs, const self& rhs) {
            bgeq = (lhs.sum >= rhs.sum);
            bleq = (lhs.sum <= rhs.sum);

            if (bgeq or bleq) {
              auto diff = lhs.bools | rhs.bools;
              bgeq = bgeq and (diff == lhs.bools);
              bleq = bleq and (diff == rhs.bools);
            }

            if (not bgeq and not bleq)
              return;

            auto po = lhs.x.partial_order (rhs.x);
            bgeq = bgeq and po.geq ();
            bleq = bleq and po.leq ();
          }

          inline bool geq () {
            return bgeq;
          }

          inline bool leq () {
            return bleq;
          }
        private:
          bool bgeq, bleq;
      };

      inline auto partial_order (const self& rhs) const {
        assert (rhs.k == k);
        return po_res (*this, rhs);
      }

      bool operator== (const self& rhs) const {
        return sum == rhs.sum and bools == rhs.bools and x == rhs.x;
      }

      bool operator!= (const self& rhs) const {
        return sum != rhs.sum or bools != rhs.bools or x != rhs.x;
      }

      value_type operator[] (size_t i) const {
        if (i >= local_bool_threshold)
          return bools[i - local_bool_threshold] - 1;
        return x[i];
      }

    public:
      self meet (const self& rhs) const {
        assert (rhs.k == k);
        return self (k, local_bool_threshold, x.meet (rhs.x), bools & rhs.bools, sum);
      }

      bool operator< (const self& rhs) const {
        int cmp = std::memcmp (&bools, &rhs.bools, sizeof (bools));
        if (cmp == 0)
          return (x < rhs.x);
        return (cmp < 0);
      }

      auto bin () const {
        auto bitset_bin = sum / (k - local_bool_threshold);

        if constexpr (has_bin<X>::value)
          bitset_bin += x.bin ();

        return bitset_bin;
      }

    private:
      const size_t k, local_bool_threshold;
      X x;
      std::bitset<Bools> bools;
      size_t sum; // The sum of all the elements of bools, seen as 0/1 values.
  };
}

template <class X, size_t Bools>
inline
std::ostream& operator<<(std::ostream& os, const vectors::X_and_bitset<X, Bools>& v)
{
  os << "{ ";
  for (size_t i = 0; i < v.size (); ++i)
    os << (int) v[i] << " ";
  os << "}";
  return os;
}