#pragma once

#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

/*

    The GUI is composed of elements with properties, the latters can depends on
    other properties of any element of the GUI. We can then represent a GUI
    with a list of properties and a list of relations between a property and
    the list of properties it depends on.

    Let's see an example, a point `P` in the middle of a box `B`. The position
    of the point depends on the size of the box:
    
    GUI =
        Properties = [
            Px, Py, Bw, Bh    
        ]
        Constraints = [
            Px depends on Bw,
            Py depends on Bh
        ]

    The objective is to find the order to update the individual property. The 
    GUI  can be seen as a directed graph where the nodes are the properties and
    the arc `A -> B` exists if `B depends on A`. Thus the problem is isomorphic
    to "Find a topological ordering of the graph", which means it must be 
    acyclic.

    Let's represent a grah as a dictionnary `A -> {A}`. To constructs such a 
    graph you intially set the entry of each node of the graph to the empty set
    then, for each arc `A -> B`, you add `A` to the set of the entry B. The 
    entry of the node `A` is the set of all node with an outcoming arc from 
    `A`. The graph with nodes `A`, `B` and `C` and the arc `A -> C`, `B -> C` 
    and `B -> B` is then represented as `[ A: {}, B:{B}, C:{B, A} ]`.

    Graph(GUI) =
        Initial set   : [ Px:{}, Py:{}, Bw:{}, Bh: {} ]
        Add `Bw -> Px`: [ Px:{ Bw }, Py:{}, Bw:{}, Bh: {} ]
        Add `Bh -> Py`: [ Px:{ Bw }, Py:{ Bh }, Bw:{}, Bh: {} ]



    Let's note `T(g)` a topological order of g.
        
        T([]) = []
        T([A: {}] U g) = concat( [A] , T(R(g, A)) )

    If the graph doesn't contains any entry with an empty set, the graph is
    cyclic, thus does not contains any topogical order. The function `R(g, A)`
    removes `A` from the set of all entries in `g`. 

    Proof:
        A directed acyclic graph does not contain a cycle which means it contains
        only finite path. With this property we can derive that at least one node
        has an indegree of 0. 
        `R(g', A)` returns the graph `[A: {}] U g'`, noted 
        `g`, without the node `A` and the arcs outcoming from A. In other words
        `R(g', A)` keep the cyclic property of `g`: a node with an outdegree of 0
        can't be part of a cycle.
        The algorithm is finite if and only if the number of nodes in the graph, 
        noted `#N(g)`, is finite: `T(g)` with `#N(g) = 0` is finite (first case) 
        otherwise it depends on `T(g')` but `#N(g') = #N(g) - 1`. 

    T(Graph(GUI)) = T([ Px:{ Bw }, Py:{ Bh }, Bw:{}, Bh: {} ]) =
        using case T([Bw: {}] U [ Px:{ Bw }, Py:{ Bh }, Bh: {} ]): concat( [Bw], T(R([ Px:{ Bw }, Py:{ Bh }, Bh: {} ], Bw)) )
            R([ Px:{ Bw }, Py:{ Bh }, Bh: {} ], Bw) = [ Px:{}, Py:{ Bh }, Bh: {} ]

        T([ Px:{}, Py:{ Bh }, Bh: {} ]) =
            using case T([Bw: {}] U [ Px:{}, Py:{ Bh } ]): concat( [Bh], T(R([ Px:{}, Py:{ Bh } ], Bh)) )
                R([ Px:{}, Py:{ Bh } ], Bh) = [ Px:{}, Py:{} ]

            T([ Px:{}, Py:{} ]) = 
                using case T([ Px:{} ] U [ Py:{} ]) = concat( [Px], T(R([ Py:{} ], Px)) )
                    R([ Py:{} ], Px) = [ Py:{} ]

                T([ Py:{} ]) = 
                    using case T([ Py:{} ] U []) = concat( [Py], T(R([], Py)) )
                        R([], Py) = []

                    T([]) = []

                    concat( [Py], [] ) = [Py]
                concat( [Px], [Py] ) = [Px, Py]
            concat( [Bh], [Px, Py] ) = [Bh, Px, Py]
        concat( [Bw], [Bh, Px, Py] ) = [Bw, Bh, Px, Py]


    
    On peut ensuite transformer T(g) en fonction non-recusive en ajoutant le 
    resultat dans les parametres:
    We car then transform `T(g)` in a tail-recursive function by adding the 
    temporary in the parameters:

        T'([], L) = L
        T'([A: {}] U g, L) = T'(R(g, A), concat( L, [A] ))

        et

        T(g) = T'(g, [])

    Proof of `forall L. concat( L, T(g) ) = T'(g, L)`:
        The base case `concat( L, T([]) ) = T'([], L)` is trivial. Now let's 
        assume `concat( L, T(g) ) = T'(g, L)` for a `g`, we need to prove
        `concat( L, T(g') ) = T'(g', L)` for `g'` the graph `g` but with a node 
        `A` and some arcs oucoming from `A`.

    concat( L, T(g') )
    = concat( L, concat( [A] , T(R(g', A)) ))   using the second case of T
    = concat( L, concat( [A] , T(g) ))          using the defintion of g'
    = concat( concat( L, [A] ), T(g) )          using the associativity of concat

    T'(g', L)
    = T'(R(g', A), concat( L, [A] ))            using the second case of T'
    = T'(g, concat( L, [A] ))                   using the defintion of g'

    with `L' = concat( L, [A] )` we have `concat( L', T(g') ) = T'(g', L')`
    which prove `forall L. concat( L, T(g) ) = T'(g, L)` and in particular
    `T(g) = T'(g, [])`.

*/

namespace pong::client::gui {

using property_id_t = std::size_t;

// prop depends on dependencies
// dependencies -> prop
struct ConstraintID {
    property_id_t prop;
    std::vector<property_id_t> dependencies;
};

// ARCS : [{A -> B}, {A -> C}]
// GRAPH: [A: {}, B:{A}, C:{A}]
using property_graph_t = std::unordered_map<property_id_t, std::unordered_set<property_id_t>>;

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