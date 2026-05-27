#pragma once
#include <string>
#include <memory>
#include <TopoDS_Shape.hxx>

class Document; // forward declare

class Operation {
public:
    virtual ~Operation() = default;

    virtual bool execute(Document& doc) = 0;
    virtual bool undo(Document& doc) = 0;
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;

    // For the properties panel — each operation renders its own ImGui editor
    virtual void renderProperties() = 0;

    // Unique type identifier for serialization
    virtual std::string typeId() const = 0;

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

protected:
    bool m_enabled = true;
};
