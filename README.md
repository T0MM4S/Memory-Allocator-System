#  Sistema e Alokatorëve të Memories - Memory Allocator System

Një librari moderne e alokatorëve të memories e shkruar në C++ , e dizajnuar për performancë të lartë dhe menaxhim efikas të memories.

##  Përmbledhje

Ky projekt ofron implementime të ndryshme të alokatorëve të memories, secili i optimizuar për raste specifike përdorimi. Sistemi përfshin stack allocator, pool allocator, benchmark tools, dhe unit tests për të garantuar cilësi dhe performancë.

##  Karakteristikat

- **Stack Allocator** - Alokim ultra i shpejtë LIFO për të dhëna të përkohshme
- **Pool Allocator** - Menaxhim efikas i objekteve me madhësi fikse
- **Performance Benchmarking** - Krahasim performancash me alokatorët standard
- **Unit Testing** - Test të plotë për të gjitha komponentët
- **Thread Safety** - Mbështetje për programim multi-threaded
- **Memory Visualization** - Vizualizim i përdorimit të memories

##  Struktura e Projektit
```
memory-allocator-system/
├── src/
│   ├── stack_allocator.cpp    
│   ├── pool_allocator.cpp     
│   ├── benchmark.cpp          
│   └── tests.cpp              
├── include/
│   ├── stack_allocator.h
│   ├── pool_allocator.h
│   └── allocator_common.h
├── docs/
│   └── architecture.md
├── README.md
└── CMakeLists.txt
```

##  Si të Fillosh

### Parakushtet

- C++17 ose më i ri
- CMake 3.15+
- Kompajler modern (GCC 9+, Clang 10+, MSVC 2019+)

### Instalimi
```bash
# Klono repository-n
git clone https://github.com/T0MM4S/memory-allocator-system.git
cd memory-allocator-system

# Krijo build directory
mkdir build && cd build

# Konfiguro me CMake
cmake ..

# Kompilo projektin
make

# Ekzekuto testet
./tests

# Ekzekuto benchmarks
./benchmark
```

## Shembuj Përdorimi

### Stack Allocator
```cpp
#include "stack_allocator.h"

AlokatorSteku alokatori(1024 * 1024); // 1MB

// Aloko memorie
int* data = static_cast<int*>(alokatori.aloko(sizeof(int) * 100));

// Perdor marker per rollback
size_t marker = alokatori.marrShenim();
// ... aloko me shume memorie ...
alokatori.kthehuNeShenim(marker); 

// Reseto te gjitha
alokatori.reseto();
```

### Pool Allocator
```cpp
#include "pool_allocator.h"

// Pool per objekte 64-byte
AlokatorPool pool(64, 1000); // 1000 objekte

void* obj = pool.aloko();
// ... perdor objektin ...
pool.liro(obj);
```

##  Branch Strategy dhe Workflow

Projekti përdor një strategji branch për zhvillim paralel:

### Branches Kryesore

- `main` - Branch i qëndrueshëm, vetëm kod i testuar
- `feature/stack-allocator` - Implementimi i stack allocator
- `feature/pool-allocator` - Implementimi i pool allocator
- `feature/benchmarking` - Sistema e testimit të performancës
- `feature/unit-tests` - Unit tests dhe dokumentim

### Workflow i Kontributit

1. **Krijo një branch të ri** nga `main`:
```bash
   git checkout main
   git pull origin main
   git checkout -b feature/emri-i-features
```

2. **Zhvillo dhe testo** ndryshimet e tua lokalisht

3. **Commit me mesazhe të qarta**:
```bash
   git add .
   git commit -m "feat: shto implementimin e pool allocator"
```

4. **Push në GitHub**:
```bash
   git push origin feature/emri-i-features
```

5. **Hap një Pull Request**:
   - Shko në GitHub repository
   - Kliko "New Pull Request"
   - Zgjidh branch-in tënd
   - Përshkruaj ndryshimet
   - Kërko code review

6. **Code Review dhe Merge**:
   - Anëtarët e tjerë rishikojnë kodin
   - Adreso komentet dhe sugjerimet
   - Pas aprovimit, merge në `main`



### Ekipi i Zhvillimit

- Gent - Stack Allocator Implementation (`feature/stack-allocator`)
- Yllka - Pool Allocator Implementation (`feature/pool-allocator`)
- Ard - Benchmarking System (`feature/benchmarking`)
- Albert - Unit Tests & Documentation (`feature/unit-tests`)

##  Commit Message Guidelines

Përdorni format të qartë për commit messages:
```
<type>: <përshkrim i shkurtër>

<përshkrim i detajuar (opsional)>
```

**Types:**
- `feat`: Feature i ri
- `fix`: Bug fix
- `docs`: Dokumentim
- `test`: Shtim/modifikim i testeve
- `refactor`: Refactoring i kodit
- `perf`: Përmirësime performancash

**Shembuj:**
```
feat: shto stack allocator me mbështetje për alignment
fix: ndreg memory leak në pool allocator
docs: përditëso README me shembuj përdorimi
test: shto unit tests për edge cases
```

##  Testing
```bash
# Ekzekuto të gjithë testet
./tests

# Ekzekuto benchmarks
./benchmark --iterations 10000

# Kontrollo memory leaks (me valgrind)
valgrind --leak-check=full ./tests
```

## Performance

Rezultate nga benchmarks (CPU: i7-10700K, RAM: 32GB):

-------------------------------------------------
| Aloktor | Alokim | Dealokim | Memory Overhead |
|---------|--------|----------|-----------------|
| Stack   | 5ns    | 1ns      | 0%              |
| Pool    | 12ns   | 8ns      | 5%              |
| malloc  | 85ns   | 72ns     | 15%             |
-------------------------------------------------


Ky projekt është zhvilluar për qëllime edukative.
Hap një Pull Request

 ## Kontakt

Për pyetje ose probleme, hap një issue në GitHub.

---

**Note**: Ky është një projekt akademik i zhvilluar si pjesë e detyres te dhene per Menaxhim të Kodit Burimor në Universitetin e Inxhinierise Elektrike dhe Kompjuterike, Departamenti : Teknologji e Informacionit dhe Komunikimit.

```





