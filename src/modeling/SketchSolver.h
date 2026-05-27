#pragma once
#include "Sketch.h"
#include "SketchConstraints.h"
#include <vector>

namespace materializr {

enum class SketchState { FullyConstrained, UnderConstrained, OverConstrained };

class SketchSolver {
public:
    SketchSolver();

    // Add/remove constraints
    int addConstraint(const Constraint& c);
    void removeConstraint(int id);
    const std::vector<Constraint>& getConstraints() const;

    // Solve: adjust point positions to satisfy constraints
    // Returns true if converged
    bool solve(Sketch& sketch, int maxIterations = 50, double tolerance = 1e-6);

    // Query state
    SketchState getState() const;
    int degreesOfFreedom() const;

    void clear();

private:
    std::vector<Constraint> m_constraints;
    int m_nextId = 1;
    SketchState m_state = SketchState::UnderConstrained;
    int m_dof = 0;

    // Per-constraint error and correction
    double computeError(const Constraint& c, const Sketch& sketch) const;
    void applyCorrection(const Constraint& c, Sketch& sketch, double error) const;
};

} // namespace materializr
