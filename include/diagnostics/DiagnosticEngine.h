#pragma once
#include "common/SourceLocation.h"
#include <string>
#include <vector>
#include <iostream>

namespace cppx86 {

enum class DiagnosticLevel {
    Note,
    Warning,
    Error,
    Fatal
};

struct Diagnostic {
    SourceLocation location;
    DiagnosticLevel level;
    std::string message;
};

class DiagnosticEngine {
public:
    void report(SourceLocation loc, DiagnosticLevel level, const std::string& message) {
        diagnostics.push_back({loc, level, message});
        
        std::string levelStr;
        switch (level) {
            case DiagnosticLevel::Note: levelStr = "note"; break;
            case DiagnosticLevel::Warning: levelStr = "warning"; break;
            case DiagnosticLevel::Error: levelStr = "error"; break;
            case DiagnosticLevel::Fatal: levelStr = "fatal error"; break;
        }

        std::cerr << loc.file << ":" << loc.line << ":" << loc.column 
                  << ": " << levelStr << ": " << message << "\n";
    }

    void error(SourceLocation loc, const std::string& message) {
        report(loc, DiagnosticLevel::Error, message);
    }

    void warning(SourceLocation loc, const std::string& message) {
        report(loc, DiagnosticLevel::Warning, message);
    }

    void note(SourceLocation loc, const std::string& message) {
        report(loc, DiagnosticLevel::Note, message);
    }

    bool hasErrors() const {
        for (const auto& diag : diagnostics) {
            if (diag.level == DiagnosticLevel::Error || diag.level == DiagnosticLevel::Fatal) {
                return true;
            }
        }
        return false;
    }

private:
    std::vector<Diagnostic> diagnostics;
};

} // namespace cppx86
