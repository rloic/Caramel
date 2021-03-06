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

#include "../../symboltable/Symbol.h"
#include "VariableDefinition.h"
#include "../expressions/atomicexpression/Constant.h"
#include "../../symboltable/VariableSymbol.h"
#include "../../../ir/BasicBlock.h"
#include "../../../ir/instructions/CopyInstruction.h"
#include "../../../ir/helpers/IROperatorHelper.h"
#include <utility>


namespace caramel::ast {

VariableDefinition::VariableDefinition(
        antlr4::Token *startToken
)
        : VariableDefinition(Constant::defaultConstant(startToken), startToken) {
}

VariableDefinition::VariableDefinition(
        std::shared_ptr<Expression> const &initializer,
        antlr4::Token *startToken
)
        : Definition(startToken, StatementType::VariableDefinition),
          mSymbol{},
          mInitializer(initializer) {}

VariableSymbol::WeakPtr VariableDefinition::getVariableSymbol() {
    return mSymbol;
}

void VariableDefinition::setVariableSymbol(std::shared_ptr<VariableSymbol> variableSymbol) {
    mSymbol = variableSymbol;
}

void VariableDefinition::acceptAstDotVisit() {
    addNode(thisId(), "VariableDefinition: " + mSymbol.lock()->getName());
    addEdge(thisId(), mSymbol.lock()->thisId());
    addEdge(thisId(), mInitializer->thisId(), "initializer");
    mInitializer->acceptAstDotVisit();
}

std::weak_ptr<Symbol> VariableDefinition::getSymbol() {
    return std::dynamic_pointer_cast<Symbol>(mSymbol.lock());
}

bool VariableDefinition::shouldReturnAnIR() const {
    return true;
}

std::shared_ptr<ir::IR> VariableDefinition::getIR(std::shared_ptr<ir::BasicBlock> &currentBasicBlock) {

//    auto ir = mInitializer->getIR(currentBasicBlock);
    std::string opName = SAFE_ADD_INSTRUCTION(mInitializer, currentBasicBlock); // currentBasicBlock->addInstruction(ir);
    std::string identifier = mSymbol.lock()->getName();

    return std::make_shared<ir::CopyInstruction>(
            currentBasicBlock,
            mSymbol.lock()->getType(),
            identifier,
            opName
    );
}

} // caramel::ast
