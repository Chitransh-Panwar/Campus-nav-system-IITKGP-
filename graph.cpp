#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <climits>
#include <utility>
#include <algorithm>
#include <emscripten/bind.h>

using namespace std;
using namespace emscripten;

// ---------------- Structures ----------------

struct node {
    int id;
    double lat;
    double lng;
};

struct edge {
    int to;
    int weight;
    bool accessible;
};

// ---------------- Graph class ----------------

class graph {
public:
    unordered_map<int, node> nodes;
    unordered_map<int, vector<edge>> adjlist;

    void addnode(int id, double lat, double lng) {
        nodes[id] = {id, lat, lng};
        if (adjlist.find(id) == adjlist.end()) {
            adjlist[id] = {};
        }
    }

    void addedge(int from, int to, int weight, bool accessible) {
        adjlist[from].push_back({to, weight, accessible});
    }
};

// ---------------- Algorithms ----------------

vector<int> bfs(graph &g, int sid, int eid) {
    queue<int> q;
    unordered_map<int, int> parent;
    unordered_set<int> vis;

    q.push(sid);
    vis.insert(sid);

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        if (current == eid) break;

        for (auto &neighbour : g.adjlist[current]) {
            if (!vis.count(neighbour.to)) {
                vis.insert(neighbour.to);
                parent[neighbour.to] = current;
                q.push(neighbour.to);
            }
        }
    }

    vector<int> path;
    int current = eid;
    while (parent.count(current) && current != sid) {
        path.insert(path.begin(), current);
        current = parent[current];
    }
    if (current == sid) path.insert(path.begin(), sid);
    return path;
}

vector<int> dfs(graph &g, int sid, int eid) {
    stack<int> st;
    unordered_map<int, int> parent;
    unordered_set<int> vis;
    st.push(sid);

    while (!st.empty()) {
        int current = st.top();
        st.pop();

        if (current == eid) break;

        if (!vis.count(current)) {
            vis.insert(current);
            for (auto &neighbour : g.adjlist[current]) {
                if (!vis.count(neighbour.to)) {
                    parent[neighbour.to] = current;
                    st.push(neighbour.to);
                }
            }
        }
    }

    vector<int> path;
    int current = eid;
    while (parent.count(current) && current != sid) {
        path.insert(path.begin(), current);
        current = parent[current];
    }
    if (current == sid) path.insert(path.begin(), sid);
    return path;
}

vector<int> dijkstra(graph &g, int sid, int eid, bool accessibility = true) {
    unordered_map<int, int> dist;
    unordered_map<int, int> parent;

    for (auto &p : g.adjlist) dist[p.first] = INT_MAX;
    dist[sid] = 0;

    using pii = pair<int, int>;
    priority_queue<pii, vector<pii>, greater<pii>> pq;
    pq.push({0, sid});

    while (!pq.empty()) {
        int d = pq.top().first;
        int node = pq.top().second;
        pq.pop();
        if (node == eid) break;

        for (auto &neighbour : g.adjlist[node]) {
            if (accessibility && !neighbour.accessible) continue;
            int alt = d + neighbour.weight;
            if (alt < dist[neighbour.to]) {
                dist[neighbour.to] = alt;
                parent[neighbour.to] = node;
                pq.push({alt, neighbour.to});
            }
        }
    }

    vector<int> path;
    int current = eid;
    while (parent.count(current) && current != sid) {
        path.insert(path.begin(), current);
        current = parent[current];
    }
    if (current == sid) path.insert(path.begin(), sid);
    return path;
}

// ---------------- Bindings ----------------

EMSCRIPTEN_BINDINGS(graph_module) {
    class_<graph>("graph")
        .constructor<>()
        .function("addnode", &graph::addnode)
        .function("addedge", &graph::addedge);

    emscripten::function("bfs", &bfs);
    emscripten::function("dfs", &dfs);
    emscripten::function("dijkstra", &dijkstra);

    register_vector<int>("vector<int>");
}
