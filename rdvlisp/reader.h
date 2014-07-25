//
//  reader.h
//  rdvlisp
//
//  Created by Ruben De Visscher on 29/06/14.
//  Copyright (c) 2014 Ruben De Visscher. All rights reserved.
//

#ifndef __rdvlisp__reader__
#define __rdvlisp__reader__

#include <iostream>
#include <vector>
#include "types.h"
#include "ast.h"

namespace rdvlisp {
    class ReadError : public std::runtime_error {
    private:
        template <typename T>
        static std::string stringify(const T& what, size_t start, size_t end) {
            std::stringstream ss;
            ss << "at position [" << start << ", ";
            if(end != -1) {
                ss << end;
            }
            ss << "[" << ": " << what;
            return ss.str();
        }
    public:
        template <typename T>
        ReadError(const T& what, size_t start, size_t end) : std::runtime_error(stringify(what, start, end)) {}
    };
    
    template <typename T, typename E=std::string>
    class Result {
        boost::variant<T, E> variant;
    public:
        size_t start, end;
        bool fail() const {
            class visitor : public boost::static_visitor<bool> {
            public:
                bool operator()(T t) const {
                    return false;
                }
                bool operator()(E e) const {
                    return true;
                }
            };
            return boost::apply_visitor(visitor(), variant);
        }
        bool good() const {
            return !fail();
        }
        T get() const {
            if(fail()) {
                throw ReadError(boost::get<E>(variant), start, end);
            } else {
                return boost::get<T>(variant);
            }
        }
        E error() const {
            if(good()) {
                return E();
            } else {
                return boost::get<E>(variant);
            }
        }
        Result(T t, size_t start, size_t end) : variant(t), start(start), end(end) {}
        Result(E e, size_t start, size_t end) : variant(e), start(start), end(end) {}
    };
    
    Result<ast::ExpressionRef> read(const std::string& source, size_t start=0);
}

#endif /* defined(__rdvlisp__reader__) */
