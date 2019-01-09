#define _CLASSIC_WALK_
#ifdef _CLASSIC_WALK_

#define _WR_DEBUG_ON_ true
#define N_PRE_DK 4
// #define MAX_LEVEL 3 // #define MAX_LEVEL {max_level}
#define FILE_IN  ".\\init\\step1.out"
#define _DATA_CLASSIC_
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
#ifdef _DATA_PPI_
const unsigned MIN_IDX = 1;  //const unsigned MIN_IDX = {min_idx};
const unsigned MAX_IDX = 3890;  //const unsigned MAX_IDX = {max_idx};
const unsigned N_NODES = 3890; //const unsigned N_NODES = {n_nodes};
#endif // _DATA_PPI_
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

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/generator_iterator.hpp>
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
struct Prob;
struct Mask;
struct TriLst;
typedef size_t LEVEL; // walk level
// level -> dk -> pr
typedef map<LEVEL, map<IDX, Prob>> M_PR;

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

struct TriLst {
    AD_LST left;
    AD_LST mid;
    AD_LST right;

    TriLst(const AD_LST& left, const AD_LST& mid, const AD_LST& right) {
        this->left = left;
        this->mid = mid;
        this->right = right;
    }

    TriLst(const AD_LST& left, const AD_LST& mid, const AD_SET& right) {
        this->left = left;
        this->mid = mid;
        this->right = util_set2lst(right);
    }

    TriLst(const AD_LST& left, const AD_SET& mid, const AD_SET& right) {
        this->left = left;
        this->mid = util_set2lst(mid);
        this->right = util_set2lst(right);
    }

    TriLst(const AD_SET& left, const AD_SET& mid, const AD_SET& right) {
        this->left = util_set2lst(left);
        this->mid = util_set2lst(mid);
        this->right = util_set2lst(right);
    }

    TriLst& operator=(const TriLst& oth) {
        this->left = oth.left;
        this->mid = oth.mid;
        this->right = oth.right;
        return *this;
    }

    AD_LST get(int delta) {
        if (delta == -1)
            return left;
        if (delta == 0)
            return mid;
        // if (delta == 1)
        return right;
    }
};

struct TriSet {
    AD_SET left;
    AD_SET mid;
    AD_SET right;

    TriSet(const AD_SET& left, const AD_SET& mid, const AD_SET& right)
        : left(left), mid(mid), right(right)
    {}

    TriSet& operator=(const TriSet& oth) {
        left = oth.left;
        mid = oth.mid;
        right = oth.right;
        return *this;
    }

    AD_SET get(int delta) {
        if (delta == -1) return left;
        if (delta == 0) return mid;
        // if(delta == -1)
        return right;
    }
};

struct Mask {
    bool left;
    bool mid;
    bool right;

    Mask(const TriLst& triLst) {
        this->left = (triLst.left.size() != 0);
        this->mid = (triLst.mid.size() != 0);
        this->right = (triLst.right.size() != 0);
    }

    Mask(const TriSet& triSet) {
        left = (triSet.left.any());
        mid = (triSet.mid.any());
        right = (triSet.right.any());
    }

    Mask(const bool& left, const bool& mid, const bool& right) {
        this->left = left;
        this->mid = mid;
        this->right = right;
    }

    Mask& operator=(const Mask& oth) {
        this->left = oth.left;
        this->mid = oth.mid;
        this->right = oth.right;
        return *this;
    }

    size_t count() const {
        size_t cnt = 0;
        if (left) cnt += 1;
        if (mid) cnt += 1;
        if (right) cnt += 1;
        return cnt;
    }

    int true_idx() const {
        assert(count() == 1);
        if (left) return -1;
        if (mid) return 0;
        // if (right)
        return 1;
    }

    int false_idx() const {
        assert(count() == 2);
        if (!left) return -1;
        if (!mid) return 0;
        // if (!right)
        return 1;
    }
};

struct Prob {
    double alpha;
    double beta;
    double gamma;

    Prob() : alpha(1.0 / 3), beta(1.0 / 3), gamma(1.0 / 3) {}

    Prob(const double alpha, const double beta, const double gamma)
    : alpha(alpha), beta(beta), gamma(gamma) {}

    Prob& operator=(const Prob& oth) {
        this->alpha = oth.alpha;
        this->beta = oth.beta;
        this->gamma = oth.gamma;
        return *this;
    }

    Prob gen_mask_prob(const Mask& mask) {
        if (mask.count() == 3) {
            return Prob{ *this };
        }
        if (mask.count() == 1) {
            int idx = mask.true_idx();
            if (idx == -1)
                return Prob{ 1.0, 0.0, 0.0 };
            if (idx == 0)
                return Prob{ 0.0, 1.0, 0.0 };
            // if (idx == 1)
            return Prob{ 0.0, 0.0, 1.0 };
        }
        int idx = mask.false_idx();
        if (idx == -1) {
            double new_beta = beta / (beta + gamma);
            double new_gamma = 1 - new_beta;
            return Prob{ 0.0, new_beta, new_gamma };
        }
        if (idx == 0) {
            double new_alpha = alpha / (alpha + gamma);
            double new_gamma = 1 - new_alpha;
            return Prob{ new_alpha, 0.0, new_gamma };
        }
        double new_alpha = alpha / (alpha + beta);
        double new_beta = 1 - new_alpha;
        return Prob{ new_alpha, new_beta, 0.0 };
    }

    int choose() {
        // produce by prob
        double rng = UNI(); 
        if (dbt_se(rng, alpha)) return -1;
        if (dbt_se(rng, alpha + beta)) return 0;
        if (dbt_equal(gamma, 0)) return 0;
        return 1;
    }

private:
// const double EPS = 1e-8;
#ifndef _EPS_
#define _EPS_ 1e-8

    inline bool dbt_equal(double dbe, double target, double eps = _EPS_) {
        return dbt_ge(dbe, target, eps) && dbt_se(dbe, target, eps);
    }

    inline bool dbt_se(double dbe, double target, double eps = _EPS_) {
        return dbe <= target + eps;
    }

    inline bool dbt_ge(double dbe, double target, double eps = _EPS_) {
        return dbe >= target - 1e-8;
    }

    inline void assert_pr(double eps = 1e-8) {
        double dbe = alpha + beta + gamma;
        assert(dbt_equal(dbe, 1.0, eps));
    }
#undef _EPS_
#endif // !_EPS_
};

static vector<Prob> ln2prob(const string& line) {
    vector<double> lst;
    const char* cbeg = line.c_str();
    const char* cstr = cbeg;
    int sz = int(line.size());
    size_t idx = 0;
    while (cstr - cbeg < sz) {
        lst.push_back(
            stod(cstr, &idx)
        );
        cstr = cstr + idx + 1;
    }
    assert(lst.size() % 3 == 0);
    vector<Prob> probs;
    idx = 0;
    while (idx < lst.size()) {
        probs.push_back(Prob{ lst[idx], lst[idx + 1], lst[idx + 2] });
        idx += 3;
    }
    return probs;
}

static M_PR file2mpr(ifstream& in) {
    string ln;
    M_PR mpr;
    LEVEL cur = 1;
    while (!in.eof()) {
        getline(in, ln);
        if (ln.size() != 0) {
            vector<Prob> probs = ln2prob(ln);
            assert(cur == probs.size());
            for (size_t idx = 1; idx <= probs.size(); ++idx) {
                mpr[cur][idx] = probs[idx-1];
            }
            cur += 1;
        }
    }
    return mpr;
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

struct Node2VecDist{
    // Global Variable
    M_AD_SET NSET;
    M_PRE_SET PRESET;
    M_AD_LST NLST;
    M_PRE_LST PRELST;
    M_PR MPR;
    LEVEL MAX_LEVEL;
    size_t WALK_STEPS;

    Node2VecDist() {}

    Node2VecDist(const M_PR& mpr, const size_t& walk_steps=80, const string& filename=FILE_IN, const size_t& n_pre_dk=N_PRE_DK) {
        MPR = mpr;
        MAX_LEVEL = mpr.size();

        WALK_STEPS = walk_steps;

        if (filename.size() > 0 && filename[0] == '*') {
            init_dk_by_prefile(filename.substr(1));
        }
        else {
            init_dk_by_stepfile(filename, n_pre_dk);
        }

        must_be_true(n_pre_dk);
    }

    Node2VecDist(const size_t& walk_steps = 80, const string& filename=FILE_IN, const size_t& n_pre_dk=N_PRE_DK) {
        WALK_STEPS = walk_steps;

        if (filename.size() > 0 && filename[0] == '*') {
            init_dk_by_prefile(filename.substr(1));
        }
        else {
            init_dk_by_stepfile(filename, n_pre_dk);
        }

        must_be_true(n_pre_dk);
    }

    void set_mpr(const M_PR& mpr) {
        MPR = mpr;
        MAX_LEVEL = mpr.size();
    }

    WALK tabular_walk(const IDX& idx) {
        // link to nothing
        if (NLST[idx][1].size() == 0)
            return WALK(WALK_STEPS, idx);

        WALK lst;
        lst.push_back(idx);
        while (lst.size() < WALK_STEPS) {
            lst.push_back(walk_step(lst));
        }
        return lst;
    }

    IDX walk_step(const WALK& walk) {
        if (walk.size() == 1) {
            IDX idx = walk.back();
            size_t sz = NLST[idx][1].size();
            // random choice
            IDX rng = UNI_INT(0, sz-1);
            return NLST[idx][1][rng];
        }
        LEVEL level = 1;
        size_t last = walk.size() - 1;
        IDX idx = walk.back();
        DK dk = 1;
        AD_LST lst = NLST[idx][dk];
        AD_SET set = NSET[idx][dk];
        //AD_LST lst_ = util_set2lst(set);
        //AD_SET set_ = util_lst2set(lst);

        while (true) {
            // generate TriLST
            // TriLst triLst = gen_triLst(lst, walk[last-level], dk);
            // generate mask
            // Mask mask = Mask{ triLst };
            // produce by mask-prob
            // Prob prob = MPR[level][dk].gen_mask_prob(mask);
            // int delta = prob.choose();
            // next...
            // level += 1;
            // dk += delta;
            // lst = triLst.get(delta);

            // generate TriSet
            TriSet triSet = gen_triSet(set, walk[last - level], dk);
            // generate mask
            Mask mask = Mask{ triSet };
            // produce by mask-prob
            Prob prob = MPR[level][dk].gen_mask_prob(mask);
            int delta = prob.choose();

            // next...
            level += 1;
            dk += delta;
            set = triSet.get(delta);

            // emit ...
            if (set.count() <= 1) {
                const AD_LST lst = util_set2lst(set);
                // `lst.size() == 0` should raise exception.
                return lst[0];
            }
            if (level > MAX_LEVEL || level > last) {
                // random choice
                const AD_LST lst = util_set2lst(set);
                IDX rng = UNI_INT(0, lst.size() - 1);
                return lst[rng];
            }
        } // !end-while

    }

private:
    void init_dk_by_prefile(const string& filename) {
        map<size_t, string> m_file;
        ifstream in(filename);
        if (!in.good()) {
            cerr << "Prefile `" << filename << "` will be loaded, but it can't open." << endl;
            exit(1);
        }
        string ln;
        while (in) {
            getline(in, ln);
            m_file[stol(ln, nullptr)] = ln.substr(ln.find_first_of(' ') + 1);
        }
        in.close();
        for (DK dk = 1; dk <= size(m_file); ++dk) {
            _WR_DEBUG_(cout << "Initing D" << dk << "... ";);
            clock_t time = clock();
            init_utils(m_file[dk], dk);
            time = clock() - time;
            _WR_DEBUG_(cout << "It took me " << time << " clicks (" << ((float) time) / CLOCKS_PER_SEC << ") seconds.\n";)
        }
    }

    void init_utils(const string& filename, const DK& dk) {
        ifstream in(filename);
        if (!in.good()) {
            cerr << "Prefile-d" << dk << " `" << filename << "` will be loaded, but it can't open." << endl;
            exit(1);
        }
        string ln;
        for (IDX idx = MIN_IDX; idx <= MAX_IDX; ++idx) {
            getline(in, ln);
            NLST[idx][dk] = ln2lst(ln);
            PRELST[idx][dk] = true;
            lst2set(idx, dk);
        }
        in.close();
    }

    void init_dk_by_stepfile(const string& filename, const size_t& n_pre_dk) {
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
        getline(in, ln); // eat out the fucking \n
        for (IDX idx = MIN_IDX; idx <= MAX_IDX; ++idx) {
            getline(in, ln);
            NLST[idx][dk] = ln2lst(ln);
            PRELST[idx][dk] = true;
            lst2set(idx, dk);
    }
        time = clock() - time;
        _WR_DEBUG_(cout << "It took me " << time << " clicks (" << ((float) time) / CLOCKS_PER_SEC << ") seconds.\n";)
        in.close();

        init_table_d(n_pre_dk);
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

    void must_be_true(const DK& max_dk) {
        _WR_DEBUG_(cout << "Check bitset and lst... ";)
        for (DK dk = 1; dk <= max_dk; ++dk) {
            for (IDX idx = MIN_IDX; idx <= MAX_IDX; ++idx) {
                if (NLST[idx][dk].size() != NSET[idx][dk].count()) {
                    cerr << idx << " " << dk << "\n";
                }
                assert(NLST[idx][dk].size() == NSET[idx][dk].count());
                if (NSET[idx][dk].test(idx)) {
                    cerr << idx << " " << dk << '\n';
                }
                assert(!NSET[idx][dk].test(idx));
            }
        }
        _WR_DEBUG_(cout << "All right :)\n";)
    }

    void pre_lst(IDX idx, DK dk) {
        if (dk == 0 || dk == 1 || PRELST[idx][dk])
            return;
        pre_set(idx, dk);
        set2lst(idx, dk);
        PRELST[idx][dk] = true;
    }

    void pre_set(IDX idx, DK dk) {
        if (dk== 0 || dk == 1 || PRESET[idx][dk])
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

    TriLst gen_triLst(const AD_LST& lst, const IDX& idx, const DK& mid_dk) {
        const AD_SET& set = util_lst2set(lst);

        pre_set(idx, mid_dk - 1);
        pre_set(idx, mid_dk);
        pre_set(idx, mid_dk + 1);
        const AD_SET& mid_set = NSET[idx][mid_dk];
        const AD_SET& right_set = NSET[idx][mid_dk + 1];

        if (mid_dk > 1) {
            const AD_SET& left_set = NSET[idx][mid_dk - 1];

            return TriLst{ set & left_set, set & mid_set, set & right_set };
        }
        else {
            AD_LST left_lst = AD_LST();
            if (set.test(idx)) left_lst.push_back(idx);

            return TriLst{ left_lst, set & mid_set, set & right_set };
        }
    }

    TriSet gen_triSet(const AD_SET& set, const IDX& idx, const DK& mid_dk) {
        pre_set(idx, mid_dk - 1);
        pre_set(idx, mid_dk);
        pre_set(idx, mid_dk + 1);
        const AD_SET& mid_set = NSET[idx][mid_dk];
        const AD_SET& right_set = NSET[idx][mid_dk + 1];

        if (mid_dk > 1) {
            const AD_SET& left_set = NSET[idx][mid_dk - 1];

            return TriSet{ set & left_set, set & mid_set, set & right_set };
        }
        else {
            AD_SET left_set = AD_SET();
            if (set.test(idx)) left_set.set(idx);

            return TriSet{ left_set, set & mid_set, set & right_set };
        }
    }
};

std::string walk_str(const WALK& walk) {
    ostringstream oss;
    for (const IDX& idx : walk) {
        oss << idx << ',';
    }
    string str = oss.str();
    if (str.size() > 0)
        str.pop_back();
    return str;
}

void get_walks(ostream& out, const size_t& n_walk_times, Node2VecDist& dist, const string& file_out, const IDX& start_idx) {
    ofstream fout(file_out);
    if (!fout.good()) {
        cerr << "Walks will be outputed to `" << file_out << "`, but it can't open." << endl;
        exit(1);
    }
    vector<WALK> walks;
    _WR_DEBUG_(out << "Begin walking...\n";)
    clock_t total_time = clock();
    double mean_walk_time = 0;
    for (size_t times = 1; times <= n_walk_times; ++times) {
        walks.push_back(dist.tabular_walk(start_idx));
    }
    total_time = clock() - total_time;
    _WR_DEBUG_(out << "[" << n_walk_times << " Walks] It took me " << total_time << " clicks (" << ((float) total_time) / CLOCKS_PER_SEC << ") seconds.\n";)

        _WR_DEBUG_(out << "Output to " << file_out << "...\n";)
        // output ofstream
        for (const WALK& walk : walks) {
            fout << walk_str(walk) << '\n';
        }
    fout.close();
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

M_PR parse_mpr(const string& filename) {
    check_file_in(filename);
    ifstream in(filename);
    M_PR mpr = file2mpr(in);
    in.close();
    return mpr;
}

void parse_mprs_outs(vector<M_PR>& mprs, vector<string>& file_outs, const string& file_config) {
    check_file_in(file_config);
    ifstream in(file_config);
    string mpr;
    string out;
    while (!in.eof()) {
        in >> mpr >> out;
        mprs.push_back(parse_mpr(mpr));
        check_file_out(out);
        file_outs.push_back(out);
    }
    in.close();
    assert(mprs.size() == file_outs.size());
}

int main(int argc, char* argv[]) {
    if(argc < 3) {
        cerr << "prog-name file-para file-out [walk-times=1000] [walk-steps="<< N_NODES <<"]\\\n\t[file-in=`" << FILE_IN << "`] [n-pre-dk=" << N_PRE_DK << "] [file-log=] [start-idx=" << MIN_IDX << "]\n" << endl;
        return 1;
    }
    // ARG: file-para & file-out
    vector<M_PR> mprs;
    vector<string> file_outs;
    const string file_param = argv[1];
    if (file_param.size() > 0 && file_param[0] == '*') {
        if (argv[2][0] != '*') {
            cerr << "`file-para` begins *. Then `file-out` should be `*`." << endl;
            exit(1);
        }
        const string config = argv[1];
        parse_mprs_outs(mprs, file_outs, config.substr(1));
    }
    else {
        mprs.push_back(parse_mpr(file_param));
        check_file_out(argv[2]);
        file_outs.push_back(argv[2]);
    }

    // ARG: walk-times
    size_t n_walk_times = 1000;
    if (argc >= 4) n_walk_times = stoul(argv[3], nullptr);
    
    // ARG: walk-steps
    size_t walk_steps = N_NODES;
    if(argc >= 5) walk_steps = stoul(argv[4], nullptr);

    // ARG: file-in
    string file_in = FILE_IN;
    if(argc >= 6) file_in = argv[5];

    // ARG: n-pre-dk
    size_t n_pre_dk = N_PRE_DK;
    if (argc >= 7) n_pre_dk = stoul(argv[6], nullptr);

    // ARG: start-idx
    IDX start_idx = MIN_IDX;
    if (argc >= 9) start_idx = stoul(argv[8], nullptr);

    // ARG: file-log
    if (argc >= 8) {
        ostream& log_out = ofstream(argv[7]);
        if (!log_out.good()) {
            cerr << "Logs will be outputed to `" << argv[7] << "`, but it can't open." << endl;
            return 1;
        }
        Node2VecDist dist(walk_steps, file_in, n_pre_dk);
        _WR_DEBUG_(cout << "Begin walking, and log at `" << argv[7] << "`...\n";);
        for (size_t i = 0; i < mprs.size(); ++i) {
            _WR_DEBUG_(cout << "Setting mpr[" << i << "]...\n";)
                dist.set_mpr(mprs[i]);
            get_walks(log_out, n_walk_times, dist, file_outs[i], start_idx);
        }
    }
    else {
        ostream& log_out = cout;
        Node2VecDist dist(walk_steps, file_in, n_pre_dk);
        _WR_DEBUG_(cout << "Begin walking, and log at `cout`...\n";);
        for (size_t i = 0; i < mprs.size(); ++i) {
            _WR_DEBUG_(cout << "Setting mpr[" << i << "]...\n";)
            dist.set_mpr(mprs[i]);
            get_walks(log_out, n_walk_times, dist, file_outs[i], start_idx);
        }
    }
    _WR_DEBUG_(cout << "Good Job :)" << endl;)
    return 0;
}

#endif // _CLASSIC_WALK_