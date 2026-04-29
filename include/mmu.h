#ifndef __MMU_H_
#define __MMU_H_

#include <cstdint>
#include <string>
#include <vector>

enum DataType : uint8_t {FreeSpace, Char, Short, Int, Float, Long, Double};

typedef struct Variable {
    std::string name;
    DataType type;
    uint32_t virtual_address;
    uint32_t size;
} Variable;

typedef struct Process {
    uint32_t pid;
    std::vector<Variable*> variables;
} Process;

class Mmu {
private:
    uint32_t _next_pid;
    uint32_t _max_size;
    std::vector<Process*> _processes;

public:
    Mmu(int memory_size);
    ~Mmu();

    uint32_t createProcess();
    uint32_t getFreeSpace(uint32_t pid, uint32_t size);
    Variable* getVariable(uint32_t pid, std::string var_name);
    std::vector<std::string> getVariableNames(uint32_t pid);
    std::vector<uint32_t> getPids();
    void removeProcess(uint32_t pid);
    bool isPageFree(uint32_t pid, int page_number, int page_size);
    void freeVariable(uint32_t pid, std::string var_name);
    void addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address);
    void print();
};

#endif // __MMU_H_
