#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

#include <pong/client/gui/constraint/Solver.hpp>

namespace pong::client::gui {

std::optional<std::vector<property_id_t>> find_update_order(property_graph_t graph) {
    std::vector<property_id_t> order;

    while(!graph.empty()) {
        auto it = std::find_if(std::begin(graph), std::end(graph), [] (auto const& node) { return node.second.empty(); });

        // If there's no node with an indegree of 0, the graph is cyclic
        if (it == std::end(graph)) {
            return std::nullopt;
        }

        auto property_removed = order.emplace_back(it->first);
        graph.erase(it);
        for(auto&[_, edges] : graph) {
            edges.erase(property_removed);
        }
    }

    return order;
}

property_graph_t make_graph(
    std::vector<property_id_t> const& properties, 
    std::vector<ConstraintID> const& constraints
) {
    property_graph_t graph;

    for(auto prop : properties) {
        graph[prop] = {};
    }

    for(auto constraint : constraints) {
        for(auto dependency : constraint.dependencies) {
            // Arc dependency -> constraint.prop
            graph[constraint.prop].insert(dependency);
        }
    }

    return graph;
}

}