#pragma once
#include "lexer/Token.h"
#include "diagnostics/DiagnosticEngine.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace cppx86 {

class Preprocessor : public TokenStream {
public:
    Preprocessor(std::unique_ptr<TokenStream> baseStream, DiagnosticEngine& diags);

    Token next() override;
    Token peek() override;

private:
    std::unique_ptr<TokenStream> stream;
    DiagnosticEngine& diags;
    
    std::unordered_map<std::string, std::vector<Token>> macros;
    
    // Stack of active conditional inclusion states
    std::vector<bool> conditionalStack;

    std::vector<Token> buffer; // For macro expansion

    Token advanceToken();
    void handleDirective(Token hashToken);
    void skipLine();
    
    bool isSkipping() const;
};

} // namespace cppx86
