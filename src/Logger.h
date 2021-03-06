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

#pragma once

#include <ostream>
#include <sstream>
#include <iostream>


enum LoggerLevel : unsigned int {
    OFF = 1,
    FATAL = 2,
    WARN = 3,
    INFO = 4,
    DEBUG = 5,
    TRACE = 6,
};

class Logger : public std::ostream {
public:
#define INNER_LOGGER(name, level)                                       \
    class name final {                                                  \
    public:                                                             \
        explicit name(Logger& logger) : mLogger(logger) {}              \
        ~name() { show(); }                                             \
        void show();                                                    \
        template<typename T>                                            \
        name &operator<<(T const &toLog) {                              \
            if (mLogger.getLevel() >= level) {                          \
                mLoggedStream << toLog;                                 \
            }                                                           \
            return *this;                                               \
        }                                                               \
    protected:                                                          \
        Logger &mLogger;                                                \
        std::stringstream mLoggedStream;                                \
    };

    INNER_LOGGER(Fatal, FATAL) // The Fatal logger
    INNER_LOGGER(Warning, WARN) // The Warning logger
    INNER_LOGGER(Info, INFO) // The Info logger
    INNER_LOGGER(Debug, DEBUG) // The Debug logger
    INNER_LOGGER(Trace, TRACE) // The Trace logger

public:
    explicit Logger(LoggerLevel level = LoggerLevel::TRACE);

    LoggerLevel getLevel() const;

    void setLevel(LoggerLevel level);

    Fatal fatal();
    Warning warning();
    Info info();
    Debug debug();
    Trace trace();

private:
    LoggerLevel mLevel;
};

extern Logger logger;
