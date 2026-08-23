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
    void setSource(const std::string& src) {
        source = src;
    }

    void report(SourceLocation loc, DiagnosticLevel level, const std::string& message) {
        diagnostics.push_back({loc, level, message});
        
        std::string levelStr;
        std::string colorCode;
        switch (level) {
            case DiagnosticLevel::Note: levelStr = "note"; colorCode = "\033[1;36m"; break;
            case DiagnosticLevel::Warning: levelStr = "warning"; colorCode = "\033[1;35m"; break;
            case DiagnosticLevel::Error: levelStr = "error"; colorCode = "\033[1;31m"; break;
            case DiagnosticLevel::Fatal: levelStr = "fatal error"; colorCode = "\033[1;31m"; break;
        }
        std::string resetCode = "\033[0m";

        std::cerr << "\033[1m" << loc.file << ":" << loc.line << ":" << loc.column << ": " 
                  << colorCode << levelStr << ": " << resetCode << message << "\n";
                  
        printSourceLine(loc);
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
    std::string source;

    void printSourceLine(SourceLocation loc) {
        if (source.empty() || loc.line == 0) return;

        // Find the start of the line
        size_t lineStart = 0;
        int currentLine = 1;
        for (size_t i = 0; i < source.length(); ++i) {
            if (currentLine == loc.line) {
                lineStart = i;
                break;
            }
            if (source[i] == '\n') currentLine++;
        }

        // Find the end of the line
        size_t lineEnd = source.find('\n', lineStart);
        if (lineEnd == std::string::npos) lineEnd = source.length();

        std::string lineStr = source.substr(lineStart, lineEnd - lineStart);
        
        // Print the line
        std::cerr << " " << lineStr << "\n";
        
        // Print the caret
        std::cerr << " ";
        for (int i = 1; i < loc.column; ++i) {
            std::cerr << " ";
        }
        std::cerr << "\033[1;32m^\033[0m\n"; // Green caret
    }
};

} // namespace cppx86
