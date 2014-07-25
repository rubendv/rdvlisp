//
//  eval.cpp
//  rdvlisp
//
//  Created by Ruben De Visscher on 24/07/14.
//  Copyright (c) 2014 Ruben De Visscher. All rights reserved.
//

#include "eval.h"

using namespace rdvlisp::runtime;
using namespace rdvlisp;

const std::array<types::TypeRef, 4> Integer::signed_types{types::sint8, types::sint16, types::sint32, types::sint64};
const std::array<types::TypeRef, 4> Integer::unsigned_types{types::uint8, types::uint16, types::uint32, types::uint64};

ValueRef Namespace::lookup(const std::string& name) {
    auto it = bindings_.find(name);
    if(it == bindings_.end()) {
        throw NameError(ast::Identifier(name));
    } else {
        return bindings_[name];
    }
}
ValueRef Namespace::lookup(const ast::Identifier& identifier) {
    Namespace * current_namespace = this;
    if(identifier.name.size() > 1) {
        for(auto component : std::vector<std::string>(identifier.name.begin(), std::prev(identifier.name.end()))) {
            auto result = current_namespace->lookup(component);
            auto new_namespace = boost::get<Namespace *>(result->variant);
            if(new_namespace != nullptr) {
                current_namespace = new_namespace;
            } else {
                throw NameError(identifier);
            }
        }
    }
    return current_namespace->lookup(*identifier.name.rbegin());
}