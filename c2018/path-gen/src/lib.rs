extern crate coord_frames;
use coord_frames::*;
#[macro_use]
extern crate dimensioned as dim;
use self::dim::si;

#[macro_use]
mod frames;
use self::frames::PathFrame;

use std::f64::consts::PI;
const PI2: f64 = PI / 2.0;

pub struct PurePursuitPoint {
    x: f64,
    y: f64,
    dist_traveled: f64,
    is_interped: bool,
}

pub struct StartPose {
    x: f64,
    y: f64,
    t: f64,
}

pub struct Path {
    points: Vec<PurePursuitPoint>,
    finish_dx: f64,
    finish_dy: f64,
    start: StartPose,
}

pub struct Paths {
    right_start_to_rocket: Path,
}

pub fn gen_paths() -> Paths {
    unimplemented!()
}

fn with_kappa(point: PointData, k: f64, dk: f64) -> MotionState<f64> {
    MotionState {
        // convert m to feet for the robot
        x: *(point.x() / si::M) * 3.28084,
        y: *(point.y() / si::M) * 3.28084,
        t: point.rot(),
        k,
        dk,
    }
}

use std::fs;
fn export_pose(point: PointData, name: &str) {
    fs::write(
        format!("out/{}.java", name),
        format!(
            "public static Pose {} = new Pose({}, {}, {}, 0.0);",
            name,
            *(point.x() / si::M) * 3.28084,
            *(point.y() / si::M) * 3.28084,
            point.rot()
        ),
    )
    .unwrap();
}

fn zero_kappa(point: PointData) -> MotionState<f64> {
    with_kappa(point, 0., 0.)
}

fn basic_param(a: f64) -> EtaParam<f64> {
    assert!(a > 0.0);
    EtaParam::new(a, a, 0., 0., 0., 0.)
}

extern crate eta3_spline;
use eta3_spline::*;
extern crate csv;
fn export_path(
    points: Vec<MotionState<f64>>,
    params: Vec<EtaParam<f64>>,
    name: &str,
    num_pts: usize,
) {
    let path = EtaCurve::new(points.as_slice(), params.as_slice()).unwrap();
    let mut wtr = csv::WriterBuilder::new()
        .has_headers(false)
        .from_path(format!("out/{}.114path", name))
        .unwrap();
    wtr.write_record(&["x", "y", "distanceSoFar", "isEndPointInterpolation"])
        .unwrap();
    let mut t = 0.0;
    let dt = 1.0 / num_pts as f64;
    let mut dist = 0.0;

    let mut last_point = path.eval(0.0);
    while t < 1.0 {
        let point = path.eval(t);

        dist += f64::sqrt((point.0 - last_point.0).powi(2) + (point.1 - last_point.1).powi(2));
        last_point = point;

        wtr.serialize((point.0, point.1, dist, "False")).unwrap();
        t += dt;
    }
    // end point interpolation
    let last_state = points.get(points.len() - 1).unwrap();
    let dx = last_state.t.cos();
    let dy = last_state.t.sin();
    // add approximately 6 feet of endpoint interpolation
    const INTERP_DIST: f64 = 6.0;
    let mut p = 0.0;
    let dp = 0.1;

    while p < INTERP_DIST {
        wtr.serialize((
            last_state.x + dx * p,
            last_state.y + dy * p,
            dist + p,
            "True",
        ))
        .unwrap();
        p += dp;
    }

    match wtr.serialize((dx, dy)) {
        Err(e) => match e.into_kind() {
            csv::ErrorKind::UnequalLengths { .. } => (),
            _ => panic!(),
        },
        _ => (),
    };
}
