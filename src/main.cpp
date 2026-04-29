#include <iostream>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include "mmu.h"
#include "pagetable.h"

// 64 MB (64 * 1024 * 1024)
#define PHYSICAL_MEMORY 67108864

void printStartMessage(int page_size);
void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table);
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table, bool print_addr);
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, std::string value, Mmu *mmu, PageTable *page_table, uint8_t *memory);
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table);
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table);

int main(int argc, char **argv)
{
    // Ensure user specified page size as a command line parameter
    if (argc < 2)
    {
        std::cerr << "Error: you must specify the page size" << std::endl;
        return 1;
    }

    // Print opening instuction message
    int page_size = std::stoi(argv[1]);
    printStartMessage(page_size);

    // Create physical 'memory' (raw array of bytes)
    uint8_t *memory = new uint8_t[PHYSICAL_MEMORY];

    // Create MMU and Page Table
    Mmu *mmu = new Mmu(PHYSICAL_MEMORY);
    PageTable *page_table = new PageTable(page_size);

    // Prompt loop
    std::string command;
    std::cout << "> ";
    std::getline(std::cin, command);
    while (command != "exit")
    {
        // Handle command
        // TODO: implement this!
        std::stringstream ss(command);
        std::string cmd;
        ss >> cmd;

        if (cmd == "create")
        {
            int text_size, data_size;
            ss >> text_size >> data_size;
            createProcess(text_size, data_size, mmu, page_table);
        }

        else if (cmd == "allocate")
        {
            uint32_t pid;

            std::string var_name;
            std::string type_string;
            uint32_t num_elements;

            ss >> pid >> var_name >> type_string >> num_elements;
            DataType type;

            if (type_string == "char") type = DataType::Char;

            else if (type_string == "short") type = DataType::Short;

            else if (type_string == "int") type = DataType::Int;

            else if (type_string == "float") type = DataType::Float;

            else if (type_string == "long") type = DataType::Long;

            else if (type_string == "double") type = DataType::Double;

            else
            {
                std::cout << "error: invalid data type" << std::endl;
                std::cout << "> ";
                std::getline(std::cin, command);
                continue;
            }

            allocateVariable(pid, var_name, type, num_elements, mmu, page_table, true);
        }

        else if (cmd == "set")
        {
            uint32_t pid;
            std::string var_name;
            uint32_t offset;

            ss >> pid >> var_name >> offset;

            std::string value;
            uint32_t current_offset = offset;

            while (ss >> value)
            {
                setVariable(pid, var_name, current_offset, value, mmu, page_table, memory);
                current_offset++;
            }
        }

        else if (cmd == "free")
        {
            uint32_t pid;

            std::string var_name;
            ss >> pid >> var_name;
            freeVariable(pid, var_name, mmu, page_table);
        }

        else if (cmd == "terminate")
        {
            uint32_t pid;
            ss >> pid;
            terminateProcess(pid, mmu, page_table);
        }

        else if (cmd == "print")
        {
            std::string object;
            ss >> object;

            if (object == "mmu") mmu->print();

            else if (object == "page") page_table->print();
        
            else if (object == "processes")
            {
                std::vector<uint32_t> pids = mmu->getPids();

                for (int i = 0; i < pids.size(); i++)
                {
                    std::cout << pids[i] << std::endl;
                }
            }

            else
            {
                size_t colon_pos = object.find(":");

                if (colon_pos != std::string::npos)
                {
                    uint32_t pid = std::stoi(object.substr(0, colon_pos));
                    std::string var_name = object.substr(colon_pos + 1);
                    Variable *var = mmu->getVariable(pid, var_name);
                    
                    if (var == NULL)
                    {
                        std::cout << "error: variable not found" << std::endl;
                    }

                    else
                    {
                        uint32_t element_size = 1;
                        if (var->type == DataType::Short) element_size = 2;
                        else if (var->type == DataType::Int || var->type == DataType::Float) element_size = 4;
                        else if (var->type == DataType::Long || var->type == DataType::Double) element_size = 8;

                        for (uint32_t i = 0; i < var->size / element_size; i++)
                        {
                            uint32_t virtual_addr = var->virtual_address + (i * element_size);
                            int physical_addr = page_table->getPhysicalAddress(pid, virtual_addr);

                            if (i > 0) std::cout << ", ";

                            if (var->type == DataType::Char) std::cout << *(char*)(memory + physical_addr);
                            else if (var->type == DataType::Int) std::cout << *(int*)(memory + physical_addr);
                            else if (var->type == DataType::Double) std::cout << *(double*)(memory + physical_addr);
                        }

                        std::cout << std::endl;
                    }
                }
                else
                {
                    std::cout << "error: command not recognized" << std::endl;
                }
            }
        }

        else
        {
            std::cout << "error: command not recognized" << std::endl;
        }

        // Get next command
        std::cout << "> ";
        std::getline(std::cin, command);
    }

    // Cean up
    delete[] memory;
    delete mmu;
    delete page_table;

    return 0;
}

void printStartMessage(int page_size)
{
    std::cout << "Welcome to the Memory Allocation Simulator! Using a page size of " << page_size << " bytes." << std:: endl;
    std::cout << "Commands:" << std:: endl;
    std::cout << "  * create <text_size> <data_size> (initializes a new process)" << std:: endl;
    std::cout << "  * allocate <PID> <var_name> <data_type> <number_of_elements> (allocated memory on the heap)" << std:: endl;
    std::cout << "  * set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N> (set the value for a variable)" << std:: endl;
    std::cout << "  * free <PID> <var_name> (deallocate memory on the heap that is associated with <var_name>)" << std:: endl;
    std::cout << "  * terminate <PID> (kill the specified process)" << std:: endl;
    std::cout << "  * print <object> (prints data)" << std:: endl;
    std::cout << "    * If <object> is \"mmu\", print the MMU memory table" << std:: endl;
    std::cout << "    * if <object> is \"page\", print the page table" << std:: endl;
    std::cout << "    * if <object> is \"processes\", print a list of PIDs for processes that are still running" << std:: endl;
    std::cout << "    * if <object> is a \"<PID>:<var_name>\", print the value of the variable for that process" << std:: endl;
    std::cout << std::endl;
}

void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table)
{
    uint32_t pid = mmu->createProcess();
    allocateVariable(pid, "<TEXT>", DataType::Char, text_size, mmu, page_table, false);
    allocateVariable(pid, "<GLOBALS>", DataType::Char, data_size, mmu, page_table, false);
    allocateVariable(pid, "<STACK>", DataType::Char, 65536, mmu, page_table, false);
    std::cout << pid << std::endl;
}

void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table, bool print_addr)
{
    uint32_t var_size = num_elements; // default to char size
    
    if (!mmu->processExists(pid))
    {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    if (mmu->variableExists(pid, var_name))
    {
        std::cout << "error: variable already exists" << std::endl;
        return;
    }

    if (type == DataType::Short) var_size = num_elements * 2;
    else if (type == DataType::Int || type == DataType::Float) var_size = num_elements * 4;
    else if (type == DataType::Long || type == DataType::Double) var_size = num_elements * 8;

    uint32_t virtual_address = mmu->getFreeSpace(pid, var_size);
    if (virtual_address == (uint32_t)-1)
    {
        std::cout << "error: not enough memory" << std::endl;
        return;
    }

    // find how many pages needed for variable (see if we need to add any)
    int page_size = page_table->getPageSize();
    int first_page = virtual_address / page_size;
    int last_page = (virtual_address + var_size - 1) / page_size;

    for (int i = first_page; i <= last_page; i++)
    {
        if (!page_table->hasEntry(pid, i)) page_table->addEntry(pid, i);
    }

    mmu->addVariableToProcess(pid, var_name, type, var_size, virtual_address);
    if (print_addr)
    {
        std::cout << virtual_address << std::endl;
    }
}

void setVariable(uint32_t pid, std::string var_name, uint32_t offset, std::string value, Mmu *mmu, PageTable *page_table, uint8_t *memory)
{
    if (!mmu->processExists(pid))
    {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    Variable *var = mmu->getVariable(pid, var_name);
    if (var == NULL)
    {
        std::cout << "error: variable not found" << std::endl;
        return;
    }

    uint32_t element_size = 1;
    if (var->type == DataType::Short) element_size = 2;
    else if (var->type == DataType::Int || var->type == DataType::Float) element_size = 4;
    else if (var->type == DataType::Long || var->type == DataType::Double) element_size = 8;

    if ((offset + 1) * element_size > var->size)
    {
        std::cout << "error: index out of range" << std::endl;
        return;
    }

    uint32_t element_virtual_address = var->virtual_address + (offset * element_size);
    int physical_address = page_table->getPhysicalAddress(pid, element_virtual_address);

    if (var->type == DataType::Char) 
    {
        char v = value[0];
        memcpy(memory + physical_address, &v, element_size);
    }

    else if (var->type == DataType::Int) 
    {
        int v = std::stoi(value);
        memcpy(memory + physical_address, &v, element_size);
    }

    else if (var->type == DataType::Float) 
    {
        float v = std::stof(value);
        memcpy(memory + physical_address, &v, element_size);
    }

    else if (var->type == DataType::Long) 
    {
        long v = std::stol(value);
        memcpy(memory + physical_address, &v, element_size);
    }

    else if (var->type == DataType::Double) 
    {
        double v = std::stod(value);
        memcpy(memory + physical_address, &v, element_size);
    }

    else if (var->type == DataType::Short) 
    {
        short v = std::stoi(value);
        memcpy(memory + physical_address, &v, element_size);
    }
}

void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table)
{
    if (!mmu->processExists(pid))
    {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    Variable *var = mmu->getVariable(pid, var_name);
    if (var == NULL)
    {
        std::cout << "error: variable not found" << std::endl;
        return;
    }

    mmu->freeVariable(pid, var_name);
    int page_size = page_table->getPageSize();
    int first_page = var->virtual_address / page_size;
    int last_page = (var->virtual_address + var->size - 1) / page_size;

    for (int i = first_page; i <= last_page; i++)
    {
        if (mmu->isPageFree(pid, i, page_size)) page_table->removeEntry(pid, i);
    }
}

void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table)
{
    if (!mmu->processExists(pid))
    {
        std::cout << "error: process not found" << std::endl;
        return;
    }
    
    std::vector<std::string> variables = mmu->getVariableNames(pid);
    for (int i = 0; i < variables.size(); i++)
    {
        freeVariable(pid, variables[i], mmu, page_table);
    }
    mmu->removeProcess(pid);
}
