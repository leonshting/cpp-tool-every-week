#pragma once

#include <queue>
#include <unordered_set>

namespace traverses {

template <class Vertex, class Graph, class Visitor>
void BreadthFirstSearch(Vertex origin_vertex, const Graph &graph, Visitor visitor) {
    std::unordered_set<Vertex> vertices_pushed;

    std::queue<Vertex> visiting_queue;
    visitor.DiscoverVertex(origin_vertex);
    visiting_queue.push(origin_vertex);
    vertices_pushed.insert(origin_vertex);

    while (!visiting_queue.empty()) {
        auto current_vertex = visiting_queue.front();
        visitor.ExamineVertex(current_vertex);
        visiting_queue.pop();

        for (const auto &edge : OutgoingEdges(graph, current_vertex)) {
            visitor.ExamineEdge(edge);
            auto node = GetTarget(graph, edge);

            if (vertices_pushed.insert(node).second) {
                visitor.DiscoverVertex(node);
                visiting_queue.push(node);
            }
        }
    }
}

template <class Vertex, class Edge>
class BfsVisitor {
public:
    virtual void DiscoverVertex(Vertex) {
    }
    virtual void ExamineEdge(const Edge &) {
    }
    virtual void ExamineVertex(Vertex) {
    }
    virtual ~BfsVisitor() = default;
};

}  // namespace traverses