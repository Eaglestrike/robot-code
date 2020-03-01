#pragma once

#include <frc/smartdashboard/SendableChooser.h>

#include <memory>
#include <optional>

#include "action.h"
#include "actions/control_flow.h"
#include "actions/drive_actions.h"

#include "modes/open_loop_forward.h"
#include "modes/test_mode.h"

#include <subsystem.h>
#include <util/constructor_macros.h>

namespace team114 {
namespace c2020 {
namespace auton {

class AutoModeSelector {
   private:
    AutoModeSelector() {
        mode_chooser_.SetDefaultOption("Null Mode", DesiredMode::NullMode);
        mode_chooser_.AddOption("Test Mode", DesiredMode::TestMode);
        mode_chooser_.AddOption("Open Loop Forward",
                                DesiredMode::OpenLoopForward);
        mode_chooser_.AddOption("Shoot and OL Reverse",
                                DesiredMode::ShootAndOLReverse);
        start_chooser_.SetDefaultOption("Assume Origin",
                                        StartingPosition::Origin);
        frc::SmartDashboard::PutData("Auto Mode", &mode_chooser_);
        frc::SmartDashboard::PutData("Auto Starting Pos", &start_chooser_);
    }

   public:
    DISALLOW_COPY_ASSIGN(AutoModeSelector)
    CREATE_SINGLETON(AutoModeSelector)
   public:
    enum class StartingPosition { Origin };

    enum class DesiredMode {
        NullMode,
        TestMode,
        OpenLoopForward,
        ShootAndOLReverse,
    };

    std::unique_ptr<Action> GetSelectedAction() {
        UpdateSelection();
        if (!cache_action_.has_value()) {
            // LOG problem
            return std::make_unique<EmptyAction>();
        }
        return std::move(cache_action_).value();
    }

    void UpdateSelection() {
        auto mode = mode_chooser_.GetSelected();
        auto start_pos = start_chooser_.GetSelected();
        if (mode == cached_mode_choice_ && start_pos == cached_start_choice_ &&
            cache_action_.has_value()) {
            return;
        }
        cache_action_ = RebuildMode(mode, start_pos);
    }

   private:
    static std::unique_ptr<Action> RebuildMode(DesiredMode mode,
                                               StartingPosition start_pos) {
        switch (mode) {
            case DesiredMode::TestMode:
                return MakeTestMode();
            case DesiredMode::OpenLoopForward:
                return MakeOpenLoopForward();
            case DesiredMode::NullMode:
                return std::make_unique<EmptyAction>();
            case DesiredMode::ShootAndOLReverse:
                return MakeShootAndOLReverse();
            default:
                // LOG
                return std::make_unique<EmptyAction>();
        }
    }
    std::optional<std::unique_ptr<Action>> cache_action_{};
    DesiredMode cached_mode_choice_;
    StartingPosition cached_start_choice_;

    frc::SendableChooser<DesiredMode> mode_chooser_;
    frc::SendableChooser<StartingPosition> start_chooser_;
};

}  // namespace auton
}  // namespace c2020
}  // namespace team114
