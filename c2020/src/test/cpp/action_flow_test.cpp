#include <auto/actions/control_flow.h>
#include <auto/executor.h>

#include <iterator>

#include "gtest/gtest.h"

using namespace team114::c2020::auton;

TEST(Actions, Executor) {
    class TestAction : public Action {
       public:
        TestAction(int runs) : runs_{runs} {}
        virtual void Start() override { EXPECT_EQ(state_++, 0); }
        virtual void Periodic() override { state_++; }
        virtual bool Finished() override {
            EXPECT_TRUE(state_ <= runs_);
            return state_ >= runs_;
        }
        virtual void Stop() override { EXPECT_EQ(state_, runs_); }

        int state_ = 0;
        const int runs_;
    };
    AutoExecutor exe{std::make_unique<TestAction>(3)};
    ASSERT_FALSE(exe.Finished());
    exe.Periodic();
    exe.Periodic();
    exe.Periodic();
    exe.Periodic();
    exe.Periodic();
    ASSERT_TRUE(exe.Finished());
    exe.Periodic();
    exe.Periodic();
    exe.Periodic();
    ASSERT_TRUE(exe.Finished());

    AutoExecutor cexe{std::make_unique<SeriesAction>(MakeActionList(
        std::make_unique<TestAction>(3), std::make_unique<TestAction>(4),
        std::make_unique<TestAction>(5),
        std::make_unique<ParallelAction>(MakeActionList(
            std::make_unique<TestAction>(2), std::make_unique<TestAction>(1),
            std::make_unique<TestAction>(4),
            std::make_unique<SeriesAction>(
                MakeActionList(std::make_unique<TestAction>(1),
                               std::make_unique<TestAction>(5),
                               std::make_unique<TestAction>(8),
                               std::make_unique<TestAction>(3))))),

        std::make_unique<TestAction>(10)))};
    for (int i = 0; i < 100; i++) {
        cexe.Periodic();
    }
    ASSERT_TRUE(cexe.Finished());
}

TEST(Actions, EarlyStop) {
    // Tests that all started actions are stopped
    class TestAction : public Action {
       public:
        ~TestAction() { EXPECT_TRUE(has_started_ == has_stopped_); }
        TestAction(int runs) : runs_{runs} {}
        virtual void Start() override {
            state_ = 0;
            has_started_ = true;
        }
        virtual void Periodic() override { state_++; }
        virtual bool Finished() override { return state_ > runs_; }
        virtual void Stop() override { has_stopped_ = true; }

        bool has_started_{false};
        bool has_stopped_{false};
        int state_{0};
        int runs_;
    };
    {
        AutoExecutor exe{std::make_unique<TestAction>(3)};
        ASSERT_FALSE(exe.Finished());
        exe.Periodic();
        ASSERT_FALSE(exe.Finished());
    }
    auto make_complex_exec = []() -> AutoExecutor {
        return AutoExecutor{std::make_unique<SeriesAction>(MakeActionList(
            std::make_unique<TestAction>(3), std::make_unique<TestAction>(4),
            std::make_unique<TestAction>(5),
            std::make_unique<ParallelAction>(
                MakeActionList(std::make_unique<TestAction>(2),
                               std::make_unique<TestAction>(1),
                               std::make_unique<TestAction>(4),
                               std::make_unique<SeriesAction>(MakeActionList(
                                   std::make_unique<TestAction>(1),
                                   std::make_unique<TestAction>(5),
                                   std::make_unique<TestAction>(8),
                                   std::make_unique<TestAction>(3))))),
            std::make_unique<TestAction>(10)))};
    };
    // halt halfway
    auto cexe1{make_complex_exec()};
    for (int i = 0; i < 15; i++) {
        cexe1.Periodic();
    }
    // run fully
    auto cexe2{make_complex_exec()};
    while (!cexe2.Finished()) {
        cexe2.Periodic();
    }
    // run nothing
    auto cexe3{make_complex_exec()};
}
