#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

// ====================================================================
// PARADIGMA FUNCȚIONALĂ - LABORATORUL 3
// PARSER XML ȘI PRELUCRAREA ARBORELUI DE ETICHETE
// --------------------------------------------------------------------
// PRINCIPII RESPECTATE:
// 1. Fără bucle repetitive ('for', 'while', 'do-while') în algoritmi.
// 2. Fără mutații in-place (fără x += 1, fără x++, doar y = x + 1).
// 3. Pur recursiv: descompunere în subprobleme (primul element + restul).
// 4. Două moduri de rulare:
//    - Modul 1: Date predefinite din fișierul catalog.xml
//    - Modul 2: Date introduse manual de la tastatură
// ====================================================================

// --------------------------------------------------------------------
// STRUCTURA DE DATE: REPREZENTAREA UNUI NOD DIN ARBORELE XML
// --------------------------------------------------------------------
// În XML, orice document este un arbore ierarhic format din:
// - tag: numele etichetei (ex: "catalog", "book", "author")
// - attrs: atributele etichetei (ex: 'id="b1" format="hardcover"')
// - text: conținutul textual dintre etichete (ex: "Harold Abelson")
// - children: lista recursivă de sub-noduri (copii)
// --------------------------------------------------------------------
struct Node {
    string tag;             // Numele etichetei
    string attrs;           // Șirul de caractere cu atributele
    string text;            // Valoarea text din interior
    vector<Node> children;  // Sub-arborii (copiii nodului curent)

    // Funcție ajutătoare pură pentru extragerea unui atribut după nume.
    // De exemplu: dacă attrs este 'id="b1"', attr("id") returnează "b1".
    string attr(const string& name) const {
        auto pos = attrs.find(name + "=\"");
        if (pos == string::npos) return "";
        auto start = pos + name.size() + 2;
        auto end = attrs.find('"', start);
        return (end == string::npos) ? "" : attrs.substr(start, end - start);
    }
};

// ====================================================================
// PUNCTUL 1: ELABORAREA UNUI PARSER PENTRU DOCUMENTE XML (3 PUNCTE)
// ====================================================================

// Elimină spațiile albe inutile de la începutul și sfârșitul unui șir
string trim(const string& s) {
    auto start = s.find_first_not_of(" \t\n\r");
    if (start == string::npos) return "";
    auto end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

// Extrage doar numele etichetei din interiorul parantezelor < ... >
// Ex: din 'book id="b1"' extrage 'book'
string extrage_tag(const string& inside) {
    auto pos = inside.find_first_of(" \t\n\r/");
    if (pos == string::npos) return inside;
    return inside.substr(0, pos);
}

// Declarație anticipată pentru recursivitate mutuală
pair<Node, size_t> parse_element(const string& s, size_t i);

// Parsare recursivă a copiilor (nodurilor descendente) ale unei etichete
// Returnează o pereche: (vectorul_de_noduri_copil, indexul_urmator)
pair<vector<Node>, size_t> parse_children(const string& s, size_t i, const string& parent_tag) {
    size_t lt = s.find('<', i);
    // Cazul de bază 1: nu mai există etichete în șir
    if (lt == string::npos) return {{}, s.size()};

    // Cazul de bază 2: am întâlnit eticheta de închidere </parent_tag>
    if (lt + 1 < s.size() && s[lt + 1] == '/') {
        size_t gt = s.find('>', lt);
        return {{}, gt + 1}; // nodul curent s-a încheiat
    }

    // Pasul recursiv:
    // 1. Parsăm primul copil pornind de la poziția curentă
    auto child = parse_element(s, lt);
    // 2. Parsăm restul copiilor recursiv, transmițând noul index
    auto rest = parse_children(s, child.second, parent_tag);

    // 3. Combinăm primul copil cu restul copiilor (fără mutații in-place)
    vector<Node> all = {child.first};
    all.insert(all.end(), rest.first.begin(), rest.first.end());
    return {all, rest.second};
}

// Parsare recursivă a unui singur element XML
pair<Node, size_t> parse_element(const string& s, size_t i) {
    size_t lt = s.find('<', i);
    if (lt == string::npos) return {{"", "", "", {}}, s.size()};

    // Ignorăm comentariile <!-- ... --> și instrucțiunile XML <?xml ... ?>
    if (lt + 1 < s.size() && (s[lt + 1] == '?' || s[lt + 1] == '!')) {
        size_t gt = s.find('>', lt);
        return parse_element(s, gt + 1);
    }

    size_t gt = s.find('>', lt);
    string inside = s.substr(lt + 1, gt - lt - 1);
    string tag = extrage_tag(inside);

    // Cazul etichetelor auto-închise: <tag atribute />
    if (inside.back() == '/') {
        return {{tag, inside, "", {}}, gt + 1};
    }

    // Extragem conținutul text dintre eticheta curentă și următoarea
    size_t next_lt = s.find('<', gt + 1);
    string text = trim(s.substr(gt + 1, next_lt - gt - 1));

    // Cazul nodurilor frunză cu text simplu: <tag>text</tag>
    if (next_lt + 1 < s.size() && s[next_lt + 1] == '/') {
        size_t close_gt = s.find('>', next_lt);
        return {{tag, inside, text, {}}, close_gt + 1};
    }

    // Cazul nodurilor compuse care au sub-etichete (copii): parsam copiii recursiv
    auto ch = parse_children(s, gt + 1, tag);
    return {{tag, inside, text, ch.first}, ch.second};
}

// Funcția principală de parsare XML
Node parse_xml(const string& s) {
    return parse_element(s, 0).first;
}

// ====================================================================
// PUNCTUL 2: GENERAREA ARBORELUI DE ETICHETE (1 PUNCT)
// ====================================================================

void afiseaza_arbore_copii(const vector<Node>& kids, size_t i, int indent);

// Afișează ierarhia vizuală a arborelui cu indentare
void afiseaza_arbore(const Node& node, int indent = 0) {
    cout << string(indent, ' ') << "<" << node.tag;
    if (!node.attr("id").empty()) cout << " id=\"" << node.attr("id") << "\"";
    cout << ">";
    if (!node.text.empty()) cout << " -> " << node.text;
    cout << "\n";
    // Apel recursiv pentru afișarea copiilor
    afiseaza_arbore_copii(node.children, 0, indent + 2);
}

// Parcurgerea recursivă a listei de copii pentru afișare (fără buclă for)
void afiseaza_arbore_copii(const vector<Node>& kids, size_t i, int indent) {
    if (i >= kids.size()) return; // Condiția de oprire
    afiseaza_arbore(kids[i], indent);
    afiseaza_arbore_copii(kids, i + 1, indent); // y = i + 1
}

// ====================================================================
// PUNCTUL 3: CĂUTAREA UNEI ETICHETE SPECIFICE ÎN ARBORE (1 PUNCT)
// ====================================================================

vector<Node> cauta_in_copii(const vector<Node>& kids, size_t i, const string& tag);

// Căutare recursivă a tuturor nodurilor ce au eticheta 'tag'
vector<Node> cauta_eticheta(const Node& node, const string& tag) {
    // 1. Verificăm dacă nodul curent se potrivește
    vector<Node> rez = (node.tag == tag) ? vector<Node>{node} : vector<Node>{};

    // 2. Căutăm recursiv în toți copiii săi
    auto din_copii = cauta_in_copii(node.children, 0, tag);

    // 3. Unim rezultatele
    rez.insert(rez.end(), din_copii.begin(), din_copii.end());
    return rez;
}

// Parcurgerea recursivă a copiilor pentru căutare (fără buclă for)
vector<Node> cauta_in_copii(const vector<Node>& kids, size_t i, const string& tag) {
    if (i >= kids.size()) return {}; // Cazul de bază: am parcurs toți copiii
    auto prim = cauta_eticheta(kids[i], tag);
    auto rest = cauta_in_copii(kids, i + 1, tag); // y = i + 1
    prim.insert(prim.end(), rest.begin(), rest.end());
    return prim;
}

// ====================================================================
// PUNCTUL 4: GENERAREA UNEI LISTE PE BAZA REZULTATULUI CĂUTĂRII (1 PUNCT)
// ====================================================================

// Extrage recursiv conținutul text din fiecare nod găsit în urma căutării
vector<string> lista_texte(const vector<Node>& noduri, size_t i = 0) {
    if (i >= noduri.size()) return {}; // Condiție de oprire
    vector<string> rez = {noduri[i].text};
    auto rest = lista_texte(noduri, i + 1); // y = i + 1
    rez.insert(rez.end(), rest.begin(), rest.end());
    return rez;
}

// ====================================================================
// PUNCTUL 5: OBȚINEREA ADÂNCIMII ARBORELUI DE ETICHETE (1 PUNCT)
// ====================================================================

int adancime_arbore(const Node& node);

// Găsește recursiv adâncimea maximă dintre toți copiii unui nod
int max_adancime_copii(const vector<Node>& kids, size_t i) {
    if (i >= kids.size()) return 0;
    int cur = adancime_arbore(kids[i]);
    int rest = max_adancime_copii(kids, i + 1);
    return (cur > rest) ? cur : rest;
}

// Adâncimea arborelui:
// - Dacă nu are copii: adâncimea este 1 (nodul însuși)
// - Dacă are copii: 1 + maximul adâncimilor copiilor săi
int adancime_arbore(const Node& node) {
    if (node.children.empty()) return 1;
    return 1 + max_adancime_copii(node.children, 0);
}

// ====================================================================
// PUNCTUL 6: OBȚINEREA LĂȚIMII ARBORELUI DE ETICHETE (1 PUNCT)
// ====================================================================
// Lățimea canonică a unui arbore reprezintă numărul maxim de noduri
// aflate pe oricare dintre nivelurile sale.

int numar_noduri_nivel(const Node& node, int nivel);

// Parcurge recursiv nivelurile de la 1 la max_lvl și determină maximul
int max_noduri_pe_nivele(const Node& node, int cur_lvl, int max_lvl) {
    if (cur_lvl > max_lvl) return 0;
    int cur = numar_noduri_nivel(node, cur_lvl);
    int rest = max_noduri_pe_nivele(node, cur_lvl + 1, max_lvl); // y = cur_lvl + 1
    return (cur > rest) ? cur : rest;
}

int latime_arbore(const Node& node) {
    return max_noduri_pe_nivele(node, 1, adancime_arbore(node));
}

// ====================================================================
// PUNCTUL 7: NUMĂRUL DE NODURI DE PE UN ANUMIT NIVEL (2 PUNCTE)
// ====================================================================

int suma_noduri_copii(const vector<Node>& kids, size_t i, int nivel) {
    if (i >= kids.size()) return 0;
    return numar_noduri_nivel(kids[i], nivel - 1) + suma_noduri_copii(kids, i + 1, nivel);
}

// Calculează recursiv câte noduri există la un nivel dat:
// - Nivel < 1: 0 noduri
// - Nivel == 1: 1 nod (nodul de start însuși)
// - Nivel > 1: suma nodurilor de pe nivelul (nivel - 1) din fiecare copil
int numar_noduri_nivel(const Node& node, int nivel) {
    if (nivel < 1) return 0;
    if (nivel == 1) return 1;
    return suma_noduri_copii(node.children, 0, nivel);
}

// ====================================================================
// FUNCȚII RECURSIVE PENTRU AFIȘARE
// ====================================================================

void afiseaza_lista_string(const vector<string>& v, size_t i = 0) {
    if (i >= v.size()) {
        cout << "\n";
        return;
    }
    cout << "  • \"" << v[i] << "\"\n";
    afiseaza_lista_string(v, i + 1); // y = i + 1
}

void afiseaza_nivele(const Node& node, int cur_lvl, int max_lvl) {
    if (cur_lvl > max_lvl) return;
    cout << "  Nivelul " << cur_lvl << ": " << numar_noduri_nivel(node, cur_lvl) << " noduri\n";
    afiseaza_nivele(node, cur_lvl + 1, max_lvl); // y = cur_lvl + 1
}

// Funcție generică ce rulează toate cerințele 1-7 pe un arbore dat
void ruleaza_analiza_xml(const Node& root, const string& tag_cautat) {
    // Punctul 1 & 2: Arborele de etichete
    cout << "\n--- 1 & 2. ARBORELE DE ETICHETE GENERAT ---\n";
    afiseaza_arbore(root);

    // Punctul 3: Căutare etichetă
    cout << "\n--- 3. CĂUTARE ETICHETĂ <" << tag_cautat << "> ---\n";
    vector<Node> gasite = cauta_eticheta(root, tag_cautat);
    cout << "S-au găsit: " << gasite.size() << " etichete <" << tag_cautat << ">\n";

    // Punctul 4: Lista pe baza căutării
    cout << "\n--- 4. LISTA CONȚINUTURILOR TEXT PENTRU <" << tag_cautat << "> ---\n";
    vector<string> texte = lista_texte(gasite);
    afiseaza_lista_string(texte);

    // Punctul 5: Adâncimea arborelui
    cout << "--- 5. ADÂNCIMEA ARBORELUI ---\n";
    cout << "Adâncime maximă: " << adancime_arbore(root) << "\n\n";

    // Punctul 6: Lățimea arborelui
    cout << "--- 6. LĂȚIMEA ARBORELUI ---\n";
    cout << "Lățime (numărul maxim de noduri pe un nivel): " << latime_arbore(root) << "\n\n";

    // Punctul 7: Număr noduri pe fiecare nivel
    cout << "--- 7. NUMĂRUL DE NODURI PE FIECARE NIVEL ---\n";
    afiseaza_nivele(root, 1, adancime_arbore(root));
}

// Citire recursivă a mai multor linii de la tastatură până când utilizatorul tastează END
string citeste_linii_tastatura_rec() {
    string linie;
    if (!getline(cin, linie) || linie == "END" || linie == "end") {
        return "";
    }
    return linie + " " + citeste_linii_tastatura_rec();
}

// ====================================================================
// PROGRAM PRINCIPAL (MENIU DUBLU - AMBELE FUNCTIONALE)
// ====================================================================
int main(int argc, char* argv[]) {
    cout << "========================================================\n";
    cout << "     LABORATORUL 3: PARADIGMA FUNCȚIONALĂ - ARBORE XML  \n";
    cout << "========================================================\n";
    cout << "Alege modul de rulare:\n";
    cout << "  1. Mod automat (fișierul predefinit catalog.xml)\n";
    cout << "  2. Mod manual  (introducere XML complet de la tastatură)\n";
    cout << "Opțiunea (1 sau 2): ";

    int optiune = 1;
    if (!(cin >> optiune)) return 0;
    cin.ignore(); // Curățăm buffer-ul după citirea opțiunii

    if (optiune == 1) {
        // --- MODUL 1: PREDEFINIT DIN FIȘIER ---
        string path = (argc > 1) ? argv[1] : "catalog.xml";
        ifstream f(path);
        if (!f.is_open()) {
            f.open("Functionala/lab3/catalog.xml");
            if (!f.is_open()) {
                cout << "Eroare: Nu s-a putut deschide fisierul catalog.xml!\n";
                return 1;
            }
        }
        stringstream ss;
        ss << f.rdbuf();

        cout << "\n[Mod 1] Se procesează fișierul catalog.xml cu eticheta predefinită 'author'...\n";
        Node root = parse_xml(ss.str());
        ruleaza_analiza_xml(root, "author");

    } else if (optiune == 2) {
        // --- MODUL 2: MANUAL DE LA TASTATURĂ ---
        cout << "\n[Mod 2] Introdu textul XML (termină tastând 'END' pe o linie nouă):\n";
        string xml_input = citeste_linii_tastatura_rec();

        if (trim(xml_input).empty()) {
            cout << "Eroare: Nu s-a introdus niciun text XML valid!\n";
            return 1;
        }

        cout << "Introdu eticheta pe care dorești să o cauți: ";
        string tag_cautat;
        getline(cin, tag_cautat);
        if (trim(tag_cautat).empty()) {
            tag_cautat = "titlu";
        }

        Node root = parse_xml(xml_input);
        ruleaza_analiza_xml(root, tag_cautat);

    } else {
        cout << "Opțiune invalidă! Alege 1 sau 2.\n";
    }

    return 0;
}
