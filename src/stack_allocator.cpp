// ============================================================================
// BRANCH: feature/stack-allocator
// ZHVILLUAR NGA: Genti 
// PERSHKRIMI: Implementimi i Stack Allocator me mbështetje për alignment,
//             markers, dhe statistika të detajuara
// ============================================================================

#ifndef STACK_ALLOCATOR_H
#define STACK_ALLOCATOR_H

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <iomanip>

/**
 * AlokatorSteku - Aloktor LIFO (Last In First Out) për memorie të përkohshme
 * 
 * Karakteristikat:
 * - Alokim ultra i shpejtë (vetëm increment pointer)
 * - Nuk ka fragmentation
 * - Mbështetje për memory alignment
 * - Sistem marker për rollback selektiv
 * - Statistika të detajuara të përdorimit
 */
class AlokatorSteku {
private:
    uint8_t* fillimi;              // Pointer në fillim të zones së memories
    size_t madhesiaTotale;         // Madhësia totale e alokuar në bytes
    size_t pozicioniAktual;        // Offset aktual nga fillimi
    size_t numriAlokimeve;         // Numërues i alokimeve të kryera
    size_t maksimumPerdorur;       // Peak memory usage
    
public:
    /**
     * Konstruktori - inicializon buffer-in e memories
     * @param madhesia: Madhësia totale në bytes
     */
    explicit AlokatorSteku(size_t madhesia) 
        : madhesiaTotale(madhesia), 
          pozicioniAktual(0),
          numriAlokimeve(0),
          maksimumPerdorur(0) {
        fillimi = new uint8_t[madhesia];
        std::cout << "[Stack Allocator] U krijua me " << madhesia 
                  << " bytes (" << (madhesia / 1024.0) << " KB)\n";
    }
    
    /**
     * Destruktori - liron memorien
     */
    ~AlokatorSteku() {
        delete[] fillimi;
        std::cout << "[Stack Allocator] U shkatërrua. Total alokime: " 
                  << numriAlokimeve << "\n";
    }
    
    /**
     * Aloko memorie me alignment të specifikuar
     * @param madhesia: Madhësia e kërkuar në bytes
     * @param alinjimi: Alignment requirement (duhet të jetë fuqi e 2)
     * @return Pointer në memorien e alokuar, ose nullptr nëse dështon
     */
    void* aloko(size_t madhesia, size_t alinjimi = 8) {
        // Valido alinjimin
        assert(eshteAlinjimValid(alinjimi) && "Alinjimi duhet të jetë fuqi e 2");
        
        // Llogarit pozicionin e alinjuar
        size_t pozicioniAlinjuar = alinjoPozicionin(pozicioniAktual, alinjimi);
        size_t padding = pozicioniAlinjuar - pozicioniAktual;
        
        // Kontrollo hapësirën e disponueshme
        if (pozicioniAlinjuar + madhesia > madhesiaTotale) {
            std::cerr << "[Stack Allocator] GABIM: Jo memorie e mjaftueshme! "
                      << "Kërkuar: " << madhesia << " bytes, "
                      << "Disponueshme: " << (madhesiaTotale - pozicioniAktual) 
                      << " bytes\n";
            return nullptr;
        }
        
        // Ruaj pointer-in për return
        void* adresa = fillimi + pozicioniAlinjuar;
        
        // Përditëso state
        pozicioniAktual = pozicioniAlinjuar + madhesia;
        numriAlokimeve++;
        
        // Track peak usage
        if (pozicioniAktual > maksimumPerdorur) {
            maksimumPerdorur = pozicioniAktual;
        }
        
        #ifdef DEBUG_ALLOCATOR
        std::cout << "[Stack Allocator] Alokuar " << madhesia << " bytes "
                  << "(padding: " << padding << ") në offset " 
                  << pozicioniAlinjuar << "\n";
        #endif
        
        return adresa;
    }
    
    /**
     * Reseto të gjithë stekun - fshi të gjitha alokimet
     */
    void reseto() {
        pozicioniAktual = 0;
        #ifdef DEBUG_ALLOCATOR
        std::cout << "[Stack Allocator] U rivendos. Alokime të fshira: " 
                  << numriAlokimeve << "\n";
        #endif
        numriAlokimeve = 0;
    }
    
    /**
     * Merr një marker (snapshot) të pozicionit aktual
     * @return Marker që mund të përdoret për rollback
     */
    size_t marrShenim() const {
        return pozicioniAktual;
    }
    
    /**
     * Kthehu në një pozicion të mëparshëm duke përdorur marker
     * @param shenim: Marker i marrë më parë me marrShenim()
     */
    void kthehuNeShenim(size_t shenim) {
        if (shenim <= pozicioniAktual && shenim <= madhesiaTotale) {
            pozicioniAktual = shenim;
            #ifdef DEBUG_ALLOCATOR
            std::cout << "[Stack Allocator] U kthye në shenimin " << shenim << "\n";
            #endif
        } else {
            std::cerr << "[Stack Allocator] GABIM: Shenim i pavlefshëm: " 
                      << shenim << "\n";
        }
    }
    
    /**
     * Shfaq statistika të detajuara të alokatorit
     */
    void shfaqStatistika() const {
        std::cout << "\n╔══════════════════════════════════════════════╗\n";
        std::cout << "║     Statistikat e Stack Allocator          ║\n";
        std::cout << "╠══════════════════════════════════════════════╣\n";
        std::cout << "║ Madhësia totale:     " << std::setw(10) 
                  << madhesiaTotale << " bytes  ║\n";
        std::cout << "║ E përdorur:          " << std::setw(10) 
                  << pozicioniAktual << " bytes  ║\n";
        std::cout << "║ E lirë:              " << std::setw(10) 
                  << (madhesiaTotale - pozicioniAktual) << " bytes  ║\n";
        std::cout << "║ Peak usage:          " << std::setw(10) 
                  << maksimumPerdorur << " bytes  ║\n";
        std::cout << "║ Numri i alokimeve:   " << std::setw(10) 
                  << numriAlokimeve << "         ║\n";
        
        double perqindja = (100.0 * pozicioniAktual) / madhesiaTotale;
        std::cout << "║ Përdorimi:           " << std::setw(9) 
                  << std::fixed << std::setprecision(2) << perqindja << "%        ║\n";
        std::cout << "╚══════════════════════════════════════════════╝\n\n";
    }
    
    /**
     * Kontrollo nëse ka memorie të disponueshme
     * @param madhesia: Madhësia për të kontrolluar
     * @return true nëse ka vend të mjaftueshëm
     */
    bool kaHapesire(size_t madhesia) const {
        return (pozicioniAktual + madhesia) <= madhesiaTotale;
    }
    
    /**
     * Merr madhësinë e lirë
     */
    size_t merrMadhesiLire() const {
        return madhesiaTotale - pozicioniAktual;
    }
    
private:
    /**
     * Alinjo një pozicion sipas kërkesës
     */
    size_t alinjoPozicionin(size_t pozicioni, size_t alinjimi) const {
        size_t mbetja = pozicioni & (alinjimi - 1);
        if (mbetja != 0) {
            pozicioni += alinjimi - mbetja;
        }
        return pozicioni;
    }
    
    /**
     * Kontrollo nëse alinjimi është fuqi e 2
     */
    bool eshteAlinjimValid(size_t alinjimi) const {
        return (alinjimi > 0) && ((alinjimi & (alinjimi - 1)) == 0);
    }
};

#endif // STACK_ALLOCATOR_H

// ============================================================================
// TESTO STACK ALLOCATOR
// ============================================================================

int main() {
    std::cout << "╔════════════════════════════════════════════════════╗\n";
    std::cout << "║      TEST: Stack Allocator Implementation        ║\n";
    std::cout << "╚════════════════════════════════════════════════════╝\n\n";
    
    // Test 1: Alokim bazë
    {
        std::cout << "TEST 1: Alokim bazë\n";
        std::cout << "─────────────────────\n";
        AlokatorSteku alokatori(1024);
        
        int* numri = static_cast<int*>(alokatori.aloko(sizeof(int)));
        *numri = 42;
        std::cout << "✓ Alokuar int: " << *numri << "\n";
        
        double* array = static_cast<double*>(alokatori.aloko(sizeof(double) * 5));
        for (int i = 0; i < 5; i++) {
            array[i] = i * 1.5;
        }
        std::cout << "✓ Alokuar array[5]: ";
        for (int i = 0; i < 5; i++) {
            std::cout << array[i] << " ";
        }
        std::cout << "\n\n";
        
        alokatori.shfaqStatistika();
    }
    
    // Test 2: Markers dhe rollback
    {
        std::cout << "TEST 2: Markers dhe Rollback\n";
        std::cout << "─────────────────────────────\n";
        AlokatorSteku alokatori(2048);
        
        int* data1 = static_cast<int*>(alokatori.aloko(sizeof(int) * 10));
        std::cout << "✓ Alokuar 10 integers\n";
        
        size_t marker = alokatori.marrShenim();
        std::cout << "✓ Ruajtur marker\n";
        
        char* temp = static_cast<char*>(alokatori.aloko(500));
        std::cout << "✓ Alokuar 500 bytes të përkohshme\n";
        alokatori.shfaqStatistika();
        
        alokatori.kthehuNeShenim(marker);
        std::cout << "✓ U kthye në marker (500 bytes u liruan)\n";
        alokatori.shfaqStatistika();
    }
    
    std::cout << "✅ Të gjithë testet kaluan me sukses!\n";
    
    return 0;
}