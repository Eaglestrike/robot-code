#![feature(const_fn)]
extern crate dimensioned as dim;

#[cfg(test)]
#[macro_use]
extern crate assert_approx_eq;
use dim::si;
use std::marker::PhantomData;

pub type Meter = dim::si::Meter<f64>;
pub type Radians = f64;

#[derive(Debug, Copy, Clone)]
pub enum Axis {
    X,
    Y,
}

// /// Represents a Rigid transformation in two dimensions, a rotation
#[derive(Debug, Copy, Clone, PartialEq)]
pub struct PointData {
    pos: (Meter, Meter),
    rot: Radians,
}

#[macro_export]
macro_rules! const_unit {
    ($val:expr) => {
        ::dim::si::SI {
            value_unsafe: $val,
            _marker: ::std::marker::PhantomData,
        }
    };
}

impl PointData {
    #[inline]
    pub const fn xyr(x: f64, y: f64, r: f64) -> Self {
        Self {
            pos: (const_unit!(x), const_unit!(y)),
            rot: r,
        }
    }

    #[inline]
    pub(crate) fn inverse_relative_to(&self, other: Self) -> Self {
        // what do I need to transform other to to get where I am
        let s = self.pos;
        let t = other.rot;
        Self {
            pos: (
                s.0 * t.cos() - s.1 * t.sin() + other.pos.0,
                s.0 * t.sin() + s.1 * t.cos() + other.pos.1,
            ),
            rot: self.rot + other.rot,
        }
    }

    #[inline]
    pub(crate) fn invert_parent_child_relation(&self) -> Self {
        let s = self.pos;
        let t = -self.rot;
        Self {
            pos: (
                -s.0 * t.cos() + s.1 * t.sin(),
                -s.0 * t.sin() - s.1 * t.cos(),
            ),
            rot: t,
        }
    }

    #[inline]
    pub fn x(&self) -> Meter {
        self.pos.0
    }

    #[inline]
    pub fn y(&self) -> Meter {
        self.pos.1
    }

    #[inline]
    pub fn pos(&self) -> (Meter, Meter) {
        self.pos
    }

    #[inline]
    pub fn rot(&self) -> Radians {
        self.rot
    }

    #[inline]
    pub fn mirror(&self, axis: Axis) -> Self {
        match axis {
            Axis::X => Self {
                pos: (self.pos.0, -self.pos.1),
                rot: -self.rot,
            },
            Axis::Y => Self {
                pos: (-self.pos.0, self.pos.1),
                rot: std::f64::consts::PI - self.rot,
            },
        }
    }
}

impl std::ops::Add for PointData {
    type Output = PointData;
    fn add(self, rhs: PointData) -> PointData {
        Self {
            pos: (self.pos.0 + rhs.pos.0, self.pos.1 + rhs.pos.1),
            rot: self.rot + rhs.rot,
        }
    }
}

impl Default for PointData {
    #[inline]
    fn default() -> Self {
        Self {
            pos: (0. * si::M, 0. * si::M),
            rot: 0.,
        }
    }
}

#[cfg(test)]
mod point_data_test {
    use super::*;
    extern crate rand;
    use self::rand::{distributions::Uniform, Rng, SeedableRng, XorShiftRng};

    #[test]
    fn frame_relative_tos() {
        let p = PointData {
            pos: (1.7 * si::M, -1.5 * si::M),
            rot: 0.73,
        };
        let frame = PointData {
            pos: (-1.6 * si::M, 0.33 * si::M),
            rot: 0.27,
        };

        let prime = p.inverse_relative_to(frame);
        assert_approx_eq!(prime.pos.0 / si::M, 0.4385, 1e-4);
        assert_approx_eq!(prime.pos.1 / si::M, -0.6622, 1e-4);
    }

    #[test]
    fn invert_parent_child_relation() {
        fn test_point(f: PointData) {
            let fprime = f
                .invert_parent_child_relation()
                .invert_parent_child_relation();
            assert_approx_eq!(fprime.pos.0 / si::M, f.pos.0 / si::M, 1e-4);
            assert_approx_eq!(fprime.pos.1 / si::M, f.pos.1 / si::M, 1e-4);
            assert_approx_eq!(fprime.rot, f.rot, 1e-4);
            // println!("Double inversion passed for {:?}", f);
        }

        let mut rng = XorShiftRng::from_seed([
            123, 243, 121, 35, 31, 76, 87, 123, 243, 121, 35, 205, 76, 87, 9, 14,
        ]);
        let dist = Uniform::new(-100.0, 100.0);
        let mut s = || rng.sample(dist);
        for _ in 0..50000 {
            test_point(PointData {
                pos: (s() * si::M, s() * si::M),
                rot: s(),
            });
        }
    }

    #[test]
    fn inverting_frame_relative_to() {
        fn test_point(p: PointData, frame: PointData) {
            let prime = p
                .inverse_relative_to(frame)
                .inverse_relative_to(frame.invert_parent_child_relation());
            assert_approx_eq!(prime.pos.0 / si::M, p.pos.0 / si::M, 1e-4);
            assert_approx_eq!(prime.pos.1 / si::M, p.pos.1 / si::M, 1e-4);
            assert_approx_eq!(prime.rot, p.rot, 1e-4);
            // println!("Inverse relative to reversal passed for {:?}", p);
        }

        fn test_point_2(p: PointData, frame: PointData, frame2: PointData) {
            let prime = p
                .inverse_relative_to(frame)
                .inverse_relative_to(frame2)
                .inverse_relative_to(frame2.invert_parent_child_relation())
                .inverse_relative_to(frame.invert_parent_child_relation());
            assert_approx_eq!(prime.pos.0 / si::M, p.pos.0 / si::M, 1e-4);
            assert_approx_eq!(prime.pos.1 / si::M, p.pos.1 / si::M, 1e-4);
            assert_approx_eq!(prime.rot, p.rot, 1e-4);
            // println!("Inverse relative to reversal passed for {:?}", p);
        }

        let mut rng = XorShiftRng::from_seed([
            123, 243, 121, 123, 31, 76, 87, 123, 243, 68, 35, 205, 76, 87, 9, 14,
        ]);
        let dist = Uniform::new(-100.0, 100.0);
        let mut s = || rng.sample(dist);
        for _ in 0..50000 {
            test_point(
                PointData {
                    pos: (s() * si::M, s() * si::M),
                    rot: s(),
                },
                PointData {
                    pos: (s() * si::M, s() * si::M),
                    rot: s(),
                },
            );
            test_point_2(
                PointData {
                    pos: (s() * si::M, s() * si::M),
                    rot: s(),
                },
                PointData {
                    pos: (s() * si::M, s() * si::M),
                    rot: s(),
                },
                PointData {
                    pos: (s() * si::M, s() * si::M),
                    rot: s(),
                },
            );
        }
    }
}

pub enum ParentFrame<S: PointHeirarchy + Sized> {
    Root,
    Parent(S),
}

impl<S: PointHeirarchy> ParentFrame<S> {
    #[inline]
    pub fn unwrap(&self) -> S {
        match *self {
            ParentFrame::Root => panic!(),
            ParentFrame::Parent(x) => x,
        }
    }
}

pub trait PointHeirarchy: Sized + Copy + Into<usize> + Eq {
    fn parent(&self) -> ParentFrame<Self>;

    /// find a path between two nodes in a transformation heirarchy
    /// In the return tuple `(a, b)` it is guaranteed that `a.iter().last().parent() == b[1].parent()`
    // TODO(Lytigas) optimize to not go all the way to root each time
    fn path_to(&self, other: Self) -> (Vec<Self>, Vec<Self>) {
        let mut other_up = Vec::new();
        let mut current = other;
        loop {
            other_up.push(current);
            match current.parent() {
                ParentFrame::Root => {
                    break;
                }
                ParentFrame::Parent(x) => {
                    current = x;
                }
            };
        }
        let mut self_up = Vec::new();
        current = *self;
        loop {
            self_up.push(current);
            match current.parent() {
                ParentFrame::Root => {
                    break;
                }
                ParentFrame::Parent(x) => {
                    current = x;
                }
            };
        }
        other_up.reverse();
        (self_up, other_up)
    }

    fn order() -> usize;
}

pub struct FrameRegistry<S: PointHeirarchy>(Vec<PointData>, PhantomData<S>);

impl<S: PointHeirarchy> FrameRegistry<S> {
    #[inline]
    pub fn new() -> Self {
        Self(vec![PointData::default(); S::order()], PhantomData)
    }

    #[inline]
    pub fn raw_tf_mut(&mut self, frame: S) -> &mut PointData {
        &mut self.0[frame.into()]
    }

    #[inline]
    pub fn raw_tf(&self, frame: S) -> PointData {
        self.0[frame.into()]
    }

    /// # Panics
    /// If you attempt to set the origin of a frame whose parent is `ParentFrame::Root`.
    #[inline]
    pub fn set_origin(&mut self, frame: S, p: TfPoint<S>) {
        let data = match frame.parent() {
            ParentFrame::Root => panic!(),
            ParentFrame::Parent(s) => p.in_frame(&self, s).1,
        };

        *self.raw_tf_mut(frame) = data;
    }
}

impl<S: PointHeirarchy> Default for FrameRegistry<S> {
    #[inline]
    fn default() -> Self {
        Self::new()
    }
}

#[derive(Debug, Copy, Clone)]
pub struct TfPoint<S: PointHeirarchy>(S, PointData);

impl<S: PointHeirarchy> TfPoint<S> {
    #[inline]
    pub const fn new(frame: S, x: Meter, y: Meter, rot: Radians) -> Self {
        Self(frame, PointData { pos: (x, y), rot })
    }

    #[inline]
    pub fn in_frame(&self, register: &FrameRegistry<S>, frame: S) -> Self {
        let (up, down) = self.0.path_to(frame);
        let mut result = self.1;
        up.iter()
            .for_each(|x| result = result.inverse_relative_to(register.raw_tf(*x)));
        down.iter().for_each(|x| {
            result = result.inverse_relative_to(register.raw_tf(*x).invert_parent_child_relation())
        });
        Self(frame, result)
    }

    #[inline]
    pub fn from_raw(raw: PointData, frame: S) -> Self {
        Self(frame, raw)
    }

    #[inline]
    pub fn raw_data(&self) -> PointData {
        self.1
    }

    #[inline]
    pub fn frame(&self) -> S {
        self.0
    }

    #[inline]
    pub fn mirror(&self, axis: Axis) -> Self {
        Self::from_raw(self.1.mirror(axis), self.0)
    }
}

impl<S: PointHeirarchy> std::ops::Add<PointData> for TfPoint<S> {
    type Output = Self;
    #[inline]
    fn add(self, rhs: PointData) -> TfPoint<S> {
        TfPoint::from_raw(self.1 + rhs, self.0)
    }
}

#[cfg(test)]
mod test {
    use super::*;
    // TODO macroify this and the impl trait
    #[derive(Copy, Clone, Debug, PartialEq, Eq)]
    #[repr(usize)]
    pub enum PathFrames {
        Robot,
        Field,
        Camera,
        Switch,
        Scale,
        ScaleEst,
        CubeDepo,
    }

    impl From<PathFrames> for usize {
        fn from(p: PathFrames) -> usize {
            p as usize
        }
    }

    impl PointHeirarchy for PathFrames {
        fn parent(&self) -> ParentFrame<Self> {
            use self::PathFrames::*;
            match *self {
                Robot => ParentFrame::Parent(Field),
                Switch => ParentFrame::Parent(Field),
                Scale => ParentFrame::Parent(Field),
                CubeDepo => ParentFrame::Parent(Switch),
                Camera => ParentFrame::Parent(Robot),
                Field => ParentFrame::Root,
                ScaleEst => ParentFrame::Parent(Camera),
            }
        }

        fn order() -> usize {
            7
        }
    }

    #[test]
    fn heirarchy() {
        println!("{:?}", PathFrames::ScaleEst.path_to(PathFrames::CubeDepo));
        use self::PathFrames::*;
        assert_eq!(
            PathFrames::ScaleEst.path_to(PathFrames::CubeDepo),
            (
                vec![ScaleEst, Camera, Robot, Field],
                vec![Field, Switch, CubeDepo]
            )
        );

        assert_eq!(
            PathFrames::ScaleEst.path_to(PathFrames::Field),
            (vec![ScaleEst, Camera, Robot, Field], vec![Field])
        );
    }

    fn near_eq<S: PointHeirarchy>(a: TfPoint<S>, b: TfPoint<S>) {
        assert!(a.0 == b.0);
        assert_approx_eq!(a.1.pos.0 / si::M, b.1.pos.0 / si::M);
        assert_approx_eq!(a.1.pos.1 / si::M, b.1.pos.1 / si::M);
        assert_approx_eq!(a.1.rot, b.1.rot);
    }

    #[test]
    fn raw_rooted_transforms() {
        let mut reg = FrameRegistry::<PathFrames>::new();

        let p = TfPoint::new(PathFrames::Scale, 5. * si::M, 0. * si::M, 0.321);
        let f = TfPoint::new(PathFrames::Switch, -1. * si::M, 2. * si::M, 0.787);

        // zero rotation
        *reg.raw_tf_mut(PathFrames::Scale) = PointData::xyr(10., 3., 0.);
        *reg.raw_tf_mut(PathFrames::Switch) = PointData::xyr(-5., 7., 0.);
        near_eq(
            p.in_frame(&reg, PathFrames::Field),
            TfPoint::new(PathFrames::Field, 15. * si::M, 3.0 * si::M, 0.321),
        );
        near_eq(
            f.in_frame(&reg, PathFrames::Scale),
            TfPoint::new(PathFrames::Scale, -16. * si::M, 6.0 * si::M, 0.787),
        );

        // axis aligned rotation
        use std::f64::consts::PI;
        *reg.raw_tf_mut(PathFrames::Scale) = PointData::xyr(10., 3., PI);
        *reg.raw_tf_mut(PathFrames::Switch) = PointData::xyr(-5., 7., PI / 2.);
        near_eq(
            p.in_frame(&reg, PathFrames::Field),
            TfPoint::new(PathFrames::Field, 5. * si::M, 3.0 * si::M, 0.321 + PI),
        );
        near_eq(
            f.in_frame(&reg, PathFrames::Scale),
            TfPoint::new(
                PathFrames::Scale,
                17. * si::M,
                -3. * si::M,
                -(PI / 2. - 0.787),
            ),
        );
    }

    #[test]
    fn transforms() {
        let mut reg = FrameRegistry::<PathFrames>::new();

        let p = TfPoint::new(PathFrames::Scale, 5. * si::M, 0. * si::M, 0.321);
        let f = TfPoint::new(PathFrames::Switch, -1. * si::M, 2. * si::M, 0.787);

        // zero rotation
        // reg.set_tf_from_parent(PathFrames::Scale, PointData::xyr(10., 3., 0.));
        // reg.set_tf_from_parent(PathFrames::Switch, PointData::xyr(-5., 7., 0.));
        reg.set_origin(
            PathFrames::Scale,
            TfPoint::new(PathFrames::Field, 10. * si::M, 3. * si::M, 0.),
        );

        reg.set_origin(
            PathFrames::Switch,
            TfPoint::new(PathFrames::Scale, -15. * si::M, 4. * si::M, 0.),
        );
        near_eq(
            p.in_frame(&reg, PathFrames::Field),
            TfPoint::new(PathFrames::Field, 15. * si::M, 3.0 * si::M, 0.321),
        );
        near_eq(
            f.in_frame(&reg, PathFrames::Scale),
            TfPoint::new(PathFrames::Scale, -16. * si::M, 6.0 * si::M, 0.787),
        );

        // TODO(Lytigas): axis aligned rotation
        // TODO(Lytigas): non axis aligned tests
        // TODO(Lytigas): tests between different frames that have root parents
    }
}
