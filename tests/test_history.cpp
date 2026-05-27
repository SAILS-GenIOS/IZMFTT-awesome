#include <gtest/gtest.h>
#include "core/History.h"
#include "core/Document.h"
#include "core/Operation.h"
#include <BRepPrimAPI_MakeBox.hxx>

// Mock operation that adds a box body to the document
class MockAddBodyOp : public Operation {
public:
    MockAddBodyOp(double size = 10.0) : m_size(size) {}

    bool execute(Document& doc) override {
        TopoDS_Shape box = BRepPrimAPI_MakeBox(m_size, m_size, m_size).Shape();
        m_createdId = doc.addBody(box, "Box");
        return true;
    }

    bool undo(Document& doc) override {
        doc.removeBody(m_createdId);
        m_createdId = -1;
        return true;
    }

    std::string name() const override { return "Add Box"; }
    std::string description() const override { return "Add test box"; }
    void renderProperties() override {}
    std::string typeId() const override { return "mock_add_body"; }

    int createdId() const { return m_createdId; }

private:
    double m_size;
    int m_createdId = -1;
};

TEST(HistoryTest, PushOperationIncreasesStepCount) {
    Document doc;
    History history;

    EXPECT_EQ(history.stepCount(), 0);

    auto op = std::make_unique<MockAddBodyOp>(10.0);
    EXPECT_TRUE(history.pushOperation(std::move(op), doc));
    EXPECT_EQ(history.stepCount(), 1);

    auto op2 = std::make_unique<MockAddBodyOp>(5.0);
    EXPECT_TRUE(history.pushOperation(std::move(op2), doc));
    EXPECT_EQ(history.stepCount(), 2);
}

TEST(HistoryTest, PushOperationExecutesOp) {
    Document doc;
    History history;

    auto op = std::make_unique<MockAddBodyOp>(10.0);
    history.pushOperation(std::move(op), doc);

    // The operation should have added a body
    EXPECT_EQ(doc.bodyCount(), 1);
}

TEST(HistoryTest, UndoEnablesRedo) {
    Document doc;
    History history;

    auto op = std::make_unique<MockAddBodyOp>(10.0);
    history.pushOperation(std::move(op), doc);

    EXPECT_FALSE(history.canRedo());
    EXPECT_TRUE(history.canUndo());

    history.undo(doc);
    EXPECT_TRUE(history.canRedo());
    EXPECT_EQ(doc.bodyCount(), 0);
}

TEST(HistoryTest, RedoRestoresState) {
    Document doc;
    History history;

    auto op = std::make_unique<MockAddBodyOp>(10.0);
    history.pushOperation(std::move(op), doc);
    EXPECT_EQ(doc.bodyCount(), 1);

    history.undo(doc);
    EXPECT_EQ(doc.bodyCount(), 0);

    history.redo(doc);
    EXPECT_EQ(doc.bodyCount(), 1);
}

TEST(HistoryTest, CanUndoCanRedoEmptyHistory) {
    History history;
    EXPECT_FALSE(history.canUndo());
    EXPECT_FALSE(history.canRedo());
}

TEST(HistoryTest, MultipleOperationsUndoVerifyCurrentStep) {
    Document doc;
    History history;

    // Push 3 operations
    for (int i = 0; i < 3; i++) {
        auto op = std::make_unique<MockAddBodyOp>(10.0 + i);
        history.pushOperation(std::move(op), doc);
    }
    EXPECT_EQ(history.stepCount(), 3);
    EXPECT_EQ(history.currentStep(), 2);

    // Undo 2
    history.undo(doc);
    EXPECT_EQ(history.currentStep(), 1);

    history.undo(doc);
    EXPECT_EQ(history.currentStep(), 0);

    EXPECT_TRUE(history.canUndo());
    EXPECT_TRUE(history.canRedo());
}

TEST(HistoryTest, RedoClearedOnNewPush) {
    Document doc;
    History history;

    auto op1 = std::make_unique<MockAddBodyOp>(10.0);
    history.pushOperation(std::move(op1), doc);

    auto op2 = std::make_unique<MockAddBodyOp>(5.0);
    history.pushOperation(std::move(op2), doc);

    // Undo the second operation
    history.undo(doc);
    EXPECT_TRUE(history.canRedo());

    // Push a new operation -- should clear redo stack
    auto op3 = std::make_unique<MockAddBodyOp>(7.0);
    history.pushOperation(std::move(op3), doc);
    EXPECT_FALSE(history.canRedo());
    EXPECT_EQ(history.stepCount(), 2); // op1 + op3 (op2 was cleared)
}

TEST(HistoryTest, BreakpointSuppressesStepsAfter) {
    Document doc;
    History history;

    // Push 3 operations
    for (int i = 0; i < 3; i++) {
        auto op = std::make_unique<MockAddBodyOp>(10.0 + i);
        history.pushOperation(std::move(op), doc);
    }
    EXPECT_EQ(doc.bodyCount(), 3);

    // Set breakpoint at step 1 (only steps 0 and 1 should execute on replay)
    history.setBreakpoint(1);
    EXPECT_EQ(history.getBreakpoint(), 1);

    // Replay all -- should only execute up to breakpoint
    history.replayAll(doc);
    EXPECT_EQ(doc.bodyCount(), 2); // Only steps 0 and 1 executed
}

TEST(HistoryTest, ReplayAllRebuildsDocument) {
    Document doc;
    History history;

    // Push 3 operations
    for (int i = 0; i < 3; i++) {
        auto op = std::make_unique<MockAddBodyOp>(10.0 + i);
        history.pushOperation(std::move(op), doc);
    }
    EXPECT_EQ(doc.bodyCount(), 3);

    // Manually clear the doc (simulating corruption)
    doc.clear();
    EXPECT_EQ(doc.bodyCount(), 0);

    // Replay all should restore the document
    history.replayAll(doc);
    EXPECT_EQ(doc.bodyCount(), 3);
}

TEST(HistoryTest, ClearResetsEverything) {
    Document doc;
    History history;

    auto op = std::make_unique<MockAddBodyOp>(10.0);
    history.pushOperation(std::move(op), doc);

    history.clear();
    EXPECT_EQ(history.stepCount(), 0);
    EXPECT_EQ(history.currentStep(), -1);
    EXPECT_FALSE(history.canUndo());
    EXPECT_FALSE(history.canRedo());
}

TEST(HistoryTest, GetStepReturnsOperation) {
    Document doc;
    History history;

    auto op = std::make_unique<MockAddBodyOp>(10.0);
    history.pushOperation(std::move(op), doc);

    const Operation* step = history.getStep(0);
    ASSERT_NE(step, nullptr);
    EXPECT_EQ(step->name(), "Add Box");
    EXPECT_EQ(step->typeId(), "mock_add_body");
}

TEST(HistoryTest, GetStepOutOfBoundsReturnsNull) {
    History history;
    EXPECT_EQ(history.getStep(-1), nullptr);
    EXPECT_EQ(history.getStep(0), nullptr);
    EXPECT_EQ(history.getStep(100), nullptr);
}
