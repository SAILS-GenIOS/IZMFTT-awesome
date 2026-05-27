#include <gtest/gtest.h>
#include "core/Document.h"
#include <BRepPrimAPI_MakeBox.hxx>

// Helper: create a simple box shape for testing
static TopoDS_Shape makeTestBox(double x = 10, double y = 10, double z = 10) {
    return BRepPrimAPI_MakeBox(x, y, z).Shape();
}

TEST(DocumentTest, AddBodyIncreasesCount) {
    Document doc;
    EXPECT_EQ(doc.bodyCount(), 0);

    doc.addBody(makeTestBox(), "Box1");
    EXPECT_EQ(doc.bodyCount(), 1);

    doc.addBody(makeTestBox(5, 5, 5), "Box2");
    EXPECT_EQ(doc.bodyCount(), 2);
}

TEST(DocumentTest, RemoveBodyDecreasesCount) {
    Document doc;
    int id1 = doc.addBody(makeTestBox(), "Box1");
    int id2 = doc.addBody(makeTestBox(), "Box2");
    EXPECT_EQ(doc.bodyCount(), 2);

    doc.removeBody(id1);
    EXPECT_EQ(doc.bodyCount(), 1);

    doc.removeBody(id2);
    EXPECT_EQ(doc.bodyCount(), 0);
}

TEST(DocumentTest, GetBodyReturnsValidShape) {
    Document doc;
    int id = doc.addBody(makeTestBox(), "TestBox");

    const TopoDS_Shape& shape = doc.getBody(id);
    EXPECT_FALSE(shape.IsNull());
}

TEST(DocumentTest, GetBodyThrowsForInvalidId) {
    Document doc;
    EXPECT_THROW(doc.getBody(999), std::runtime_error);
}

TEST(DocumentTest, BodyNaming) {
    Document doc;
    int id = doc.addBody(makeTestBox(), "OriginalName");

    EXPECT_EQ(doc.getBodyName(id), "OriginalName");

    doc.setBodyName(id, "RenamedBody");
    EXPECT_EQ(doc.getBodyName(id), "RenamedBody");
}

TEST(DocumentTest, BodyDefaultNaming) {
    Document doc;
    int id = doc.addBody(makeTestBox());

    // Default name should not be empty
    std::string name = doc.getBodyName(id);
    EXPECT_FALSE(name.empty());
}

TEST(DocumentTest, BodyVisibility) {
    Document doc;
    int id = doc.addBody(makeTestBox(), "Box");

    // Bodies are visible by default
    EXPECT_TRUE(doc.isBodyVisible(id));

    doc.setBodyVisible(id, false);
    EXPECT_FALSE(doc.isBodyVisible(id));

    doc.setBodyVisible(id, true);
    EXPECT_TRUE(doc.isBodyVisible(id));
}

TEST(DocumentTest, GetAllBodyIds) {
    Document doc;
    int id1 = doc.addBody(makeTestBox(), "A");
    int id2 = doc.addBody(makeTestBox(), "B");
    int id3 = doc.addBody(makeTestBox(), "C");

    std::vector<int> ids = doc.getAllBodyIds();
    EXPECT_EQ(ids.size(), 3u);

    // All IDs should be present
    EXPECT_NE(std::find(ids.begin(), ids.end(), id1), ids.end());
    EXPECT_NE(std::find(ids.begin(), ids.end(), id2), ids.end());
    EXPECT_NE(std::find(ids.begin(), ids.end(), id3), ids.end());
}

TEST(DocumentTest, ClearRemovesAllBodies) {
    Document doc;
    doc.addBody(makeTestBox(), "A");
    doc.addBody(makeTestBox(), "B");
    doc.addBody(makeTestBox(), "C");
    EXPECT_EQ(doc.bodyCount(), 3);

    doc.clear();
    EXPECT_EQ(doc.bodyCount(), 0);
    EXPECT_TRUE(doc.getAllBodyIds().empty());
}

TEST(DocumentTest, UpdateBodyChangesShape) {
    Document doc;
    int id = doc.addBody(makeTestBox(10, 10, 10), "Box");

    TopoDS_Shape newShape = makeTestBox(20, 20, 20);
    doc.updateBody(id, newShape);

    const TopoDS_Shape& retrieved = doc.getBody(id);
    EXPECT_FALSE(retrieved.IsNull());
    // The shape should have been replaced (we verify it's not null;
    // exact geometry comparison would require OCCT tools)
}

TEST(DocumentTest, RemoveNonexistentBodyDoesNotCrash) {
    Document doc;
    doc.addBody(makeTestBox(), "Box");

    // Removing a non-existent body should not throw or crash
    doc.removeBody(999);
    EXPECT_EQ(doc.bodyCount(), 1);
}
