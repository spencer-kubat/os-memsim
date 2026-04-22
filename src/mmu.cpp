#include <iostream>
#include <algorithm>
#include "mmu.h"
#include <cstdint>

Mmu::Mmu(int memory_size)
{
    _next_pid = 1024; 
    _max_size = memory_size;
}

Mmu::~Mmu()
{
    
}

uint32_t Mmu::createProcess()
{
    Process *proc = new Process();
    proc->pid = _next_pid;

    Variable *var = new Variable();
    var->name = "<FREE_SPACE>";
    var->type = DataType::FreeSpace;
    var->virtual_address = 0;
    var->size = _max_size;
    proc->variables.push_back(var);

    _processes.push_back(proc);

    _next_pid++;
    
    return proc->pid;
}

uint32_t Mmu::getFreeSpace(uint32_t pid, uint32_t size)
{
    Process *proc = NULL;
    for (int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
            break;    
        }
    }

    for (int i = 0; i < proc->variables.size(); i++)
    {
        Variable *var = proc->variables[i];
        if (var->type == DataType::FreeSpace && var->size >= size)
        {
            uint32_t address = var->virtual_address;

            // shrink/shift free space to accomodate size
            var->virtual_address += size;
            var->size -= size;
            return address;
        }
    }

    return -1;
}

void Mmu::addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address)
{
    int i;
    Process *proc = NULL;
    std::vector<Process*>::iterator it = std::find_if(_processes.begin(), _processes.end(), [pid](Process* p)
    { 
        return p != nullptr && p->pid == pid; 
    });

    proc = *it;
    if (it != _processes.end())
    {
        Variable *var = new Variable();
        var->name = var_name;
        var->type = type;
        var->virtual_address = address;
        var->size = size;
        proc->variables.push_back(var);
    }
}

void Mmu::print()
{
    int i, j;

    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;
    for (i = 0; i < _processes.size(); i++)
    {
        for (j = 0; j < _processes[i]->variables.size(); j++)
        {
            // TODO: print all variables (excluding those of type DataType::FreeSpace)
        }
    }
}
