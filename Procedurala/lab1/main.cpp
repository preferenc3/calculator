#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cctype>
#include <cassert>

using namespace std;

// ==============================================================================
// LABORATORUL 1 - PARADIGMA PROCEDURALA (C++)
// Redactor de text simplu cu proceduri de analiza textuala
// ==============================================================================
// Principii procedurale:
// - Fara clase (doar functii/proceduri independente).
// - Fara recursivitate inutila (doar bucle simple 'for' si 'while').
// - Starea aplicatiei este pastrata in variabile globale simple.
// ==============================================================================

// Variabile globale de stare (redactorul de text)
string TEXT_CURENT = "";
string CALE_FISIER_CURENT = "";
bool TEXT_MODIFICAT = false;
int ULTIMA_DEPLASARE = 3;

const string TEXT_DEMO =
    "Paradigma procedurala este un model fundamental de programare.\n"
    "Programarea procedurala se bazeaza pe apeluri de functii si proceduri independente!\n"
    "Fiecare procedura rezolva o problema specifica.\n"
    "Oare este eficienta paradigma procedurala in prelucrarea textelor?\n"
    "Da, paradigma procedurala este simpla, eleganta si foarte usor de urmarit.\n"
    "In laboratorul 1 analizam frecventa cuvintelor si calculam statistici diverse.\n"
    "Succes in explorarea conceptelor procedurale!";

// ==============================================================================
// FUNCTII AJUTATOARE SIMPLE
// ==============================================================================

// Elimina spatiile si liniile noi de la inceputul si sfarsitul unui sir
string curata_spatii(string s) {
    int st = 0;
    while (st < (int)s.size() && isspace((unsigned char)s[st])) st++;
    int dr = (int)s.size() - 1;
    while (dr >= st && isspace((unsigned char)s[dr])) dr--;
    if (st > dr) return "";
    return s.substr(st, dr - st + 1);
}

// Transforma toate literele mari in litere mici
string litere_mici(string s) {
    for (int i = 0; i < (int)s.size(); i++) {
        s[i] = (char)tolower((unsigned char)s[i]);
    }
    return s;
}

// Extrage toate cuvintele din text (separa dupa spatii si semne de punctuatie)
vector<string> extrage_cuvinte(string text) {
    vector<string> cuvinte;
    string mic = litere_mici(text);
    string curent = "";

    for (int i = 0; i < (int)mic.size(); i++) {
        unsigned char c = (unsigned char)mic[i];
        if (isalnum(c) || c == '_' || c >= 128) {
            curent += mic[i];
        } else {
            if (!curent.empty()) {
                cuvinte.push_back(curent);
                curent = "";
            }
        }
    }
    if (!curent.empty()) {
        cuvinte.push_back(curent);
    }
    return cuvinte;
}

// ==============================================================================
// A. CAUTAREA CELUI MAI FRECVENT CUVANT SAU EXPRESIE (2 puncte)
// ==============================================================================

// Calculeaza frecventa cuvintelor sau a expresiilor (de 'lungime_expresie' cuvinte)
// si returneaza elementul maxim (iar prin referinta 'max_aparitii' numarul de aparitii).
string gaseste_cel_mai_frecvent(string text, string tip, int lungime_expresie, int& max_aparitii) {
    vector<string> cuvinte = extrage_cuvinte(text);
    map<string, int> frecv;

    if (tip == "cuvant") {
        // Numaram fiecare cuvant
        for (int i = 0; i < (int)cuvinte.size(); i++) {
            frecv[cuvinte[i]]++;
        }
    } else if (tip == "expresie") {
        // Formam grupuri de 'lungime_expresie' cuvinte consecutive
        for (int i = 0; i <= (int)cuvinte.size() - lungime_expresie; i++) {
            string exp = cuvinte[i];
            for (int j = 1; j < lungime_expresie; j++) {
                exp += " " + cuvinte[i + j];
            }
            frecv[exp]++;
        }
    }

    // Cautam elementul cu cele mai multe aparitii
    string elem_max = "";
    max_aparitii = 0;
    for (auto pereche : frecv) {
        if (pereche.second > max_aparitii) {
            elem_max = pereche.first;
            max_aparitii = pereche.second;
        }
    }
    return elem_max;
}

// Numara de cate ori apare o expresie exacta in text (fara diferenta intre litere mari/mici)
int numara_expresie_specifica(string text, string expresie) {
    string t = litere_mici(text);
    string e = litere_mici(curata_spatii(expresie));
    if (e.empty()) return 0;

    int contor = 0;
    size_t pos = t.find(e, 0);
    while (pos != string::npos) {
        contor++;
        pos = t.find(e, pos + e.size());
    }
    return contor;
}

// ==============================================================================
// B. HASH-UL TEXTULUI (1 punct)
// ==============================================================================

// Metoda 1: Suma simpla a codurilor ASCII ale tuturor caracterelor
unsigned long long hash_ascii_sum(string text) {
    unsigned long long suma = 0;
    for (int i = 0; i < (int)text.size(); i++) {
        suma += (unsigned char)text[i];
    }
    return suma;
}

// Metoda 2: Algoritmul clasic DJB2: hash = hash * 33 + caracter
unsigned long long hash_djb2(string text) {
    unsigned long long h = 5381;
    for (int i = 0; i < (int)text.size(); i++) {
        h = (h * 33) + (unsigned char)text[i];
    }
    return h;
}

// Procedura ceruta: returneaza hash-ul in functie de algoritmul ales
unsigned long long hash_text(string text, string algoritm = "djb2") {
    if (algoritm == "ascii_sum") return hash_ascii_sum(text);
    return hash_djb2(text);
}

// ==============================================================================
// C. CODIFICAREA SI DECODIFICAREA TEXTULUI (2 puncte)
// ==============================================================================

// Aplica cifrul Cezar: deplaseaza fiecare caracter cu 'deplasare' in codul ASCII
// (pastreaza caracterul '\n' intact pentru a nu strica liniile)
string codifica_text(string text, int deplasare = 3) {
    string rez = "";
    for (int i = 0; i < (int)text.size(); i++) {
        unsigned char c = (unsigned char)text[i];
        if (c == '\n') {
            rez += '\n';
        } else {
            int cod = (c + deplasare) % 256;
            if (cod < 0) cod += 256;
            rez += (char)cod;
        }
    }
    return rez;
}

// Operatia inversa codificarii: scade 'deplasare' din codul ASCII
string decodifica_text(string text, int deplasare = 3) {
    return codifica_text(text, -deplasare);
}

// ==============================================================================
// E. STATISTICA TEXTUALA (2 puncte) - Fiecare calcul prin procedura separata
// ==============================================================================

// 1. Numarul total de cuvinte
int numara_cuvinte(string text) {
    return (int)extrage_cuvinte(text).size();
}

// 2. Numarul de caractere (cu sau fara spatii)
int numara_caractere(string text, bool cu_spatii = true) {
    if (cu_spatii) return (int)text.size();
    int nr = 0;
    for (int i = 0; i < (int)text.size(); i++) {
        if (!isspace((unsigned char)text[i])) nr++;
    }
    return nr;
}

// 3. Numarul de linii din text
int numara_linii(string text) {
    if (text.empty()) return 0;
    int linii = 1;
    for (int i = 0; i < (int)text.size(); i++) {
        if (text[i] == '\n' && i + 1 < (int)text.size()) {
            linii++;
        }
    }
    return linii;
}

// 4. Numarul de cuvinte unice (distincte)
int numara_cuvinte_unice(string text) {
    vector<string> cuvinte = extrage_cuvinte(text);
    set<string> unice;
    for (int i = 0; i < (int)cuvinte.size(); i++) {
        unice.insert(cuvinte[i]);
    }
    return (int)unice.size();
}

// ==============================================================================
// D. ANALIZA PROPOZITIILOR (2 puncte) - Fiecare operatie ca procedura separata
// ==============================================================================

// 1. Imparte textul in propozitii (dupa '.', '!', '?' sau linie noua)
vector<string> imparte_in_propozitii(string text) {
    vector<string> propozitii;
    string cur = "";

    for (int i = 0; i < (int)text.size(); i++) {
        char c = text[i];
        if (c == '\n') {
            string p = curata_spatii(cur);
            if (!p.empty()) propozitii.push_back(p);
            cur = "";
        } else if (c == '.' || c == '!' || c == '?') {
            cur += c;
            if (i + 1 >= (int)text.size() || isspace((unsigned char)text[i + 1])) {
                string p = curata_spatii(cur);
                if (!p.empty()) propozitii.push_back(p);
                cur = "";
            }
        } else {
            cur += c;
        }
    }
    string ramas = curata_spatii(cur);
    if (!ramas.empty()) propozitii.push_back(ramas);
    return propozitii;
}

// 2. Calculeaza lungimea medie a propozitiilor (in cuvinte)
double calculeaza_lungime_medie_propozitii(vector<string> propozitii) {
    if (propozitii.empty()) return 0.0;
    int total_cuvinte = 0;
    for (int i = 0; i < (int)propozitii.size(); i++) {
        total_cuvinte += numara_cuvinte(propozitii[i]);
    }
    return (double)total_cuvinte / (double)propozitii.size();
}

// 3. Gaseste si returneaza propozitia cea mai lunga (dupa numarul de cuvinte)
string gaseste_propozitia_cea_mai_lunga(vector<string> propozitii, int& max_cuvinte) {
    string prop_max = "";
    max_cuvinte = 0;

    for (int i = 0; i < (int)propozitii.size(); i++) {
        int nr_cuv = numara_cuvinte(propozitii[i]);
        if (nr_cuv > max_cuvinte || (nr_cuv == max_cuvinte && propozitii[i].size() > prop_max.size())) {
            max_cuvinte = nr_cuv;
            prop_max = propozitii[i];
        }
    }
    return prop_max;
}

// ==============================================================================
// F. SORTAREA LINIILOR DUPA LUNGIME (1 punct)
// ==============================================================================

// Imparte textul in linii individuale (pentru sortare)
vector<string> imparte_in_linii(string text) {
    vector<string> linii;
    istringstream iss(text);
    string linie;
    while (getline(iss, linie)) {
        if (!linie.empty() && linie.back() == '\r') linie.pop_back();
        if (!curata_spatii(linie).empty()) {
            linii.push_back(linie);
        }
    }
    return linii;
}

// Sorteaza liniile crescator sau descrescator folosind metoda bulelor (Bubble Sort)
vector<string> sorteaza_linii_dupa_lungime(string text, bool descrescator = false) {
    vector<string> linii = imparte_in_linii(text);
    int n = (int)linii.size();

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            bool trebuie_schimbat = descrescator
                ? (linii[j].size() < linii[j + 1].size())
                : (linii[j].size() > linii[j + 1].size());

            if (trebuie_schimbat) {
                string temp = linii[j];
                linii[j] = linii[j + 1];
                linii[j + 1] = temp;
            }
        }
    }
    return linii;
}

// ==============================================================================
// G. LUCRUL CU FISIERE SI EDITARE IN CONSOLA
// ==============================================================================

void deschide_fisier(string cale) {
    ifstream f(cale);
    if (!f.is_open()) {
        cout << "[EROARE] Nu s-a putut deschide fisierul: " << cale << "\n";
        return;
    }
    stringstream ss;
    ss << f.rdbuf();
    TEXT_CURENT = ss.str();
    CALE_FISIER_CURENT = cale;
    TEXT_MODIFICAT = false;
    cout << "[OK] Fisier incarcat cu succes!\n";
}

void salveaza_fisier(string cale) {
    ofstream f(cale);
    if (!f.is_open()) {
        cout << "[EROARE] Nu s-a putut salva in fisierul: " << cale << "\n";
        return;
    }
    f << TEXT_CURENT;
    CALE_FISIER_CURENT = cale;
    TEXT_MODIFICAT = false;
    cout << "[OK] Text salvat cu succes in: " << cale << "\n";
}

void editeaza_text() {
    cout << "Introdu textul nou (scrie 'END' pe o linie noua cand termini):\n";
    string nou = "";
    string linie;
    while (getline(cin, linie)) {
        if (linie == "END" || linie == "end") break;
        if (!nou.empty()) nou += "\n";
        nou += linie;
    }
    TEXT_CURENT = nou;
    TEXT_MODIFICAT = true;
    cout << "[OK] Text actualizat!\n";
}

// ==============================================================================
// H. TESTE AUTOMATE (--test)
// ==============================================================================

void ruleaza_teste_automate() {
    cout << "====================================================\n";
    cout << " TESTE AUTOMATE - LAB 1 PARADIGMA PROCEDURALA (C++)\n";
    cout << "====================================================\n";

    string text_test =
        "Ana are mere rosii si dulci. "
        "Ionel are mere verzi si pere galbene! "
        "Bunica vrea mere proaspete. "
        "Merele sunt foarte bune? "
        "Da, merele sunt delicioase.";

    // Test A: Frecventa
    int nr_c = 0, nr_e = 0;
    string cuv = gaseste_cel_mai_frecvent(text_test, "cuvant", 1, nr_c);
    string exp = gaseste_cel_mai_frecvent(text_test, "expresie", 2, nr_e);
    assert(cuv == "mere" && nr_c == 3);
    assert(nr_e >= 1);
    assert(numara_expresie_specifica(text_test, "are mere") == 2);
    cout << "  [OK] A. Cel mai frecvent: '" << cuv << "' (" << nr_c << "x), expresie: '" << exp << "'\n";

    // Test B: Hash
    assert(hash_text("ABC", "ascii_sum") == 198);
    assert(hash_text("test", "djb2") > 0);
    cout << "  [OK] B. Hash ASCII('ABC') = 198, DJB2('test') = " << hash_text("test", "djb2") << "\n";

    // Test C: Codificare & Decodificare
    string orig = "Programare Procedurala 2026!\nLinia 2.";
    string cod = codifica_text(orig, 5);
    string decod = decodifica_text(cod, 5);
    assert(decod == orig);
    cout << "  [OK] C. Codificare si decodificare corecta.\n";

    // Test D: Analiza propozitii
    vector<string> props = imparte_in_propozitii(text_test);
    int max_cuv = 0;
    string pmax = gaseste_propozitia_cea_mai_lunga(props, max_cuv);
    double medie = calculeaza_lungime_medie_propozitii(props);
    assert(props.size() == 5 && max_cuv == 7 && medie == 5.0);
    cout << "  [OK] D. Propozitii: " << props.size() << ", medie: " << medie << ", max: " << max_cuv << " cuvinte\n";

    // Test E: Statistici
    assert(numara_cuvinte(text_test) == 25);
    assert(numara_caractere(text_test, true) == 147);
    assert(numara_caractere(text_test, false) == 123);
    assert(numara_linii(text_test) == 1);
    assert(numara_cuvinte_unice(text_test) == 19);
    cout << "  [OK] E. Statistici: 25 cuvinte (19 unice), 147 caractere.\n";

    // Test F: Sortare linii
    string lin = "Scurta\nAceasta este cea mai lunga linie\nMedie";
    vector<string> s_cresc = sorteaza_linii_dupa_lungime(lin, false);
    assert(s_cresc[0] == "Medie" && s_cresc[2] == "Aceasta este cea mai lunga linie");
    cout << "  [OK] F. Sortarea liniilor dupa lungime functioneaza.\n";

    cout << "\nTOATE TESTELE AU TRECUT 100%!\n";
}

// ==============================================================================
// MENIU PRINCIPAL
// ==============================================================================

void ruleaza_toate_analizele() {
    int nr_c = 0, nr_e = 0;
    string cuv = gaseste_cel_mai_frecvent(TEXT_CURENT, "cuvant", 1, nr_c);
    string exp = gaseste_cel_mai_frecvent(TEXT_CURENT, "expresie", 2, nr_e);
    cout << "\n[A] Cel mai frecvent cuvant  : '" << cuv << "' (" << nr_c << " aparitii)\n";
    cout << "    Cea mai frecventa expresie: '" << exp << "' (" << nr_e << " aparitii)\n";

    cout << "\n[B] Hash DJB2: " << hash_text(TEXT_CURENT, "djb2")
         << " | Hash ASCII: " << hash_text(TEXT_CURENT, "ascii_sum") << "\n";

    string cod = codifica_text(TEXT_CURENT, ULTIMA_DEPLASARE);
    string decod = decodifica_text(cod, ULTIMA_DEPLASARE);
    cout << "\n[C] Codificare/Decodificare (pas " << ULTIMA_DEPLASARE << "): "
         << (decod == TEXT_CURENT ? "CORECT" : "GRESIT") << "\n";

    vector<string> props = imparte_in_propozitii(TEXT_CURENT);
    int max_cuv = 0;
    string pmax = gaseste_propozitia_cea_mai_lunga(props, max_cuv);
    cout << "\n[D] Propozitii: " << props.size()
         << " | Lungime medie: " << calculeaza_lungime_medie_propozitii(props) << " cuvinte\n";
    cout << "    Cea mai lunga: \"" << pmax << "\" (" << max_cuv << " cuvinte)\n";

    cout << "\n[E] Statistici: " << numara_cuvinte(TEXT_CURENT) << " cuvinte ("
         << numara_cuvinte_unice(TEXT_CURENT) << " unice), "
         << numara_caractere(TEXT_CURENT, true) << " caractere ("
         << numara_caractere(TEXT_CURENT, false) << " fara spatii), "
         << numara_linii(TEXT_CURENT) << " linii\n";

    cout << "\n[F] Linii sortate crescator dupa lungime:\n";
    vector<string> sortate = sorteaza_linii_dupa_lungime(TEXT_CURENT, false);
    for (int i = 0; i < (int)sortate.size(); i++) {
        cout << "    [" << sortate[i].size() << " car] " << sortate[i] << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1 && (string(argv[1]) == "--test" || string(argv[1]) == "-t")) {
        ruleaza_teste_automate();
        return 0;
    }

    TEXT_CURENT = TEXT_DEMO;

    while (true) {
        cout << "\n================ REDACTOR PROCEDURAL (LAB 1) ================\n";
        cout << "Fisier: " << (CALE_FISIER_CURENT.empty() ? "Text Demo" : CALE_FISIER_CURENT)
             << (TEXT_MODIFICAT ? " *" : "") << "\n";
        cout << " 1. Afiseaza textul\n";
        cout << " 2. Editeaza textul de la tastatura\n";
        cout << " 3. Deschide fisier | 4. Salveaza fisier\n";
        cout << " 5. [a] Frecventa cuvant / expresie\n";
        cout << " 6. [b] Hash text (DJB2 & ASCII)\n";
        cout << " 7. [c] Codificare / Decodificare\n";
        cout << " 8. [d] Analiza propozitiilor\n";
        cout << " 9. [e] Statistica textuala\n";
        cout << "10. [f] Sortare linii dupa lungime\n";
        cout << "11. Ruleaza toate analizele (a - f)\n";
        cout << " 0. Iesire\n";
        cout << "Optiunea: ";

        int opt;
        if (!(cin >> opt)) break;
        cin.ignore();

        if (opt == 0) break;
        else if (opt == 1) cout << "\n" << TEXT_CURENT << "\n";
        else if (opt == 2) editeaza_text();
        else if (opt == 3) {
            cout << "Calea fisierului: ";
            string c; getline(cin, c);
            deschide_fisier(c);
        } else if (opt == 4) {
            cout << "Calea fisierului: ";
            string c; getline(cin, c);
            salveaza_fisier(c);
        } else if (opt == 5) {
            int nc = 0, ne = 0;
            string cuv = gaseste_cel_mai_frecvent(TEXT_CURENT, "cuvant", 1, nc);
            string exp = gaseste_cel_mai_frecvent(TEXT_CURENT, "expresie", 2, ne);
            cout << "Cuvant maxim  : '" << cuv << "' (" << nc << ")\n";
            cout << "Expresie maxima: '" << exp << "' (" << ne << ")\n";
        } else if (opt == 6) {
            cout << "DJB2: " << hash_text(TEXT_CURENT, "djb2")
                 << " | ASCII: " << hash_text(TEXT_CURENT, "ascii_sum") << "\n";
        } else if (opt == 7) {
            cout << "Deplasare [3]: ";
            string d; getline(cin, d);
            int pas = d.empty() ? 3 : stoi(d);
            string cod = codifica_text(TEXT_CURENT, pas);
            cout << "Codificat:\n" << cod << "\nDecodificat identic: "
                 << (decodifica_text(cod, pas) == TEXT_CURENT ? "DA" : "NU") << "\n";
        } else if (opt == 8) {
            vector<string> props = imparte_in_propozitii(TEXT_CURENT);
            int max_c = 0;
            string pmax = gaseste_propozitia_cea_mai_lunga(props, max_c);
            cout << "Propozitii: " << props.size() << " | Medie: "
                 << calculeaza_lungime_medie_propozitii(props) << "\nCea mai lunga: " << pmax << "\n";
        } else if (opt == 9) {
            cout << "Cuvinte: " << numara_cuvinte(TEXT_CURENT)
                 << " | Unice: " << numara_cuvinte_unice(TEXT_CURENT)
                 << " | Caractere: " << numara_caractere(TEXT_CURENT, true)
                 << " (" << numara_caractere(TEXT_CURENT, false) << " fara spatii)"
                 << " | Linii: " << numara_linii(TEXT_CURENT) << "\n";
        } else if (opt == 10) {
            vector<string> sortate = sorteaza_linii_dupa_lungime(TEXT_CURENT, false);
            for (int i = 0; i < (int)sortate.size(); i++) {
                cout << "  [" << sortate[i].size() << "] " << sortate[i] << "\n";
            }
        } else if (opt == 11) {
            ruleaza_toate_analizele();
        }
    }
    return 0;
}
