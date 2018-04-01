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

#include "X86_64IRVisitor.h"
#include "../BasicBlock.h"
#include "../instructions/CopyInstruction.h"
#include "../instructions/EmptyInstruction.h"
#include "../instructions/PrologInstruction.h"
#include "../instructions/EpilogInstruction.h"
#include "../instructions/LDConstInstruction.h"
#include "../instructions/FunctionCallInstruction.h"
#include "../instructions/NopInstruction.h"


namespace caramel::ir::x86_64 {

static constexpr size_t NB_FUNCTION_CALL_REGISTERS = 6;

std::string X86_64IRVisitor::address(std::string const &symbol) {
    return "(" + symbol + ")";
}

std::string X86_64IRVisitor::registerToAssembly(std::string const &register_, size_t bitSize) {
    logger.trace() << "[x86_64] " << "registerToAssembly(" << register_ << ")";
    std::string r;

    std::string sizePrefix;
    switch (bitSize) {
        case 8:
            logger.warning() << "X86_64IRVisitor::registerToAssembly(): using 16bits for 8bits value.";
            sizePrefix = "";
            break;
        case 16:
            sizePrefix = "";
            break;
        case 32:
            sizePrefix = "e";
            break;
        case 64:
            sizePrefix = "r";
            break;
        default:
            logger.fatal() << "Wrong bitSize in X86_64IRVisitor::registerToAssembly().";
            exit(1);
    }

    if (register_ == ir::IR::REGISTER_BASE_POINTER) {
        r = "%" + sizePrefix + "bp";
    } else if (register_ == ir::IR::REGISTER_STACK_POINTER) {
        r = "%" + sizePrefix + "sp";
    } else if (register_ == ir::IR::ACCUMULATOR) {
        r = "%" + sizePrefix + "ax";
    } else {
        throw std::runtime_error("Not valid register for " + register_);
    }

    logger.trace() << "[x86_64] " << "  => " << r;
    return r;
}

std::string X86_64IRVisitor::toAssembly(ir::IR *ir, std::string const &anySymbol, size_t bitSize) {
    logger.trace() << "[x86_64] " << "toAssembly(" << "ir" << ", " << anySymbol << ")";
    std::string r;

    // Is a register
    if (anySymbol[0] == '%') {
        r = registerToAssembly(anySymbol, bitSize);
        // Is a temp var
    } else if (anySymbol[0] == '!') {
        r = registerToAssembly(IR::ACCUMULATOR, bitSize);
        // Is a constant
    } else if (anySymbol[0] >= '0' && anySymbol[0] <= '9') {
        r = "$" + anySymbol;
    } else {
        r = std::to_string(ir->getParentBlock()->getSymbolIndex(anySymbol)) +
            address(registerToAssembly(IR::REGISTER_BASE_POINTER, bitSize));
    }

    logger.trace() << "[x86_64] " << "  => " << r;
    return r;
}

std::string X86_64IRVisitor::getSizeSuffix(size_t bitSize) {
/*
    b (byte) = 8 bits
    w (word) = 16 bits
    l (long) = 32 bits
    q (quad) = 64 bits
 */
    switch (bitSize) {
        case 8: return "b";
        case 16: return "w";
        case 32: return "l";
        case 64: return "q";
        default:
            logger.fatal() << "Wrong bitSize in X86_64IRVisitor::getSizeSuffix(" << bitSize << ").";
            exit(1);
    }
}

std::string X86_64IRVisitor::getFunctionCallRegister(size_t index, size_t bitSize) {
    static bool init = false;
    static std::map<size_t, std::map<size_t, std::string>> FC_REGISTERS;
    if (!init) {
        FC_REGISTERS[0][8] = "%di"; // < using the 16-bit register
        FC_REGISTERS[0][16] = "%di";
        FC_REGISTERS[0][32] = "%edi";
        FC_REGISTERS[0][64] = "%rdi";
        FC_REGISTERS[1][8] = "%si"; // < using the 16-bit register
        FC_REGISTERS[1][16] = "%si";
        FC_REGISTERS[1][32] = "%esi";
        FC_REGISTERS[1][64] = "%rsi";
        FC_REGISTERS[2][8] = "%dx"; // < using the 16-bit register
        FC_REGISTERS[2][16] = "%dx";
        FC_REGISTERS[2][32] = "%edx";
        FC_REGISTERS[2][64] = "%rdx";
        FC_REGISTERS[3][8] = "%cx"; // < using the 16-bit register
        FC_REGISTERS[3][16] = "%cx";
        FC_REGISTERS[3][32] = "%ecx";
        FC_REGISTERS[3][64] = "%rcx";
        FC_REGISTERS[4][8] = "%r8d"; // < using the 32-bit register
        FC_REGISTERS[4][16] = "%r8d"; // < using the 32-bit register
        FC_REGISTERS[4][32] = "%r8d";
        FC_REGISTERS[4][64] = "%r8";
        FC_REGISTERS[5][8] = "%r9d"; // < using the 32-bit register
        FC_REGISTERS[5][16] = "%r9d"; // < using the 32-bit register
        FC_REGISTERS[5][32] = "%r9d";
        FC_REGISTERS[5][64] = "%r9";
        init = true;
    }

    if (index >= NB_FUNCTION_CALL_REGISTERS) {
        logger.fatal() << "There are only " << NB_FUNCTION_CALL_REGISTERS << " function call "
                       << "registers, but you're asking for the " << index << "-th one!";
        exit(1);
    }
    if (FC_REGISTERS[index].find(bitSize) == FC_REGISTERS[index].end()) {
        logger.fatal() << "The size " << bitSize << " is not valid!";
        exit(1);
    }
    return FC_REGISTERS[index][bitSize];
}

void X86_64IRVisitor::visitCopy(caramel::ir::CopyInstruction *instruction, std::ostream &os) {
    logger.trace() << "[x86_64] " << "visiting copy: " << instruction->getReturnName();

    os << "  movl    " << toAssembly(instruction, instruction->getSource())
       << ", " << toAssembly(instruction, instruction->getDestination());
}

void X86_64IRVisitor::visitEmpty(caramel::ir::EmptyInstruction *instruction, std::ostream &os) {
    logger.trace() << "[x86_64] " << "visiting empty";

    CARAMEL_UNUSED(instruction);
    CARAMEL_UNUSED(os);
}

void X86_64IRVisitor::visitProlog(caramel::ir::PrologInstruction *instruction, std::ostream &os) {
    logger.trace() << "[x86_64] " << "visiting prolog: " << instruction->getFunctionName();

/*
 * Example function, with 9 32-bit arguments.

baz:
  pushq %rbp
  movq %rsp, %rbp
  subq $24, %rsp
  movl %edi, -4(%rbp)
  movl %esi, -8(%rbp)
  movl %edx, -12(%rbp)
  movl %ecx, -16(%rbp)
  movl %r8d, -20(%rbp)
  movl %r9d, -24(%rbp)
  nop
  leave
  ret
 */

    os << "  pushq   %rbp" << '\n'
       << "  movq    %rsp, %rbp" << '\n';

    size_t rspOffset = 0;
    for (auto const &parameter : instruction->getParameters()) {
        rspOffset += parameter.primaryType->getMemoryLength() / 8U;
    }
    if (rspOffset > 0) {
        os << "  subq    $" << rspOffset << ", %rsp" << '\n';
    }

    size_t i = 0;
    for (auto const &parameter : instruction->getParameters()) {
        size_t parameterSize = parameter.primaryType->getMemoryLength();
        if (i != 0) os << '\n';
        instruction->getParentBlock()->addSymbol(parameter.name, parameter.primaryType);
        os << "  mov" + getSizeSuffix(parameterSize) + "    " << getFunctionCallRegister(i, parameterSize)
           << ", " << toAssembly(instruction, parameter.name, 64); // it's always %rbp
        i++;
    }
}

void X86_64IRVisitor::visitEpilog(caramel::ir::EpilogInstruction *instruction, std::ostream &os) {
    logger.trace() << "[x86_64] " << "visiting epilog";

    CARAMEL_UNUSED(instruction);
//    os << "  popq    %rbp" << std::endl;
    os << "  leave" << std::endl;
    os << "  ret";
}

void X86_64IRVisitor::visitAddition(caramel::ir::AdditionInstruction *instruction, std::ostream &os) {
    logger.trace() << "[x86_64] " << "visiting addition: "; // TODO: complete the trace

    // Todo: create visitAddition
    CARAMEL_UNUSED(instruction);
    CARAMEL_UNUSED(os);
}

void X86_64IRVisitor::visitLdConst(caramel::ir::LDConstInstruction *instruction, std::ostream &os) {
    logger.trace() << "[x86_64] " << "visiting ldconst: " << instruction->getReturnName() << " = "
                   << instruction->getValue();

    os << "  movl    " << toAssembly(instruction, instruction->getValue())
       << ", " << toAssembly(instruction, instruction->getDestination());
}

void X86_64IRVisitor::visitNope(caramel::ir::NopInstruction *instruction, std::ostream &os) {
    logger.trace() << "[x86_64] " << "visiting nop";

    CARAMEL_UNUSED(instruction);
    os << "  nop";
}

void X86_64IRVisitor::visitFunctionCall(caramel::ir::FunctionCallInstruction *instruction, std::ostream &os) {
    logger.trace() << "[x86_64] " << "visiting functionCall: " << instruction->getFunctionName();

/*
 * Example function call, with 9 arguments.

int32_t bar(int32_t a, int32_t b, int32_t c,
            int32_t d, int32_t e, int32_t f,
            int32_t g, int32_t h, int32_t i) {}

pushq $9        // < 8
pushq $8        // < 7
pushq $7        // < 6
movl $6, %r9d   // < 5
movl $5, %r8d   // < 4
movl $4, %ecx   // < 3
movl $3, %edx   // < 2
movl $2, %esi   // < 1
movl $1, %edi   // < 0
call bar
addq $24, %rsp  // 24 = 3 * 8
*/

    auto const &arguments = instruction->getArguments();
    auto const &parameters = instruction->getParameters();
    const size_t nbParameters = parameters.size();
    size_t stackOffset = 0;
    for (size_t i = 0; i < nbParameters; ++i) {
        const size_t j = nbParameters - (i + 1);
        const size_t parameterSize = parameters[j].primaryType->getMemoryLength();

        if (j >= NB_FUNCTION_CALL_REGISTERS) {
            if (parameterSize < 32) {
                logger.warning() << "Wow! I don't know if I can handle a less-than-32-bit parameter!";
            }
            os << "  pushq   " << toAssembly(instruction, arguments[j], 64) << '\n';
            stackOffset += 8;
//            stackOffset += parameters[j].primaryType->getMemoryLength() / 8U;
        } else {
            os << "  mov" + getSizeSuffix(64) + "    " << toAssembly(instruction, arguments[j], 64)
               << ", " << getFunctionCallRegister(j, 64) << '\n';
        }
    }

    os << "  call    " << instruction->getFunctionName();

    if (stackOffset > 0) {
        os << '\n'
           << "  addq    $" << stackOffset << ", %rsp";
    }
}

} // namespace caramel::ir::x86_64