// Tail: ShellOp wired to topo::Ref/resolveSet. A shell's opened face is named
// topologically, so it FOLLOWS a dimension edit that MOVES the face — the case
// ShellOp's geometric normal+point rebind can't handle (a moved wall's stored
// point lands in empty space / on the wrong face). Here the opened RIGHT wall
// moves x=20 -> x=40 when the sketch widens; the shell must re-open it.

#include "modeling/TopoName.h"
#include "modeling/ShellOp.h"
#include "core/Document.h"
#include "modeling/Sketch.h"
#include "modeling/ExtrudeOp.h"

#include <gtest/gtest.h>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <gp_Ax3.hxx>
#include <gp_Pln.hxx>
#include <memory>

using materializr::Sketch;
using namespace materializr;

namespace {

double volumeOf(const TopoDS_Shape& s) {
    GProp_GProps g; BRepGProp::VolumeProperties(s, g); return g.Mass();
}

std::shared_ptr<Sketch> makeRect(double w, double h, int pid[4]) {
    auto sk = std::make_shared<Sketch>();
    sk->setPlane(gp_Pln(gp_Ax3(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0))));
    pid[0] = sk->addPoint({0.0f, 0.0f});
    pid[1] = sk->addPoint({(float)w, 0.0f});
    pid[2] = sk->addPoint({(float)w, (float)h});
    pid[3] = sk->addPoint({0.0f, (float)h});
    sk->addLine(pid[0], pid[1]);
    sk->addLine(pid[1], pid[2]);
    sk->addLine(pid[2], pid[3]);
    sk->addLine(pid[3], pid[0]);
    return sk;
}

// The rightmost planar face (max centroid X) — the +X wall of the box.
TopoDS_Face rightWall(const TopoDS_Shape& body) {
    TopoDS_Face best; double bestX = -1e18;
    for (TopExp_Explorer ex(body, TopAbs_FACE); ex.More(); ex.Next()) {
        TopoDS_Face f = TopoDS::Face(ex.Current());
        GProp_GProps g; BRepGProp::SurfaceProperties(f, g);
        if (g.CentreOfMass().X() > bestX) { bestX = g.CentreOfMass().X(); best = f; }
    }
    return best;
}

} // namespace

TEST(TopoShell, OpenedWallFollowsResize) {
    Document doc;
    int pid[4];
    auto sk = makeRect(20.0, 10.0, pid);
    int sid = doc.addSketch(sk);
    ExtrudeOp ext; ext.setSketchSource(sid); ext.setDistance(10.0);
    ASSERT_TRUE(ext.rebuildProfileFromSketch(doc));
    ASSERT_TRUE(ext.execute(doc));
    int body = doc.getAllBodyIds().front();

    // Shell, opening the RIGHT wall (x=20).
    TopoDS_Face wall = rightWall(doc.getBody(body));
    ASSERT_FALSE(wall.IsNull());
    ShellOp sh;
    sh.setBody(body);
    sh.setThickness(1.0);
    sh.addFaceToRemove(wall);
    ASSERT_TRUE(sh.execute(doc));
    ASSERT_LT(volumeOf(doc.getBody(body)), 2000.0 * 0.9) << "initial shell hollows the box";

    // WIDEN 20 -> 40: the opened right wall moves to x=40. Re-derive the base.
    sk->movePoint(pid[1], {40.0f, 0.0f});
    sk->movePoint(pid[2], {40.0f, 10.0f});
    ASSERT_TRUE(ext.rebuildProfileFromSketch(doc));
    ASSERT_TRUE(ext.execute(doc));
    const double plainWide = volumeOf(doc.getBody(body));
    ASSERT_NEAR(plainWide, 40.0 * 10.0 * 10.0, 1e-6) << "solid widened to 4000";

    // The shell must re-open the MOVED wall and hollow the widened box.
    ASSERT_TRUE(sh.execute(doc)) << "shell must re-find the moved opened wall";
    const TopoDS_Shape shelled = doc.getBody(body);
    EXPECT_TRUE(BRepCheck_Analyzer(shelled).IsValid());
    EXPECT_LT(volumeOf(shelled), plainWide * 0.8)
        << "widened box must still be hollow (wall re-opened, not lost)";
}

// Serialization round-trip of the face refs (additive `facerefs=` key).
TEST(TopoShell, FaceRefsRoundTrip) {
    Document doc;
    int pid[4];
    auto sk = makeRect(20.0, 10.0, pid);
    int sid = doc.addSketch(sk);
    ExtrudeOp ext; ext.setSketchSource(sid); ext.setDistance(10.0);
    ASSERT_TRUE(ext.rebuildProfileFromSketch(doc));
    ASSERT_TRUE(ext.execute(doc));
    int body = doc.getAllBodyIds().front();

    ShellOp sh;
    sh.setBody(body);
    sh.setThickness(1.0);
    sh.addFaceToRemove(rightWall(doc.getBody(body)));
    ASSERT_TRUE(sh.execute(doc));

    const std::string blob = sh.serializeParams();
    EXPECT_NE(blob.find("facerefs="), std::string::npos)
        << "a nameable opened face should persist a topo ref";

    ShellOp sh2;
    ASSERT_TRUE(sh2.deserializeParams(blob));
    // (Deeper resolution is covered by OpenedWallFollowsResize; here we just
    // confirm the additive key parses without disturbing the other params.)
    EXPECT_EQ(sh2.getThickness(), 1.0);
    EXPECT_EQ(sh2.getBodyId(), body);
}
