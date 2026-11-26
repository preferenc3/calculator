#include <iostream>
#include <string>
#include <cstring>
#include <cassert>
#include <iomanip>
#include <dlfcn.h>

#include "mylib.h"

using namespace std;

// ============================================================================
// Laboratorul 3: Paradigma Procedurala - Lucrul cu Biblioteci Partajate (DLL/dylib)
// Fisier: main.cpp
// Descriere: Aplicatie C++ scrisa in stil procedural care incarca dinamic
//            biblioteca partajata C (mylib.dylib / mylib.so) prin dlopen,
//            rezolva functiile si resursele prin dlsym, le utilizeaza si
//            elibereaza biblioteca din memorie prin dlclose.
// ============================================================================

// Tipuri de pointeri la functii pentru functiile exportate din DLL
typedef int (*AdunareFn)(int, int);
typedef int (*InmultireFn)(int, int);
typedef double (*PutereFn)(double, int);
typedef long long (*FactorialFn)(int);
typedef int (*CmmdcFn)(int, int);
typedef int (*NumaraVocaleFn)(const char*);
typedef void (*InverseazaSirFn)(char*);
typedef int (*GetNumarResurseFn)(void);
typedef int (*GetResursaIndexFn)(int, ProdusResursa*);
typedef const char* (*GetDescriereFn)(void);
typedef char* (*GenereazaRaportFn)(void);
typedef void (*ElibereazaMemFn)(void*);

// Structura procedurala ce grupeaza handle-ul si pointerii la functiile bibliotecii
struct BibliotecaDinamica {
    void* handle;
    AdunareFn adunare;
    InmultireFn inmultire;
    PutereFn putere;
    FactorialFn factorial;
    CmmdcFn cmmdc;
    NumaraVocaleFn numara_vocale;
    InverseazaSirFn inverseaza_sir;
    GetNumarResurseFn get_numar_resurse;
    GetResursaIndexFn get_resursa_dupa_index;
    GetDescriereFn get_descriere_biblioteca;
    GenereazaRaportFn genereaza_raport_resurse;
    ElibereazaMemFn elibereaza_memorie;
};

// ============================================================================
// PROCEDURI PENTRU GESTIUNEA CICLULUI DE VIATA AL BIBLIOTECII (CERINTELE A & D)
// ============================================================================

string obtine_cale_biblioteca() {
#if defined(__APPLE__)
    return "./mylib.dylib";
#elif defined(_WIN32)
    return "./mylib.dll";
#else
    return "./mylib.so";
#endif
}

bool incarca_biblioteca(const string& cale, BibliotecaDinamica& lib) {
    lib.handle = dlopen(cale.c_str(), RTLD_LAZY);
    if (lib.handle == nullptr) {
        cout << "[EROARE] Nu s-a putut incarca biblioteca '" << cale << "': " << dlerror() << "\n";
        return false;
    }

    cout << "[SUCCES] Biblioteca a fost incarcata dinamic din: " << cale << "\n";
    cout << "         Handle memorie alocat: " << lib.handle << "\n";

    // Rezolvarea simbolurilor (functiilor exportate) prin dlsym
    lib.adunare = (AdunareFn)dlsym(lib.handle, "adunare");
    lib.inmultire = (InmultireFn)dlsym(lib.handle, "inmultire");
    lib.putere = (PutereFn)dlsym(lib.handle, "putere");
    lib.factorial = (FactorialFn)dlsym(lib.handle, "factorial");
    lib.cmmdc = (CmmdcFn)dlsym(lib.handle, "cmmdc");
    lib.numara_vocale = (NumaraVocaleFn)dlsym(lib.handle, "numara_vocale");
    lib.inverseaza_sir = (InverseazaSirFn)dlsym(lib.handle, "inverseaza_sir");
    lib.get_numar_resurse = (GetNumarResurseFn)dlsym(lib.handle, "get_numar_resurse");
    lib.get_resursa_dupa_index = (GetResursaIndexFn)dlsym(lib.handle, "get_resursa_dupa_index");
    lib.get_descriere_biblioteca = (GetDescriereFn)dlsym(lib.handle, "get_descriere_biblioteca");
    lib.genereaza_raport_resurse = (GenereazaRaportFn)dlsym(lib.handle, "genereaza_raport_resurse");
    lib.elibereaza_memorie = (ElibereazaMemFn)dlsym(lib.handle, "elibereaza_memorie");

    return true;
}

bool descarca_biblioteca(BibliotecaDinamica& lib) {
    if (lib.handle == nullptr) {
        cout << "[INFO] Nicio biblioteca de descarcat.\n";
        return false;
    }

    cout << "\n[CERINTA D] Descarcare dinamica: eliberare handle " << lib.handle << " din memoria RAM...\n";
    int res = dlclose(lib.handle);
    if (res == 0) {
        lib.handle = nullptr;
        cout << "[SUCCES] Biblioteca partajata a fost eliminata din spatiul de adrese al procesului!\n";
        return true;
    } else {
        cout << "[EROARE] dlclose a esuat: " << dlerror() << "\n";
        return false;
    }
}

// ============================================================================
// PROCEDURI PENTRU TESTAREA CERINTELOR LABORATORULUI
// ============================================================================

void executa_cerinta_a(const BibliotecaDinamica& lib) {
    cout << "\n" << string(70, '=') << "\n";
    cout << "CERINTA A: Conectarea si apelul unei functii din DLL (cerinta minima)\n";
    cout << string(70, '=') << "\n";

    int val1 = 18;
    int val2 = 24;
    int rezultat = lib.adunare(val1, val2);
    cout << "Apel functie C: adunare(" << val1 << ", " << val2 << ")\n";
    cout << "Rezultat returnat din DLL: " << rezultat << "\n";
    assert(rezultat == (val1 + val2));
    cout << ">> Cerinta A a fost indeplinita cu succes.\n";
}

void executa_cerinta_b(const BibliotecaDinamica& lib) {
    cout << "\n" << string(70, '=') << "\n";
    cout << "CERINTA B: Crearea unei DLL cu functii proprii (Matematica si Siruri)\n";
    cout << string(70, '=') << "\n";

    int a = 7, b = 9;
    cout << "1. Inmultire: inmultire(" << a << ", " << b << ") = " << lib.inmultire(a, b) << "\n";

    double baza = 2.5;
    int exp = 3;
    cout << "2. Ridicare la putere: putere(" << baza << ", " << exp << ") = " << lib.putere(baza, exp) << "\n";

    int n = 10;
    cout << "3. Factorial: factorial(" << n << ") = " << lib.factorial(n) << "\n";

    int x = 108, y = 48;
    cout << "4. CMMDC: cmmdc(" << x << ", " << y << ") = " << lib.cmmdc(x, y) << "\n";

    const char* text = "Programare Procedurala in C si C++";
    cout << "5. Numarare vocale: textul '" << text << "' contine " << lib.numara_vocale(text) << " vocale.\n";

    char buffer_sir[64] = "Antigravity2026";
    string sir_original = buffer_sir;
    lib.inverseaza_sir(buffer_sir);
    cout << "6. Inversare sir (in memorie C): '" << sir_original << "' -> '" << buffer_sir << "'\n";

    cout << ">> Cerinta B a fost indeplinita cu succes.\n";
}

void executa_cerinta_c(const BibliotecaDinamica& lib) {
    cout << "\n" << string(70, '=') << "\n";
    cout << "CERINTA C: Crearea unei DLL cu resurse proprii\n";
    cout << string(70, '=') << "\n";

    // 1. Accesare variabile globale exportate direct din DLL prin dlsym
    const char* const* p_nume = (const char* const*)dlsym(lib.handle, "NUME_BIBLIOTECA");
    const char* const* p_vers = (const char* const*)dlsym(lib.handle, "VERSIUNE_BIBLIOTECA");
    const double* p_pi = (const double*)dlsym(lib.handle, "CONSTANTA_PI");

    cout << "[Resurse globale exportate]:\n";
    if (p_nume) cout << "  - Nume biblioteca: '" << *p_nume << "'\n";
    if (p_vers) cout << "  - Versiune       : '" << *p_vers << "'\n";
    if (p_pi)   cout << "  - Constanta PI   : " << setprecision(15) << *p_pi << setprecision(2) << "\n";

    // 2. Resursa text informativa
    cout << "\n[Resursa descriptiva]: " << lib.get_descriere_biblioteca() << "\n";

    // 3. Catalogul de produse din structura interna a DLL-ului
    int total_resurse = lib.get_numar_resurse();
    cout << "\n[Catalog intern de resurse (Structuri C)] Total elemente: " << total_resurse << "\n";
    cout << "  " << left << setw(6) << "ID" << " | " << setw(28) << "Denumire Produs"
         << " | " << setw(12) << "Pret (RON)" << " | " << setw(6) << "Stoc" << "\n";
    cout << "  " << string(60, '-') << "\n";

    for (int i = 0; i < total_resurse; i++) {
        ProdusResursa res;
        if (lib.get_resursa_dupa_index(i, &res)) {
            cout << "  " << left << setw(6) << res.id << " | " << setw(28) << res.denumire
                 << " | " << fixed << setprecision(2) << setw(12) << res.pret
                 << " | " << setw(6) << res.cantitate << "\n";
        }
    }

    // 4. Resursa dinamica alocata pe heap in C si eliberata explicit
    cout << "\n[Resursa dinamica generata pe heap in C]:\n";
    char* ptr_raport = lib.genereaza_raport_resurse();
    if (ptr_raport != nullptr) {
        cout << ptr_raport;
        lib.elibereaza_memorie(ptr_raport);
        cout << "  -> Memoria heap pentru raport a fost eliberata de DLL.\n";
    }

    cout << ">> Cerinta C a fost indeplinita cu succes.\n";
}

void executa_cerinta_d_demonstratie_la_necesitate(const string& cale_biblioteca) {
    cout << "\n" << string(70, '=') << "\n";
    cout << "CERINTA D: Conectare dinamica la necesitate si eliminare din memorie\n";
    cout << string(70, '=') << "\n";

    cout << "Faza 1: Se solicita un calcul specific (ex: calcul factorial mare)...\n";
    BibliotecaDinamica lib_temp{};
    if (!incarca_biblioteca(cale_biblioteca, lib_temp)) {
        cout << "[EROARE] Nu s-a putut incarca biblioteca temporara.\n";
        return;
    }

    cout << "Faza 2: Utilizarea bibliotecii pentru calcul...\n";
    int numar = 8;
    long long rez = lib_temp.factorial(numar);
    cout << "        Calcul efectuat: factorial(" << numar << ") = " << rez << "\n";

    cout << "Faza 3: Operatia a fost finalizata. Eliminarea bibliotecii din memorie...\n";
    descarca_biblioteca(lib_temp);
    cout << ">> Cerinta D a fost indeplinita cu succes.\n";
}

// ============================================================================
// PROCEDURA PRINCIPALA
// ============================================================================

int main() {
    cout << string(70, '=') << "\n";
    cout << "LABORATORUL 3: PARADIGMA PROCEDURALA - UTILIZARE DLL / DYLIB (C++)\n";
    cout << string(70, '=') << "\n";

    string cale_lib = obtine_cale_biblioteca();
    cout << "Cale tinta biblioteca: " << cale_lib << "\n\n";

    BibliotecaDinamica lib{};
    if (!incarca_biblioteca(cale_lib, lib)) {
        return 1;
    }

    executa_cerinta_a(lib);
    executa_cerinta_b(lib);
    executa_cerinta_c(lib);

    // Descarcare biblioteca principala (Cerinta D)
    descarca_biblioteca(lib);

    // Demonstratie specifica pentru conectare dinamica doar la necesitate (Cerinta D)
    executa_cerinta_d_demonstratie_la_necesitate(cale_lib);

    cout << "\n" << string(70, '=') << "\n";
    cout << "TOATE CERINTELE AU FOST EXECUTATE CU SUCCES CONFORM CONDITIILOR!\n";
    cout << string(70, '=') << "\n";

    return 0;
}
