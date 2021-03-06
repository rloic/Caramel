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

#include "EmptyInstruction.h"
#include "../IRVisitor.h"
#include "../../ast/symboltable/PrimaryType.h"

namespace caramel::ir {

EmptyInstruction::EmptyInstruction(
        std::shared_ptr<BasicBlock> parentBlock,
        ast::PrimaryType::Ptr const &type
) : IR(Operation::empty, parentBlock, type) {}

EmptyInstruction::EmptyInstruction(
        std::string const &returnName, std::shared_ptr<BasicBlock> parentBlock,
        std::shared_ptr<ast::PrimaryType> const &type
) : IR(
        returnName,
        Operation::empty,
        parentBlock,
        type
) {}

void EmptyInstruction::accept(std::shared_ptr<IRVisitor> const &visitor, std::ostream &os) {
    visitor->visitEmpty(this, os);
}

} // namespace caramel::ir
