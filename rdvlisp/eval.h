//
//  eval.h
//  rdvlisp
//
//  Created by Ruben De Visscher on 24/07/14.
//  Copyright (c) 2014 Ruben De Visscher. All rights reserved.
//

#ifndef __rdvlisp__eval__
#define __rdvlisp__eval__
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/bind.hpp>
#include <map>
#include <string>
#include <vector>
#include "ast.h"
#include <initializer_list>
#include <memory>
#include "types.h"
#include <array>

namespace rdvlisp {
    namespace runtime {
        class Value;
        typedef std::shared_ptr<Value> ValueRef;
        
        class Typed {
        public:
            types::TypeRef type;
            Typed(types::TypeRef type) : type(type) {}
        };
        
        class String : public Typed {
        public:
            std::string contents;
            String(const std::string& contents) : contents(contents), Typed(types::string) {}
        };
        
        class Integer : public Typed {
            // signed_types[i] is the signed integer type with i*8 bits
            static const std::array<types::TypeRef, 4> signed_types;
            // unsigned_types[i] is the unsigned integer type with i*8 bits
            static const std::array<types::TypeRef, 4> unsigned_types;
        public:
            boost::variant<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t> value;
            Integer(int8_t x) : value(x), Typed(types::sint8) {}
            Integer(uint8_t x) : value(x), Typed(types::uint8) {}
            Integer(int16_t x) : value(x), Typed(types::sint16) {}
            Integer(uint16_t x) : value(x), Typed(types::uint16) {}
            Integer(int32_t x) : value(x), Typed(types::sint32) {}
            Integer(uint32_t x) : value(x), Typed(types::uint32) {}
            Integer(int64_t x) : value(x), Typed(types::sint64) {}
            Integer(uint64_t x) : value(x), Typed(types::uint64) {}
        };
        
        class Array : public Typed {
        public:
            std::vector<ValueRef> elements;
            Array(const std::vector<ValueRef>& elements, types::TypeRef inner_type) : Typed(std::make_shared<types::Type>(types::Array(inner_type))), elements(elements) {}
            Array(const std::vector<ValueRef>& elements, types::TypeRef inner_type, size_t length) : Typed(std::make_shared<types::Type>(types::Array(inner_type, length))), elements(elements) {}
        };
        
        class Function {
        public:
            std::vector<ast::Identifier> argument_names;
            ast::ExpressionRef body;
        };
        
        typedef float float32_t;
        typedef double float64_t;
        
        class FloatingPoint {
        public:
            uint8_t bits;
            boost::variant<float32_t, float64_t> value;
            FloatingPoint(float32_t x) : value(x), bits(32) {}
            FloatingPoint(float64_t x) : value(x), bits(64) {}
        };
        
        class NameError : public std::runtime_error {
        public:
            enum class Reason {
                Ambiguous,
                NotFound
            };
        private:
            Reason reason_;
            ast::Identifier identifier_;
            static std::string construct_message(const ast::Identifier& identifier, Reason reason) {
                std::stringstream ss;
                if(reason == Reason::Ambiguous) {
                    ss << "Identifier " << identifier << " found in more than one imported namespace";
                } else if(reason == Reason::NotFound) {
                    ss << "Identifier " << identifier << " not found in any imported namespace";
                }
                return ss.str();
            }
        public:
            NameError(const ast::Identifier& identifier, Reason reason=Reason::NotFound) : identifier_(identifier), reason_(reason), std::runtime_error(construct_message(identifier, reason)) {}
        };
        
        class Namespace {
            std::string name_;
            std::map<std::string, ValueRef> bindings_;
        public:
            Namespace(const std::string& name, std::initializer_list<std::pair<const std::string, ValueRef>> bindings={}) : name_(name), bindings_(bindings) {}
            ValueRef lookup(const std::string& name);
            ValueRef lookup(const ast::Identifier& identifier);
        };
        
        class Value {
        public:
            boost::variant<String, Integer, Array, Namespace, FloatingPoint, Function> variant;
        };
        
        
        class CombinedNamespace {
            std::shared_ptr<Namespace> root_namespace;
            std::vector<std::shared_ptr<Namespace>> imported_namespaces;
        public:
            CombinedNamespace() : root_namespace(new Namespace("")) {
                imported_namespaces.push_back(root_namespace);
            }
            
            ValueRef lookup(const ast::Identifier& identifier) {
                ValueRef result;
                for(auto current_namespace : imported_namespaces) {
                    if(result.get() != nullptr) {
                        result = current_namespace->lookup(identifier);
                    } else {
                        throw NameError(identifier, NameError::Reason::Ambiguous);
                    }
                }
                if(result.get() != nullptr) {
                    return result;
                } else {
                    throw NameError(identifier, NameError::Reason::NotFound);
                }
            }
        };
        
        class Runtime {
        public:
            CombinedNamespace value_namespace;
            CombinedNamespace type_namespace;
        private:
            class eval_visitor : public boost::static_visitor<ValueRef> {
                Runtime& runtime;
            public:
                eval_visitor(Runtime& runtime) : runtime(runtime) {}
                template <typename T>
                ValueRef operator()(T t) {
                    return runtime.value_namespace.lookup(ast::Identifier("void"));
                }
                
                ValueRef operator()(const ast::Identifier& identifier) {
                    return runtime.value_namespace.lookup(identifier);
                }
            };
        public:
            Runtime() {}
            ValueRef eval(const ast::ExpressionRef& expression) {
                eval_visitor v(*this);
                return expression->variant.apply_visitor(v);
            }
        };
    }
}

#endif /* defined(__rdvlisp__eval__) */
