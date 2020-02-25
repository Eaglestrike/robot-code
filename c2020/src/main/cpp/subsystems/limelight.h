#pragma once

#include <array>
#include <memory>

#include "frc/geometry/Pose2d.h"
#include "frc/geometry/Rotation2d.h"
#include "frc/geometry/Translation2d.h"
#include "frc/smartdashboard/SmartDashboard.h"

#include "networktables/NetworkTable.h"
#include "networktables/NetworkTableInstance.h"

#include <robot_state.h>
#include "subsystem.h"

namespace team114 {
namespace c2020 {

/**
 * Subsystem for interacting with the Limelight 2
 */
class Limelight : public Subsystem {
   public:
    SUBSYSTEM_PRELUDE(Limelight)
    units::second_t kImageCaptureLatency = 11_ms;

    struct TargetInfo {
        double x = 1.0;
        double y;
        double z;
    };
    struct LimelightConfig {
        std::string name;
        std::string table_name;
        double kHeight;
        frc::Pose2d robot_to_lens;
    };
    const LimelightConfig cfg_;

    enum class LedMode : int {
        PIPELINE = 0,
        OFF = 1,
        BLINK = 2,
        ON = 3,
    };

    Limelight(LimelightConfig cfg) : cfg_{std::move(cfg)} {
        network_table_ =
            nt::NetworkTableInstance::GetDefault().GetTable(cfg_.table_name);
    }

   private:
    std::shared_ptr<nt::NetworkTable> network_table_;

    struct PeriodicIn {
        units::second_t latency;
        int led_mode;
        int pipeline;
        double x_off;
        double y_off;
        double area;
    };
    struct PeriodicOut {
        int led_mode = 1;  // 0 - use pipeline mode, 1 - off, 2 - blink, 3 - on
        int cam_mode = 0;  // 0 - vision processing, 1 - driver camera
        int pipeline = 0;  // 0 - 9
        int stream = 2;    // sets stream layout if another webcam is attached
        int snapshot = 0;  // 0 - stop snapshots, 1 - 2 Hz
    };
    PeriodicIn per_in_;
    PeriodicOut per_out_;
    bool per_out_dirty_ = true;
    bool sees_target_ = false;
    List<TargetInfo> mTargets = new ArrayList<>();

    void Periodic() final override {
        ReadPeriodicIn();
        WritePeriodicOut();
    }

    void ReadPeriodicIn() {
        per_in_.latency =
            network_table_->GetNumber("tl", 0) / 1000.0 + kImageCaptureLatency;
        per_in_.led_mode = (int)network_table_->GetNumber("ledMode", 1.0);
        per_in_.pipeline = (int)network_table_->GetNumber("pipeline", 0);
        per_in_.x_off = network_table_->GetNumber("tx", 0.0);
        per_in_.y_off = network_table_->GetNumber("ty", 0.0);
        per_in_.area = network_table_->GetNumber("ta", 0.0);
        // paranoid about double equality...
        sees_target_ = network_table_->GetNumber("tv", 0) > 0.5;
    }

    void WritePeriodicOut() {
        if (per_out_.led_mode != per_in_.led_mode ||
            per_out_.pipeline != per_in_.pipeline) {
            // LOG
            per_out_dirty_ = true;
        }
        if (per_out_dirty_) {
            network_table_->PutNumber("ledMode", per_out_.led_mode);
            network_table_->PutNumber("camMode", per_out_.cam_mode);
            network_table_->PutNumber("pipeline", per_out_.pipeline);
            network_table_->PutNumber("stream", per_out_.stream);
            network_table_->PutNumber("snapshot", per_out_.snapshot);
            per_out_dirty_ = false;
        }
    }

    void SetLedMode(LedMode mode) {
        int mode_int = static_cast<int>(mode);
        if (mode_int != per_out_.led_mode) {
            per_out_.led_mode = mode_int;
            per_out_dirty_ = true;
        }
    }

    void SetPipeline(int pipeline) {
        if (pipeline != per_out_.pipeline) {
            // TODO reset veision here
            // RobotState::GetInstance().ResetVision();
            per_out_.pipeline = pipeline;
            // LOG
            per_out_dirty_ = true;
        }
    }

    int GetPipeline() { return per_out_.pipeline; }

    void ForceFlushOuts() { per_out_dirty_ = true; }

    bool SeesTarget() { return sees_target_; }

    /**
     * @return two targets that make up one hatch/port or null if less than two
     * targets are found
     */
   public
    synchronized List<TargetInfo> getTarget() {
        List<TargetInfo> targets = getRawTargetInfos();
        if (seesTarget() && targets != null) {
            return targets;
        }

        return null;
    }

   private
    synchronized List<TargetInfo> getRawTargetInfos() {
        List<double[]> corners = getTopCorners();
        if (corners == null) {
            return null;
        }

        double slope = 1.0;
        if (Math.abs(corners.get(1)[0] - corners.get(0)[0]) > Util.kEpsilon) {
            slope = (corners.get(1)[1] - corners.get(0)[1]) /
                    (corners.get(1)[0] - corners.get(0)[0]);
        }

        mTargets.clear();
        for (int i = 0; i < 2; ++i) {
            // Average of y and z;
            double y_pixels = corners.get(i)[0];
            double z_pixels = corners.get(i)[1];

            // Redefine to robot frame of reference.
            double nY = -((y_pixels - 160.0) / 160.0);
            double nZ = -((z_pixels - 120.0) / 120.0);

            double y = Constants.kVPW / 2 * nY;
            double z = Constants.kVPH / 2 * nZ;

            TargetInfo target = new TargetInfo(y, z);
            target.setSkew(slope);
            mTargets.add(target);
        }

        return mTargets;
    }

    /**
     * Returns raw top-left and top-right corners
     *
     * @return list of corners: index 0 - top left, index 1 - top right
     */
   private
    List<double[]> getTopCorners() {
        double[] xCorners =
            mNetworkTable.getEntry("tcornx").getDoubleArray(mZeroArray);
        double[] yCorners =
            mNetworkTable.getEntry("tcorny").getDoubleArray(mZeroArray);
        mSeesTarget = mNetworkTable.getEntry("tv").getDouble(0) == 1.0;

        // something went wrong
        if (!mSeesTarget || Arrays.equals(xCorners, mZeroArray) ||
            Arrays.equals(yCorners, mZeroArray) || xCorners.length != 8 ||
            yCorners.length != 8) {
            return null;
        }

        return extractTopCornersFromBoundingBoxes(xCorners, yCorners);
    }

   private
    static final Comparator<Translation2d> xSort =
        Comparator.comparingDouble(Translation2d::x);
   private
    static final Comparator<Translation2d> ySort =
        Comparator.comparingDouble(Translation2d::y);

    /**
     * Returns raw top-left and top-right corners
     *
     * @return list of corners: index 0 - top left, index 1 - top right
     */
   public
    static List<double[]> extractTopCornersFromBoundingBoxes(
        double[] xCorners, double[] yCorners) {
        List<Translation2d> corners = new ArrayList<>();
        for (int i = 0; i < xCorners.length; i++) {
            corners.add(new Translation2d(xCorners[i], yCorners[i]));
        }

        corners.sort(xSort);

        List<Translation2d> left = corners.subList(0, 4);
        List<Translation2d> right = corners.subList(4, 8);

        left.sort(ySort);
        right.sort(ySort);

        List<Translation2d> leftTop = left.subList(0, 2);
        List<Translation2d> rightTop = right.subList(0, 2);

        leftTop.sort(xSort);
        rightTop.sort(xSort);

        Translation2d leftCorner = leftTop.get(0);
        Translation2d rightCorner = rightTop.get(1);

        return List.of(new double[]{leftCorner.x(), leftCorner.y()},
                       new double[]{rightCorner.x(), rightCorner.y()});
    }

    GetLatency() { return per_in_.latency; }
}

}  // namespace c2020
}  // namespace team114
