#include "preprocessor/Preprocessor.h"

namespace cppx86 {

Preprocessor::Preprocessor(std::unique_ptr<TokenStream> baseStream, DiagnosticEngine& diags)
    : stream(std::move(baseStream)), diags(diags) {
}

bool Preprocessor::isSkipping() const {
    for (bool state : conditionalStack) {
        if (!state) return true;
    }
    return false;
}

Token Preprocessor::next() {
    if (!buffer.empty()) {
        Token t = buffer.front();
        buffer.erase(buffer.begin());
        return t;
    }
    return advanceToken();
}

Token Preprocessor::peek() {
    if (!buffer.empty()) {
        return buffer.front();
    }
    Token t = advanceToken();
    buffer.push_back(t);
    return t;
}

Token Preprocessor::advanceToken() {
    while (true) {
        Token t = stream->next();
        
        if (t.kind == TokenKind::EndOfFile) {
            return t;
        }
        
        if (t.kind == TokenKind::Hash) {
            // Check if this hash is the first token on its line
            // A true C++ preprocessor knows if it's the first non-whitespace character.
            // For now, we assume any stray '#' initiates a directive.
            handleDirective(t);
            continue;
        }

        if (isSkipping()) {
            continue;
        }

        // Macro expansion
        if (t.kind == TokenKind::Identifier) {
            auto it = macros.find(t.text);
            if (it != macros.end()) {
                // simple object-like macro expansion
                for (const auto& macroTok : it->second) {
                    buffer.push_back(macroTok);
                }
                if (!buffer.empty()) {
                    Token expanded = buffer.front();
                    buffer.erase(buffer.begin());
                    return expanded;
                }
                continue;
            }
        }
        
        return t;
    }
}

void Preprocessor::skipLine() {
    int currentLine = stream->peek().location.line;
    while (stream->peek().kind != TokenKind::EndOfFile && stream->peek().location.line == currentLine) {
        stream->next();
    }
}

void Preprocessor::handleDirective(Token hashToken) {
    int directiveLine = hashToken.location.line;
    Token directive = stream->next();
    
    if (directive.location.line != directiveLine) {
        diags.error(hashToken.location, "Null directive");
        return;
    }
    
    if (directive.kind != TokenKind::Identifier) {
        diags.error(directive.location, "Expected directive name");
        skipLine();
        return;
    }

    std::string name = directive.text;
    
    if (name == "ifdef") {
        Token macroName = stream->next();
        if (macroName.kind != TokenKind::Identifier) {
            diags.error(macroName.location, "Expected macro name");
        } else {
            bool isDefined = macros.find(macroName.text) != macros.end();
            conditionalStack.push_back(isDefined);
        }
        skipLine();
    } else if (name == "ifndef") {
        Token macroName = stream->next();
        if (macroName.kind != TokenKind::Identifier) {
            diags.error(macroName.location, "Expected macro name");
        } else {
            bool isDefined = macros.find(macroName.text) != macros.end();
            conditionalStack.push_back(!isDefined);
        }
        skipLine();
    } else if (name == "else") {
        if (conditionalStack.empty()) {
            diags.error(directive.location, "#else without #ifdef/#ifndef");
        } else {
            conditionalStack.back() = !conditionalStack.back();
        }
        skipLine();
    } else if (name == "endif") {
        if (conditionalStack.empty()) {
            diags.error(directive.location, "#endif without #ifdef/#ifndef");
        } else {
            conditionalStack.pop_back();
        }
        skipLine();
    } else if (isSkipping()) {
        // If we are skipping, ignore #define, #undef, etc.
        skipLine();
    } else if (name == "define") {
        Token macroName = stream->next();
        if (macroName.kind != TokenKind::Identifier) {
            diags.error(macroName.location, "Expected macro name");
            skipLine();
            return;
        }
        std::vector<Token> replacement;
        while (stream->peek().kind != TokenKind::EndOfFile && stream->peek().location.line == directiveLine) {
            replacement.push_back(stream->next());
        }
        macros[macroName.text] = replacement;
    } else if (name == "undef") {
        Token macroName = stream->next();
        if (macroName.kind != TokenKind::Identifier) {
            diags.error(macroName.location, "Expected macro name");
        } else {
            macros.erase(macroName.text);
        }
        skipLine();
    } else {
        diags.warning(directive.location, "Unknown preprocessor directive: " + name);
        skipLine();
    }
}

} // namespace cppx86
