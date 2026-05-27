#pragma once
#include "../core/Operation.h"
#include "../core/Document.h"
#include <TopoDS_Wire.hxx>
#include <vector>
#include <string>

class LoftOp : public Operation {
public:
    LoftOp();
    ~LoftOp() override = default;

    // Parameters
    void addProfile(const TopoDS_Wire& wire);
    void clearProfiles();
    void setSolid(bool solid);   // true = solid, false = shell
    void setRuled(bool ruled);   // true = ruled surface, false = smooth

    // Getters
    bool isSolid() const { return m_solid; }
    bool isRuled() const { return m_ruled; }
    int profileCount() const { return static_cast<int>(m_profiles.size()); }

    // Operation interface
    bool execute(Document& doc) override;
    bool undo(Document& doc) override;
    std::string name() const override { return "Loft"; }
    std::string description() const override;
    void renderProperties() override;
    std::string typeId() const override { return "loft"; }

private:
    std::vector<TopoDS_Wire> m_profiles;
    bool m_solid = true;
    bool m_ruled = false;

    // For undo
    int m_createdBodyId = -1;
};
