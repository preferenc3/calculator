#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>
#include <unistd.h>
#include <curl/curl.h>

using namespace std;

// ============================================================================
// Laboratorul 2: Paradigma Procedurala (C++)
// Tema: Interactiunea cu alte aplicatii prin intermediul API (libcurl)
// ============================================================================
// Cerinte implementate conform conditii.md:
//   a. Conectarea si transmiterea informatiei intr-o aplicatie externa (POST /posts)
//   b. Citirea informatiei din aplicatia externa (GET /users/{id})
//   c. Salvarea si deschiderea informatiei receptionate in fisier local (JSON)
//   d. Utilizarea a minim 3 operatii diferite prin API:
//      1. Preluare detalii utilizator (GET /users/{id})
//      2. Transmitere si creare postare noua (POST /posts)
//      3. Filtrare si obtinere sarcini to-do (GET /todos?userId={id}&completed=false)
//      4. (Bonus) Preluare comentarii (GET /comments?postId={id})
//   e. Inchiderea aplicatiei externe (cleanup sesiune libcurl) fara a opri
//      aplicatia locala, demonstrand continuitatea pe date locale.
// ============================================================================

const string API_BASE_URL = "https://jsonplaceholder.typicode.com";
const long TIMEOUT_SECUNDE = 10L;
const string FISIER_DATE_LOCAL = "date_api_salvate.json";

// ============================================================================
// Utilitare Procedurale HTTP (libcurl) si JSON simplu
// ============================================================================

// Callback procedural necesar libcurl pentru colectarea raspunsului HTTP
size_t scrie_raspuns_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    string* buffer = (string*)userp;
    buffer->append((const char*)contents, total);
    return total;
}

struct RaspunsHTTP {
    long cod_status;
    string corp;
    bool succes;
    string eroare;
};

RaspunsHTTP executa_cerere_get(CURL* sesiune, const string& url) {
    RaspunsHTTP rez{0, "", false, ""};
    if (sesiune == nullptr) {
        rez.eroare = "Sesiune nula";
        return rez;
    }

    curl_easy_reset(sesiune);
    curl_easy_setopt(sesiune, CURLOPT_URL, url.c_str());
    curl_easy_setopt(sesiune, CURLOPT_TIMEOUT, TIMEOUT_SECUNDE);
    curl_easy_setopt(sesiune, CURLOPT_USERAGENT, "Procedural-Client-Lab2-Cpp/1.0");
    curl_easy_setopt(sesiune, CURLOPT_WRITEFUNCTION, scrie_raspuns_callback);
    curl_easy_setopt(sesiune, CURLOPT_WRITEDATA, &rez.corp);

    CURLcode res = curl_easy_perform(sesiune);
    if (res != CURLE_OK) {
        rez.eroare = curl_easy_strerror(res);
        return rez;
    }

    curl_easy_getinfo(sesiune, CURLINFO_RESPONSE_CODE, &rez.cod_status);
    rez.succes = (rez.cod_status >= 200 && rez.cod_status < 300);
    return rez;
}

RaspunsHTTP executa_cerere_post_json(CURL* sesiune, const string& url, const string& json_payload) {
    RaspunsHTTP rez{0, "", false, ""};
    if (sesiune == nullptr) {
        rez.eroare = "Sesiune nula";
        return rez;
    }

    curl_easy_reset(sesiune);
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json; charset=UTF-8");

    curl_easy_setopt(sesiune, CURLOPT_URL, url.c_str());
    curl_easy_setopt(sesiune, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(sesiune, CURLOPT_POST, 1L);
    curl_easy_setopt(sesiune, CURLOPT_POSTFIELDS, json_payload.c_str());
    curl_easy_setopt(sesiune, CURLOPT_TIMEOUT, TIMEOUT_SECUNDE);
    curl_easy_setopt(sesiune, CURLOPT_USERAGENT, "Procedural-Client-Lab2-Cpp/1.0");
    curl_easy_setopt(sesiune, CURLOPT_WRITEFUNCTION, scrie_raspuns_callback);
    curl_easy_setopt(sesiune, CURLOPT_WRITEDATA, &rez.corp);

    CURLcode res = curl_easy_perform(sesiune);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        rez.eroare = curl_easy_strerror(res);
        return rez;
    }

    curl_easy_getinfo(sesiune, CURLINFO_RESPONSE_CODE, &rez.cod_status);
    rez.succes = (rez.cod_status >= 200 && rez.cod_status < 300);
    return rez;
}

// Functii procedurale simple pentru extragerea valorilor din text JSON
string extrage_camp_json_string(const string& json, const string& cheie, size_t start_pos = 0) {
    string pattern = "\"" + cheie + "\"";
    size_t pos = json.find(pattern, start_pos);
    if (pos == string::npos) return "";

    size_t colon = json.find(':', pos + pattern.size());
    if (colon == string::npos) return "";

    size_t first_quote = json.find('"', colon + 1);
    if (first_quote == string::npos) return "";

    size_t cur = first_quote + 1;
    string val = "";
    while (cur < json.size()) {
        if (json[cur] == '\\' && cur + 1 < json.size()) {
            val += json[cur + 1];
            cur += 2;
        } else if (json[cur] == '"') {
            break;
        } else {
            val += json[cur];
            cur++;
        }
    }
    return val;
}

string extrage_camp_json_numar(const string& json, const string& cheie, size_t start_pos = 0) {
    string pattern = "\"" + cheie + "\"";
    size_t pos = json.find(pattern, start_pos);
    if (pos == string::npos) return "";

    size_t colon = json.find(':', pos + pattern.size());
    if (colon == string::npos) return "";

    size_t s = colon + 1;
    while (s < json.size() && (json[s] == ' ' || json[s] == '\t' || json[s] == '\n')) s++;

    string num = "";
    while (s < json.size() && (isdigit((unsigned char)json[s]) || json[s] == '-' || json[s] == '.')) {
        num += json[s];
        s++;
    }
    return num;
}

vector<string> imparte_obiecte_array_json(const string& json_array) {
    vector<string> obiecte;
    int adancime = 0;
    bool in_string = false;
    size_t start = string::npos;

    for (size_t i = 0; i < json_array.size(); i++) {
        char c = json_array[i];
        if (c == '"' && (i == 0 || json_array[i - 1] != '\\')) {
            in_string = !in_string;
        }
        if (in_string) continue;

        if (c == '{') {
            if (adancime == 0) start = i;
            adancime++;
        } else if (c == '}') {
            adancime--;
            if (adancime == 0 && start != string::npos) {
                obiecte.push_back(json_array.substr(start, i - start + 1));
                start = string::npos;
            }
        }
    }
    return obiecte;
}

string extrage_sub_json(const string& json, const string& cheie) {
    string pattern = "\"" + cheie + "\"";
    size_t pos = json.find(pattern);
    if (pos == string::npos) return "";
    size_t colon = json.find(':', pos + pattern.size());
    if (colon == string::npos) return "";

    size_t s = colon + 1;
    while (s < json.size() && (json[s] == ' ' || json[s] == '\t' || json[s] == '\n' || json[s] == '\r')) s++;
    if (s >= json.size()) return "";

    char open_ch = json[s];
    char close_ch = (open_ch == '{') ? '}' : (open_ch == '[' ? ']' : '\0');
    if (close_ch == '\0') return "";

    int adancime = 0;
    bool in_string = false;
    for (size_t i = s; i < json.size(); i++) {
        char c = json[i];
        if (c == '"' && (i == 0 || json[i - 1] != '\\')) in_string = !in_string;
        if (in_string) continue;
        if (c == open_ch) adancime++;
        else if (c == close_ch) {
            adancime--;
            if (adancime == 0) return json.substr(s, i - s + 1);
        }
    }
    return "";
}

string escape_json_string(const string& s) {
    string out = "";
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '"') out += "\\\"";
        else if (s[i] == '\\') out += "\\\\";
        else if (s[i] == '\n') out += "\\n";
        else out += s[i];
    }
    return out;
}

// ============================================================================
// a. Conectarea si transmiterea informatiei in aplicatia externa
// ============================================================================

CURL* initializeaza_conexiune(const string& url_baza = API_BASE_URL) {
    cout << "\n--- [A] Conectare la aplicatia externa ---\n";
    cout << "Se initiaza conexiunea cu API-ul: " << url_baza << " ...\n";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* sesiune = curl_easy_init();
    if (sesiune == nullptr) {
        cout << "[EROARE] Nu s-a putut initializa sesiunea libcurl.\n";
        return nullptr;
    }

    RaspunsHTTP r = executa_cerere_get(sesiune, url_baza + "/users");
    if (r.succes) {
        cout << "[OK] Conexiune stabilita cu succes! Cod raspuns: " << r.cod_status << "\n";
        return sesiune;
    } else if (r.cod_status > 0) {
        cout << "[AVERTISMENT] API-ul a raspuns cu codul: " << r.cod_status << "\n";
        return sesiune;
    } else {
        cout << "[EROARE] Nu s-a putut realiza conexiunea: " << r.eroare << "\n";
        curl_easy_cleanup(sesiune);
        curl_global_cleanup();
        return nullptr;
    }
}

string transmite_informatie_postare(CURL* sesiune, const string& url_baza, int id_utilizator, const string& titlu, const string& corp) {
    cout << "\n--- [A & D] Transmitere date catre API (POST /posts) ---\n";
    if (sesiune == nullptr) {
        cout << "[EROARE] Sesiunea externa nu este activa!\n";
        return "";
    }

    string url = url_baza + "/posts";
    ostringstream payload;
    payload << "{\n"
            << "  \"userId\": " << id_utilizator << ",\n"
            << "  \"title\": \"" << escape_json_string(titlu) << "\",\n"
            << "  \"body\": \"" << escape_json_string(corp) << "\"\n"
            << "}";

    cout << "URL: " << url << "\n";
    cout << "Date transmise (payload):\n" << payload.str() << "\n";

    RaspunsHTTP r = executa_cerere_post_json(sesiune, url, payload.str());
    if (r.cod_status > 0) {
        cout << "Status HTTP receptionat: " << r.cod_status << " (Created)\n";
        string id_nou = extrage_camp_json_numar(r.corp, "id");
        cout << "Raspunsul aplicatiei externe: Resursa creata cu ID-ul " << id_nou << "\n";
        return r.corp;
    } else {
        cout << "[EROARE] Transmiterea datelor a esuat: " << r.eroare << "\n";
        return "";
    }
}

// ============================================================================
// b. Citirea informatiei din aplicatia externa
// ============================================================================

string citeste_detalii_utilizator(CURL* sesiune, const string& url_baza, int id_utilizator) {
    cout << "\n--- [B & D] Citire date utilizator (GET /users/" << id_utilizator << ") ---\n";
    if (sesiune == nullptr) {
        cout << "[EROARE] Sesiunea externa nu este activa!\n";
        return "";
    }

    string url = url_baza + "/users/" + to_string(id_utilizator);
    RaspunsHTTP r = executa_cerere_get(sesiune, url);
    if (r.succes) {
        string nume = extrage_camp_json_string(r.corp, "name");
        string username = extrage_camp_json_string(r.corp, "username");
        string email = extrage_camp_json_string(r.corp, "email");
        string addr_sub = extrage_sub_json(r.corp, "address");
        string oras = extrage_camp_json_string(addr_sub, "city");
        string comp_sub = extrage_sub_json(r.corp, "company");
        string companie = extrage_camp_json_string(comp_sub, "name");

        cout << "[OK] Date citite pentru utilizatorul: " << nume << " (@" << username << ")\n";
        cout << "     Email: " << email << "\n";
        cout << "     Oras: " << oras << "\n";
        cout << "     Companie: " << companie << "\n";
        return r.corp;
    } else {
        cout << "[EROARE] Citirea a esuat (" << r.cod_status << " " << r.eroare << ")\n";
        return "";
    }
}

// ============================================================================
// d. Efectuarea a minim 3 operatii diferite prin API
// ============================================================================

string citeste_sarcini_utilizator(CURL* sesiune, const string& url_baza, int id_utilizator, bool doar_nefinalizate = true) {
    string stare_text = doar_nefinalizate ? "nefinalizate" : "toate";
    cout << "\n--- [D] Operatia 3: Citire sarcini " << stare_text << " (GET /todos) ---\n";
    if (sesiune == nullptr) {
        cout << "[EROARE] Sesiunea externa nu este activa!\n";
        return "[]";
    }

    string url = url_baza + "/todos?userId=" + to_string(id_utilizator);
    if (doar_nefinalizate) {
        url += "&completed=false";
    }

    RaspunsHTTP r = executa_cerere_get(sesiune, url);
    if (r.succes) {
        vector<string> sarcini = imparte_obiecte_array_json(r.corp);
        cout << "[OK] S-au receptionat " << sarcini.size() << " sarcini " << stare_text << ":\n";
        size_t limita = (sarcini.size() < 5) ? sarcini.size() : 5;
        for (size_t i = 0; i < limita; i++) {
            string id_s = extrage_camp_json_numar(sarcini[i], "id");
            string titlu_s = extrage_camp_json_string(sarcini[i], "title");
            bool finalizata = (sarcini[i].find("\"completed\": true") != string::npos);
            cout << "     [" << (finalizata ? "✓" : "✗") << "] #" << id_s << ": " << titlu_s << "\n";
        }
        if (sarcini.size() > 5) {
            cout << "     ... si inca " << (sarcini.size() - 5) << " sarcini.\n";
        }
        return r.corp;
    } else {
        cout << "[EROARE] Cererea todos a esuat (" << r.cod_status << " " << r.eroare << ")\n";
        return "[]";
    }
}

string citeste_comentarii_postare(CURL* sesiune, const string& url_baza, int id_postare) {
    cout << "\n--- [D] Operatia 4 (Bonus): Citire comentarii (GET /comments?postId=" << id_postare << ") ---\n";
    if (sesiune == nullptr) {
        cout << "[EROARE] Sesiunea externa nu este activa!\n";
        return "[]";
    }

    string url = url_baza + "/comments?postId=" + to_string(id_postare);
    RaspunsHTTP r = executa_cerere_get(sesiune, url);
    if (r.succes) {
        vector<string> comentarii = imparte_obiecte_array_json(r.corp);
        cout << "[OK] S-au receptionat " << comentarii.size() << " comentarii pentru postarea #" << id_postare << ":\n";
        size_t limita = (comentarii.size() < 3) ? comentarii.size() : 3;
        for (size_t i = 0; i < limita; i++) {
            cout << "     - De la " << extrage_camp_json_string(comentarii[i], "email")
                 << ": \"" << extrage_camp_json_string(comentarii[i], "name") << "\"\n";
        }
        return r.corp;
    } else {
        cout << "[EROARE] Preluarea comentariilor a esuat (" << r.cod_status << ")\n";
        return "[]";
    }
}

// ============================================================================
// c. Salvare + Deschidere informatii receptionate de la API
// ============================================================================

bool salveaza_date_in_fisier(const string& date_json, const string& cale_fisier = FISIER_DATE_LOCAL) {
    cout << "\n--- [C] Salvare informatii in fisier local ---\n";
    ofstream f(cale_fisier);
    if (!f.is_open()) {
        cout << "[EROARE] Salvarea fisierului a esuat: " << cale_fisier << "\n";
        return false;
    }
    f << date_json;
    f.close();

    ifstream fin(cale_fisier, ios::ate | ios::binary);
    streamsize dim = fin.tellg();
    cout << "[OK] Datele au fost salvate cu succes in: " << cale_fisier << "\n";
    cout << "     Dimensiune fisier: " << dim << " octeti\n";
    return true;
}

string deschide_date_din_fisier(const string& cale_fisier = FISIER_DATE_LOCAL) {
    cout << "\n--- [C] Deschidere si citire date din fisier local ---\n";
    ifstream f(cale_fisier);
    if (!f.is_open()) {
        cout << "[EROARE] Fisierul " << cale_fisier << " nu exista!\n";
        return "";
    }
    stringstream ss;
    ss << f.rdbuf();
    string continut = ss.str();
    cout << "[OK] Fisierul a fost deschis si incarcat in memorie cu succes! (" << continut.size() << " octeti)\n";
    return continut;
}

// ============================================================================
// e. Inchiderea aplicatiei externe, fara a inchide aplicatia dvs
// ============================================================================

CURL* inchide_conexiune_aplicatie_externa(CURL* sesiune) {
    cout << "\n--- [E] Inchiderea aplicatiei externe ---\n";
    if (sesiune != nullptr) {
        curl_easy_cleanup(sesiune);
        curl_global_cleanup();
        cout << "[OK] Sesiunea de comunicare cu API-ul extern a fost INCHISA.\n";
        cout << "     Canalele de retea si socket-urile catre serverul extern au fost eliberate.\n";
    } else {
        cout << "[INFO] Sesiunea externa era deja inchisa sau nula.\n";
    }
    cout << "[STATUS] Aplicatia externa a fost deconectata/inchisa.\n";
    cout << "[STATUS] Aplicatia noastra C++ continua sa ruleze in siguranta!\n";
    return nullptr;
}

void demonstreaza_continuitate_locala(const string& cale_fisier = FISIER_DATE_LOCAL) {
    cout << "\n--- [E] Demonstrare: Prelucrare locala continua dupa inchiderea API-ului extern ---\n";
    cout << "Se acceseaza datele salvate local pentru procesare si raportare offline...\n";

    string date_locale = deschide_date_din_fisier(cale_fisier);
    if (date_locale.empty()) {
        cout << "[EROARE] Nu s-au gasit date locale pentru procesare!\n";
        return;
    }

    string utilizator = extrage_sub_json(date_locale, "utilizator");
    string comp = extrage_sub_json(utilizator, "company");
    string postare = extrage_sub_json(date_locale, "postare_creata");
    string sarcini_raw = extrage_sub_json(date_locale, "sarcini_nefinalizate");
    vector<string> sarcini = imparte_obiecte_array_json(sarcini_raw);

    cout << "\n================== RAPORT LOCAL GENERAT OFFLINE ==================\n";
    cout << "Utilizator analizat         : " << extrage_camp_json_string(utilizator, "name")
         << " (" << extrage_camp_json_string(utilizator, "email") << ")\n";
    cout << "Companie                    : " << extrage_camp_json_string(comp, "name") << "\n";
    cout << "Postare creata prin API (ID): #" << extrage_camp_json_numar(postare, "id") << "\n";
    cout << "Numar sarcini nefinalizate  : " << sarcini.size() << "\n";
    cout << "Exemple de sarcini restante :\n";
    size_t lim = (sarcini.size() < 3) ? sarcini.size() : 3;
    for (size_t i = 0; i < lim; i++) {
        cout << "  " << (i + 1) << ". " << extrage_camp_json_string(sarcini[i], "title") << "\n";
    }
    cout << "==================================================================\n";
    cout << "Prelucrarea locala a fost finalizata cu succes fara apeluri externe active!\n";
}

// ============================================================================
// Fluxul principal procedural si Meniul Interactiv
// ============================================================================

void executa_flux_complet() {
    cout << string(70, '=') << "\n";
    cout << "  LABORATOR 2: PARADIGMA PROCEDURALA - INTERACTIUNEA CU API (C++)  \n";
    cout << string(70, '=') << "\n";

    // 1. Conectare
    CURL* sesiune = initializeaza_conexiune(API_BASE_URL);
    if (sesiune == nullptr) {
        cout << "[AVERTISMENT] Nu s-a putut conecta la API-ul extern.\n";
        cout << "[INFO] Se demonstreaza continuitatea pe fisierul local salvat...\n";
        demonstreaza_continuitate_locala(FISIER_DATE_LOCAL);
        return;
    }

    // 2. Operatia 1 (b + d): Citire detalii utilizator
    string date_utilizator = citeste_detalii_utilizator(sesiune, API_BASE_URL, 1);

    // 3. Operatia 2 (a + d): Transmitere postare noua
    string postare_creata = transmite_informatie_postare(
        sesiune, API_BASE_URL, 1,
        "Implementare Procedurala Lab 2 (C++)",
        "Acest articol a fost transmis cu succes din aplicatia C++ folosind libcurl."
    );

    // 4. Operatia 3 (d): Preluare sarcini To-Do
    string sarcini_nefinalizate = citeste_sarcini_utilizator(sesiune, API_BASE_URL, 1, true);

    // 5. Operatia 4 (d - bonus): Preluare comentarii
    string comentarii_postare = citeste_comentarii_postare(sesiune, API_BASE_URL, 1);

    // 6. Structurare JSON pentru salvare locala (c)
    ostringstream pachet;
    pachet << "{\n"
           << "  \"sursa_api\": \"" << API_BASE_URL << "\",\n"
           << "  \"utilizator\": " << (date_utilizator.empty() ? "{}" : date_utilizator) << ",\n"
           << "  \"postare_creata\": " << (postare_creata.empty() ? "{}" : postare_creata) << ",\n"
           << "  \"sarcini_nefinalizate\": " << sarcini_nefinalizate << ",\n"
           << "  \"comentarii_postare_1\": " << comentarii_postare << "\n"
           << "}\n";

    // 7. Salvare + 8. Deschidere locala (c)
    salveaza_date_in_fisier(pachet.str(), FISIER_DATE_LOCAL);
    deschide_date_din_fisier(FISIER_DATE_LOCAL);

    // 9. Inchiderea aplicatiei externe fara a inchide aplicatia noastra (e)
    sesiune = inchide_conexiune_aplicatie_externa(sesiune);

    // 10. Continuitate locala offline (e)
    demonstreaza_continuitate_locala(FISIER_DATE_LOCAL);

    cout << "\n" << string(70, '=') << "\n";
    cout << "  Toate cerintele laboratorului 2 au fost executate cu succes!  \n";
    cout << string(70, '=') << "\n";
}

void afiseaza_meniu_interactiv() {
    CURL* sesiune = nullptr;

    while (true) {
        cout << "\n" << string(50, '-') << "\n";
        cout << "       MENIU PROCEDURAL - LAB 2 API (C++)         \n";
        cout << string(50, '-') << "\n";
        cout << "Stare conexiune externa: [" << (sesiune != nullptr ? "CONECTAT" : "DECONECTAT") << "]\n";
        cout << "1. Executa fluxul complet demonstrativ (Toate cerintele a-e)\n";
        cout << "2. [a] Conecteaza-te la API\n";
        cout << "3. [b & d1] Citeste date utilizator (GET /users/{id})\n";
        cout << "4. [a & d2] Transmite postare noua (POST /posts)\n";
        cout << "5. [d3] Citeste sarcini To-Do (GET /todos)\n";
        cout << "6. [c] Deschide datele locale (JSON)\n";
        cout << "7. [e] Inchide aplicatia externa (pastreaza aplicatia locala)\n";
        cout << "8. [e] Ruleaza raportul local offline\n";
        cout << "0. Iesire din aplicatia locala\n";
        cout << string(50, '-') << "\n";
        cout << "Alege o optiune (0-8): ";

        string opt;
        if (!getline(cin, opt)) break;

        if (opt == "1") {
            executa_flux_complet();
        } else if (opt == "2") {
            if (sesiune == nullptr) sesiune = initializeaza_conexiune(API_BASE_URL);
            else cout << "[INFO] Sesiunea este deja conectata.\n";
        } else if (opt == "3") {
            if (sesiune == nullptr) sesiune = initializeaza_conexiune(API_BASE_URL);
            cout << "Introdu ID utilizator (1-10) [implicit 1]: ";
            string uid_s;
            getline(cin, uid_s);
            int uid = uid_s.empty() ? 1 : stoi(uid_s);
            citeste_detalii_utilizator(sesiune, API_BASE_URL, uid);
        } else if (opt == "4") {
            if (sesiune == nullptr) sesiune = initializeaza_conexiune(API_BASE_URL);
            cout << "Titlul postarii [implicit 'Titlu test']: ";
            string titlu;
            getline(cin, titlu);
            if (titlu.empty()) titlu = "Titlu test";
            cout << "Continutul postarii [implicit 'Continut test']: ";
            string corp;
            getline(cin, corp);
            if (corp.empty()) corp = "Continut test";
            transmite_informatie_postare(sesiune, API_BASE_URL, 1, titlu, corp);
        } else if (opt == "5") {
            if (sesiune == nullptr) sesiune = initializeaza_conexiune(API_BASE_URL);
            citeste_sarcini_utilizator(sesiune, API_BASE_URL, 1, true);
        } else if (opt == "6") {
            deschide_date_din_fisier(FISIER_DATE_LOCAL);
        } else if (opt == "7") {
            sesiune = inchide_conexiune_aplicatie_externa(sesiune);
        } else if (opt == "8") {
            demonstreaza_continuitate_locala(FISIER_DATE_LOCAL);
        } else if (opt == "0") {
            if (sesiune != nullptr) {
                curl_easy_cleanup(sesiune);
                curl_global_cleanup();
            }
            cout << "Aplicatia locala se inchide. La revedere!\n";
            break;
        } else {
            cout << "Optiune invalida!\n";
        }
    }
}

int main(int argc, char* argv[]) {
    bool mod_auto = false;
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--auto" || string(argv[i]) == "-a") {
            mod_auto = true;
        }
    }

    if (mod_auto || !isatty(fileno(stdin))) {
        executa_flux_complet();
    } else {
        executa_flux_complet();
        cout << "\nDoresti sa accesezi meniul interactiv? (d/n) [implicit: n]: ";
        string raspuns;
        if (getline(cin, raspuns) && (raspuns == "d" || raspuns == "da" || raspuns == "y" || raspuns == "yes")) {
            afiseaza_meniu_interactiv();
        } else {
            cout << "Finalizat cu succes.\n";
        }
    }
    return 0;
}
