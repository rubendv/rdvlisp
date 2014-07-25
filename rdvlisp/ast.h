//
//  ast.h
//  rdvlisp
//
//  Created by Ruben De Visscher on 24/07/14.
//  Copyright (c) 2014 Ruben De Visscher. All rights reserved.
//

#ifndef rdvlisp_ast_h
#define rdvlisp_ast_h

#include <memory>
#include <iostream>
#include <boost/variant.hpp>

namespace rdvlisp {
    namespace ast {
        class Expression;
        typedef std::shared_ptr<Expression> ExpressionRef;
        
        class Identifier {
        public:
            std::vector<std::string> name;
            Identifier(const std::vector<std::string>& name) : name(name) {
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
            Identifier(const std::string& name) : name(std::vector<std::string>{name}) {}
        };
        std::ostream& operator<<(std::ostream& os, Identifier identifier);
        
        class Keyword {
        public:
            std::string name;
            Keyword(const std::string& name) : name(name) {
                if(name.size() == 0) {
                    throw std::invalid_argument("name must be non-empty");
                }
            }
        };
        std::ostream& operator<<(std::ostream& os, Keyword keyword);
        
        class Integer {
        public:
            std::string value;
            Integer(const std::string& value) : value(value) {}
        };
        std::ostream& operator<<(std::ostream& os, Integer integer);
        class FloatingPoint {
        public:
            std::string value;
            FloatingPoint(const std::string& value) : value(value) {}
        };
        std::ostream& operator<<(std::ostream& os, FloatingPoint floating_point);
        
        class Tuple {
        public:
            std::vector<ExpressionRef> elements;
            Tuple(const std::vector<ExpressionRef>& elements) : elements(elements) {}
        };
        std::ostream& operator<<(std::ostream& os, Tuple array);
        
        class String {
        public:
            std::string contents;
            String(const std::string& contents) : contents(contents) {}
        };
        std::ostream& operator<<(std::ostream& os, String string);
        
        class Expression {
        public:
            boost::variant<Identifier, Integer, FloatingPoint, Tuple, String, Keyword> variant;
            Expression(decltype(variant) variant) : variant(variant) {}
            Expression(const Expression& expression) : variant(expression.variant) {}
            Expression(ExpressionRef expression_ref) : variant(expression_ref->variant) {}
            Expression(Identifier x) : variant(x) {}
            Expression(FloatingPoint x) : variant(x) {}
            Expression(Tuple x) : variant(x) {}
            Expression(String x) : variant(x) {}
            Expression(Integer x) : variant(x) {}
            Expression(Keyword x) : variant(x) {}
        };
        std::ostream& operator<<(std::ostream& os, Expression expression);
    }
}

#endif
