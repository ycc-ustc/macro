#include <yaml-cpp/yaml.h>

#include "marco/config.h"
#include "marco/log.h"

marco::ConfigVar<int>::ptr g_int_value_config =
    marco::Config::Lookup("system.port", (int)8080, "system port");

marco::ConfigVar<float>::ptr g_float_value_config =
    marco::Config::Lookup("system.value", (float)10.2f, "system value");

void print_yaml(const YAML::Node& node, int level) {
    if (node.IsScalar()) {
        MARCO_LOG_INFO(MARCO_LOG_ROOT()) << std::string(level * 4, ' ') << node.Scalar() << " - "
                                         << node.Type() << " - " << level;
    } else if (node.IsNull()) {
        MARCO_LOG_INFO(MARCO_LOG_ROOT())
            << std::string(level * 4, ' ') << "NULL - " << node.Type() << " - " << level;
    } else if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            MARCO_LOG_INFO(MARCO_LOG_ROOT()) << std::string(level * 4, ' ') << it->first << " - "
                                             << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if (node.IsSequence()) {
        for (size_t i = 0; i < node.size(); ++i) {
            MARCO_LOG_INFO(MARCO_LOG_ROOT())
                << std::string(level * 4, ' ') << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml() {
    YAML::Node root = YAML::LoadFile("/home/dev/macro/bin/config/log.yaml");
    print_yaml(root, 0);
    MARCO_LOG_INFO(MARCO_LOG_ROOT()) << root;
}

void test_config() {
    MARCO_LOG_INFO(MARCO_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    MARCO_LOG_INFO(MARCO_LOG_ROOT()) << "before: " << g_float_value_config->toString();

    YAML::Node root = YAML::LoadFile("/home/dev/macro/bin/config/log.yaml");
    marco::Config::LoadFromYaml(root);

    MARCO_LOG_INFO(MARCO_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    MARCO_LOG_INFO(MARCO_LOG_ROOT()) << "after: " << g_float_value_config->toString();
}

int main(int argc, char** argv) {
    test_config();
    return 0;
}