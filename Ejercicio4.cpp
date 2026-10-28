#include <iostream>
#include <memory>
#include <queue>
#include <stack>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Node {
    int id_;
    std::vector<int> adj_;  // ids de los vecinos, en orden de inserción
public:
    explicit Node(int id) : id_(id) {}
    int id() const { return id_; }
    void addNeighbor(int to) { adj_.push_back(to); }
    const std::vector<int>& neighbors() const { return adj_; }
};

class Graph {
    std::unordered_map<int, Node> nodes_;

    Node& getOrCreate(int id) {
        auto it = nodes_.find(id);
        if (it == nodes_.end()) it = nodes_.emplace(id, Node(id)).first;
        return it->second;
    }
public:
    void addEdge(int from, int to) {
        getOrCreate(from).addNeighbor(to);
        getOrCreate(to);  // asegura que el destino exista como nodo
    }

    bool hasNode(int id) const { return nodes_.count(id) > 0; }

    // Devuelve una copia: el iterador no puede modificar ni ver la estructura interna.
    std::vector<int> neighbors(int id) const {
        auto it = nodes_.find(id);
        if (it == nodes_.end()) return {};
        return it->second.neighbors();
    }
};

class GraphIterator {
public:
    virtual ~GraphIterator() = default;
    virtual bool hasNext() const = 0;
    virtual int next() = 0;
};

class BFSIterator : public GraphIterator {
    const Graph& graph_;
    std::queue<int> pending_;
    std::unordered_set<int> visited_;
public:
    BFSIterator(const Graph& g, int start) : graph_(g) {
        if (g.hasNode(start)) {
            pending_.push(start);
            visited_.insert(start);  // se marca al encolar
        }
    }

    bool hasNext() const override { return !pending_.empty(); }

    int next() override {
        if (!hasNext()) throw std::out_of_range("BFSIterator: no hay mas nodos");
        int current = pending_.front();
        pending_.pop();
        for (int n : graph_.neighbors(current)) {
            if (visited_.insert(n).second)  // true solo si no estaba visitado
                pending_.push(n);
        }
        return current;
    }
};

class DFSIterator : public GraphIterator {
    const Graph& graph_;
    std::stack<int> pending_;
    std::unordered_set<int> visited_;

    // Descarta nodos visitados
    void skipVisited() {
        while (!pending_.empty() && visited_.count(pending_.top()))
            pending_.pop();
    }
public:
    DFSIterator(const Graph& g, int start) : graph_(g) {
        if (g.hasNode(start)) pending_.push(start);
    }

    bool hasNext() const override {
        auto copy = pending_;
        while (!copy.empty()) {
            if (!visited_.count(copy.top())) return true;
            copy.pop();
        }
        return false;
    }

    int next() override {
        skipVisited();
        if (pending_.empty()) throw std::out_of_range("DFSIterator: no hay mas nodos");
        int current = pending_.top();
        pending_.pop();
        visited_.insert(current);

        // Se apilan en reversa para que el primer vecino se visite primero
        auto ns = graph_.neighbors(current);
        for (auto it = ns.rbegin(); it != ns.rend(); ++it)
            if (!visited_.count(*it)) pending_.push(*it);
        return current;
    }
};

// Prueba
void recorrer(GraphIterator& it) {
    while (it.hasNext()) std::cout << it.next() << " ";
    std::cout << "\n";
}

int main() {
    Graph g;
    g.addEdge(1, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 4);

    // Validación mínima del enunciado
    std::unique_ptr<GraphIterator> it = std::make_unique<BFSIterator>(g, 1);
    while (it->hasNext()) {
        std::cout << it->next() << " ";
    }
    std::cout << "\n";

    // Cambio iterador (BFS->DFS)
    it = std::make_unique<DFSIterator>(g, 1);
    while (it->hasNext()) {
        std::cout << it->next() << " ";
    }
    std::cout << "\n";

    Graph c;
    c.addEdge(1, 2);
    c.addEdge(2, 3);
    c.addEdge(3, 1);
    std::cout << "Ciclo BFS: ";
    BFSIterator b(c, 1);
    recorrer(b);
    std::cout << "Ciclo DFS: ";
    DFSIterator d(c, 1);
    recorrer(d);
}