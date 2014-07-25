//
//  types.h
//  rdvlisp
//
//  Created by Ruben De Visscher on 29/06/14.
//  Copyright (c) 2014 Ruben De Visscher. All rights reserved.
//

#ifndef __rdvlisp__types__
#define __rdvlisp__types__

#include <iostream>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <vector>
#include <sstream>

namespace rdvlisp {
    namespace types {
        class Type;
        typedef std::shared_ptr<Type> TypeRef;
        template <typename T>
        TypeRef ref(T t) {
            return std::make_shared<Type>(t);
        }
        
        class pod {
        public:
            uint64_t bits;
            pod(uint64_t bits) : bits(bits) {
                if(bits == 0) {
                    throw std::out_of_range("number of bits must be non-zero");
                }
            }
        };
        class Integer : public pod {
        public:
            bool is_signed;
            Integer(uint64_t bits, bool is_signed=true) : pod(bits), is_signed(is_signed) {}
        };
        class FloatingPoint : public pod {
        public:
            FloatingPoint(uint64_t bits) : pod(bits) {}
        };
        class Boolean : public pod {
        public:
            Boolean() : pod(1) {}
        };
        class Array {
        public:
            boost::optional<uint64_t> length;
            TypeRef inner_type;
            Array(TypeRef inner_type, uint64_t length) : inner_type(inner_type), length(length) {
                if(length == 0) {
                    throw std::out_of_range("array length must be non-zero");
                }
            }
            Array(TypeRef inner_type) : inner_type(inner_type), length() {}
        };
        class Function {
        public:
            TypeRef return_type;
            std::vector<TypeRef> argument_types;
            bool is_vararg;
            Function(TypeRef return_type, std::vector<TypeRef> argument_types, bool is_vararg=false)
            : return_type(return_type), argument_types(argument_types), is_vararg(is_vararg) {}
        };
        class TypeVariable {
        public:
            std::string name;
            TypeVariable(const std::string& name) : name(name) {
                if(name.length() == 0) {
                    throw std::invalid_argument("name must not be an empty string");
                }
            }
        };
        class Undetermined {
        public:
        };
        class String {
        public:
        };
        class Keyword {
        public:
        };
        
        class Type {
        public:
            boost::variant<Integer, FloatingPoint, Array, Function, TypeVariable, Undetermined, String, Keyword> variant;
            template <typename T>
            Type(T t) : variant(t) {}
        };
        
        static const types::TypeRef sint8(new types::Type(Integer(8, true)));
        static const types::TypeRef uint8(new types::Type(Integer(8, false)));
        static const types::TypeRef sint16(new types::Type(Integer(16, true)));
        static const types::TypeRef uint16(new types::Type(Integer(16, false)));
        static const types::TypeRef sint32(new types::Type(Integer(32, true)));
        static const types::TypeRef uint32(new types::Type(Integer(32, false)));
        static const types::TypeRef sint64(new types::Type(Integer(64, true)));
        static const types::TypeRef uint64(new types::Type(Integer(64, false)));
        
        static const types::TypeRef float32(new types::Type(FloatingPoint(32)));
        static const types::TypeRef float64(new types::Type(FloatingPoint(64)));
        
        static const types::TypeRef string(new types::Type(String()));
        static const types::TypeRef undetermined(new types::Type(Undetermined()));
        
        class print_visitor : public boost::static_visitor<std::string> {
        public:
            std::string operator()(Undetermined t) const {
                return "undetermined";
            }
            std::string operator()(String t) const {
                return "string";
            }
            std::string operator()(Keyword t) const {
                return "keyword";
            }
            std::string operator()(TypeVariable t) const {
                std::string result;
                return t.name;
            }
            std::string operator()(Array t) const {
                std::stringstream ss;
                ss << "(array " << boost::apply_visitor(print_visitor(), t.inner_type->variant);
                if(t.length) {
                    ss << " " << *t.length;
                }
                ss << ")";
                return ss.str();
            }
            std::string operator()(Integer t) const {
                std::stringstream ss;
                ss << "(integer " << t.bits << " " << t.is_signed << ")";
                return ss.str();
            }
            std::string operator()(FloatingPoint t) const {
                std::stringstream ss;
                ss << "(floating-point " << t.bits << ")";
                return ss.str();
            }
            std::string operator()(Function t) const {
                std::stringstream ss;
                ss << "(function " << boost::apply_visitor(print_visitor(), t.return_type->variant);
                if(t.argument_types.size() > 0) {
                    for(auto at : t.argument_types) {
                        ss << " " << boost::apply_visitor(print_visitor(), at->variant);
                    }
                } else {
                    ss << " void";
                }
                return ss.str();
            }
        };
    }
}

#endif /* defined(__rdvlisp__types__) */
