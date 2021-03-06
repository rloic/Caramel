/*
 * MIT License
 *
 * Copyright (c) 2018 Kalate Hexanome, 4IF, INSA Lyon
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

#include "PrefixOperatorIndex.h"
#include "PreIncrOperator.h"
#include "PreDecrOperator.h"
#include "LogicalNotOperator.h"
#include "PreNegOperator.h"

#define BIND(op) index.insert(make_pair(op::SYMBOL, dynamic_pointer_cast<UnaryOperator>(make_shared<op>())))

using namespace std;

caramel::ast::PrefixOperatorIndex::PrefixOperatorIndex() {

    BIND(PreIncrOperator);
    BIND(PreDecrOperator);

    BIND(LogicalNotOperator);

    BIND(PreNegOperator);
}

caramel::ast::UnaryOperator::Ptr caramel::ast::PrefixOperatorIndex::getOpForToken(std::string token) {
    auto it = index.find(token);

    if (it == index.end()) {
        return nullptr;
    }

    return it->second;
}