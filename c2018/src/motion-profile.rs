pub fn trapezoid_motion_profile(initial_velocity: f64, final_velocity: f64, initial_error: f64, time_since_start_sec: f64, max_velocity: f64, max_acceleration: f64) -> f64 {
    //find velocity at time using a trapezoid motion profile
    let time_to_max_velocity = (max_velocity - initial_velocity) / max_acceleration;
    let distance_to_max_velocity = time_to_max_velocity * max_velocity / 2.0;
    let time_to_ending_velocity = (max_velocity - final_velocity) / max_acceleration;
    let distance_to_ending_velocity = time_to_max_velocity * max_velocity / 2.0;
    let total_time = (initial_error - distance_to_max_velocity - distance_to_ending_velocity) / max_velocity + time_to_max_velocity + time_to_ending_velocity;

    if initial_error < (distance_to_max_velocity + distance_to_ending_velocity) {
        //The trapazoid needs to be turned into a triangle because the robot does not have to enough time to reach max velocity
        let swap_time = (max_acceleration * total_time  + final_velocity - initial_velocity)/(2.0 * max_acceleration); // The time you have to swap to un-accelerating from accelerating
        if 0.0 >= time_since_start_sec{
            return initial_velocity; //If time is negetive, return the initial velocity.
        }else if(time_since_start_sec <= swap_time){
            return max_acceleration * time_since_start_sec + initial_velocity; //Used if accelerating
        }else if time_since_start_sec <= total_time{
            return max_acceleration * (total_time - time_since_start_sec) + final_velocity; // Used if un-accelerating
        }else{
            return final_velocity;
        }

    }else{
        //Motion profiling is done normally
        if 0.0 >= time_since_start_sec{
            return initial_velocity; //If time is negetive, return the initial velocity.
        } else if 0.0 <= time_since_start_sec && time_since_start_sec <= time_to_max_velocity {
            return max_acceleration * time_since_start_sec + initial_velocity; //Used if you are still accelerating
        } else if time_to_max_velocity <= time_since_start_sec && time_since_start_sec <= (total_time - time_to_ending_velocity) {
            return max_velocity; //Used after you accelerate to your max velocity
        } else if (total_time - time_to_ending_velocity) <= time_since_start_sec && time_since_start_sec <= total_time {
            return max_acceleration * (total_time - time_since_start_sec) + final_velocity; //Used if un-accelerationg
        } else {
            return final_velocity;
        };

    }

}


