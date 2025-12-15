#ifndef DECISION_TREE_H
#define DECISION_TREE_H

#include <iostream>
#include <memory>
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <variant>
#include <any>

using Context = std::map<std::string, std::any>;

template<typename T>
T getContextValue(const Context& ctx, const std::string& key, T defaultValue){
    auto it = ctx.find(key);
    if (it != ctx.end()){
        try{
            return std::any_cast<T>(it->second);
        } catch(const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    return defaultValue;
}
