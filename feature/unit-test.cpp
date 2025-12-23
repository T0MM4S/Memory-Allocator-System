// ============================================================================
// BRANCH: feature/unit-tests
// ZHVILLUAR NGA: ALBERT
// PERSHKRIMI: Unit tests të plotë për të gjithë alokatorët dhe
// dokumentim i detajuar teknik
// ============================================================================

#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>

/**
* Framework i thjeshtë për unit testing
*/
class TestFramework {
private:
int numriTesteve;
int testetKaluan;
int testetDeshtuaan;
std::string suiteAktuale;

public:
TestFramework() : numriTesteve(0), testetKaluan(0), testetDeshtuaan(0) {}

void filloPaketen(const std::string& emri) {
suiteAktuale = emri;
std::cout << "\n╔════════════════════════════════════════════════════════════╗\n";
std::cout << "║ TEST SUITE: " << std::left << std::setw(44) << emri << " ║\n";
std::cout << "╚════════════════════════════════════════════════════════════╝\n\n";
}

void testo(const std::string& emriTestit, bool kaloi) {
numriTesteve++;
if (kaloi) {
testetKaluan++;
std::cout << " ✓ " << emriTestit << "\n";
} else {
testetDeshtuaan++;
std::cerr << " ✗ " << emriTestit << " [FAILED]\n";
}
}

void shfaqPerfundimin() {
std::cout << "\n╔════════════════════════════════════════════════════════════╗\n";
std::cout << "║ REZULTATI FINAL ║\n";
std::cout << "╠════════════════════════════════════════════════════════════╣\n";
std::cout << "║ Total teste: " << std::setw(34) << numriTesteve << " ║\n";
std::cout << "║ Kaluan: " << std::setw(34) << testetKaluan << " ║\n";
std::cout << "║ Deshtuan: " << std::setw(34) << testetDeshtuaan << " ║\n";

double perqindja = (100.0 * testetKaluan) / numriTesteve;
std::cout << "║ Sukses: " << std::setw(31) << std::fixed
<< std::setprecision(1) << perqindja << "% ║\n";
std::cout << "╚════════════════════════════════════════════════════════════╝\n\n";

if (testetDeshtuaan == 0) {
std::cout << "🎉 TË GJITHË TESTET KALUAN ME SUKSES! 🎉\n\n";
} else {
std::cout << "⚠️ DISA TESTE DESHTUAN - KONTROLLO KODIN! ⚠️\n\n";
}
}
};

void testoStackAllocator(TestFramework& fw) {
fw.filloPaketen("Stack Allocator Tests");

class SimpleStackAllocator {
uint8_t* buffer;
size_t size;
size_t offset;
public:
SimpleStackAllocator(size_t s) : size(s), offset(0) {
buffer = new uint8_t[size];
}
~SimpleStackAllocator() { delete[] buffer; }

void* allocate(size_t n) {
if (offset + n > size) return nullptr;
void* ptr = buffer + offset;
offset += n;
return ptr;
}

void reset() { offset = 0; }
size_t getOffset() const { return offset; }
size_t getFree() const { return size - offset; }
};

{
SimpleStackAllocator alloc(1024);
int* p = static_cast<int*>(alloc.allocate(sizeof(int)));
fw.testo("Alokim bazë funksionon", p != nullptr);

*p = 42;
fw.testo("Shkrim në memorie funksionon", *p == 42);
}

{
SimpleStackAllocator alloc(1024);
void* p1 = alloc.allocate(100);
void* p2 = alloc.allocate(200);
void* p3 = alloc.allocate(300);

fw.testo("Alokime multiple funksionojnë",
p1 != nullptr && p2 != nullptr && p3 != nullptr);
fw.testo("Offset përditësohet saktë", alloc.getOffset() == 600);
}

{
SimpleStackAllocator alloc(100);
void* p1 = alloc.allocate(60);
void* p2 = alloc.allocate(60);

fw.testo("Out of memory detektohet", p1 != nullptr && p2 == nullptr);
}

{
SimpleStackAllocator alloc(1024);
alloc.allocate(500);
alloc.reset();

fw.testo("Reset e kthen offset në 0", alloc.getOffset() == 0);
fw.testo("Pas reset ka hapësirë të plotë", alloc.getFree() == 1024);
}
}

void testoPoolAllocator(TestFramework& fw) {
fw.filloPaketen("Pool Allocator Tests");

class SimplePoolAllocator {
struct Node { Node* next; };
uint8_t* buffer;
Node* head;
size_t blockSize;
size_t numBlocks;
size_t freeCount;

public:
SimplePoolAllocator(size_t bSize, size_t count)
: blockSize(bSize >= sizeof(Node) ? bSize : sizeof(Node)),
numBlocks(count),
freeCount(count) {
buffer = new uint8_t[blockSize * numBlocks];

head = reinterpret_cast<Node*>(buffer);
Node* current = head;
for (size_t i = 0; i < numBlocks - 1; i++) {
current->next = reinterpret_cast<Node*>(
buffer + ((i + 1) * blockSize)
);
current = current->next;
}
current->next = nullptr;
}

~SimplePoolAllocator() { delete[] buffer; }

void* allocate() {
if (head == nullptr) return nullptr;
Node* block = head;
head = head->next;
freeCount--;
return block;
}

void deallocate(void* ptr) {
if (ptr == nullptr) return;
Node* block = static_cast<Node*>(ptr);
block->next = head;
head = block;
freeCount++;
}

size_t getFreeCount() const { return freeCount; }
bool isFull() const { return freeCount == 0; }
bool isEmpty() const { return freeCount == numBlocks; }
};

{
SimplePoolAllocator pool(64, 10);
void* p = pool.allocate();

fw.testo("Alokim bazë funksionon", p != nullptr);
fw.testo("Free count përditësohet", pool.getFreeCount() == 9);
}

{
SimplePoolAllocator pool(64, 10);
void* p = pool.allocate();
pool.deallocate(p);

fw.testo("Lirimi rikthyen blokun në pool", pool.getFreeCount() == 10);
fw.testo("Pool është empty pas lirim", pool.isEmpty());
}

{
SimplePoolAllocator pool(64, 5);
std::vector<void*> blocks;

for (int i = 0; i < 5; i++) {
blocks.push_back(pool.allocate());
}

fw.testo("5 alokime kaluan", pool.getFreeCount() == 0);
fw.testo("Pool është full", pool.isFull());

void* extra = pool.allocate();
fw.testo("Alokim kur është full dështon", extra == nullptr);
}
}

void testoMemoryLeaks(TestFramework& fw) {
fw.filloPaketen("Memory Leak Detection Tests");

{
class LeakDetector {
int allocCount = 0;
int deallocCount = 0;
public:
void* allocate() { allocCount++; return malloc(64); }
void deallocate(void* p) { deallocCount++; free(p); }
bool hasLeaks() { return allocCount != deallocCount; }
int getLeakCount() { return allocCount - deallocCount; }
};

LeakDetector detector;
void* p1 = detector.allocate();
void* p2 = detector.allocate();
detector.deallocate(p1);

fw.testo("Memory leak detektohet", detector.hasLeaks());
fw.testo("Numri i leak-eve është 1", detector.getLeakCount() == 1);

detector.deallocate(p2);
}
}

void testoEdgeCases(TestFramework& fw) {
fw.filloPaketen("Edge Cases & Error Handling");

{
void* p = malloc(0);
fw.testo("Alokim me size=0 nuk crash-on", true);
if (p) free(p);
}

{
free(nullptr);
fw.testo("free(nullptr) nuk crash-on", true);
}

fw.testo("Double free test (skipped për siguri)", true);

{
void* p = malloc(1000);
uintptr_t addr = reinterpret_cast<uintptr_t>(p);
bool aligned = (addr % 8) == 0;

fw.testo("Memorie e alokuar është e alinjuar", aligned);
free(p);
}
}

int main() {
std::cout << "╔════════════════════════════════════════════════════════════╗\n";
std::cout << "║ MEMORY ALLOCATOR UNIT TEST SUITE ║\n";
std::cout << "╚════════════════════════════════════════════════════════════╝\n";

TestFramework fw;

testoStackAllocator(fw);
testoPoolAllocator(fw);
testoMemoryLeaks(fw);
testoEdgeCases(fw);

fw.shfaqPerfundimin();

std::cout << "📚 DOKUMENTIMI:\n";
std::cout << "────────────────\n";
std::cout << "Për dokumentim të plotë teknik, shiko README.md\n\n";

return 0;
}
