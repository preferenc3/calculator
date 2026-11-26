#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include <string>

using namespace std;

// ============================================================================
// LABORATORUL 2 - PARADIGMA FUNCTIONALA (C++)
// Probleme clasice de sah rezolvate STRICT prin recursivitate pura
// ============================================================================

// O coordonata pe tabla de sah: r = rand (linie), c = coloana
struct Coord
{
    int r;
    int c;
};

// O configuratie de piese plasate pe tabla (ex: lista de pozitii)
using Lista = vector<Coord>;

// ============================================================================
// FUNCTII AJUTATOARE PURE (FARA BUCLE, FARA i++, DOAR RETURN)
// ============================================================================

// Verifica recursiv daca doua pozitii sunt identice
bool sunt_egale(Coord a, Coord b)
{
    return a.r == b.r && a.c == b.c;
}

// Verifica recursiv daca un numar 'val' exista intr-o lista de numere
bool contine_nr(vector<int> v, int val, int i = 0)
{
    if (i >= (int)v.size())
        return false; // Am ajuns la final -> nu exista
    if (v[i] == val)
        return true;                  // L-am gasit -> adevarat
    return contine_nr(v, val, i + 1); // Cautam mai departe la i + 1
}

// Verifica recursiv daca o coordonata 'p' exista in lista de pozitii
bool contine_poz(Lista v, Coord p, int i = 0)
{
    if (i >= (int)v.size())
        return false;
    if (sunt_egale(v[i], p))
        return true;
    return contine_poz(v, p, i + 1);
}

// Returneaza o COPIE noua a listei cu elementul 'p' adaugat la final
// (Lista originala a apelantului ramane neatinsa pe stiva!)
Lista adauga_poz(Lista v, Coord p)
{
    v.push_back(p);
    return v;
}

// Returneaza o COPIE noua a vectorului de numere cu 'x' adaugat la final
vector<int> adauga_nr(vector<int> v, int x)
{
    v.push_back(x);
    return v;
}

// Uneste doua liste de solutii intr-o singura lista noua
vector<Lista> uneste(vector<Lista> a, vector<Lista> b)
{
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

// Uneste doua liste de pozitii (folosit la combinarea nebunilor albi cu cei negri)
Lista imbina_poz(Lista a, Lista b)
{
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

// ============================================================================
// FUNCTII PURE PENTRU AFISAREA TABLEI DE SAH (RECURSIV)
// ============================================================================

// Afiseaza celulele de pe randul 'r', de la coloana 'c' pana la n-1
void afiseaza_rand(int r, int c, int n, Lista piese, char simbol)
{
    if (c >= n)
        return;
    if (contine_poz(piese, {r, c}))
    {
        cout << " " << simbol << " ";
    }
    else
    {
        cout << " . ";
    }
    afiseaza_rand(r, c + 1, n, piese, simbol);
}

// Afiseaza tabla n x n rand cu rand (de la r = 0 la n - 1)
void afiseaza_tabla(int n, Lista piese, char simbol, int r = 0)
{
    if (r >= n)
        return;
    cout << "  " << r << " |";
    afiseaza_rand(r, 0, n, piese, simbol);
    cout << "\n";
    afiseaza_tabla(n, piese, simbol, r + 1);
}

// Cauta al catelea pas (1..64) este pozitia 'p' in drumul calului
int gaseste_pas(Lista drum, Coord p, int i = 0)
{
    if (i >= (int)drum.size())
        return 0;
    if (sunt_egale(drum[i], p))
        return i + 1;
    return gaseste_pas(drum, p, i + 1);
}

// Afiseaza un rand din matricea pasilor pentru Drumul Calului
void afiseaza_rand_drum(int r, int c, int n, Lista drum)
{
    if (c >= n)
        return;
    int pas = gaseste_pas(drum, {r, c});
    if (pas < 10)
        cout << "  " << pas << " ";
    else
        cout << " " << pas << " ";
    afiseaza_rand_drum(r, c + 1, n, drum);
}

// Afiseaza toata matricea drumului calului (1 -> n*n)
void afiseaza_drum_cal(int n, Lista drum, int r = 0)
{
    if (r >= n)
        return;
    cout << "  " << r << " |";
    afiseaza_rand_drum(r, 0, n, drum);
    cout << "\n";
    afiseaza_drum_cal(n, drum, r + 1);
}

// ============================================================================
// [1] NEBUNI (Bishops) - Maxim 2*n - 2 nebuni pe tabla n x n (1 punct)
// ============================================================================
// IDEE SIMPLA:
// - Pe fiecare diagonala principala (d = r - c), poate sta CEL MULT UN nebun.
// - Nebunii de pe patrate albe (d par) NU ii ataca NICIODATA pe cei de pe
//   patrate negre (d impar), deci ii rezolvam separat si apoi ii combinam!
// - Doua piese au aceeasi diagonala secundara daca au acelasi (r + c).

vector<Lista> nebuni_pe_culoare(int n, int d, int max_d, int ramasi, Lista plasati, vector<int> anti_folosite);

// Incearca pe rand fiecare patrat (r, c = r - d) de pe diagonala curenta 'd'
vector<Lista> incearca_patrate_diagonala(
    int n, int d, int max_d,
    int r, int max_r,
    int ramasi, Lista plasati, vector<int> anti_folosite)
{
    if (r > max_r)
        return {}; // Am terminat toate patratele de pe diagonala d

    int c = r - d;    // Coloana corespunzatoare randului r pe diagonala d
    int anti = r + c; // Diagonala secundara a acestui patrat

    // Ramura 1: Daca diagonala secundara 'anti' este libera, punem un nebun aici
    vector<Lista> cu_nebun;
    if (!contine_nr(anti_folosite, anti))
    {
        cu_nebun = nebuni_pe_culoare(
            n, d + 2, max_d, ramasi - 1,
            adauga_poz(plasati, {r, c}),
            adauga_nr(anti_folosite, anti));
    }

    // Ramura 2: Trecem la urmatorul patrat de pe aceeasi diagonala (r + 1)
    vector<Lista> urm_patrat = incearca_patrate_diagonala(n, d, max_d, r + 1, max_r, ramasi, plasati, anti_folosite);
    return uneste(cu_nebun, urm_patrat);
}

// Parcurge diagonalele unei singure culori din 2 in 2 (d + 2)
vector<Lista> nebuni_pe_culoare(int n, int d, int max_d, int ramasi, Lista plasati, vector<int> anti_folosite)
{
    if (ramasi == 0)
        return {plasati}; // Succes: am plasat toti nebunii ceruti
    if (d > max_d)
        return {}; // Nu mai avem diagonale

    // Daca au ramas mai putine diagonale decat nebuni de pus, oprim devreme
    int diag_ramase = (max_d - d) / 2 + 1;
    if (diag_ramase < ramasi)
        return {};

    // Limitele randului r pe diagonala d (astfel incat 0 <= r < n si 0 <= r - d < n)
    int r_start = (d > 0) ? d : 0;
    int r_end = (n - 1 + d < n - 1) ? (n - 1 + d) : (n - 1);

    // Optiunea A: punem nebun pe diagonala d
    vector<Lista> ramura_cu = incearca_patrate_diagonala(n, d, max_d, r_start, r_end, ramasi, plasati, anti_folosite);
    // Optiunea B: sarim peste diagonala d fara sa punem nebun pe ea
    vector<Lista> ramura_fara = nebuni_pe_culoare(n, d + 2, max_d, ramasi, plasati, anti_folosite);

    return uneste(ramura_cu, ramura_fara);
}

// Combina o solutie de nebuni albi cu fiecare solutie de nebuni negri
vector<Lista> combina_o_solutie_alba(Lista sol_alba, vector<Lista> negri, int j = 0)
{
    if (j >= (int)negri.size())
        return {};
    vector<Lista> prima = {imbina_poz(sol_alba, negri[j])};
    return uneste(prima, combina_o_solutie_alba(sol_alba, negri, j + 1));
}

// Produsul cartezian dintre toate solutiile albe si toate solutiile negre
vector<Lista> combina_albi_cu_negri(vector<Lista> albi, vector<Lista> negri, int i = 0)
{
    if (i >= (int)albi.size())
        return {};
    return uneste(combina_o_solutie_alba(albi[i], negri, 0), combina_albi_cu_negri(albi, negri, i + 1));
}

// Functia principala pentru Nebuni
vector<Lista> rezolva_nebuni(int n)
{
    if (n <= 0)
        return {};
    if (n == 1)
        return {{{0, 0}}};

    // Prima diagonala alba (para) si prima diagonala neagra (impara)
    int d_alb = (n % 2 == 0) ? -(n - 2) : -(n - 1);
    int d_negru = (n % 2 == 0) ? -(n - 1) : -(n - 2);

    vector<Lista> albi = nebuni_pe_culoare(n, d_alb, -d_alb, n - 1, {}, {});
    vector<Lista> negri = nebuni_pe_culoare(n, d_negru, -d_negru, n - 1, {}, {});
    return combina_albi_cu_negri(albi, negri, 0);
}

// ============================================================================
// [2] TURNURI (Rooks) - Maxim n turnuri pe tabla n x n (1 punct)
// ============================================================================
// IDEE SIMPLA:
// - Turnul ataca pe linie si pe coloana.
// - Punem exact un turn pe fiecare rand (r = 0, 1, ..., n-1) pe o coloana
//   care nu a fost inca folosita (total n! solutii, ex: 8! = 40.320).

vector<Lista> rezolva_turnuri(int n, int r = 0, vector<int> col_folosite = {}, Lista plasati = {});

// Incearca pe rand coloanele 0 .. n-1 pentru randul 'r'
vector<Lista> incearca_coloane_turn(int n, int r, int c, vector<int> col_folosite, Lista plasati)
{
    if (c >= n)
        return {};

    vector<Lista> cu_turn;
    if (!contine_nr(col_folosite, c))
    {
        cu_turn = rezolva_turnuri(
            n, r + 1,
            adauga_nr(col_folosite, c),
            adauga_poz(plasati, {r, c}));
    }

    vector<Lista> urm_coloana = incearca_coloane_turn(n, r, c + 1, col_folosite, plasati);
    return uneste(cu_turn, urm_coloana);
}

vector<Lista> rezolva_turnuri(int n, int r, vector<int> col_folosite, Lista plasati)
{
    if (r == n)
        return {plasati}; // Am pus cate un turn pe fiecare rand
    return incearca_coloane_turn(n, r, 0, col_folosite, plasati);
}

// ============================================================================
// [3] CAI (Knights) - Maxim ceil(n*n / 2) cai pe tabla n x n (2 puncte)
// ============================================================================
// IDEE SIMPLA:
// - Calul sare in 'L': de pe un patrat alb ataca MEREU doar un patrat negru!
// - Deci caii de pe patrate albe nu se ataca niciodata intre ei!
// - Pentru n=8, maximul este 32 de cai: ori toti pe alb, ori toti pe negru.

// Colecteaza recursiv toate patratele de o anumita culoare: (r + c) % 2 == culoare
Lista patrate_culoare(int n, int r, int c, int culoare)
{
    if (r >= n)
        return {};
    if (c >= n)
        return patrate_culoare(n, r + 1, 0, culoare);

    Lista rest = patrate_culoare(n, r, c + 1, culoare);
    if ((r + c) % 2 == culoare)
    {
        return imbina_poz({{r, c}}, rest);
    }
    return rest;
}

vector<Lista> rezolva_cai(int n)
{
    Lista albe = patrate_culoare(n, 0, 0, 0);
    Lista negre = patrate_culoare(n, 0, 0, 1);

    if (n % 2 == 0)
    {
        return {albe, negre}; // La tabla para (ex: 8x8), ambele au cate 32 cai
    }
    // La tabla impara, culoarea cu mai multe patrate are (n*n + 1) / 2 cai
    return (albe.size() >= negre.size()) ? vector<Lista>{albe} : vector<Lista>{negre};
}

// ============================================================================
// [4] REGINE (Queens) - Maxim n regine pe tabla n x n (3 puncte)
// ============================================================================
// IDEE SIMPLA:
// - Regina ataca pe linie, coloana si diagonale.
// - Punem cate o regina pe randul r, verificand sa nu aiba aceeasi coloana
//   sau aceeasi diagonala (abs(r1 - r2) == abs(c1 - c2)) cu reginele puse.

bool e_sigur_regina(Coord cand, Lista plasati, int i = 0)
{
    if (i >= (int)plasati.size())
        return true; // Niciun conflict cu reginele anterioare
    Coord q = plasati[i];
    if (q.c == cand.c || abs(q.r - cand.r) == abs(q.c - cand.c))
    {
        return false; // Se ataca pe coloana sau diagonala
    }
    return e_sigur_regina(cand, plasati, i + 1);
}

vector<Lista> rezolva_regine(int n, int r = 0, Lista plasati = {});

vector<Lista> incearca_coloane_regina(int n, int r, int c, Lista plasati)
{
    if (c >= n)
        return {};

    vector<Lista> cu_regina;
    if (e_sigur_regina({r, c}, plasati))
    {
        cu_regina = rezolva_regine(n, r + 1, adauga_poz(plasati, {r, c}));
    }

    vector<Lista> urm_coloana = incearca_coloane_regina(n, r, c + 1, plasati);
    return uneste(cu_regina, urm_coloana);
}

vector<Lista> rezolva_regine(int n, int r, Lista plasati)
{
    if (r == n)
        return {plasati}; // Toate cele n regine au fost plasate
    return incearca_coloane_regina(n, r, 0, plasati);
}

// ============================================================================
// [5] DRUMUL CALULUI (Knight's Tour) - Toate cele n*n patrate (3 puncte)
// ============================================================================
// IDEE SIMPLA (Euristicul lui Warnsdorff):
// - La fiecare pas alegem mai intai mutarea care are CELE MAI PUTINE
//   continuari posibile mai departe, ca sa nu ramanem blocati in colturi.

const Lista SARITURI_CAL = {
    {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};

// Returneaza lista mutarilor valide si nevizitate din pozitia 'p'
Lista mutari_posibile_cal(Coord p, Lista vizitate, int n, int idx = 0)
{
    if (idx >= (int)SARITURI_CAL.size())
        return {};

    int nr = p.r + SARITURI_CAL[idx].r;
    int nc = p.c + SARITURI_CAL[idx].c;
    Lista rest = mutari_posibile_cal(p, vizitate, n, idx + 1);

    if (nr >= 0 && nr < n && nc >= 0 && nc < n && !contine_poz(vizitate, {nr, nc}))
    {
        return imbina_poz({{nr, nc}}, rest);
    }
    return rest;
}

// Calculeaza cate iesiri libere are patratul 'p' (gradul Warnsdorff)
int grad_warnsdorff(Coord p, Lista vizitate, int n)
{
    return (int)mutari_posibile_cal(p, adauga_poz(vizitate, p), n).size();
}

// Insereaza mutarea 'm' in lista sortata crescator dupa grad_warnsdorff
Lista insereaza_sortat(Coord m, Lista sortate, Lista vizitate, int n, int i = 0)
{
    if (i >= (int)sortate.size())
        return {m};
    if (grad_warnsdorff(m, vizitate, n) <= grad_warnsdorff(sortate[i], vizitate, n))
    {
        Lista coada(sortate.begin() + i, sortate.end());
        return imbina_poz({m}, coada);
    }
    return imbina_poz({sortate[i]}, insereaza_sortat(m, sortate, vizitate, n, i + 1));
}

// Sorteaza recursiv mutarile (cel mai mic grad Warnsdorff primul)
Lista sorteaza_mutari_cal(Lista mutari, Lista vizitate, int n, int i = 0)
{
    if (i >= (int)mutari.size())
        return {};
    return insereaza_sortat(mutari[i], sorteaza_mutari_cal(mutari, vizitate, n, i + 1), vizitate, n);
}

Lista cauta_drum_cal(int n, Coord pos, Lista drum, Lista vizitate);

// Incearca pe rand mutarile sortate; cand gaseste un drum complet, il returneaza
Lista incearca_mutari_drum(int n, Lista mutari, int i, Lista drum, Lista vizitate)
{
    if (i >= (int)mutari.size())
        return {}; // Fundatura -> intoarcem lista vida

    Coord urm = mutari[i];
    Lista gasit = cauta_drum_cal(n, urm, adauga_poz(drum, urm), adauga_poz(vizitate, urm));
    if (!gasit.empty())
        return gasit; // Am gasit drumul complet!

    return incearca_mutari_drum(n, mutari, i + 1, drum, vizitate);
}

Lista cauta_drum_cal(int n, Coord pos, Lista drum, Lista vizitate)
{
    if ((int)drum.size() == n * n)
        return drum; // Am vizitat toate cele n*n patrate!

    Lista posibile = mutari_posibile_cal(pos, vizitate, n);
    Lista sortate = sorteaza_mutari_cal(posibile, vizitate, n);
    return incearca_mutari_drum(n, sortate, 0, drum, vizitate);
}

Lista rezolva_drum_cal(int n, Coord start = {0, 0})
{
    return cauta_drum_cal(n, start, {start}, {start});
}

// ============================================================================
// VALIDARE AUTOMATA (--test) - VERIFICA TOATE CELE 5 PROBLEME RECURSIV
// ============================================================================

void verifica_perechi_nebuni(Lista sol, int i, int j)
{
    if (i >= (int)sol.size())
        return;
    if (j >= (int)sol.size())
    {
        verifica_perechi_nebuni(sol, i + 1, i + 2);
        return;
    }
    assert(abs(sol[i].r - sol[j].r) != abs(sol[i].c - sol[j].c));
    verifica_perechi_nebuni(sol, i, j + 1);
}

void valideaza_nebuni(vector<Lista> sols, int i = 0)
{
    if (i >= (int)sols.size())
        return;
    assert(sols[i].size() == 14);
    verifica_perechi_nebuni(sols[i], 0, 1);
    valideaza_nebuni(sols, i + 1);
}

void verifica_perechi_turnuri(Lista sol, int i, int j)
{
    if (i >= (int)sol.size())
        return;
    if (j >= (int)sol.size())
    {
        verifica_perechi_turnuri(sol, i + 1, i + 2);
        return;
    }
    assert(sol[i].r != sol[j].r && sol[i].c != sol[j].c);
    verifica_perechi_turnuri(sol, i, j + 1);
}

void valideaza_turnuri(vector<Lista> sols, int i = 0, int limita = 2000)
{
    if (i >= (int)sols.size() || i >= limita)
        return;
    assert(sols[i].size() == 8);
    verifica_perechi_turnuri(sols[i], 0, 1);
    valideaza_turnuri(sols, i + 1, limita);
}

void verifica_perechi_cai(Lista sol, int i, int j)
{
    if (i >= (int)sol.size())
        return;
    if (j >= (int)sol.size())
    {
        verifica_perechi_cai(sol, i + 1, i + 2);
        return;
    }
    int dr = abs(sol[i].r - sol[j].r);
    int dc = abs(sol[i].c - sol[j].c);
    assert(!((dr == 1 && dc == 2) || (dr == 2 && dc == 1)));
    verifica_perechi_cai(sol, i, j + 1);
}

void valideaza_cai(vector<Lista> sols, int i = 0)
{
    if (i >= (int)sols.size())
        return;
    assert(sols[i].size() == 32);
    verifica_perechi_cai(sols[i], 0, 1);
    valideaza_cai(sols, i + 1);
}

void verifica_perechi_regine(Lista sol, int i, int j)
{
    if (i >= (int)sol.size())
        return;
    if (j >= (int)sol.size())
    {
        verifica_perechi_regine(sol, i + 1, i + 2);
        return;
    }
    assert(sol[i].r != sol[j].r && sol[i].c != sol[j].c &&
           abs(sol[i].r - sol[j].r) != abs(sol[i].c - sol[j].c));
    verifica_perechi_regine(sol, i, j + 1);
}

void valideaza_regine(vector<Lista> sols, int i = 0)
{
    if (i >= (int)sols.size())
        return;
    assert(sols[i].size() == 8);
    verifica_perechi_regine(sols[i], 0, 1);
    valideaza_regine(sols, i + 1);
}

void valideaza_pasi_drum(Lista drum, int i = 0)
{
    if (i + 1 >= (int)drum.size())
        return;
    assert(!contine_poz(drum, drum[i], i + 1)); // Nu viziteaza de 2 ori aceeasi casuta
    int dr = abs(drum[i].r - drum[i + 1].r);
    int dc = abs(drum[i].c - drum[i + 1].c);
    assert((dr == 1 && dc == 2) || (dr == 2 && dc == 1)); // Pas valid in L
    valideaza_pasi_drum(drum, i + 1);
}

void ruleaza_teste()
{
    cout << "Verificare automata a celor 5 probleme (tabla 8x8)...\n";

    vector<Lista> nebuni = rezolva_nebuni(8);
    assert(nebuni.size() == 256);
    valideaza_nebuni(nebuni);
    cout << "  [OK] 1. Nebuni: 256 solutii (cate 14 piese)\n";

    vector<Lista> turnuri = rezolva_turnuri(8);
    assert(turnuri.size() == 40320);
    valideaza_turnuri(turnuri);
    cout << "  [OK] 2. Turnuri: 40.320 solutii (cate 8 piese)\n";

    vector<Lista> cai = rezolva_cai(8);
    assert(cai.size() == 2);
    valideaza_cai(cai);
    cout << "  [OK] 3. Cai: 2 solutii (cate 32 piese)\n";

    vector<Lista> regine = rezolva_regine(8);
    assert(regine.size() == 92);
    valideaza_regine(regine);
    cout << "  [OK] 4. Regine: 92 solutii (cate 8 piese)\n";

    Lista drum = rezolva_drum_cal(8, {0, 0});
    assert(drum.size() == 64);
    valideaza_pasi_drum(drum);
    cout << "  [OK] 5. Drumul calului: 64 pasi valizi\n";

    cout << "\nTOATE TESTELE AU TRECUT 100%!\n";
}

// ============================================================================
// PROGRAM PRINCIPAL (USOR DE MODIFICAT IN FATA PROFESORULUI)
// ============================================================================
int main(int argc, char *argv[])
{
    // Daca rulam cu ./main --test, verificam automat toate constrangerile
    if (argc > 1 && (string(argv[1]) == "--test" || string(argv[1]) == "-t"))
    {
        ruleaza_teste();
        return 0;
    }

    // ------------------------------------------------------------------------
    // PARAMETRI USOR DE SCHIMBAT:
    // Poti schimba 'n' (ex: 4, 5, 8) sau pozitia de start pentru cal!
    // ------------------------------------------------------------------------
    int n = 8;
    Coord start_cal = {0, 0};

    cout << "=====================================================\n";
    cout << " PARADIGMA FUNCTIONALA - LAB 2: PROBLEME DE SAH (" << n << "x" << n << ")\n";
    cout << "=====================================================\n\n";

    // 1. NEBUNI
    vector<Lista> nebuni = rezolva_nebuni(n);
    cout << "[1] NEBUNI: max " << nebuni[0].size() << " piese -> " << nebuni.size() << " solutii\n";
    afiseaza_tabla(n, nebuni[0], 'N');

    // 2. TURNURI
    vector<Lista> turnuri = rezolva_turnuri(n);
    cout << "\n[2] TURNURI: max " << n << " piese -> " << turnuri.size() << " solutii\n";
    afiseaza_tabla(n, turnuri[0], 'T');

    // 3. CAI
    vector<Lista> cai = rezolva_cai(n);
    cout << "\n[3] CAI: max " << cai[0].size() << " piese -> " << cai.size() << " solutii\n";
    cout << "  Solutia 1 (patrate albe):\n";
    afiseaza_tabla(n, cai[0], 'C');
    if (cai.size() > 1)
    {
        cout << "  Solutia 2 (patrate negre):\n";
        afiseaza_tabla(n, cai[1], 'C');
    }

    // 4. REGINE
    vector<Lista> regine = rezolva_regine(n);
    cout << "\n[4] REGINE: max " << n << " piese -> " << regine.size() << " solutii\n";
    afiseaza_tabla(n, regine[0], 'R');

    // 5. DRUMUL CALULUI
    Lista drum = rezolva_drum_cal(n, start_cal);
    cout << "\n[5] DRUMUL CALULUI (start: " << start_cal.r << "," << start_cal.c << "):\n";
    if (!drum.empty())
    {
        cout << "  Gasit drum complet de " << drum.size() << " pasi:\n";
        afiseaza_drum_cal(n, drum);
    }
    else
    {
        cout << "  Nu exista drum complet pentru aceste dimensiuni.\n";
    }

    return 0;
}
