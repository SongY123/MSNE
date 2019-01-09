#define _WR_DEBUG_ON_ true
#define FILE_IN ".\\init\\step1.out"
#define FILE_TMP_OUT ".\\dk\\"
#define _DATA_WIKI_
#ifdef _DATA_ARXIV_
const unsigned MIN_IDX = 0;  //const unsigned MIN_IDX = {min_idx};
const unsigned MAX_IDX = 18771;  //const unsigned MAX_IDX = {max_idx};
const unsigned N_NODES = 18772; //const unsigned N_NODES = {n_nodes};
#endif // _DATA_FACEBOOK_
#ifdef _DATA_BLOG_CATALOG_
const unsigned MIN_IDX = 1;  //const unsigned MIN_IDX = {min_idx};
const unsigned MAX_IDX = 10312;  //const unsigned MAX_IDX = {max_idx};
const unsigned N_NODES = 10312; //const unsigned N_NODES = {n_nodes};
#endif // _DATA_BLOG_CATALOG_
#ifdef _DATA_FACEBOOK_
const unsigned MIN_IDX = 0;  //const unsigned MIN_IDX = {min_idx};
const unsigned MAX_IDX = 4038;  //const unsigned MAX_IDX = {max_idx};
const unsigned N_NODES = 4039; //const unsigned N_NODES = {n_nodes};
#endif // _DATA_FACEBOOK_
#ifdef _DATA_WIKI_
const unsigned MIN_IDX = 0;  //const unsigned MIN_IDX = {min_idx};
const unsigned MAX_IDX = 2404;  //const unsigned MAX_IDX = {max_idx};
const unsigned N_NODES = 2405; //const unsigned N_NODES = {n_nodes};
#endif // _DATA_WIKI_

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

#include <boost\random\linear_congruential.hpp>
#include <boost\random\uniform_real.hpp>
#include <boost\random\uniform_int.hpp>
#include <boost\random\variate_generator.hpp>
#include <boost\generator_iterator.hpp>
using namespace std;

#define _WR_DEBUG_(statements) if(_WR_DEBUG_ON_) {statements}
static_assert(MAX_IDX - MIN_IDX + 1 == N_NODES, "Invalid data");

// Random Generator
typedef boost::minstd_rand base_generator_type;
static base_generator_type RNG_GEN(1);
static boost::uniform_real<> UNI_DIST(0, 1);
static boost::variate_generator<base_generator_type&, boost::uniform_real<>> UNI(RNG_GEN, UNI_DIST);
// `beg` and `end` inclusive
static int UNI_INT(int beg, int end) {
    boost::uniform_int<> dist(beg, end);
    boost::variate_generator<base_generator_type&, boost::uniform_int<>> uni_int(RNG_GEN, dist);
    return uni_int();
}

// Declare Type
typedef size_t IDX; // index of node
typedef size_t DK; // steps between nodes
typedef bitset<MAX_IDX + 1> AD_SET;
typedef vector<IDX> AD_LST;
typedef vector<IDX> WALK;
// idx -> dk -> ad_set
typedef map<IDX, map<DK, AD_SET>> M_AD_SET;
// whether M_AD_SET[IDX][DK] is computed?
// idx -> dk -> ad_pre
typedef map<IDX, map<DK, bool>> M_PRE_SET;
// idx -> dk -> ad_lst
typedef map<IDX, map<DK, AD_LST>> M_AD_LST;
// whether M_LST_PRE[IDX][DK] is computed?
typedef map<IDX, map<DK, bool>> M_PRE_LST;

// UTILs Function
static AD_SET util_lst2set(const AD_LST& lst) {
    AD_SET set = AD_SET();

    for (IDX idx : lst) {
        set.set(idx);
    }
    return set;
}

static AD_LST util_set2lst(const AD_SET& set) {
    AD_LST lst = AD_LST();

    for (IDX idx = MIN_IDX; idx <= MAX_IDX; ++idx) {
        if (set[idx]) lst.push_back(idx);
    }
    return lst;
}

static std::string walk_str(const WALK& walk) {
    ostringstream oss;
    for (const IDX& idx : walk) {
        oss << idx << ',';
    }
    string str = oss.str();
    if (str.size() > 0)
        str.pop_back();
    return str;
}

static std::string adlst_str(const AD_LST& lst) {
    ostringstream ss;
    for (const IDX& idx : lst) {
        ss << idx << ",";
    }
    string str = ss.str();
    if (str.size() > 0)
        str.pop_back();
    return str;
}

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

// idx -> ad_lst
map<IDX, AD_LST> input_d(const string& filename, bool isbinary = false) {
    ifstream in(filename);
    _WR_DEBUG_(cout << "Loading dk"  << " from " << filename << "...\n";)
        if (!in.good()) {
            cerr <<  filename << " is going to be loaded, but it can't open." << endl;
            exit(1);
        }
    map<IDX, AD_LST> mdk;
    string ln;
    for (IDX idx = MIN_IDX; idx <= MAX_IDX; ++idx) {
        getline(in, ln);
        mdk[idx] = ln2lst(ln);
    }
    in.close();
    return mdk;
}

struct Node2VecDist {
    // Global Variable
    M_AD_SET NSET;
    M_PRE_SET PRESET;
    M_AD_LST NLST;
    M_PRE_LST PRELST;
    
    // Node2VecDist(const string& filename) {
    Node2VecDist() {
        const string& filename = FILE_IN;
        ifstream in(filename);
        if (!in.good()) {
            _WR_DEBUG_(cerr << "Init from " << filename << ". But it is invalid." << endl; )
                exit(1);
        }
        _WR_DEBUG_(cout << "Init from " << filename << " ...\n";)
            string ln;
        in >> ln;
        IDX min_idx = atoi(ln.c_str());
        in >> ln;
        IDX max_idx = atoi(ln.c_str());
        in >> ln;
        size_t n_nodes = atoi(ln.c_str());
        _WR_DEBUG_(
            cout << "min_idx: " << min_idx << "\n"\
            << "max_idx: " << max_idx << "\n"\
            << "n_nodes: " << n_nodes << endl;
        )
        // ensure: file is valid.
        assert(max_idx - min_idx + 1 == n_nodes);
        // ensure: MAX_IDX equals max_idx from file.
        assert(max_idx == MAX_IDX);
        assert(min_idx == MIN_IDX);

        _WR_DEBUG_(cout << "Computing D1... ";);
        clock_t time = clock();
        const DK dk = 1;
        for (IDX idx = MIN_IDX; idx <= MAX_IDX; ++idx) {
            in >> ln;
            NLST[idx][dk] = ln2lst(ln);
            PRELST[idx][dk] = true;
            lst2set(idx, dk);
        }
        time = clock() - time;
        _WR_DEBUG_(cout << "It took me " << time << " clicks (" << ((float) time) / CLOCKS_PER_SEC << ") seconds.\n";)
        in.close();

        init_table_d(10);
    }

    // `dk_limit` is inclusive
    void init_table_d(const DK& dk_limit) {
        for (DK dk = 2; dk <= dk_limit; ++dk) {
            _WR_DEBUG_(cout << "Computing D" << dk << "... ";);
            clock_t time = clock();
            for (IDX idx = MIN_IDX; idx <= MAX_IDX; ++idx) {
                pre_set(idx, dk);
                pre_lst(idx, dk);
            }
            time = clock() - time;
            _WR_DEBUG_(cout << "It took me " << time << " clicks (" << ((float) time) / CLOCKS_PER_SEC << ") seconds.\n";)
        }
    }

    void init_table_d(const DK& dk, map<IDX, AD_LST>& lst) {
        assert(lst.size() == N_NODES);
        for (IDX idx = MIN_IDX; idx <= MAX_IDX; ++idx) {
            NLST[idx][dk] = lst[idx];
        }
    }

private:
    void pre_lst(IDX idx, DK dk) {
        if (dk == 0 || dk == 1 || PRELST[idx][dk])
            return;
        pre_set(idx, dk);
        set2lst(idx, dk);
        PRELST[idx][dk] = true;
    }

    void pre_set(IDX idx, DK dk) {
        if (dk == 0 || dk == 1 || PRESET[idx][dk])
            return;
        pre_lst(idx, dk - 1);
        const AD_LST& lst = NLST[idx][dk - 1];
        AD_SET& set = NSET[idx][dk];
        for (IDX idx_v : lst) {
            set |= NSET[idx_v][1];
        }
        set &= ~NSET[idx][dk - 1];
        if (dk > 2) {
            pre_set(idx, dk - 2);
            set &= ~NSET[idx][dk - 2];
        }
        else {
            set.reset(idx);
        }
        PRESET[idx][dk] = true;
    }

    // Declare UTILs function
    // UTILs
    void lst2set(IDX idx, DK dk) {
        const AD_LST& lst = NLST[idx][dk];
        AD_SET& set = NSET[idx][dk];

        for (IDX idx : lst) {
            set.set(idx);
        }
        PRESET[idx][dk] = true;
    }

    void set2lst(IDX idx, DK dk) {
        const AD_SET& set = NSET[idx][dk];
        AD_LST& lst = NLST[idx][dk];

        for (IDX idx = MIN_IDX; idx <= MAX_IDX; ++idx) {
            if (set[idx]) lst.push_back(idx);
        }
    }
};

void output_d(const DK& dk, const string& filename, Node2VecDist& dist) {
    ofstream out(filename);
    _WR_DEBUG_(cout << "Output d"<< dk << "... ";)
        if (!out.good()) {
            cerr << "D" << dk << " will be outputed to " << filename << ", but it can't open." << endl;
            exit(1);
        }
    for (IDX idx = MIN_IDX; idx <= MAX_IDX; ++idx) {
        out << adlst_str(dist.NLST[idx][dk]) << "\n";
    }
    out.close();
    _WR_DEBUG_(cout << "You can find at " << filename << '\n'; )
}

#if 1
int main() {
    vector<string> filenames = {
        std::string{}
    };
    const string tmp = FILE_TMP_OUT;
    filenames.push_back(tmp + "01");
    filenames.push_back(tmp + "02");
    filenames.push_back(tmp + "03");
    filenames.push_back(tmp + "04");
    filenames.push_back(tmp + "05");
    filenames.push_back(tmp + "06");
    filenames.push_back(tmp + "07");
    filenames.push_back(tmp + "08");
    filenames.push_back(tmp + "09");
    filenames.push_back(tmp + "10");

    Node2VecDist dist;
    for (DK dk = 1; dk <= 10; ++dk) {
        output_d(dk, filenames[dk], dist);
    }

    return 0;
}
#endif