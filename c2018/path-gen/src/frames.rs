macro_rules! const_meter {
    ($name:ident, $value:expr) => {
        pub const $name: ::dimensioned::si::SI<
            f64,
            tarr![
                ::dimensioned::typenum::P1,
                ::dimensioned::typenum::Z0,
                ::dimensioned::typenum::Z0,
                ::dimensioned::typenum::Z0,
                ::dimensioned::typenum::Z0,
                ::dimensioned::typenum::Z0,
                ::dimensioned::typenum::Z0
            ],
        > = ::dimensioned::si::SI {
            value_unsafe: $value,
            _marker: ::std::marker::PhantomData,
        };
    };
}

use coord_frames::*;
// TODO macroify this and the impl trait
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
#[repr(usize)]
pub enum PathFrame {
    Field,
}

impl From<PathFrame> for usize {
    fn from(p: PathFrame) -> usize {
        p as usize
    }
}

impl PointHeirarchy for PathFrame {
    fn parent(&self) -> ParentFrame<Self> {
        match *self {
            PathFrame::Field => ParentFrame::Root,
        }
    }

    fn order() -> usize {
        1
    }
}
