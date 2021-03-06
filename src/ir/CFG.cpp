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

#include "CFG.h"
#include "BasicBlock.h"
#include "../Console.h"
#include "../utils/Common.h"

namespace caramel::ir {

using namespace colors;
using namespace caramel::utils;

CFG::CFG(
        std::string const &fileName,
        ast::Context::Ptr treeContext
) : mFileName{fileName},
    mRootContext{std::move(treeContext)},
    mSymbols{},
    mSymbolIndex{},
    mNextBasicBlockNumber{0},
    mNextFunctionContext{0},
    mFunctionsBasicBlocks{},
    mGlobalContext{} {
    logger.debug() << "New CFG for " << mFileName << ".";

    for (ast::Statement::Ptr const &statement : mRootContext->getStatements()) {
        if (statement->getType() == ast::StatementType::FunctionDefinition) {

            auto[function_begin, function_end] = statement->getBasicBlock(this);
            mFunctionsBasicBlocks.push_back(function_begin);
            CARAMEL_UNUSED(function_end);

        } else {
            logger.warning() << "Skipping not-yet-handled non function definition statement in CFG::CFG().";

            if (statement->shouldReturnAnIR()) {
                mGlobalContext.push_back(castTo<caramel::ast::Declaration::Ptr>(statement));
            }

            // old code:
//            if (statement->shouldReturnAnIR()) {
//                mBasicBlocks[0]->addInstruction(statement->getIR(mBasicBlocks[0]));
//            } else if (statement->shouldReturnABasicBlock()) {
//                mBasicBlocks.push_back(statement->getFunctionBasicBlock(this));
//            } else {
//                if (statement->getType() != ast::StatementType::FunctionDeclaration) {
//                    logger.warning() << "[CFG] Statement return neither IR nor BB: " << statement->getType();
//                }
//            }
        }
    }
}

void CFG::addBasicBlock(std::shared_ptr<BasicBlock> basicBlock) {
    mFunctionsBasicBlocks.push_back(basicBlock);
}

bool CFG::hasSymbol(size_t controlBlockId, std::string const &symbolName) {
    return mSymbols[controlBlockId].find(symbolName) != mSymbols[controlBlockId].end()
           || mSymbols[0].find(symbolName) != mSymbols[0].end();
}

bool CFG::isSymbolParamArray(size_t controlBlockId, std::string const &symbolName) {
    if (!hasSymbol(controlBlockId, symbolName)) {
        logger.fatal() << "Calling CFG::isSymbolArray(" << controlBlockId << ", " << symbolName << ") on unregistered symbol.";
        exit(1);
    }
    return mSymbolIsParamArray[controlBlockId][symbolName];
}

long CFG::addParamArraySymbol(size_t controlBlockId, std::string const &symbolName, ast::PrimaryType::Ptr type) {
    mSymbolIsParamArray[controlBlockId][symbolName] = true;
    long index = mStackSize[controlBlockId] - type->getMemoryLength() / 8U - 4;
    return addSymbol(controlBlockId, symbolName, type, index);
}

long CFG::addArraySymbol(size_t controlBlockId,
                         std::string const &symbolName, ast::PrimaryType::Ptr type, size_t length) {
    if (length == 0) {
        logger.fatal() << "You can't use 0-length arrays.";
        exit(1);
    }
    for (size_t i = 0; i < length - 1; ++i) {
        size_t ip = length - (i + 1);
        logger.trace() << "[CFG] Adding array symbol element " << yellow << "@" << controlBlockId << magenta << ": "
                       << grey << symbolName << "[" << ip << "]" << " of type " << type->getIdentifier();
        addSymbol(controlBlockId, symbolName + '[' + std::to_string(ip) + ']', type);
    }
    logger.trace() << "[CFG] Adding array symbol element " << yellow << "@" << controlBlockId << magenta << ": "
                   << grey << symbolName << "[0]" << " of type " << type->getIdentifier();
    return addSymbol(controlBlockId, symbolName, type);
}

long CFG::addSymbol(size_t controlBlockId, std::string const &symbolName, caramel::ast::PrimaryType::Ptr type) {
    logger.trace() << "[CFG] Adding symbol " << yellow << "@" << controlBlockId << magenta << ": "
                   << grey << symbolName << " of type " << type->getIdentifier();

    if (hasSymbol(controlBlockId, symbolName)) {
        long existingIndex = getSymbolIndex(controlBlockId, symbolName);
        logger.warning() << "[CFG] Symbol " << symbolName << " already exists with index " << existingIndex << ".";
        return existingIndex;
    }

    if (type->getMemoryLength() == 0) {
        logger.fatal() << "Can't add symbol of size 0!";
        exit(1);
    }

    if (mSymbolIsParamArray[controlBlockId].find(symbolName) == mSymbolIsParamArray[controlBlockId].end()) {
        mSymbolIsParamArray[controlBlockId][symbolName] = false;
    }
    mSymbols[controlBlockId][symbolName] = type;
    mStackSize[controlBlockId] = mStackSize[controlBlockId] - type->getMemoryLength() / 8U;
    mTopStackMemberSize[controlBlockId] = type->getMemoryLength();
    mSymbolIndex[controlBlockId][symbolName] = mStackSize[controlBlockId];

    logger.trace() << "[CFG] " << "  => " << mStackSize[controlBlockId];
    return mStackSize[controlBlockId];
}

long CFG::addSymbol(size_t controlBlockId, std::string const &symbolName, ast::PrimaryType::Ptr type, long index) {
    logger.trace() << "[CFG] Adding symbol: " << grey << symbolName << " of type " << type->getIdentifier()
                   << yellow << " at " << index;

    if (hasSymbol(controlBlockId, symbolName)) {
        long existingIndex = getSymbolIndex(controlBlockId, symbolName);
        logger.warning() << "[CFG] Symbol " << symbolName << "already exists with index " << existingIndex << ".";
        return existingIndex;
    }

    if (mSymbolIsParamArray[controlBlockId].find(symbolName) == mSymbolIsParamArray[controlBlockId].end()) {
        mSymbolIsParamArray[controlBlockId][symbolName] = false;
    }
    mSymbols[controlBlockId][symbolName] = type;
    mSymbolIndex[controlBlockId][symbolName] = index;
    mTopStackMemberSize[controlBlockId] = type->getMemoryLength();
    if (index < 0) {
        mStackSize[controlBlockId] = size_t(index);
    }

    logger.trace() << "[CFG] " << "  => " << index;
    return index;
}

long CFG::getSymbolIndex(size_t controlBlockId, std::string const &symbolName) {
    if (mSymbolIndex[controlBlockId].find(symbolName) != mSymbolIndex[controlBlockId].end()) {
        return mSymbolIndex[controlBlockId][symbolName];
    } else if (mSymbolIndex[0].find(symbolName) != mSymbolIndex[0].end()) {
        return mSymbolIndex[0][symbolName];
    } else {
        return 0;
    }
}

void CFG::enterFunction(size_t controlBlockId) {
    mStackSize[controlBlockId] = 0;
    // FIXME: This won't work for nested BB
}

void CFG::leaveFunction(size_t controlBlockId) {
    CARAMEL_UNUSED(controlBlockId);
}

std::shared_ptr<BasicBlock> CFG::generateBasicBlock(std::string entryName) {
    return std::make_shared<BasicBlock>(++mNextBasicBlockNumber, mNextFunctionContext, this, entryName);
}

std::shared_ptr<BasicBlock> CFG::generateFunctionBlock(std::string entryName) {
    return std::make_shared<BasicBlock>(++mNextBasicBlockNumber, ++mNextFunctionContext, this, entryName);
}

std::vector<std::shared_ptr<BasicBlock>> &CFG::getBasicBlocks() {
    return mFunctionsBasicBlocks;
}

std::string &CFG::getFileName() {
    return mFileName;
}

std::ostream &operator<<(std::ostream &os, CFG const &cfg) {
    os << "CFG:\n"
       << " - mFileName: " << cfg.mFileName << '\n'
       << " - mRootContext: " << *cfg.mRootContext << '\n'
       << " - mSymbols & mSymbolsIndex:\n";
    for (auto const &[functionContextId, symbols] : cfg.mSymbols) {
        for (auto const &[name, type] : symbols) {
            os << "    - BB=" << functionContextId
               << ", name=" << name
               << ", type=" << type->getIdentifier()
               << ", index=" << cfg.mSymbolIndex.at(functionContextId).at(name)
               << ", isParamArray=" << std::boolalpha << cfg.mSymbolIsParamArray.at(functionContextId).at(name)
               << '\n';
        }
    }
    os << " - mStackSize:\n";
    for (auto const &[basicBlockId, size] : cfg.mStackSize) {
        os << "    - BB=" << basicBlockId << ", size=" << size << '\n';
    }
    os << " - mTopStackMemberSize:\n";
    for (auto const &[basicBlockId, size] : cfg.mTopStackMemberSize) {
        os << "    - BB=" << basicBlockId << ", size=" << size << '\n';
    }
    os << " - mNextBasicBlockNumber: " << cfg.mNextBasicBlockNumber << '\n'
       << " - mBasicBlocks: " << cfg.mFunctionsBasicBlocks.size() << " BBs";
    return os;
}

std::shared_ptr<BasicBlock> CFG::generateNamedBasicBlock() {
    return std::make_shared<BasicBlock>(++mNextBasicBlockNumber, mNextFunctionContext, this,
                                        BasicBlock::getNextNumberName());
}

size_t CFG::getStackSize(size_t functionBasicBlockIndex) const {
    long topStackMemberSize = mTopStackMemberSize.find(functionBasicBlockIndex) == mTopStackMemberSize.end()
                              ? 0U : mTopStackMemberSize.at(functionBasicBlockIndex);
    long stackSize = -mStackSize.at(functionBasicBlockIndex) + topStackMemberSize / 8U;
    if (stackSize < 0) {
        logger.fatal() << "Negative stack size: " << stackSize << ". CFG: " << *this;
        exit(0);
    }
    return size_t(stackSize);
}

void CFG::addFunctionBBEnd(size_t functionId, BasicBlock::Ptr functionBBEnd) {
    mFunctionBBEnd[functionId] = std::move(functionBBEnd);
}

std::shared_ptr<BasicBlock> CFG::getFunctionEndBasicBlock(size_t functionBasicBlockIndex) {
    return mFunctionBBEnd.at(functionBasicBlockIndex);
}

void CFG::pushCurrentControlBlockEndBB(BasicBlock::Ptr bbend) {
    logger.trace() << "[CFG] Pushing control-block end-BB: " << bbend->getLabelName();

    mControlBlockEndBBStack.push(bbend);
}

void CFG::popCurrentControlBlockEndBB() {
    logger.trace() << "[CFG] Popping control-block end-BB.";

    mControlBlockEndBBStack.pop();
}

BasicBlock::Ptr CFG::getCurrentControlBlockEndBB() {
    logger.trace() << "[CFG] Getting control-block end-BB:";

    if (mControlBlockEndBBStack.empty()) {
        logger.fatal() << "The control-block end-BB stack is empty!";
        exit(1);
    }
    logger.trace() << "[CFG] => " << mControlBlockEndBBStack.top()->getLabelName();
    return mControlBlockEndBBStack.top();
}

} // namespace caramel::ir
