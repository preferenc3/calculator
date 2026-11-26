#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <cassert>

using namespace std;

// ==============================================================================
// Lucrarea de Laborator 1 - Paradigma Procedurala (C++)
// Redactor de Text Procedural cu Instrumente de Analiza si Prelucrare Textuala
// ==============================================================================
// Principii procedurale respectate:
// - Fara clase definite (zero OOP: doar structuri de date simple si proceduri).
// - Starea aplicatiei gestionata prin variabile globale explicite si parametri.
// - Descompunere modulara in proceduri independente pentru fiecare cerinta:
//   a. gaseste_cel_mai_frecvent() (cuvant si expresie)
//   b. hash_text() (DJB2 si suma ASCII)
//   c. codifica_text() si decodifica_text() + verificare
//   d. imparte_in_propozitii(), lungime medie, propozitia cea mai lunga
//   e. Statistica textuala (cuvinte, caractere, linii, cuvinte unice)
//   f. sorteaza_linii_dupa_lungime() si sorteaza_propozitii_dupa_lungime()
// ==============================================================================

// ==============================================================================
// VARIABILE GLOBALE DE STARE (PARADIGMA PROCEDURALA)
// ==============================================================================
string TEXT_CURENT = "";
string CALE_FISIER_CURENT = "";
bool TEXT_MODIFICAT = false;
int ULTIMA_DEPLASARE = 3;

const string TEXT_EXEMPLU_DEMO =
    "Paradigma procedurala este un model fundamental de programare.\n"
    "Programarea procedurala se bazeaza pe apeluri de functii si proceduri independente!\n"
    "Fiecare procedura rezolva o problema specifica.\n"
    "Oare este eficienta paradigma procedurala in prelucrarea textelor?\n"
    "Da, paradigma procedurala este simpla, eleganta si foarte usor de urmarit.\n"
    "In laboratorul 1 analizam frecventa cuvintelor si calculam statistici diverse.\n"
    "Succes in explorarea conceptelor procedurale!";

// Structuri simple de date (record-uri procedurale)
struct RezultatFrecventa {
    string element;
    int aparitii;
    map<string, int> frecvente;
};

struct RezultatPropozitieMax {
    string propozitie;
    int numar_cuvinte;
    int numar_caractere;
};

struct AnalizaPropozitii {
    vector<string> propozitii;
    int numar_propozitii;
    double lungime_medie_cuvinte;
    string propozitie_maxima;
    int cuvinte_propozitie_maxima;
    int caractere_propozitie_maxima;
};

struct StatisticiText {
    int numar_cuvinte;
    int numar_caractere_cu_spatii;
    int numar_caractere_fara_spatii;
    int numar_linii;
    int numar_cuvinte_unice;
};

// ==============================================================================
// FUNCTII AUXILIARE DE PRELUCRARE SIRURI
// ==============================================================================

string trim_text(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

string to_lower_ascii(const string& s) {
    string rez = s;
    for (size_t i = 0; i < rez.size(); i++) {
        rez[i] = (char)tolower((unsigned char)rez[i]);
    }
    return rez;
}

bool este_caracter_cuvant(unsigned char c) {
    // Recunoaste litere ASCII, cifre, '_' si octeti UTF-8 (diacritice)
    return isalnum(c) || c == '_' || c >= 128;
}

// ==============================================================================
// A. CAUTAREA CELUI MAI FRECVENT CUVANT SAU EXPRESIE (2 puncte)
// ==============================================================================

vector<string> extrage_cuvinte(const string& text) {
    vector<string> cuvinte;
    if (text.empty()) return cuvinte;

    string lower = to_lower_ascii(text);
    string curent = "";

    for (size_t i = 0; i < lower.size(); i++) {
        unsigned char c = (unsigned char)lower[i];
        if (este_caracter_cuvant(c)) {
            curent += lower[i];
        } else {
            if (!curent.empty()) {
                cuvinte.push_back(curent);
                curent.clear();
            }
        }
    }
    if (!curent.empty()) {
        cuvinte.push_back(curent);
    }
    return cuvinte;
}

map<string, int> calculeaza_frecventa_cuvinte(const string& text) {
    vector<string> cuvinte = extrage_cuvinte(text);
    map<string, int> frecvente;
    for (size_t i = 0; i < cuvinte.size(); i++) {
        frecvente[cuvinte[i]]++;
    }
    return frecvente;
}

map<string, int> calculeaza_frecventa_expresii(const string& text, int lungime = 2) {
    vector<string> cuvinte = extrage_cuvinte(text);
    map<string, int> frecvente;
    if ((int)cuvinte.size() < lungime || lungime <= 0) return frecvente;

    for (size_t i = 0; i <= cuvinte.size() - (size_t)lungime; i++) {
        string expresie = cuvinte[i];
        for (int j = 1; j < lungime; j++) {
            expresie += " " + cuvinte[i + j];
        }
        frecvente[expresie]++;
    }
    return frecvente;
}

int calculeaza_frecventa_expresie_specifica(const string& text, const string& expresie) {
    string exp_curata = to_lower_ascii(trim_text(expresie));
    string text_lower = to_lower_ascii(text);
    if (text_lower.empty() || exp_curata.empty()) return 0;

    int aparitii = 0;
    size_t pos = 0;
    while ((pos = text_lower.find(exp_curata, pos)) != string::npos) {
        aparitii++;
        pos += exp_curata.size();
    }
    return aparitii;
}

RezultatFrecventa gaseste_cel_mai_frecvent(const string& text, const string& tip = "cuvant", int lungime_expresie = 2) {
    RezultatFrecventa rez;
    rez.aparitii = 0;

    if (tip == "cuvant") {
        rez.frecvente = calculeaza_frecventa_cuvinte(text);
    } else if (tip == "expresie") {
        rez.frecvente = calculeaza_frecventa_expresii(text, lungime_expresie);
    } else {
        cout << "[EROARE] Parametrul 'tip' trebuie sa fie 'cuvant' sau 'expresie'.\n";
        return rez;
    }

    for (const auto& kv : rez.frecvente) {
        if (kv.second > rez.aparitii) {
            rez.element = kv.first;
            rez.aparitii = kv.second;
        }
    }
    return rez;
}

// ==============================================================================
// B. HASH-UL TEXTULUI (1 punct)
// ==============================================================================

uint64_t hash_ascii_sum(const string& text) {
    uint64_t suma = 0;
    for (size_t i = 0; i < text.size(); i++) {
        suma += (unsigned char)text[i];
    }
    return suma;
}

uint64_t hash_djb2(const string& text) {
    uint64_t hash_val = 5381;
    for (size_t i = 0; i < text.size(); i++) {
        hash_val = ((hash_val << 5) + hash_val) + (unsigned char)text[i];
    }
    return hash_val;
}

uint64_t hash_text(const string& text, const string& algoritm = "djb2") {
    if (algoritm == "djb2") {
        return hash_djb2(text);
    } else if (algoritm == "ascii_sum") {
        return hash_ascii_sum(text);
    }
    return hash_djb2(text);
}

// ==============================================================================
// C. CODIFICAREA SI DECODIFICAREA TEXTULUI (2 puncte)
// ==============================================================================

string codifica_text(const string& text, int deplasare = 3) {
    string rezultat = "";
    rezultat.reserve(text.size());

    for (size_t i = 0; i < text.size(); i++) {
        unsigned char c = (unsigned char)text[i];
        if (c == '\n') {
            rezultat += '\n';
        } else {
            // Deplasare ciclica in spatiul ASCII/byte [0..255]
            int cod_nou = ((int)c + deplasare) % 256;
            if (cod_nou < 0) cod_nou += 256;
            rezultat += (char)(unsigned char)cod_nou;
        }
    }
    return rezultat;
}

string decodifica_text(const string& text, int deplasare = 3) {
    string rezultat = "";
    rezultat.reserve(text.size());

    for (size_t i = 0; i < text.size(); i++) {
        unsigned char c = (unsigned char)text[i];
        if (c == '\n') {
            rezultat += '\n';
        } else {
            int cod_nou = ((int)c - deplasare) % 256;
            if (cod_nou < 0) cod_nou += 256;
            rezultat += (char)(unsigned char)cod_nou;
        }
    }
    return rezultat;
}

bool verifica_codificare_decodificare(const string& text, int deplasare, string& out_codificat, string& out_decodificat) {
    out_codificat = codifica_text(text, deplasare);
    out_decodificat = decodifica_text(out_codificat, deplasare);
    return (out_decodificat == text);
}

// ==============================================================================
// E (declarare anticipata numara_cuvinte pentru D)
// ==============================================================================

int numara_cuvinte(const string& text) {
    if (text.empty()) return 0;
    return (int)extrage_cuvinte(text).size();
}

int numara_caractere(const string& text, bool include_spatii = true) {
    if (text.empty()) return 0;
    if (include_spatii) return (int)text.size();

    int contor = 0;
    for (size_t i = 0; i < text.size(); i++) {
        if (!isspace((unsigned char)text[i])) {
            contor++;
        }
    }
    return contor;
}

int numara_linii(const string& text) {
    if (text.empty()) return 0;
    int linii = 1;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '\n') {
            if (i + 1 < text.size()) {
                linii++;
            }
        }
    }
    return linii;
}

int numara_cuvinte_unice(const string& text) {
    if (text.empty()) return 0;
    vector<string> cuvinte = extrage_cuvinte(text);
    set<string> unice(cuvinte.begin(), cuvinte.end());
    return (int)unice.size();
}

StatisticiText calculeaza_statistici_text(const string& text) {
    StatisticiText st;
    st.numar_cuvinte = numara_cuvinte(text);
    st.numar_caractere_cu_spatii = numara_caractere(text, true);
    st.numar_caractere_fara_spatii = numara_caractere(text, false);
    st.numar_linii = numara_linii(text);
    st.numar_cuvinte_unice = numara_cuvinte_unice(text);
    return st;
}

// ==============================================================================
// D. ANALIZA PROPOZITIILOR (2 puncte) - Proceduri separate
// ==============================================================================

vector<string> imparte_in_propozitii(const string& text) {
    vector<string> propozitii;
    string cur = "";

    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        if (c == '\n') {
            string curatat = trim_text(cur);
            if (!curatat.empty()) propozitii.push_back(curatat);
            cur.clear();
        } else if (c == '.' || c == '!' || c == '?') {
            cur += c;
            // Daca urmeaza spatiu, newline sau sfarsit de text, incheiem propozitia
            if (i + 1 >= text.size() || isspace((unsigned char)text[i + 1])) {
                string curatat = trim_text(cur);
                if (!curatat.empty()) propozitii.push_back(curatat);
                cur.clear();
            }
        } else {
            cur += c;
        }
    }
    string ramas = trim_text(cur);
    if (!ramas.empty()) {
        propozitii.push_back(ramas);
    }
    return propozitii;
}

double calculeaza_lungime_medie_propozitii(const vector<string>& propozitii) {
    if (propozitii.empty()) return 0.0;
    int total_cuvinte = 0;
    for (size_t i = 0; i < propozitii.size(); i++) {
        total_cuvinte += numara_cuvinte(propozitii[i]);
    }
    double medie = (double)total_cuvinte / (double)propozitii.size();
    return round(medie * 100.0) / 100.0;
}

RezultatPropozitieMax gaseste_propozitia_cea_mai_lunga(const vector<string>& propozitii) {
    RezultatPropozitieMax rez{"", 0, 0};
    for (size_t i = 0; i < propozitii.size(); i++) {
        int nr_cuv = numara_cuvinte(propozitii[i]);
        int nr_car = (int)propozitii[i].size();
        if (nr_cuv > rez.numar_cuvinte || (nr_cuv == rez.numar_cuvinte && nr_car > rez.numar_caractere)) {
            rez.propozitie = propozitii[i];
            rez.numar_cuvinte = nr_cuv;
            rez.numar_caractere = nr_car;
        }
    }
    return rez;
}

AnalizaPropozitii analizeaza_propozitii(const string& text) {
    AnalizaPropozitii ap;
    ap.propozitii = imparte_in_propozitii(text);
    ap.numar_propozitii = (int)ap.propozitii.size();
    ap.lungime_medie_cuvinte = calculeaza_lungime_medie_propozitii(ap.propozitii);
    RezultatPropozitieMax pmax = gaseste_propozitia_cea_mai_lunga(ap.propozitii);
    ap.propozitie_maxima = pmax.propozitie;
    ap.cuvinte_propozitie_maxima = pmax.numar_cuvinte;
    ap.caractere_propozitie_maxima = pmax.numar_caractere;
    return ap;
}

// ==============================================================================
// F. SORTAREA LINIILOR SI PROPOZITIILOR DUPA LUNGIME (1 punct)
// ==============================================================================

vector<string> imparte_in_linii(const string& text) {
    vector<string> linii;
    istringstream iss(text);
    string linie;
    while (getline(iss, linie)) {
        if (!linie.empty() && linie.back() == '\r') {
            linie.pop_back();
        }
        linii.push_back(linie);
    }
    return linii;
}

vector<string> sorteaza_linii_dupa_lungime(const string& text, bool descrescator = false, bool elimina_goale = false) {
    if (text.empty()) return {};
    vector<string> linii = imparte_in_linii(text);

    if (elimina_goale) {
        vector<string> filtrate;
        for (size_t i = 0; i < linii.size(); i++) {
            if (!trim_text(linii[i]).empty()) {
                filtrate.push_back(linii[i]);
            }
        }
        linii = filtrate;
    }

    stable_sort(linii.begin(), linii.end(), [descrescator](const string& a, const string& b) {
        if (descrescator) return a.size() > b.size();
        return a.size() < b.size();
    });
    return linii;
}

vector<string> sorteaza_propozitii_dupa_lungime(const string& text, bool descrescator = false, const string& criteriu = "caractere") {
    vector<string> propozitii = imparte_in_propozitii(text);
    stable_sort(propozitii.begin(), propozitii.end(), [descrescator, &criteriu](const string& a, const string& b) {
        size_t la = (criteriu == "cuvinte") ? (size_t)numara_cuvinte(a) : a.size();
        size_t lb = (criteriu == "cuvinte") ? (size_t)numara_cuvinte(b) : b.size();
        if (descrescator) return la > lb;
        return la < lb;
    });
    return propozitii;
}

// ==============================================================================
// G. GESTIUNE FISIERE SI TEXT (REDACTOR DE TEXT PROCEDURAL)
// ==============================================================================

bool deschide_fisier(const string& cale) {
    ifstream f(cale);
    if (!f.is_open()) {
        cout << "[EROARE] Nu s-a putut deschide fisierul: " << cale << "\n";
        return false;
    }
    stringstream ss;
    ss << f.rdbuf();
    TEXT_CURENT = ss.str();
    CALE_FISIER_CURENT = cale;
    TEXT_MODIFICAT = false;
    cout << "[OK] Fisier incarcat: " << cale << " (" << TEXT_CURENT.size() << " octeti)\n";
    return true;
}

bool salveaza_fisier(const string& cale) {
    if (cale.empty()) {
        cout << "[EROARE] Nu este specificata o cale de fisier!\n";
        return false;
    }
    ofstream f(cale);
    if (!f.is_open()) {
        cout << "[EROARE] Nu s-a putut scrie in fisierul: " << cale << "\n";
        return false;
    }
    f << TEXT_CURENT;
    CALE_FISIER_CURENT = cale;
    TEXT_MODIFICAT = false;
    cout << "[OK] Text salvat cu succes in: " << cale << "\n";
    return true;
}

void editeaza_text_consola() {
    cout << "\nIntrodu noul text (termina cu o linie ce contine doar 'END'):\n";
    string nou = "";
    string linie;
    while (getline(cin, linie)) {
        if (linie == "END" || linie == "end") break;
        if (!nou.empty()) nou += "\n";
        nou += linie;
    }
    TEXT_CURENT = nou;
    TEXT_MODIFICAT = true;
    cout << "[OK] Text actualizat in editor (" << TEXT_CURENT.size() << " caractere).\n";
}

// ==============================================================================
// H. TESTE AUTOMATE (--test)
// ==============================================================================

void ruleaza_teste_automate() {
    cout << "============================================================\n";
    cout << "RULARE SUITA DE TESTE AUTOMATE PENTRU PROCEDURILE DIN LAB 1\n";
    cout << "============================================================\n";

    string text_test =
        "Ana are mere rosii si dulci. "
        "Ionel are mere verzi si pere galbene! "
        "Bunica vrea mere proaspete. "
        "Merele sunt foarte bune? "
        "Da, merele sunt delicioase.";

    // Test A: Frecventa
    cout << "\n[TEST A] Frecventa cuvinte si expresii...\n";
    RezultatFrecventa rf_c = gaseste_cel_mai_frecvent(text_test, "cuvant");
    cout << "  -> Cel mai frecvent cuvant: '" << rf_c.element << "' cu " << rf_c.aparitii << " aparitii.\n";
    assert(rf_c.element == "mere");
    assert(rf_c.aparitii == 3);

    RezultatFrecventa rf_e = gaseste_cel_mai_frecvent(text_test, "expresie", 2);
    cout << "  -> Cea mai frecventa expresie (2 cuv): '" << rf_e.element << "' cu " << rf_e.aparitii << " aparitii.\n";
    assert(rf_e.aparitii >= 1);

    int nr_sp = calculeaza_frecventa_expresie_specifica(text_test, "are mere");
    cout << "  -> Frecventa expresiei 'are mere': " << nr_sp << "\n";
    assert(nr_sp == 2);
    cout << "  [OK] Testul A a trecut cu succes.\n";

    // Test B: Hash
    cout << "\n[TEST B] Hash-ul textului...\n";
    uint64_t h_djb2 = hash_text("test", "djb2");
    uint64_t h_ascii = hash_text("ABC", "ascii_sum");
    cout << "  -> Hash DJB2 ('test'): " << h_djb2 << " (0x" << hex << h_djb2 << dec << ")\n";
    cout << "  -> Hash ASCII sum ('ABC'): " << h_ascii << " (A=65 + B=66 + C=67 = 198)\n";
    assert(h_ascii == 65 + 66 + 67);
    assert(h_djb2 > 0);
    cout << "  [OK] Testul B a trecut cu succes.\n";

    // Test C: Codificare si Decodificare
    cout << "\n[TEST C] Codificare si decodificare...\n";
    string original = "Programare Procedurala 2026!\nLinia a doua.";
    string codificat, decodificat;
    bool ok_cod = verifica_codificare_decodificare(original, 5, codificat, decodificat);
    cout << "  -> Original   : Programare Procedurala 2026! (2 linii)\n";
    cout << "  -> Decodificat coincide cu originalul: " << (ok_cod ? "DA" : "NU") << "\n";
    assert(ok_cod);
    cout << "  [OK] Testul C a trecut cu succes.\n";

    // Test D: Analiza propozitiilor
    cout << "\n[TEST D] Analiza propozitiilor...\n";
    AnalizaPropozitii ap = analizeaza_propozitii(text_test);
    cout << "  -> Propozitii identificate: " << ap.numar_propozitii << "\n";
    cout << "  -> Lungime medie: " << ap.lungime_medie_cuvinte << " cuvinte/propozitie\n";
    cout << "  -> Propozitia cea mai lunga: '" << ap.propozitie_maxima << "' (" << ap.cuvinte_propozitie_maxima << " cuvinte)\n";
    assert(ap.numar_propozitii == 5);
    assert(ap.cuvinte_propozitie_maxima > 0);
    cout << "  [OK] Testul D a trecut cu succes.\n";

    // Test E: Statistica textuala
    cout << "\n[TEST E] Statistica textuala...\n";
    StatisticiText st = calculeaza_statistici_text(text_test);
    cout << "  -> Total cuvinte       : " << st.numar_cuvinte << "\n";
    cout << "  -> Caractere cu spatii : " << st.numar_caractere_cu_spatii << "\n";
    cout << "  -> Caractere fara spatii: " << st.numar_caractere_fara_spatii << "\n";
    cout << "  -> Linii               : " << st.numar_linii << "\n";
    cout << "  -> Cuvinte unice       : " << st.numar_cuvinte_unice << "\n";
    assert(st.numar_cuvinte > 0 && st.numar_caractere_cu_spatii > st.numar_caractere_fara_spatii);
    assert(st.numar_linii >= 1 && st.numar_cuvinte_unice <= st.numar_cuvinte);
    cout << "  [OK] Testul E a trecut cu succes.\n";

    // Test F: Sortare dupa lungime
    cout << "\n[TEST F] Sortarea liniilor si propozitiilor dupa lungime...\n";
    string text_linii = "O linie scurta\nAceasta este o linie mult mai lunga decat prima\nMedie";
    vector<string> sort_cresc = sorteaza_linii_dupa_lungime(text_linii, false);
    vector<string> sort_desc = sorteaza_linii_dupa_lungime(text_linii, true);
    assert(sort_cresc.front().size() <= sort_cresc.back().size());
    assert(sort_desc.front().size() >= sort_desc.back().size());

    vector<string> prop_sort = sorteaza_propozitii_dupa_lungime(text_test, false);
    assert(prop_sort.front().size() <= prop_sort.back().size());
    cout << "  [OK] Testul F a trecut cu succes.\n";

    cout << "\n============================================================\n";
    cout << "TOATE CELE 6 TESTE PROCEDURALE AU FOST TRECUTE CU SUCCES!\n";
    cout << "============================================================\n";
}

// ==============================================================================
// INTERFATA PROCEDURALA IN CONSOLA (MENIU REDACTOR DE TEXT)
// ==============================================================================

void afiseaza_meniu() {
    string nume_fisier = CALE_FISIER_CURENT.empty() ? "Text Demo / Nesalvat" : CALE_FISIER_CURENT;
    cout << "\n============================================================\n";
    cout << "     REDACTOR DE TEXT PROCEDURAL (LAB 1 - C++)              \n";
    cout << "  Fisier: " << nume_fisier << (TEXT_MODIFICAT ? " *" : "") << "\n";
    cout << "============================================================\n";
    cout << " 1. Afiseaza textul curent\n";
    cout << " 2. Introdu / editeaza text manual\n";
    cout << " 3. Deschide fisier text\n";
    cout << " 4. Salveaza text in fisier\n";
    cout << " 5. [a] Cel mai frecvent cuvant / expresie\n";
    cout << " 6. [b] Hash-ul textului (DJB2 & Suma ASCII)\n";
    cout << " 7. [c] Codificare / Decodificare text (Cezar ASCII)\n";
    cout << " 8. [d] Analiza propozitiilor\n";
    cout << " 9. [e] Statistica textuala completa\n";
    cout << "10. [f] Sortarea liniilor / propozitiilor dupa lungime\n";
    cout << "11. Ruleaza toate analizele (a - f) pe textul curent\n";
    cout << " 0. Iesire\n";
    cout << "------------------------------------------------------------\n";
    cout << "Alege optiunea: ";
}

void ruleaza_toate_analizele() {
    cout << "\n--- [A] CEL MAI FRECVENT CUVANT / EXPRESIE ---\n";
    RezultatFrecventa rc = gaseste_cel_mai_frecvent(TEXT_CURENT, "cuvant");
    RezultatFrecventa re = gaseste_cel_mai_frecvent(TEXT_CURENT, "expresie", 2);
    cout << "Cel mai frecvent cuvant  : '" << rc.element << "' (" << rc.aparitii << " aparitii)\n";
    cout << "Cea mai frecventa expresie: '" << re.element << "' (" << re.aparitii << " aparitii)\n";

    cout << "\n--- [B] HASH-UL TEXTULUI ---\n";
    cout << "Hash DJB2      : " << hash_text(TEXT_CURENT, "djb2") << "\n";
    cout << "Hash ASCII sum : " << hash_text(TEXT_CURENT, "ascii_sum") << "\n";

    cout << "\n--- [C] CODIFICARE SI DECODIFICARE ---\n";
    string cod, decod;
    bool ok = verifica_codificare_decodificare(TEXT_CURENT, ULTIMA_DEPLASARE, cod, decod);
    cout << "Deplasare: " << ULTIMA_DEPLASARE << " | Verificare decodificare exacta: " << (ok ? "CORECT" : "GRESIT") << "\n";

    cout << "\n--- [D] ANALIZA PROPOZITIILOR ---\n";
    AnalizaPropozitii ap = analizeaza_propozitii(TEXT_CURENT);
    cout << "Numar propozitii     : " << ap.numar_propozitii << "\n";
    cout << "Lungime medie cuvinte: " << ap.lungime_medie_cuvinte << "\n";
    cout << "Propozitia cea mai lunga: \"" << ap.propozitie_maxima << "\" (" << ap.cuvinte_propozitie_maxima << " cuvinte)\n";

    cout << "\n--- [E] STATISTICA TEXTUALA ---\n";
    StatisticiText st = calculeaza_statistici_text(TEXT_CURENT);
    cout << "Cuvinte totale        : " << st.numar_cuvinte << "\n";
    cout << "Cuvinte unice         : " << st.numar_cuvinte_unice << "\n";
    cout << "Caractere (cu spatii) : " << st.numar_caractere_cu_spatii << "\n";
    cout << "Caractere (fara spatii): " << st.numar_caractere_fara_spatii << "\n";
    cout << "Linii                 : " << st.numar_linii << "\n";

    cout << "\n--- [F] SORTARE LINII DUPA LUNGIME (CRESCATOR) ---\n";
    vector<string> sortate = sorteaza_linii_dupa_lungime(TEXT_CURENT, false, true);
    for (size_t i = 0; i < sortate.size(); i++) {
        cout << "  [" << setw(2) << sortate[i].size() << " car] " << sortate[i] << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1 && (string(argv[1]) == "--test" || string(argv[1]) == "-t" || string(argv[1]) == "test")) {
        ruleaza_teste_automate();
        return 0;
    }

    TEXT_CURENT = TEXT_EXEMPLU_DEMO;

    while (true) {
        afiseaza_meniu();
        int opt;
        if (!(cin >> opt)) break;
        cin.ignore();

        if (opt == 0) {
            cout << "La revedere!\n";
            break;
        } else if (opt == 1) {
            cout << "\n--- TEXT CURENT ---\n" << TEXT_CURENT << "\n-------------------\n";
        } else if (opt == 2) {
            editeaza_text_consola();
        } else if (opt == 3) {
            cout << "Calea fisierului de deschis: ";
            string cale;
            getline(cin, cale);
            deschide_fisier(cale);
        } else if (opt == 4) {
            cout << "Calea fisierului de salvat: ";
            string cale;
            getline(cin, cale);
            salveaza_fisier(cale);
        } else if (opt == 5) {
            RezultatFrecventa rc = gaseste_cel_mai_frecvent(TEXT_CURENT, "cuvant");
            RezultatFrecventa re = gaseste_cel_mai_frecvent(TEXT_CURENT, "expresie", 2);
            cout << "Cel mai frecvent cuvant   : '" << rc.element << "' (" << rc.aparitii << " aparitii)\n";
            cout << "Cea mai frecventa expresie: '" << re.element << "' (" << re.aparitii << " aparitii)\n";
        } else if (opt == 6) {
            cout << "Hash DJB2     : " << hash_text(TEXT_CURENT, "djb2") << "\n";
            cout << "Hash ASCII sum: " << hash_text(TEXT_CURENT, "ascii_sum") << "\n";
        } else if (opt == 7) {
            cout << "Deplasare ASCII [implicit 3]: ";
            string dep_str;
            getline(cin, dep_str);
            int dep = dep_str.empty() ? 3 : stoi(dep_str);
            ULTIMA_DEPLASARE = dep;
            string cod, decod;
            bool ok = verifica_codificare_decodificare(TEXT_CURENT, dep, cod, decod);
            cout << "Text codificat:\n" << cod << "\n";
            cout << "Decodificat corect inapoi: " << (ok ? "DA" : "NU") << "\n";
        } else if (opt == 8) {
            AnalizaPropozitii ap = analizeaza_propozitii(TEXT_CURENT);
            cout << "Total propozitii     : " << ap.numar_propozitii << "\n";
            cout << "Lungime medie cuvinte: " << ap.lungime_medie_cuvinte << "\n";
            cout << "Cea mai lunga        : \"" << ap.propozitie_maxima << "\" (" << ap.cuvinte_propozitie_maxima << " cuvinte)\n";
        } else if (opt == 9) {
            StatisticiText st = calculeaza_statistici_text(TEXT_CURENT);
            cout << "Cuvinte       : " << st.numar_cuvinte << "\n";
            cout << "Cuvinte unice : " << st.numar_cuvinte_unice << "\n";
            cout << "Caractere (+) : " << st.numar_caractere_cu_spatii << "\n";
            cout << "Caractere (-) : " << st.numar_caractere_fara_spatii << "\n";
            cout << "Linii         : " << st.numar_linii << "\n";
        } else if (opt == 10) {
            vector<string> sortate = sorteaza_linii_dupa_lungime(TEXT_CURENT, false, true);
            for (size_t i = 0; i < sortate.size(); i++) {
                cout << "  [" << setw(2) << sortate[i].size() << " car] " << sortate[i] << "\n";
            }
        } else if (opt == 11) {
            ruleaza_toate_analizele();
        } else {
            cout << "Optiune invalida!\n";
        }
    }
    return 0;
}
