#[cfg(test)]
extern crate approx;

use num_traits::Float;

pub trait Interpolatable {
    fn interp(low: Self, high: Self, percent: f64) -> Self;
}

impl Interpolatable for f64 {
    fn interp(low: Self, high: Self, percent: f64) -> Self {
        (low + percent * (high - low))
    }
}

impl Interpolatable for f32 {
    fn interp(low: Self, high: Self, percent: f64) -> Self {
        let percent = percent as f32;
        (low + percent * (high - low))
    }
}

pub trait InverseInterpolatable {
    fn percent_to(value: Self, low: Self, high: Self) -> f64;
}

impl<T: Float + Into<f64>> InverseInterpolatable for T {
    fn percent_to(value: Self, low: Self, high: Self) -> f64 {
        ((value - low) / (high - low)).into()
    }
}

// Float type where NaN is the lowest value and equal to itself
#[derive(Debug, Copy, Clone, PartialOrd)]
pub struct OrdFloat<T: Float>(pub T);

impl<T: Float> PartialEq for OrdFloat<T> {
    fn eq(&self, other: &Self) -> bool {
        (self.0.is_nan() && other.0.is_nan()) || self.0 == other.0
    }
}
impl<T: Float> Eq for OrdFloat<T> {}

use std::cmp::Ordering;
impl<T: Float> Ord for OrdFloat<T> {
    fn cmp(&self, other: &Self) -> Ordering {
        // covers the case that both are NaN
        if self.eq(other) {
            return Ordering::Equal;
        }
        // if only we are NaN we are less
        if self.0.is_nan() {
            return Ordering::Less;
        }
        // if only they are NaN we are greater
        if other.0.is_nan() {
            return Ordering::Greater;
        }
        if self.0 < other.0 {
            Ordering::Less
        } else {
            Ordering::Greater
        }
    }
}

use std::ops::Deref;
impl<T: Float> Deref for OrdFloat<T> {
    type Target = T;
    fn deref(&self) -> &T {
        &self.0
    }
}

impl<T: Float> From<T> for OrdFloat<T> {
    fn from(t: T) -> Self {
        OrdFloat(t)
    }
}

impl<T: Float + Interpolatable> Interpolatable for OrdFloat<T> {
    fn interp(low: Self, high: Self, percent: f64) -> Self {
        OrdFloat(T::interp(low.0, high.0, percent))
    }
}

impl<T: Float + InverseInterpolatable + Into<f64>> InverseInterpolatable for OrdFloat<T> {
    fn percent_to(value: Self, low: Self, high: Self) -> f64 {
        ((value.0 - low.0) / (high.0 - low.0)).into()
    }
}

use std::collections::BTreeMap;
pub struct InterpolatingBTreeMap<K: InverseInterpolatable + Ord + Clone, V: Interpolatable + Clone>
{
    map: BTreeMap<K, V>,
    max_size: usize,
}

impl<K: InverseInterpolatable + Ord + Clone, V: Interpolatable + Clone>
    InterpolatingBTreeMap<K, V>
{
    pub fn new(max_size: usize) -> Self {
        let max_size = max_size.max(1);
        Self {
            map: BTreeMap::new(),
            max_size,
        }
    }

    pub fn insert(&mut self, key: K, value: V) {
        self.map.insert(key, value);
        if self.map.len() > self.max_size {
            let first_key = self.map.keys().next().unwrap().clone();
            self.map.remove(&first_key);
        }
    }

    pub fn get(&self, key: K) -> Option<V> {
        use std::ops::Bound::Included;
        use std::ops::Bound::Unbounded;
        if let Some(val) = self.map.get(&key) {
            return Some(val.clone());
        }
        let floor = self.map.range((Unbounded, Included(&key))).next_back();
        let ceiling = self.map.range((Included(&key), Unbounded)).next();
        match (floor, ceiling) {
            (None, None) => None,
            (Some((_low_key, low_value)), None) => Some(low_value.clone()),
            (None, Some((_high_key, high_value))) => Some(high_value.clone()),
            (Some((low_key, low_value)), Some((high_key, high_value))) => Some(V::interp(
                low_value.clone(),
                high_value.clone(),
                K::percent_to(key.clone(), low_key.clone(), high_key.clone()),
            )),
        }
    }
}

#[cfg(test)]
mod tests {

    use super::*;
    use approx::assert_abs_diff_eq;

    #[test]
    fn test() {
        let mut map = InterpolatingBTreeMap::new(2);
        assert_eq!(map.get((0.0).into()), None);

        map.insert(OrdFloat(0.0f64), 10.0f64);
        map.insert(5.0.into(), 20.0);

        assert_abs_diff_eq!(map.get(2.5.into()).unwrap(), 15.0);
        assert_abs_diff_eq!(map.get(1.25.into()).unwrap(), 12.5);
        assert_abs_diff_eq!(map.get(3.75.into()).unwrap(), 17.5);

        assert_abs_diff_eq!(map.get((-0.1).into()).unwrap(), 10.0);
        assert_abs_diff_eq!(map.get(5.1.into()).unwrap(), 20.0);

        // insert one more, should remove the lowest elem
        map.insert(10.0.into(), -10.0);
        assert_abs_diff_eq!(map.get(4.9.into()).unwrap(), 20.0);
        assert_abs_diff_eq!(map.get(8.33333.into()).unwrap(), 0.0, epsilon = 0.0001);
    }
}
