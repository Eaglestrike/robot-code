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
    // TODO(josh) finish test
}
