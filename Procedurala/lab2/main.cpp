#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <dlfcn.h>
#include <curl/curl.h>

using namespace std;

// ============================================================================
// LABORATORUL 2 - PARADIGMA PROCEDURALA (C++)
// Interactiunea cu o aplicatie externa prin API (libcurl incarcat automat)
// ============================================================================
// NOTA: Biblioteca sistem 'libcurl.dylib' este incarcata automat in cod prin
//       dlopen(), astfel incat programul ruleaza direct din orice IDE (VS Code,
//       Code Runner, CLion) FARA erori "Undefined symbols" si fara flag-ul -lcurl.
//
// CELE 3 CERERI HTTP PRINCIPALE (Cerinta d):
//   1. GET  /users/1                        -> Citirea datelor unui utilizator (Cerinta b)
//   2. POST /posts                          -> Transmiterea / crearea unei postari noi (Cerinta a)
//   3. GET  /todos?userId=1&completed=false -> Filtrarea sarcinilor nefinalizate (Cerinta d)
//
// CUM SE INCHIDE CONEXIUNEA EXTERNA (Cerinta e):
//   Prin apelul 'p_easy_cleanup(sesiune)' -> 'p_global_cleanup()' -> 'dlclose(CURL_LIB)',
//   care inchid socket-ul TCP/TLS catre server, in timp ce programul nostru
//   C++ continua sa ruleze offline pe fisierul local JSON.
// ============================================================================

const string API_URL = "https://jsonplaceholder.typicode.com";
const string FISIER_LOCAL = "date_api_salvate.json";

// ============================================================================
// INCARCAREA DINAMICA A BIBLIOTECII LIBCURL (ELIMINA EROAREA DE LINKER ARM64)
// ============================================================================

void* CURL_LIB = nullptr;
CURLcode (*p_global_init)(long) = nullptr;
void (*p_global_cleanup)(void) = nullptr;
CURL* (*p_easy_init)(void) = nullptr;
void (*p_easy_cleanup)(CURL*) = nullptr;
CURLcode (*p_easy_setopt)(CURL*, CURLoption, ...) = nullptr;
CURLcode (*p_easy_perform)(CURL*) = nullptr;
CURLcode (*p_easy_getinfo)(CURL*, CURLINFO, ...) = nullptr;
const char* (*p_easy_strerror)(CURLcode) = nullptr;
struct curl_slist* (*p_slist_append)(struct curl_slist*, const char*) = nullptr;
void (*p_slist_free_all)(struct curl_slist*) = nullptr;

// Helper pentru conversia sigura a simbolurilor dlsym fara avertismente -Wpedantic
template <typename FnPtr>
void incarca_simbol(void* lib, const char* nume, FnPtr& destinatie) {
    void* sym = dlsym(lib, nume);
    __extension__ destinatie = reinterpret_cast<FnPtr>(sym);
}

bool incarca_libcurl() {
    if (CURL_LIB != nullptr) return true;
    CURL_LIB = dlopen("libcurl.dylib", RTLD_LAZY);
    if (!CURL_LIB) CURL_LIB = dlopen("/usr/lib/libcurl.4.dylib", RTLD_LAZY);
    if (!CURL_LIB) CURL_LIB = dlopen("libcurl.so.4", RTLD_LAZY);
    if (!CURL_LIB) return false;

    incarca_simbol(CURL_LIB, "curl_global_init", p_global_init);
    incarca_simbol(CURL_LIB, "curl_global_cleanup", p_global_cleanup);
    incarca_simbol(CURL_LIB, "curl_easy_init", p_easy_init);
    incarca_simbol(CURL_LIB, "curl_easy_cleanup", p_easy_cleanup);
    incarca_simbol(CURL_LIB, "curl_easy_setopt", p_easy_setopt);
    incarca_simbol(CURL_LIB, "curl_easy_perform", p_easy_perform);
    incarca_simbol(CURL_LIB, "curl_easy_getinfo", p_easy_getinfo);
    incarca_simbol(CURL_LIB, "curl_easy_strerror", p_easy_strerror);
    incarca_simbol(CURL_LIB, "curl_slist_append", p_slist_append);
    incarca_simbol(CURL_LIB, "curl_slist_free_all", p_slist_free_all);

    return (p_easy_init != nullptr && p_easy_perform != nullptr);
}

// ============================================================================
// FUNCTII PROCEDURALE PENTRU HTTP SI PARSARE SIMPLA JSON
// ============================================================================

// Functie callback apelata automat de libcurl cand primeste date de la server
size_t primeste_date_callback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    string* raspuns = (string*)userdata;
    raspuns->append((const char*)ptr, size * nmemb);
    return size * nmemb;
}

// Trimite o cerere HTTP prin sesiunea 'sesiune' (reutilizeaza conexiunea TLS pentru viteza):
// - Daca 'json_post' este gol -> trimite cerere GET (citire date)
// - Daca 'json_post' nu este gol -> trimite cerere POST cu body JSON (transmitere date)
string cerere_http(CURL* sesiune, string url, long& cod_http, string json_post = "") {
    cod_http = 0;
    if (sesiune == nullptr) return "";

    string raspuns = "";
    p_easy_setopt(sesiune, CURLOPT_URL, url.c_str());
    p_easy_setopt(sesiune, CURLOPT_CONNECTTIMEOUT, 5L);
    p_easy_setopt(sesiune, CURLOPT_TIMEOUT, 8L);
    p_easy_setopt(sesiune, CURLOPT_WRITEFUNCTION, primeste_date_callback);
    p_easy_setopt(sesiune, CURLOPT_WRITEDATA, &raspuns);

    struct curl_slist* headere = nullptr;
    if (!json_post.empty()) {
        // CERERE POST: trimitem JSON catre server
        headere = p_slist_append(headere, "Content-Type: application/json; charset=UTF-8");
        p_easy_setopt(sesiune, CURLOPT_HTTPHEADER, headere);
        p_easy_setopt(sesiune, CURLOPT_POST, 1L);
        p_easy_setopt(sesiune, CURLOPT_POSTFIELDS, json_post.c_str());
    } else {
        // CERERE GET: citim date de la server
        p_easy_setopt(sesiune, CURLOPT_HTTPHEADER, nullptr);
        p_easy_setopt(sesiune, CURLOPT_HTTPGET, 1L);
    }

    CURLcode res = p_easy_perform(sesiune);
    if (headere != nullptr) p_slist_free_all(headere);

    if (res == CURLE_OK) {
        p_easy_getinfo(sesiune, CURLINFO_RESPONSE_CODE, &cod_http);
    } else {
        cout << "[EROARE RETEA] " << p_easy_strerror(res) << "\n";
    }
    return raspuns;
}

// Extrage valoarea unui camp din JSON (functioneaza atat pentru "text", cat si pentru numere)
string extrage_valoare(string json, string cheie) {
    size_t pos = json.find("\"" + cheie + "\"");
    if (pos == string::npos) return "";
    size_t colon = json.find(':', pos + cheie.size() + 2);
    if (colon == string::npos) return "";

    size_t i = colon + 1;
    while (i < json.size() && (json[i] == ' ' || json[i] == '\n' || json[i] == '\t')) i++;
    if (i >= json.size()) return "";

    if (json[i] == '"') {
        size_t sfarsit = json.find('"', i + 1);
        while (sfarsit != string::npos && json[sfarsit - 1] == '\\') {
            sfarsit = json.find('"', sfarsit + 1);
        }
        return (sfarsit == string::npos) ? "" : json.substr(i + 1, sfarsit - i - 1);
    } else {
        size_t sfarsit = i;
        while (sfarsit < json.size() && json[sfarsit] != ',' && json[sfarsit] != '}' && json[sfarsit] != '\n') {
            sfarsit++;
        }
        return json.substr(i, sfarsit - i);
    }
}

// Extrage un sub-obiect { ... } sau sub-lista [ ... ] asociata unei chei
string extrage_bloc_json(string json, string cheie) {
    size_t pos = json.find("\"" + cheie + "\"");
    if (pos == string::npos) return "";
    size_t colon = json.find(':', pos + cheie.size() + 2);
    if (colon == string::npos) return "";

    size_t s = colon + 1;
    while (s < json.size() && isspace((unsigned char)json[s])) s++;
    if (s >= json.size()) return "";

    char deschide = json[s];
    char inchide = (deschide == '{') ? '}' : (deschide == '[' ? ']' : '\0');
    if (inchide == '\0') return "";

    int nivel = 0;
    for (size_t i = s; i < json.size(); i++) {
        if (json[i] == deschide) nivel++;
        else if (json[i] == inchide) {
            nivel--;
            if (nivel == 0) return json.substr(s, i - s + 1);
        }
    }
    return "";
}

// Imparte o lista JSON [ {...}, {...} ] in obiecte individuale
vector<string> imparte_lista_json(string json_array) {
    vector<string> lista;
    int nivel = 0;
    size_t start = string::npos;

    for (size_t i = 0; i < json_array.size(); i++) {
        if (json_array[i] == '{') {
            if (nivel == 0) start = i;
            nivel++;
        } else if (json_array[i] == '}') {
            nivel--;
            if (nivel == 0 && start != string::npos) {
                lista.push_back(json_array.substr(start, i - start + 1));
                start = string::npos;
            }
        }
    }
    return lista;
}

// ============================================================================
// PROCEDURILE CERINTEI (A - E)
// ============================================================================

// [A] Deschide sesiunea de comunicare cu API-ul extern
CURL* initializeaza_conexiune() {
    cout << "\n--- [A] Initializare conexiune API extern ---\n";
    if (!incarca_libcurl()) {
        cout << "[EROARE] Nu s-a putut incarca libcurl.dylib!\n";
        return nullptr;
    }
    p_global_init(CURL_GLOBAL_DEFAULT);
    CURL* sesiune = p_easy_init();
    if (sesiune != nullptr) {
        cout << "[OK] Sesiune HTTP pregatita pentru: " << API_URL << "\n";
    } else {
        cout << "[EROARE] Nu s-a putut initializa libcurl!\n";
    }
    return sesiune;
}

// [B & Cererea 1] Citirea detaliilor unui utilizator (GET /users/{id})
string citeste_utilizator(CURL* sesiune, int id_utilizator) {
    cout << "\n--- [B & Cererea 1] GET " << API_URL << "/users/" << id_utilizator << " ---\n";
    long cod = 0;
    string corp = cerere_http(sesiune, API_URL + "/users/" + to_string(id_utilizator), cod);
    if (cod == 200) {
        cout << "[OK] HTTP " << cod << " | Utilizator: " << extrage_valoare(corp, "name")
             << " | Email: " << extrage_valoare(corp, "email") << "\n";
        string comp = extrage_bloc_json(corp, "company");
        cout << "     Companie: " << extrage_valoare(comp, "name") << "\n";
    }
    return corp;
}

// [A & Cererea 2] Transmiterea unei postari noi catre server (POST /posts)
string transmite_postare(CURL* sesiune, int id_utilizator, string titlu, string mesaj) {
    cout << "\n--- [A & Cererea 2] POST " << API_URL << "/posts ---\n";
    string payload = "{\"userId\": " + to_string(id_utilizator) +
                     ", \"title\": \"" + titlu + "\", \"body\": \"" + mesaj + "\"}";

    long cod = 0;
    string raspuns = cerere_http(sesiune, API_URL + "/posts", cod, payload);
    if (cod >= 200 && cod < 300) {
        cout << "[OK] HTTP " << cod << " (Created) | ID primit de la server: "
             << extrage_valoare(raspuns, "id") << "\n";
    }
    return raspuns;
}

// [Cererea 3] Filtrarea si citirea sarcinilor nefinalizate (GET /todos?userId=1&completed=false)
string citeste_sarcini(CURL* sesiune, int id_utilizator) {
    string url = API_URL + "/todos?userId=" + to_string(id_utilizator) + "&completed=false";
    cout << "\n--- [Cererea 3] GET " << url << " ---\n";
    long cod = 0;
    string corp = cerere_http(sesiune, url, cod);
    if (cod == 200) {
        vector<string> sarcini = imparte_lista_json(corp);
        cout << "[OK] HTTP " << cod << " | S-au primit " << sarcini.size() << " sarcini nefinalizate:\n";
        size_t lim = (sarcini.size() < 5) ? sarcini.size() : 5;
        for (size_t i = 0; i < lim; i++) {
            cout << "     - #" << extrage_valoare(sarcini[i], "id") << ": "
                 << extrage_valoare(sarcini[i], "title") << "\n";
        }
    }
    return corp;
}

// [C] Salvare si Deschidere a datelor receptionate in fisier local JSON
void salveaza_in_fisier(string date_json, string cale = FISIER_LOCAL) {
    cout << "\n--- [C] Salvare in fisier local: " << cale << " ---\n";
    ofstream f(cale);
    if (f.is_open()) {
        f << date_json;
        cout << "[OK] Date salvate cu succes (" << date_json.size() << " octeti).\n";
    } else {
        cout << "[EROARE] Nu s-a putut scrie fisierul.\n";
    }
}

string deschide_din_fisier(string cale = FISIER_LOCAL) {
    cout << "\n--- [C] Deschidere fisier local: " << cale << " ---\n";
    ifstream f(cale);
    if (!f.is_open()) {
        cout << "[EROARE] Fisierul nu exista!\n";
        return "";
    }
    stringstream ss;
    ss << f.rdbuf();
    cout << "[OK] Fisier incarcat in memorie (" << ss.str().size() << " octeti).\n";
    return ss.str();
}

// [E] Inchiderea aplicatiei externe fara a inchide aplicatia noastra
CURL* inchide_conexiune_externa(CURL* sesiune) {
    cout << "\n--- [E] Inchiderea conexiunii externe ---\n";
    if (sesiune != nullptr) {
        p_easy_cleanup(sesiune);   // Inchide socket-ul si elibereaza handle-ul sesiunii
        p_global_cleanup();        // Elibereaza resursele globale libcurl
        cout << "[OK] Conexiunea catre API-ul extern a fost INCHISA!\n";
        cout << "     Aplicatia noastra C++ ramane activa si continua offline.\n";
    }
    if (CURL_LIB != nullptr) {
        dlclose(CURL_LIB);
        CURL_LIB = nullptr;
    }
    return nullptr;
}

// [E] Demonstratie ca aplicatia locala functioneaza offline pe datele salvate
void raport_local_offline(string cale = FISIER_LOCAL) {
    cout << "\n--- [E] Prelucrare locala offline (fara API activ) ---\n";
    string date = deschide_din_fisier(cale);
    if (date.empty()) return;

    string util = extrage_bloc_json(date, "utilizator");
    string post = extrage_bloc_json(date, "postare_creata");
    vector<string> sarcini = imparte_lista_json(extrage_bloc_json(date, "sarcini_nefinalizate"));

    cout << "\n=========== RAPORT OFFLINE ===========\n";
    cout << "Utilizator     : " << extrage_valoare(util, "name") << "\n";
    cout << "Postare creata : #" << extrage_valoare(post, "id") << "\n";
    cout << "Sarcini ramase : " << sarcini.size() << "\n";
    cout << "======================================\n";
}

// ============================================================================
// FLUXUL COMPLET + MENIU PROCEDURAL
// ============================================================================

void executa_flux_complet() {
    cout << "=================================================================\n";
    cout << "   LABORATOR 2: PARADIGMA PROCEDURALA - INTERACTIUNE API (C++)   \n";
    cout << "=================================================================\n";

    CURL* sesiune = initializeaza_conexiune();
    if (!sesiune) {
        raport_local_offline();
        return;
    }

    // Executam cele 3 cereri HTTP pe aceeasi conexiune reutilizata (Keep-Alive):
    string util = citeste_utilizator(sesiune, 1);
    string post = transmite_postare(sesiune, 1, "Lab 2 Procedural C++", "Date transmise prin libcurl.");
    string sarc = citeste_sarcini(sesiune, 1);

    // Daca nu exista conexiune la internet, folosim datele deja salvate offline
    if (util.empty() && post.empty()) {
        sesiune = inchide_conexiune_externa(sesiune);
        raport_local_offline();
        return;
    }

    // Construim pachetul JSON si il salvam local (Cerinta c)
    string pachet = "{\n"
                    "  \"utilizator\": " + (util.empty() ? "{}" : util) + ",\n"
                    "  \"postare_creata\": " + (post.empty() ? "{}" : post) + ",\n"
                    "  \"sarcini_nefinalizate\": " + (sarc.empty() ? "[]" : sarc) + "\n"
                    "}\n";

    salveaza_in_fisier(pachet);
    deschide_din_fisier();

    // Inchidem API-ul extern si rulam local offline (Cerinta e)
    sesiune = inchide_conexiune_externa(sesiune);
    raport_local_offline();
}

int main(int argc, char* argv[]) {
    bool mod_auto = (argc > 1 && (string(argv[1]) == "--auto" || string(argv[1]) == "-a"));
    if (mod_auto || !isatty(fileno(stdin))) {
        executa_flux_complet();
        return 0;
    }

    executa_flux_complet();
    cout << "\nDoresti sa intri in meniul interactiv? (d/n): ";
    string r;
    if (getline(cin, r) && (r == "d" || r == "da")) {
        CURL* sesiune = nullptr;
        while (true) {
            cout << "\n--- MENIU LAB 2 (" << (sesiune ? "CONECTAT" : "DECONECTAT") << ") ---\n";
            cout << "1. Flux complet | 2. Conectare | 3. GET user | 4. POST articol\n";
            cout << "5. GET sarcini | 6. Citeste fisier | 7. Inchide API | 8. Raport offline | 0. Iesire\n";
            cout << "Optiunea: ";
            string opt;
            if (!getline(cin, opt) || opt == "0") break;

            if (opt == "1") executa_flux_complet();
            else if (opt == "2" && !sesiune) sesiune = initializeaza_conexiune();
            else if (opt == "3") { if (!sesiune) sesiune = initializeaza_conexiune(); citeste_utilizator(sesiune, 1); }
            else if (opt == "4") { if (!sesiune) sesiune = initializeaza_conexiune(); transmite_postare(sesiune, 1, "Test", "Corp"); }
            else if (opt == "5") { if (!sesiune) sesiune = initializeaza_conexiune(); citeste_sarcini(sesiune, 1); }
            else if (opt == "6") deschide_din_fisier();
            else if (opt == "7") sesiune = inchide_conexiune_externa(sesiune);
            else if (opt == "8") raport_local_offline();
        }
        if (sesiune) inchide_conexiune_externa(sesiune);
    }
    return 0;
}
