/*
//To use:
let settings = MotionProfileSettings{
    initial_velocity: initial_velocity,
    final_velocity: final_velocity,
    initial_error: initial_error,
    max_velocity: max_velocity,
    max_acceleration: max_acceleration
};
let profile: MotionProfileStructure = MotionProfile::new(settings);
profile.velocity_after_time(t);

*/

#[derive(Clone)]
struct MotionProfileSettings {
    initial_velocity: f64,
    final_velocity: f64,
    initial_error: f64,
    max_velocity: f64,
    max_acceleration: f64
}
#[derive(Debug)]
struct MotionProfileInternalStructure {
    max: f64,
    data: [f64;2]
}
#[derive(Debug)]
struct MotionProfileStructure{
    default: f64,
    values: [MotionProfileInternalStructure;4]
}

trait MotionProfile{
    fn new(settings: MotionProfileSettings)-> Self;
    fn find_motion_profile_structure(settings: MotionProfileSettings) -> MotionProfileStructure;
    fn velocity_after_time(&self,t: f64) -> f64;
    fn velocity_given_param(t: f64, value: [f64;2])-> f64;
}
impl MotionProfile for MotionProfileStructure{
    fn new(settings: MotionProfileSettings) -> Self{
        return Self::find_motion_profile_structure(settings);
    }
    fn find_motion_profile_structure(settings: MotionProfileSettings) -> MotionProfileStructure{
        let initial_velocity = settings.initial_velocity;
        let final_velocity = settings.final_velocity;
        let initial_error = settings.initial_error;
        let max_velocity = settings.max_velocity;
        let max_acceleration = settings.max_acceleration;

        let time_to_max_velocity = (max_velocity - initial_velocity) / max_acceleration;
        let distance_to_max_velocity = time_to_max_velocity * (max_velocity - initial_velocity) / 2.0 + time_to_max_velocity*initial_velocity;
        let time_to_ending_velocity = (max_velocity - final_velocity) / max_acceleration;
        let distance_to_ending_velocity = time_to_ending_velocity * (max_velocity-final_velocity) / 2.0 + time_to_ending_velocity*final_velocity;
        let total_time = (initial_error - distance_to_max_velocity - distance_to_ending_velocity) / max_velocity + time_to_max_velocity + time_to_ending_velocity;
        let use_modified_profile = initial_error < (distance_to_max_velocity + distance_to_ending_velocity);
        let output_structure: MotionProfileStructure;
        if use_modified_profile{
            // Solve for the modified total time with the quadratic formula
            let quad_a = max_acceleration/4.0;
            let quad_b = (initial_velocity+final_velocity)/2.0;
            let quad_c = (-(final_velocity-initial_velocity).powi(2))/(4.0*max_acceleration) - initial_error;
            let new_total_time = (-quad_b+(quad_b.powi(2)-4.0*quad_a*quad_c).sqrt())/(2.0*quad_a);
            println!("new total time: {}", new_total_time);
            let swap_time = (max_acceleration * new_total_time  + final_velocity - initial_velocity)/(2.0 * max_acceleration); // The time you have to swap to un-accelerating from accelerating
            println!("Swap Time: {}", swap_time);
            output_structure = MotionProfileStructure{
                default: final_velocity,
                values: [
                    MotionProfileInternalStructure{
                        max: 0.0,
                        data: [0.0,initial_velocity]
                    },
                    MotionProfileInternalStructure{ //Temp line to make it have a size of 4.
                        max: -1.0,
                        data: [0.0, initial_velocity]
                    },
                    MotionProfileInternalStructure{
                        max: swap_time,
                        data: [max_acceleration, initial_velocity]
                    },
                    MotionProfileInternalStructure{
                        max: new_total_time,
                        data: [-max_acceleration, max_acceleration*new_total_time + final_velocity]
                    }
                ]
            };
        }else{
            output_structure = MotionProfileStructure{
                default: final_velocity,
                values: [
                    MotionProfileInternalStructure{
                        max: 0.0,
                        data: [0.0, initial_velocity]
                    },
                    MotionProfileInternalStructure{
                        max: time_to_max_velocity,
                        data: [max_acceleration, initial_velocity]
                    },
                    MotionProfileInternalStructure{
                        max: total_time - time_to_ending_velocity,
                        data: [0.0, max_velocity]
                    },
                    MotionProfileInternalStructure{
                        max: total_time,
                        data: [-max_acceleration, max_acceleration*total_time + final_velocity]
                    }
                ]
            };
        }
        return output_structure;
    }
    fn velocity_after_time(&self, t: f64) -> f64{
        let values = &self.values;
        for value in values{
            if value.max > t{

                return Self::velocity_given_param(t, value.data);
            }
        }
        return self.default;
    }
    fn velocity_given_param(t: f64, value: [f64;2])-> f64{
        return value[0]*t+value[1]; //written as [m, b] for mx+b
    }

}