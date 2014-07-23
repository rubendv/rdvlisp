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

namespace rdvlisp {
    namespace ast {
        class expression;
        typedef std::shared_ptr<expression> expression_ref;
        
        class identifier {
        public:
            std::vector<std::string> name;
            identifier(const std::vector<std::string>& name) : name(name) {
                if(name.size() == 0) {
                    throw std::invalid_argument("name must have at least one component");
                } else {
                    for(auto component : name) {
                        if(component.length() == 0) {
                            throw std::invalid_argument("all components in name must be non-empty");
                        }
                    }
                }
            }
            identifier(const std::string& name) : identifier(std::vector<std::string>{name}) {}
        };
        std::ostream& operator<<(std::ostream& os, identifier identifier);
        
        class keyword {
        public:
            std::string name;
            keyword(const std::string& name) : name(name) {
                if(name.size() == 0) {
                    throw std::invalid_argument("name must be non-empty");
                }
            }
        };
        std::ostream& operator<<(std::ostream& os, keyword keyword);
        
        class integer {
        public:
            std::string value;
            std::string postfix;
            integer(const std::string& value, const std::string& postfix="") : value(value), postfix(postfix) {}
        };
        std::ostream& operator<<(std::ostream& os, integer integer);
        class floating_point {
        public:
            std::string value;
            std::string postfix;
            floating_point(const std::string& value, const std::string& postfix="") : value(value), postfix(postfix) {}
        };
        std::ostream& operator<<(std::ostream& os, floating_point floating_point);
        
        class array {
        public:
            std::vector<expression_ref> elements;
            array(const std::vector<expression_ref>& elements) : elements(elements) {}
        };
        std::ostream& operator<<(std::ostream& os, array array);
        
        class string {
        public:
            std::string prefix;
            std::string contents;
            string(const std::string& contents, const std::string& prefix="") : contents(contents), prefix(prefix) {}
        };
        std::ostream& operator<<(std::ostream& os, string string);
        
        class expression {
        public:
            boost::variant<identifier, integer, floating_point, array, string, keyword> variant;
            expression(decltype(variant) variant) : variant(variant) {}
            expression(const expression& expression) : variant(expression.variant) {}
            expression(expression_ref expression_ref) : variant(expression_ref->variant) {}
            expression(identifier x) : variant(x) {}
            expression(floating_point x) : variant(x) {}
            expression(array x) : variant(x) {}
            expression(string x) : variant(x) {}
            expression(integer x) : variant(x) {}
            expression(keyword x) : variant(x) {}
        };
        std::ostream& operator<<(std::ostream& os, expression expression);
    }
    
    class read_error : public std::runtime_error {
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
        read_error(const T& what, size_t start, size_t end) : std::runtime_error(stringify(what, start, end)) {}
    };
    
    template <typename T, typename E=std::string>
    class result {
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
                throw read_error(boost::get<E>(variant), start, end);
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
        result(T t, size_t start, size_t end) : variant(t), start(start), end(end) {}
        result(E e, size_t start, size_t end) : variant(e), start(start), end(end) {}
    };
    
    result<ast::expression_ref> read(const std::string& source, size_t start=0);
}

#endif /* defined(__rdvlisp__reader__) */
