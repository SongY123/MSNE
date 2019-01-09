#define _CLASSIC_WALK_
#ifndef _CLASSIC_WALK_

#define _WR_DEBUG_ON_ true
#define FILE_IN  ".\\init\\step1.out"
#define _DATA_CLASSIC_
#ifdef _DATA_CLASSIC_
const unsigned MIN_IDX = 0;  //const unsigned MIN_IDX = {min_idx};
const unsigned MAX_IDX = 74;  //const unsigned MAX_IDX = {max_idx};
const unsigned N_NODES = 75; //const unsigned N_NODES = {n_nodes};
#endif // _DATA_CLASSIC_

#include <cstdlib>
#include <cassert>
#include <ctime>

#include <bitset>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <stack>
#include <queue>
using namespace std;

#define _WR_DEBUG_(statements) if(_WR_DEBUG_ON_) {statements}

// Declare Type
typedef size_t IDX; // index of node
typedef size_t DK; // steps between nodes
typedef vector<IDX> AD_LST;
typedef bitset<MAX_IDX + 1> AD_SET;
typedef map<IDX, AD_LST> GRAPH;
typedef map<IDX, AD_SET> VISITED;
typedef vector<IDX> WALK;
typedef map<size_t, AD_LST> LEVELS;

// UTILs Function
static AD_LST ln2lst(const string& line) {
    AD_LST lst;
    const char* cbeg = line.c_str();
    const char* cstr = cbeg;
    int sz = int(line.size());
    size_t idx = 0;
    while (cstr - cbeg < sz) {
        lst.push_back(
            stoul(cstr, &idx)
        );
        cstr = cstr + idx + 1;
    }
    return lst;
}

void check_file_out(const string& file_out) {
    ofstream out(file_out);
    if (!out.good()) {
        cerr << "Some results will be outputed to `" << file_out << "`, but it can't open." << endl;
        exit(1);
    }
    out.close();
}

void check_file_in(const string& file_in) {
    ifstream in(file_in);
    if (!in.good()) {
        cerr << "Some data will be loaded from `" << file_in << "`, but it can't open." << endl;
        exit(1);
    }
    in.close();
}

GRAPH parse_graph(const string& filename) {
    GRAPH graph;
    VISITED visited;
    ifstream in(filename);
    _WR_DEBUG_(cout << "Init from " << filename << "...\n";);
    string ln;
    in >> ln;
    IDX min_idx = atoi(ln.c_str());
    in >> ln;
    IDX max_idx = atoi(ln.c_str());
    in >> ln;
    size_t n_nodes = atoi(ln.c_str());
    getline(in, ln);
    for (IDX idx = min_idx; idx <= max_idx; ++idx) {
        getline(in, ln);
        const AD_LST& lst = ln2lst(ln);
        for (const IDX& tgt : lst) {
            if (visited[idx].test(tgt) == false) {
                graph[idx].push_back(tgt);
                visited[idx].set(tgt);
            }
        }
        for (const IDX& tgt : graph[idx]) {
            if (visited[tgt].test(idx) == false) {
                graph[tgt].push_back(idx);
                visited[tgt].set(idx);
            }
        }
    }
    return graph;
}

WALK dfs(GRAPH& graph, const IDX& idx) {
    WALK walk;
    AD_SET visited;
    stack<IDX> stk;
    stk.push(idx);
    visited.set(idx);
    while (stk.empty() == false) {
        const IDX& last = stk.top();
        walk.push_back(last);
        stk.pop();
        for (const IDX& item : graph[last]) {
            if (visited.test(item) == false) {
                stk.push(item);
                visited.set(item);
            }
        }
    }
    return walk;
}

WALK bfs(GRAPH& graph, const IDX& idx) {
    WALK walk;
    AD_SET visited;
    queue<IDX> que;
    que.push(idx);
    visited.set(idx);
    while (que.empty() == false) {
        const IDX& front = que.front();
        walk.push_back(front);
        que.pop();
        for (const IDX& item : graph[front]) {
            if (visited.test(item) == false) {
                que.push(item);
                visited.set(item);
            }
        }
    }
    return walk;
}

LEVELS get_levels(GRAPH& graph, const IDX& idx) {
    VISITED visited;
    LEVELS levels;
    size_t cur_level = 0;
    size_t cnt_visited = 0;
    levels[cur_level].push_back(idx);
    visited[cur_level].set(idx);
    cnt_visited += 1;
    while (cnt_visited < N_NODES) {
        const AD_LST& pre_level_lst = levels[cur_level];
        cur_level += 1;
        AD_LST level_lst;
        for (const IDX& adj : pre_level_lst) {
            for (const IDX& item : graph[adj]) {
                bool is_visited = false;
                for (size_t pre = 0; pre <= cur_level; ++pre) {
                    if (visited[pre].test(item)) {
                        is_visited = true;
                        break;
                    }
                }
                if (is_visited == false) {
                    level_lst.push_back(item);
                    visited[cur_level].set(item);
                }
            }
        }
        levels[cur_level] = level_lst;
        cnt_visited += level_lst.size();
    }
    return levels;
}

void output(const LEVELS levels, const WALK& walk, const string& filename) {
    ofstream out(filename);
    out << "# WALKS\n";
    for (const IDX& idx : walk) {
        out << idx << ' ';
    }
    out << '\n';
    out << "# LEVELS\n";
    for (const pair<IDX, AD_LST>& item : levels) {
        out << item.first << ": ";
        for (const IDX& idx : item.second) {
            out << idx << ' ';
        }
        out << '\n';
    }
    out.close();
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "prog-name file-in file-out type [start-vertex=0]\n" << endl;
        exit(1);
    }
    check_file_in(argv[1]);
    check_file_out(argv[2]);
    string type = argv[3];
    IDX idx = MIN_IDX;
    if(argc >= 5) idx = stoul(argv[4], nullptr);
    if (idx < MIN_IDX || idx >MAX_IDX) {
        cerr << "`" << idx << "` is invalid, where MIN_IDX is " << MIN_IDX << ", MAX_IDX is " << MAX_IDX << "."<< endl;
        exit(1);
    }
    // assert(idx >= MIN_IDX && idx <= MAX_IDX);
    GRAPH graph = parse_graph(argv[1]);
    LEVELS levels = get_levels(graph, idx);
    if (type == "dfs") {
        WALK walk = dfs(graph, idx);
        output(levels, walk, argv[2]);
    }
    else if (type == "bfs") {
        WALK walk = bfs(graph, idx);
        output(levels, walk, argv[2]);
    }
    else {
        cerr << "The `type` choices is `dfs` or `bfs`." << endl;
        exit(1);
    }
    return 0;
}

#endif // !_CLASSIC_WALK_