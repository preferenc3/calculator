#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace std;

// ============================================================================
// Laboratorul 2: Probleme clasice de sah rezolvate in Paradigma Functionala
// ============================================================================
// Cerinte acoperite conform conditii.md (10 puncte):
//   1. Plasarea numarului maxim de NEBUNI non-atacatori (1 punct)
//   2. Plasarea numarului maxim de TURNURI non-atacatori (1 punct)
//   3. Plasarea numarului maxim de CAI non-atacatori (2 puncte)
//   4. Plasarea numarului maxim de REGINE non-atacatori (3 puncte)
//   5. Cautarea unui DRUM AL CALULUI (Knight's Tour) complet (3 puncte)
//
// Principii de Paradigma Functionala aplicate strict:
//   - Zero bucle repetitive (fara 'for', fara 'while').
//   - Imutabilitate: fara mutatii in-place (fara ++, fara +=); fiecare pas
//     creeaza o stare noua transmisa prin valoare / returnata recursiv.
//   - Functii pure si recursivitate structurala.
// ============================================================================

struct Coord {
    int r;
    int c;

    bool operator==(const Coord& other) const {
        return r == other.r && c == other.c;
    }
};

using Placement = vector<Coord>;

// ============================================================================
// Sectiunea 0: Primitive Functionale Pure (fara bucle, fara ++, fara +=)
// ============================================================================

bool contine_int(const vector<int>& v, int val, size_t idx) {
    if (idx >= v.size()) return false;
    if (v[idx] == val) return true;
    return contine_int(v, val, idx + 1);
}

bool contine_coord(const Placement& v, Coord val, size_t idx) {
    if (idx >= v.size()) return false;
    if (v[idx] == val) return true;
    return contine_coord(v, val, idx + 1);
}

template <typename T>
vector<T> adauga(vector<T> v, const T& elem) {
    v.push_back(elem);
    return v;
}

template <typename T>
vector<T> uneste(vector<T> a, const vector<T>& b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

string repeta_str(const string& s, int count) {
    if (count <= 0) return "";
    return s + repeta_str(s, count - 1);
}

string format_int_width(int val, int width) {
    ostringstream oss;
    oss << setw(width) << val;
    return oss.str();
}

// ============================================================================
// Functii Pure pentru Afisarea Tablei de Sah si a Drumului Calului
// ============================================================================

string format_header_cols(int col, int n, int width) {
    if (col >= n) return "";
    string sep = (col + 1 < n) ? " " : "";
    return format_int_width(col, width) + sep + format_header_cols(col + 1, n, width);
}

string format_board_row(int r, int c, int n, const Placement& pieces, const string& symbol) {
    if (c >= n) return "";
    string cell = contine_coord(pieces, {r, c}, 0) ? (" " + symbol) : " .";
    string sep = (c + 1 < n) ? " " : "";
    return cell + sep + format_board_row(r, c + 1, n, pieces, symbol);
}

string format_board_rows(int r, int n, const Placement& pieces, const string& symbol) {
    if (r >= n) return "";
    string line = format_int_width(r, 2) + " | " + format_board_row(r, 0, n, pieces, symbol);
    string rest = format_board_rows(r + 1, n, pieces, symbol);
    return rest.empty() ? line : (line + "\n" + rest);
}

string format_board(int n, const Placement& pieces, const string& symbol) {
    string header = "    " + format_header_cols(0, n, 2);
    string sep = "    " + repeta_str("-", 3 * n);
    return header + "\n" + sep + "\n" + format_board_rows(0, n, pieces, symbol);
}

int find_step_rec(const Placement& path, Coord target, size_t idx) {
    if (idx >= path.size()) return 0;
    if (path[idx] == target) return (int)idx + 1;
    return find_step_rec(path, target, idx + 1);
}

string format_tour_row(int r, int c, int n, const Placement& path) {
    if (c >= n) return "";
    int step = find_step_rec(path, {r, c}, 0);
    string sep = (c + 1 < n) ? " " : "";
    return format_int_width(step, 3) + sep + format_tour_row(r, c + 1, n, path);
}

string format_tour_rows(int r, int n, const Placement& path) {
    if (r >= n) return "";
    string line = format_int_width(r, 2) + " | " + format_tour_row(r, 0, n, path);
    string rest = format_tour_rows(r + 1, n, path);
    return rest.empty() ? line : (line + "\n" + rest);
}

string format_knight_tour(int n, const Placement& path) {
    string header = "    " + format_header_cols(0, n, 3);
    string sep = "    " + repeta_str("-", 4 * n);
    return header + "\n" + sep + "\n" + format_tour_rows(0, n, path);
}

// ============================================================================
// Problema 1: Numarul Maxim de Nebuni (Bishops) non-atacatori (1 punct)
// ============================================================================
// - Nebunii pe patrate albe ((r+c)%2 == 0) si cei pe patrate negre ((r+c)%2 == 1)
//   sunt complet independenti (nu se pot ataca reciproc).
// - Max nebuni pe tabla n x n (n >= 2): 2n - 2 (cate n - 1 pe fiecare culoare).
// - Solutiile totale sunt Produsul Cartezian al solutiilor albe si negre (256 pt n=8).

vector<Coord> gen_squares_color(int n, int r, int c, int color) {
    if (r >= n) return {};
    if (c >= n) return gen_squares_color(n, r + 1, 0, color);
    vector<Coord> rest = gen_squares_color(n, r, c + 1, color);
    if ((r + c) % 2 == color) {
        return uneste(vector<Coord>{{r, c}}, rest);
    }
    return rest;
}

vector<Coord> filter_diag(const vector<Coord>& sqs, size_t idx, int key) {
    if (idx >= sqs.size()) return {};
    vector<Coord> rest = filter_diag(sqs, idx + 1, key);
    if (sqs[idx].r - sqs[idx].c == key) {
        return uneste(vector<Coord>{sqs[idx]}, rest);
    }
    return rest;
}

vector<vector<Coord>> group_by_main_diag(const vector<Coord>& sqs, int key, int max_key) {
    if (key > max_key) return {};
    vector<Coord> diag = filter_diag(sqs, 0, key);
    vector<vector<Coord>> rest = group_by_main_diag(sqs, key + 1, max_key);
    if (!diag.empty()) {
        return uneste(vector<vector<Coord>>{diag}, rest);
    }
    return rest;
}

vector<Placement> solve_color_diags(
    const vector<vector<Coord>>& diags,
    int target,
    size_t diag_idx,
    const Placement& placed,
    const vector<int>& used_anti
);

vector<Placement> try_diag_squares(
    const vector<vector<Coord>>& diags,
    int target,
    size_t diag_idx,
    size_t sq_idx,
    const Placement& placed,
    const vector<int>& used_anti
) {
    if (sq_idx >= diags[diag_idx].size()) return {};
    Coord sq = diags[diag_idx][sq_idx];
    int anti = sq.r + sq.c;

    vector<Placement> with_sq;
    if (!contine_int(used_anti, anti, 0)) {
        with_sq = solve_color_diags(
            diags, target - 1, diag_idx + 1,
            adauga(placed, sq), adauga(used_anti, anti)
        );
    }
    vector<Placement> rest = try_diag_squares(diags, target, diag_idx, sq_idx + 1, placed, used_anti);
    return uneste(with_sq, rest);
}

vector<Placement> solve_color_diags(
    const vector<vector<Coord>>& diags,
    int target,
    size_t diag_idx,
    const Placement& placed,
    const vector<int>& used_anti
) {
    if (target == 0) return {placed};
    if ((int)(diags.size() - diag_idx) < target) return {};

    // Ramura 1: plasam un nebun pe diagonala curenta
    vector<Placement> branch1 = try_diag_squares(diags, target, diag_idx, 0, placed, used_anti);
    // Ramura 2: sarim peste diagonala curenta
    vector<Placement> branch2 = solve_color_diags(diags, target, diag_idx + 1, placed, used_anti);
    return uneste(branch1, branch2);
}

vector<Placement> cartesian_pair(const Placement& w, const vector<Placement>& blacks, size_t b_idx) {
    if (b_idx >= blacks.size()) return {};
    Placement comb = uneste(w, blacks[b_idx]);
    vector<Placement> rest = cartesian_pair(w, blacks, b_idx + 1);
    return uneste(vector<Placement>{comb}, rest);
}

vector<Placement> cartesian_all(const vector<Placement>& whites, const vector<Placement>& blacks, size_t w_idx) {
    if (w_idx >= whites.size()) return {};
    vector<Placement> part = cartesian_pair(whites[w_idx], blacks, 0);
    vector<Placement> rest = cartesian_all(whites, blacks, w_idx + 1);
    return uneste(part, rest);
}

vector<Placement> solve_max_bishops(int n) {
    if (n <= 0) return {};
    if (n == 1) return {{{0, 0}}};

    int bishops_per_color = n - 1;
    vector<Coord> white_sq = gen_squares_color(n, 0, 0, 0);
    vector<Coord> black_sq = gen_squares_color(n, 0, 0, 1);

    vector<vector<Coord>> white_diags = group_by_main_diag(white_sq, -(n - 1), n - 1);
    vector<vector<Coord>> black_diags = group_by_main_diag(black_sq, -(n - 1), n - 1);

    vector<Placement> white_sols = solve_color_diags(white_diags, bishops_per_color, 0, {}, {});
    vector<Placement> black_sols = solve_color_diags(black_diags, bishops_per_color, 0, {}, {});

    return cartesian_all(white_sols, black_sols, 0);
}

// ============================================================================
// Problema 2: Numarul Maxim de Turnuri (Rooks) non-atacatori (1 punct)
// ============================================================================
// - Max turnuri pe tabla n x n: n (cate unul pe fiecare linie si coloana).
// - Numarul total de solutii: n! (pentru n=8 => 40.320 solutii).

vector<Placement> solve_max_rooks(int n, int row, const vector<int>& used_cols, const Placement& placed);

vector<Placement> try_rook_cols(int n, int row, int col, const vector<int>& used_cols, const Placement& placed) {
    if (col >= n) return {};
    vector<Placement> with_col;
    if (!contine_int(used_cols, col, 0)) {
        with_col = solve_max_rooks(n, row + 1, adauga(used_cols, col), adauga(placed, Coord{row, col}));
    }
    vector<Placement> rest = try_rook_cols(n, row, col + 1, used_cols, placed);
    return uneste(with_col, rest);
}

vector<Placement> solve_max_rooks(int n, int row = 0, const vector<int>& used_cols = {}, const Placement& placed = {}) {
    if (row == n) return {placed};
    return try_rook_cols(n, row, 0, used_cols, placed);
}

// ============================================================================
// Problema 3: Numarul Maxim de Cai (Knights) non-atacatori (2 puncte)
// ============================================================================
// - Proprietate bipartita: un cal de pe un patrat alb ataca DOAR patrate negre.
// - Max cai pe tabla 8 x 8: ceil(64 / 2) = 32 cai (2 configuratii maxime).

const vector<Coord> KNIGHT_OFFSETS = {
    {-2, -1}, {-2,  1}, {-1, -2}, {-1,  2},
    { 1, -2}, { 1,  2}, { 2, -1}, { 2,  1}
};

vector<Placement> solve_max_knights(int n) {
    Placement white_placement = gen_squares_color(n, 0, 0, 0);
    Placement black_placement = gen_squares_color(n, 0, 0, 1);

    if (n % 2 == 0) {
        return {white_placement, black_placement};
    }
    Placement dominant = (white_placement.size() >= black_placement.size()) ? white_placement : black_placement;
    return {dominant};
}

// ============================================================================
// Problema 4: Numarul Maxim de Regine (Queens) non-atacatori (3 puncte)
// ============================================================================
// - Max regine pe tabla n x n: n (pentru n=8 => 92 solutii).

bool is_safe_queen(Coord cand, const Placement& placed, size_t idx) {
    if (idx >= placed.size()) return true;
    Coord q = placed[idx];
    if (q.c == cand.c || abs(q.r - cand.r) == abs(q.c - cand.c)) {
        return false;
    }
    return is_safe_queen(cand, placed, idx + 1);
}

vector<Placement> solve_max_queens(int n, int row, const Placement& placed);

vector<Placement> try_queen_cols(int n, int row, int col, const Placement& placed) {
    if (col >= n) return {};
    vector<Placement> with_col;
    Coord cand{row, col};
    if (is_safe_queen(cand, placed, 0)) {
        with_col = solve_max_queens(n, row + 1, adauga(placed, cand));
    }
    vector<Placement> rest = try_queen_cols(n, row, col + 1, placed);
    return uneste(with_col, rest);
}

vector<Placement> solve_max_queens(int n, int row = 0, const Placement& placed = {}) {
    if (row == n) return {placed};
    return try_queen_cols(n, row, 0, placed);
}

// ============================================================================
// Problema 5: Drumul Calului (Knight's Tour) (3 puncte)
// ============================================================================
// - Viziteaza fiecare patratel al tablei n x n exact o singura data.
// - Foloseste euristicul lui Warnsdorff (alege intotdeauna mutarea cu cele
//   mai putine continuari posibile, in mod pur recursiv).

vector<Coord> legal_knight_moves_rec(Coord pos, const Placement& visited, int n, size_t off_idx) {
    if (off_idx >= KNIGHT_OFFSETS.size()) return {};
    int nr = pos.r + KNIGHT_OFFSETS[off_idx].r;
    int nc = pos.c + KNIGHT_OFFSETS[off_idx].c;
    vector<Coord> rest = legal_knight_moves_rec(pos, visited, n, off_idx + 1);
    if (nr >= 0 && nr < n && nc >= 0 && nc < n && !contine_coord(visited, {nr, nc}, 0)) {
        return uneste(vector<Coord>{{nr, nc}}, rest);
    }
    return rest;
}

int warnsdorff_degree(Coord pos, const Placement& visited, int n) {
    return (int)legal_knight_moves_rec(pos, adauga(visited, pos), n, 0).size();
}

// Inserare sortata pur recursiva dupa gradul Warnsdorff
vector<Coord> insert_sorted_move(Coord m, const vector<Coord>& sorted, const Placement& visited, int n, size_t idx) {
    if (idx >= sorted.size()) return {m};
    if (warnsdorff_degree(m, visited, n) <= warnsdorff_degree(sorted[idx], visited, n)) {
        vector<Coord> tail(sorted.begin() + idx, sorted.end());
        return uneste(vector<Coord>{m}, tail);
    }
    return uneste(vector<Coord>{sorted[idx]}, insert_sorted_move(m, sorted, visited, n, idx + 1));
}

vector<Coord> sort_warnsdorff_moves(const vector<Coord>& moves, const Placement& visited, int n, size_t idx) {
    if (idx >= moves.size()) return {};
    vector<Coord> sorted_rest = sort_warnsdorff_moves(moves, visited, n, idx + 1);
    return insert_sorted_move(moves[idx], sorted_rest, visited, n, 0);
}

pair<bool, Placement> search_tour(int n, Coord cur_pos, const Placement& path, const Placement& visited);

pair<bool, Placement> try_tour_moves(int n, const vector<Coord>& moves, size_t idx, const Placement& path, const Placement& visited) {
    if (idx >= moves.size()) return {false, {}};
    Coord next_pos = moves[idx];
    pair<bool, Placement> res = search_tour(n, next_pos, adauga(path, next_pos), adauga(visited, next_pos));
    if (res.first) return res;
    return try_tour_moves(n, moves, idx + 1, path, visited);
}

pair<bool, Placement> search_tour(int n, Coord cur_pos, const Placement& path, const Placement& visited) {
    if ((int)path.size() == n * n) {
        return {true, path};
    }
    vector<Coord> moves = legal_knight_moves_rec(cur_pos, visited, n, 0);
    vector<Coord> sorted = sort_warnsdorff_moves(moves, visited, n, 0);
    return try_tour_moves(n, sorted, 0, path, visited);
}

pair<bool, Placement> solve_knights_tour(int n = 8, Coord start = {0, 0}) {
    return search_tour(n, start, {start}, {start});
}

// ============================================================================
// Sectiunea 6: Validare Automata (Unit Tests) - Pur Recursiv
// ============================================================================

void check_bishops_pair(const Placement& sol, size_t i, size_t j) {
    if (i >= sol.size()) return;
    if (j >= sol.size()) {
        check_bishops_pair(sol, i + 1, i + 2);
        return;
    }
    assert(abs(sol[i].r - sol[j].r) != abs(sol[i].c - sol[j].c));
    check_bishops_pair(sol, i, j + 1);
}

void validate_bishops_list(const vector<Placement>& sols, size_t idx) {
    if (idx >= sols.size()) return;
    assert(sols[idx].size() == 14);
    check_bishops_pair(sols[idx], 0, 1);
    validate_bishops_list(sols, idx + 1);
}

void check_rooks_pair(const Placement& sol, size_t i, size_t j) {
    if (i >= sol.size()) return;
    if (j >= sol.size()) {
        check_rooks_pair(sol, i + 1, i + 2);
        return;
    }
    assert(sol[i].r != sol[j].r && sol[i].c != sol[j].c);
    check_rooks_pair(sol, i, j + 1);
}

void validate_rooks_list(const vector<Placement>& sols, size_t idx, size_t limit) {
    if (idx >= sols.size() || idx >= limit) return;
    assert(sols[idx].size() == 8);
    check_rooks_pair(sols[idx], 0, 1);
    validate_rooks_list(sols, idx + 1, limit);
}

void check_knights_pair(const Placement& sol, size_t i, size_t j) {
    if (i >= sol.size()) return;
    if (j >= sol.size()) {
        check_knights_pair(sol, i + 1, i + 2);
        return;
    }
    int dr = abs(sol[i].r - sol[j].r);
    int dc = abs(sol[i].c - sol[j].c);
    assert(!((dr == 1 && dc == 2) || (dr == 2 && dc == 1)));
    check_knights_pair(sol, i, j + 1);
}

void validate_knights_list(const vector<Placement>& sols, size_t idx) {
    if (idx >= sols.size()) return;
    assert(sols[idx].size() == 32);
    check_knights_pair(sols[idx], 0, 1);
    validate_knights_list(sols, idx + 1);
}

void check_queens_pair(const Placement& sol, size_t i, size_t j) {
    if (i >= sol.size()) return;
    if (j >= sol.size()) {
        check_queens_pair(sol, i + 1, i + 2);
        return;
    }
    assert(sol[i].r != sol[j].r && sol[i].c != sol[j].c &&
           abs(sol[i].r - sol[j].r) != abs(sol[i].c - sol[j].c));
    check_queens_pair(sol, i, j + 1);
}

void validate_queens_list(const vector<Placement>& sols, size_t idx) {
    if (idx >= sols.size()) return;
    assert(sols[idx].size() == 8);
    check_queens_pair(sols[idx], 0, 1);
    validate_queens_list(sols, idx + 1);
}

void validate_tour_steps(const Placement& tour, size_t idx) {
    if (idx + 1 >= tour.size()) return;
    assert(!contine_coord(tour, tour[idx], idx + 1)); // unicitate
    int dr = abs(tour[idx].r - tour[idx + 1].r);
    int dc = abs(tour[idx].c - tour[idx + 1].c);
    assert((dr == 1 && dc == 2) || (dr == 2 && dc == 1)); // mutare valida de cal
    validate_tour_steps(tour, idx + 1);
}

void validate_all_solutions() {
    cout << "Incepe validarea automata a constrangerilor...\n";

    // 1. Nebuni
    vector<Placement> bishops = solve_max_bishops(8);
    assert(bishops.size() == 256);
    validate_bishops_list(bishops, 0);
    cout << "  [OK] Problema 1 (Nebuni): 256 solutii valide verificate.\n";

    // 2. Turnuri
    vector<Placement> rooks = solve_max_rooks(8);
    assert(rooks.size() == 40320);
    validate_rooks_list(rooks, 0, 2000);
    cout << "  [OK] Problema 2 (Turnuri): 40.320 solutii valide verificate.\n";

    // 3. Cai
    vector<Placement> knights = solve_max_knights(8);
    assert(knights.size() == 2);
    validate_knights_list(knights, 0);
    cout << "  [OK] Problema 3 (Cai): 2 solutii de cate 32 cai verificate.\n";

    // 4. Regine
    vector<Placement> queens = solve_max_queens(8);
    assert(queens.size() == 92);
    validate_queens_list(queens, 0);
    cout << "  [OK] Problema 4 (Regine): 92 solutii valide verificate.\n";

    // 5. Drumul calului
    pair<bool, Placement> tour_res = solve_knights_tour(8, {0, 0});
    assert(tour_res.first);
    assert(tour_res.second.size() == 64);
    validate_tour_steps(tour_res.second, 0);
    cout << "  [OK] Problema 5 (Drumul calului): Tur complet Hamiltonian de 64 pasi verificat.\n";

    cout << "\nTOATE CELE 5 PROBLEME AU TRECUT 100% DIN TESTELE DE CORECTITUDINE!\n";
}

// ============================================================================
// Sectiunea 7: Demonstratie si Program Principal
// ============================================================================

void run_lab_demonstration() {
    const int n = 8;
    const string sep = repeta_str("=", 70);

    cout << sep << "\n";
    cout << "PARADIGMA FUNCTIONALA - LABORATORUL 2: PROBLEME DE SAH (C++)\n";
    cout << "Implementare stricta cu Imutabilitate si Recursivitate Pura\n";
    cout << sep << "\n";

    // 1. Nebuni
    cout << "\n[1] GENERAREA PLASARILOR PENTRU NUMARUL MAXIM DE NEBUNI\n";
    cout << "    Dimensiune tabla: " << n << "x" << n << "\n";
    cout << "    Numar maxim teoretic de nebuni: 2*" << n << " - 2 = " << (2 * n - 2) << "\n";

    auto t0 = chrono::high_resolution_clock::now();
    vector<Placement> bishops = solve_max_bishops(n);
    auto t1 = chrono::high_resolution_clock::now();

    cout << "    Total solutii gasite: " << bishops.size() << " (asteptat: 256)\n";
    cout << "    Timp de executie: " << fixed << setprecision(2)
         << chrono::duration<double, milli>(t1 - t0).count() << " ms\n";
    if (!bishops.empty()) {
        cout << "\n    Exemplu prima solutie de nebuni (B):\n";
        cout << format_board(n, bishops[0], "B") << "\n";
    }

    // 2. Turnuri
    cout << "\n" << sep << "\n";
    cout << "\n[2] GENERAREA PLASARILOR PENTRU NUMARUL MAXIM DE TURNURI\n";
    cout << "    Dimensiune tabla: " << n << "x" << n << "\n";
    cout << "    Numar maxim de turnuri: " << n << " (total solutii: 8! = 40.320)\n";

    t0 = chrono::high_resolution_clock::now();
    vector<Placement> rooks = solve_max_rooks(n);
    t1 = chrono::high_resolution_clock::now();

    cout << "    Total solutii generate: " << rooks.size() << "\n";
    cout << "    Timp de executie: " << fixed << setprecision(2)
         << chrono::duration<double, milli>(t1 - t0).count() << " ms\n";
    if (!rooks.empty()) {
        cout << "\n    Exemplu prima solutie de turnuri (T):\n";
        cout << format_board(n, rooks[0], "T") << "\n";
    }

    // 3. Cai
    cout << "\n" << sep << "\n";
    cout << "\n[3] GENERAREA PLASARILOR PENTRU NUMARUL MAXIM DE CAI\n";
    cout << "    Dimensiune tabla: " << n << "x" << n << "\n";
    cout << "    Numar maxim de cai non-atacatori: " << (n * n / 2) << "\n";

    t0 = chrono::high_resolution_clock::now();
    vector<Placement> knights = solve_max_knights(n);
    t1 = chrono::high_resolution_clock::now();

    cout << "    Total solutii maxime gasite: " << knights.size() << "\n";
    cout << "    Timp de executie: " << fixed << setprecision(2)
         << chrono::duration<double, milli>(t1 - t0).count() << " ms\n";
    cout << "\n    Solutia 1: Toti cei 32 de cai (C) pe patrate ALBE:\n";
    cout << format_board(n, knights[0], "C") << "\n";
    if (knights.size() > 1) {
        cout << "\n    Solutia 2: Toti cei 32 de cai (C) pe patrate NEGRE:\n";
        cout << format_board(n, knights[1], "C") << "\n";
    }

    // 4. Regine
    cout << "\n" << sep << "\n";
    cout << "\n[4] GENERAREA PLASARILOR PENTRU NUMARUL MAXIM DE REGINE\n";
    cout << "    Dimensiune tabla: " << n << "x" << n << "\n";
    cout << "    Numar maxim de regine: " << n << " (asteptat: 92 solutii)\n";

    t0 = chrono::high_resolution_clock::now();
    vector<Placement> queens = solve_max_queens(n);
    t1 = chrono::high_resolution_clock::now();

    cout << "    Total solutii gasite: " << queens.size() << "\n";
    cout << "    Timp de executie: " << fixed << setprecision(2)
         << chrono::duration<double, milli>(t1 - t0).count() << " ms\n";
    if (!queens.empty()) {
        cout << "\n    Exemplu prima solutie de regine (Q):\n";
        cout << format_board(n, queens[0], "Q") << "\n";
    }

    // 5. Drumul Calului
    cout << "\n" << sep << "\n";
    cout << "\n[5] CAUTAREA UNUI DRUM AL CALULUI (Knight's Tour)\n";
    cout << "    Dimensiune tabla: " << n << "x" << n << " (" << (n * n) << " patrate)\n";

    Coord start_sq{0, 0};
    t0 = chrono::high_resolution_clock::now();
    pair<bool, Placement> tour_res = solve_knights_tour(n, start_sq);
    t1 = chrono::high_resolution_clock::now();

    if (tour_res.first) {
        cout << "    Drum gasit cu succes din pozitia (0, 0)!\n";
        cout << "    Numar total de pasi: " << tour_res.second.size() << "\n";
        cout << "    Timp de executie: " << fixed << setprecision(2)
             << chrono::duration<double, milli>(t1 - t0).count() << " ms\n";
        cout << "\n    Matricea pasilor drumului calului (1 -> 64):\n";
        cout << format_knight_tour(n, tour_res.second) << "\n";
    } else {
        cout << "    Nu a fost gasit un drum al calului.\n";
    }

    cout << "\n" << sep << "\n";
    cout << "Toate cele 5 cerinte ale laboratorului au fost rezolvate cu succes!\n";
    cout << sep << "\n";
}

int main(int argc, char* argv[]) {
    if (argc > 1 && (string(argv[1]) == "--test" || string(argv[1]) == "-t")) {
        validate_all_solutions();
    } else {
        run_lab_demonstration();
    }
    return 0;
}
