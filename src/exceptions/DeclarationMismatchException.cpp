/*
 * MIT License
 *
 * Copyright (c) 2018 insa.4if.hexanome_kalate
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include "DeclarationMismatchException.h"
#include "../ast/symboltable/Symbol.h"


namespace caramel::exceptions {

using namespace ast;


DeclarationMismatchException::DeclarationMismatchException(antlr4::ParserRuleContext *antlrContext,
                                                           std::string const &name,
                                                           ast::SymbolType const &requiredSymbolType,
                                                           std::shared_ptr<ast::Symbol> symbol)
        : SemanticError(buildMismatchSymbolTypeErrorMessage(name, requiredSymbolType, symbol), antlrContext) {}

DeclarationMismatchException::DeclarationMismatchException(antlr4::ParserRuleContext *antlrContext,
                                                           std::string const &name,
                                                           std::shared_ptr<PrimaryType> const &requiredType,
                                                           std::shared_ptr<ast::Symbol> symbol)
        : SemanticError(buildMismatchTypeErrorMessage(name, requiredType, symbol), antlrContext) {}

std::string DeclarationMismatchException::buildMismatchSymbolTypeErrorMessage(std::string const &variableName,
                                                                              ast::SymbolType const &requiredSymbolType,
                                                                              std::shared_ptr<ast::Symbol> symbol) {
    std::stringstream res;
    res << "Cannot define a ";
    switch (requiredSymbolType) {
        case SymbolType::VariableSymbol:
            res << "variable";
            break;
        case SymbolType::ArraySymbol:
            res << "array";
            break;
        case SymbolType::FunctionSymbol:
            res << "function";
            break;
        case SymbolType::TypeSymbol:
            res << "type";
            break;
    }
    res << " which has a previous occurrence as a " << symbol->getSymbolType();
    return res.str();
}

std::string
DeclarationMismatchException::buildMismatchTypeErrorMessage(std::string const &variableName,
                                                            std::shared_ptr<PrimaryType> const &requiredType,
                                                            std::shared_ptr<ast::Symbol> symbol) {
    //TODO never trigered in tests
    std::stringstream res;
    res << "Mismatch type for " << variableName << " between the type definition " << requiredType->getIdentifier()
        << " and declaration type " <<symbol->getType()->getIdentifier() << '.';
    return res.str();
}

} // namespace caramel::exceptions