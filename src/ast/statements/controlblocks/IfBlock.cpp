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

#include "IfBlock.h"


namespace caramel::ast {

IfBlock::IfBlock(
        std::shared_ptr<caramel::ast::Expression> const &condition,
        std::vector<std::shared_ptr<caramel::ast::Statement>> const &thenBlock,
        std::vector<std::shared_ptr<caramel::ast::Statement>> const &elseBlock,
        antlr4::Token *startToken
) : ControlBlock(startToken), mCondition{condition}, mThenBlock{thenBlock}, mElseBlock{elseBlock} {
}

void IfBlock::acceptAstDotVisit() {
    addNode(thisId(), "If");
    visitChildrenAstDot();
}

void IfBlock::visitChildrenAstDot() {
    addEdge(thisId(), mCondition->thisId(), "condition");
    mCondition->acceptAstDotVisit();

    addNode(thisId() + 1, "Then block");
    addEdge(thisId(), thisId() + 1);
    for (const auto &thenStatement : mThenBlock) {
        addEdge(thisId() + 1, thenStatement->thisId());
        thenStatement->acceptAstDotVisit();
    }

    addNode(thisId() + 2, "Else block");
    addEdge(thisId(), thisId() + 2);
    for (const auto &elseStatement : mElseBlock) {
        addEdge(thisId() + 2, elseStatement->thisId());
        elseStatement->acceptAstDotVisit();
    }
}

} // namespace caramel::ast