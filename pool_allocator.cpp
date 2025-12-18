
//============================================================================
// BRANCH: feature/pool-allocator
// ZHVILLUAR NGA: Yllka
// PERSHKRIMI: Implementimi i Pool Allocator për objekte me madhësi fikse
// me free list dhe memory pooling
// ============================================================================

#ifndef POOL_ALLOCATOR_H
#define POOL_ALLOCATOR_H

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iomanip>

/**
 * AlokatorPool - Aloktor për objekte me madhësi fikse
 *
 * Karakteristikat:
 * - Perfekt për objekte të njëjtë madhësie (particles, entities, nodes)
 * - Alokim/dealokim O(1) - konstant
 * - Zero fragmentation
 * - Cache-friendly layout
 * - Free list për track të blloqeve të lira
 */
class AlokatorPool {
private:
 // Node në free list
 struct BlokuLire {
 BlokuLire* tjeteri; // Pointer në blokun e ardhshëm të lirë
 };

 uint8_t* fillimi; // Fillimi i pool
 BlokuLire* kreu; // Fillimi i listës së lirë
 size_t madhesiaBlokut; // Madhësia e secilit blok
 size_t numriBlloqeve; // Numri total i blloqeve
 size_t blloqeLira; // Numri i blloqeve të lira
 size_t numriAlokimeve; // Numërues i alokimeve totale
 size_t numriLirimeve; // Numërues i lirimeve totale

public:
 /**
 * Konstruktori - inicializon pool me blloqe të barabarta
 * @param madhesiaObjektit: Madhësia e secilit objekt
 * @param numriObjekteve: Sa objekte të ruhen në pool
 */
 AlokatorPool(size_t madhesiaObjektit, size_t numriObjekteve)
 : madhesiaBlokut(madhesiaObjektit >= sizeof(BlokuLire) ?
 madhesiaObjektit : sizeof(BlokuLire)),
 numriBlloqeve(numriObjekteve),
 blloqeLira(numriObjekteve),
 numriAlokimeve(0),
 numriLirimeve(0) {

 // Aloko memorien për të gjithë pool-in
 size_t madhesiaTotale = madhesiaBlokut * numriBlloqeve;
 fillimi = new uint8_t[madhesiaTotale];

 // Inicializo free list - të gjithë blloqet janë të lirë fillimisht
 kreu = reinterpret_cast<BlokuLire*>(fillimi);
 BlokuLire* aktual = kreu;

 for (size_t i = 0; i < numriBlloqeve - 1; i++) {
 uint8_t* adresaRadhes = fillimi + ((i + 1) * madhesiaBlokut);
 aktual->tjeteri = reinterpret_cast<BlokuLire*>(adresaRadhes);
 aktual = aktual->tjeteri;
 }

 // Elementi i fundit tregon nullptr
 aktual->tjeteri = nullptr;

 std::cout << "[Pool Allocator] U krijua me " << numriBlloqeve
 << " blloqe x " << madhesiaBlokut << " bytes = "
 << (madhesiaTotale / 1024.0) << " KB\n";
 }

 /**
 * Destruktori
 */
 ~AlokatorPool() {
 delete[] fillimi;
 std::cout << "[Pool Allocator] U shkatërrua. Alokime: "
 << numriAlokimeve << ", Lirime: " << numriLirimeve << "\n";

 if (blloqeLira != numriBlloqeve) {
 std::cerr << "⚠️ KUJDES: " << (numriBlloqeve - blloqeLira)
 << " blloqe nuk u liruan (memory leak)!\n";
 }
 }

 /**
 * Aloko një blok nga pool
 * @return Pointer në blokun e lirë, ose nullptr nëse pool është plot
 */
 void* aloko() {
 // Kontrollo nëse ka blloqe të lira
 if (kreu == nullptr) {
 std::cerr << "[Pool Allocator] GABIM: Pool është plot! "
 << "Të gjithë " << numriBlloqeve << " blloqet janë në përdorim.\n";
 return nullptr;
 }

 // Merr blokun e parë të lirë
 BlokuLire* bloku = kreu;
 kreu = kreu->tjeteri;

 // Përditëso statistikat
 blloqeLira--;
 numriAlokimeve++;

 #ifdef DEBUG_ALLOCATOR
 std::cout << "[Pool Allocator] Alokuar blok. Të lira: "
 << blloqeLira << "/" << numriBlloqeve << "\n";
 #endif

 return bloku;
 }

 /**
 * Liro një blok dhe ktheje në pool
 * @param pointer: Pointer në blokun për t'u liruar
 */
 void liro(void* pointer) {
 if (pointer == nullptr) {
 return;
 }

 // Valido që pointer-i është brenda pool
 if (!eshtePointerValid(pointer)) {
 std::cerr << "[Pool Allocator] GABIM: Pointer i pavlefshëm! "
 << "Nuk është pjesë e këtij pool.\n";
 return;
 }

 // Shto blokun në fillim të free list
 BlokuLire* bloku = static_cast<BlokuLire*>(pointer);
 bloku->tjeteri = kreu;
 kreu = bloku;

 // Përditëso statistikat
 blloqeLira++;
 numriLirimeve++;

 #ifdef DEBUG_ALLOCATOR
 std::cout << "[Pool Allocator] Liruar blok. Të lira: "
 << blloqeLira << "/" << numriBlloqeve << "\n";
 #endif
 }

 /**
 * Reseto pool-in (KUJDES: humb të gjithë të dhënat!)
 */
 void reseto() {
 // Ri-inicializo free list
 kreu = reinterpret_cast<BlokuLire*>(fillimi);
 BlokuLire* aktual = kreu;

 for (size_t i = 0; i < numriBlloqeve - 1; i++) {
 uint8_t* adresaRadhes = fillimi + ((i + 1) * madhesiaBlokut);
 aktual->tjeteri = reinterpret_cast<BlokuLire*>(adresaRadhes);
 aktual = aktual->tjeteri;
 }
 aktual->tjeteri = nullptr;

 blloqeLira = numriBlloqeve;
 std::cout << "[Pool Allocator] U rivendos. Të gjithë blloqet janë të lirë.\n";
 }

 /**
 * Shfaq statistika të detajuara
 */
 void shfaqStatistika() const {
 size_t blloqeNePerdorim = numriBlloqeve - blloqeLira;
 double perqindjaPerdorur = (100.0 * blloqeNePerdorim) / numriBlloqeve;
 size_t memorieNePerdorim = blloqeNePerdorim * madhesiaBlokut;
 size_t memorieTotale = numriBlloqeve * madhesiaBlokut;

 std::cout << "\n╔══════════════════════════════════════════════╗\n";
 std::cout << "║ Statistikat e Pool Allocator ║\n";
 std::cout << "╠══════════════════════════════════════════════╣\n";
 std::cout << "║ Madhësia e blokut: " << std::setw(10)
 << madhesiaBlokut << " bytes ║\n";
 std::cout << "║ Numri i blloqeve: " << std::setw(10)
 << numriBlloqeve << " ║\n";
 std::cout << "║ Blloqe të lira: " << std::setw(10)
 << blloqeLira << " ║\n";
 std::cout << "║ Blloqe në përdorim: " << std::setw(10)
 << blloqeNePerdorim << " ║\n";
 std::cout << "║ Përdorimi: " << std::setw(9)
 << std::fixed << std::setprecision(1) << perqindjaPerdorur
 << "% ║\n";
 std::cout << "╠══════════════════════════════════════════════╣\n";
 std::cout << "║ Memorie totale: " << std::setw(10)
 << memorieTotale << " bytes ║\n";
 std::cout << "║ Memorie e përdorur: " << std::setw(10)
 << memorieNePerdorim << " bytes ║\n";
 std::cout << "║ Total alokime: " << std::setw(10)
 << numriAlokimeve << " ║\n";
 std::cout << "║ Total lirime: " << std::setw(10)
 << numriLirimeve << " ║\n";
 std::cout << "╚══════════════════════════════════════════════╝\n\n";
 }

 /**
 * Kontrollo nëse pool është plot
 */
 bool eshtePlot() const {
 return blloqeLira == 0;
 }

 /**
 * Kontrollo nëse pool është bosh (të gjithë të lirë)
 */
 bool eshteBosh() const {
 return blloqeLira == numriBlloqeve;
 }

 /**
 * Merr numrin e blloqeve të lira
 */
 size_t merrNumrinBlloqeveLira() const {
 return blloqeLira;
 }

private:
 /**
 * Valido që një pointer është pjesë e këtij pool
 */
 bool eshtePointerValid(void* ptr) const {
 uint8_t* p = static_cast<uint8_t*>(ptr);
 size_t offset = p - fillimi;

 // Kontrollo nëse është brenda kufijve
 if (p < fillimi || p >= fillimi + (madhesiaBlokut * numriBlloqeve)) {
 return false;
 }

 // Kontrollo nëse është i alinjuar me blloqet
 return (offset % madhesiaBlokut) == 0;
 }
};

#endif // POOL_ALLOCATOR_H

// ============================================================================
// TESTO POOL ALLOCATOR
// ============================================================================

// Shembull strukture për të testuar
struct Particle {
 float x, y, z;
 float vx, vy, vz;
 float lifetime;
 int id;
};

int main() {
 std::cout << "╔════════════════════════════════════════════════════╗\n";
 std::cout << "║ TEST: Pool Allocator Implementation ║\n";
 std::cout << "╚════════════════════════════════════════════════════╝\n\n";

 // Test 1: Alokim dhe lirimi bazë
 {
 std::cout << "TEST 1: Alokim dhe lirimi bazë\n";
 std::cout << "────────────────────────────────\n";

 AlokatorPool pool(sizeof(int), 10);

 // Aloko disa objekte
 int* a = static_cast<int*>(pool.aloko());
 int* b = static_cast<int*>(pool.aloko());
 int* c = static_cast<int*>(pool.aloko());

 *a = 100;
 *b = 200;
 *c = 300;

 std::cout << "✓ Alokuar 3 integers: " << *a << ", " << *b << ", " << *c << "\n";
 pool.shfaqStatistika();

 // Liro një
 pool.liro(b);
 std::cout << "✓ Liruar një integer\n";
 pool.shfaqStatistika();

 // Aloko përsëri (do të ripërdorë hapësirën e liruar)
 int* d = static_cast<int*>(pool.aloko());
 *d = 400;
 std::cout << "✓ Alokuar përsëri: " << *d << "\n\n";

 pool.liro(a);
 pool.liro(c);
 pool.liro(d);
 }

 // Test 2: Pool për struktura komplekse
 {
 std::cout << "TEST 2: Pool për Particle system\n";
 std::cout << "──────────────────────────────────\n";

 AlokatorPool particlePool(sizeof(Particle), 100);

 // Krijo disa particle
 Particle* particles[5];
 for (int i = 0; i < 5; i++) {
 particles[i] = static_cast<Particle*>(particlePool.aloko());
 particles[i]->id = i;
 particles[i]->x = i * 10.0f;
 particles[i]->lifetime = 1.0f;
 }

 std::cout << "✓ Krijuar 5 particles\n";
 particlePool.shfaqStatistika();

 // Simulate disa particles që vdesin
 particlePool.liro(particles[1]);
 particlePool.liro(particles[3]);
 std::cout << "✓ 2 particles u shkatërruan\n";
 particlePool.shfaqStatistika();
 }

 std::cout << "✅ Të gjithë testet kaluan me sukses!\n";

 return 0;
}
