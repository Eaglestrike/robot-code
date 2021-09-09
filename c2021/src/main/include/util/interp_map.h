#pragma once

#include <map>

#include <frc/geometry/Pose2d.h>

namespace team114 {
namespace c2020 {

template <class Key,                              // map::key_type
          class T,                                // map::mapped_type
          typename KeyInverseInterpolateFunctor,  // (lower_key, upper_key,
                                                  // interp_key) -> double [0,
                                                  // 1)
          typename TInterpolateFunctor,    // (lower_val, upper_val, double [0,
                                           // 1)) -> T
          class Compare = std::less<Key>,  // map::key_compare
          class Alloc =
              std::allocator<std::pair<const Key, T> >  // map::allocator_type
          >
class InterpolatingMap {
   public:
    typedef std::map<Key, T, Compare, Alloc> InnerMap;
    InterpolatingMap(typename InnerMap::size_type max_size)
        : inverse_interp_{}, interp_{}, map_{}, max_size_{max_size} {}
    InnerMap& Inner() { return map_; }
    typename InnerMap::value_type InterpAt(Key key) {
        // gets prev item, if key exists return that item
        auto below_iter = GetLowerOrEqual(key);
        // gets next item, if key exists returns next item
        auto above_iter = map_.upper_bound(key);
        auto end = map_.end();
        if (below_iter == end && above_iter == end) {
            // map is empty, we'd rather not return an past-the-end iter as STL
            // would, and this is a degenerate case, so throw
            throw std::out_of_range{"interp_map is empty"};
        }
        if (above_iter == end) {
            // return map end
            return *below_iter;
        }
        if (below_iter == end) {
            // return map beginning
            return *above_iter;
        }
        // both iterators are valid, interpolate:
        double interp =
            inverse_interp_(below_iter->first, above_iter->first, key);
        const T value = interp_(below_iter->second, above_iter->second, interp);
        typename InnerMap::value_type ret{key, value};
        return ret;
    }

    // naming assumes time-based and that comparator is less
    typename InnerMap::value_type Latest() {
        auto it = map_.rbegin();
        if (it == map_.rend()) {
            throw std::out_of_range{"interp_map is empty"};
        }
        return *it;
    }

    // DOES NOT INTERPOLATE, for insertion only
    T& operator[](const Key&& key) {
        if (map_.size() >= max_size_) {
            map_.erase(map_.begin());
        }
        return map_[key];
    }
    T& operator[](const Key& key) {
        if (map_.size() >= max_size_) {
            map_.erase(map_.begin());
        }
        return map_[key];
    }

    void CheckSize() {
        if (map_.size() > max_size_) {
            map_.erase(map_.begin());
        }
    }

   private:
    auto GetLowerOrEqual(Key key) {
        auto it = map_.upper_bound(key);
        if (it == map_.begin()) {
            // if the map is empty, we find nothing and end == begin, in which
            // case return end, thats covered
            // if our key is below all other keys, our result will be the first
            // element, which is begin in that case there is no lower or equal,
            // so return end
            return map_.end();
        }
        // now we either have:
        // 1. our key was past all other keys, we got one-past-end, want the
        // last element
        // 2. our key was not in the map, we got one-past what we want
        // 3. our key was in the map, we got one-past what we want
        --it;
        return it;
    }

    KeyInverseInterpolateFunctor inverse_interp_;
    TInterpolateFunctor interp_;
    InnerMap map_;
    const typename InnerMap::size_type max_size_;
};

template <typename T>
struct ArithmeticInverseInterp {
    double operator()(T low, T high, T key) {
        T total_diff = high - low;
        T diff = key - low;
        return diff / total_diff;
    }
};

template <typename T>
struct ArithmeticInterp {
    T operator()(T low, T high, double pct) {
        T total_diff = high - low;
        return low + pct * total_diff;
    }
};

struct Pose2dInterp {
    frc::Pose2d operator()(const frc::Pose2d& low, const frc::Pose2d& high,
                           double pct) {
        if (pct <= 0) {
            return low;
        }
        if (pct >= 1) {
            return high;
        }
        frc::Twist2d twist = low.Log(high);
        frc::Twist2d interped_twist;
        interped_twist.dx = pct * twist.dx;
        interped_twist.dy = pct * twist.dy;
        interped_twist.dtheta = pct * twist.dtheta;
        return low.Exp(interped_twist);
    }
};

}  // namespace c2020
}  // namespace team114