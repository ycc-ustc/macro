#ifndef __MARCO_CONFIG_H__
#define __MARCO_CONFIG_H__

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cctype>
#include <exception>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#include <boost/lexical_cast.hpp>

#include "log.h"

namespace marco {
class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& description) : m_name(name), m_description(description) {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    };
    virtual ~ConfigVarBase() {}
    const std::string& getName() {
        return m_name;
    }
    const std::string& getmDescription() {
        return m_description;
    }

    virtual std::string toString() = 0;
    virtual bool        fromString(const std::string& val) = 0;

private:
    std::string m_name;
    std::string m_description;
};

template <class T>
class ConfigVar : public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;

    ConfigVar(const std::string& name, const T& default_value, const std::string description = "")
        : ConfigVarBase(name, description), m_val(default_value) {}
    std::string toString() override {
        try {
            return boost::lexical_cast<std::string>(m_val);
        } catch (std::exception& e) {
            MARCO_LOG_ERROR(MARCO_LOG_ROOT())
                << "ConfigVar::toString exception" << e.what() << "convert: " << typeid(m_val).name() << " to string";
        }
        return "";
    }
    bool fromString(const std::string& val) override {
        try {
            m_val =  boost::lexical_cast<T>(val);
        } catch (std::exception& e) {
            MARCO_LOG_ERROR(MARCO_LOG_ROOT())
                << "ConfigVar::toString exception" << e.what() << "convert: " << typeid(m_val).name() << " to string";
        }
        return false;
    }

    const T getValue() const {
        return m_val;
    }
    void setValue(const T& val) {
        m_val = val;
    }

private:
    T m_val;
};

class Config {
public:
    typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& default_value,
                                             const std::string& description = "") {
        auto tmp = Lookup<T>(name);
        if (tmp) {
            MARCO_LOG_INFO(MARCO_LOG_ROOT()) << "Lookup done" << name << " exists";
            return tmp;
        }

        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._012345678") != std::string::npos) {
            MARCO_LOG_ERROR(MARCO_LOG_ROOT()) << "Lookup name invalid" << name;
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        s_datas[name] = v;
        return v;
    }

    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        auto it = Config::s_datas.find(name);
        if (it == Config::s_datas.end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }
    static void               LoadFromYaml(const YAML::Node& root);
    static ConfigVarBase::ptr LookupBase(const std::string& name);

private:
    static ConfigVarMap s_datas;
};
}  // namespace marco

#endif