#include <iostream>
#include <algorithm>
#include "mmu.h"
#include <cstdint>
#include <sstream>

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

Variable* Mmu::getVariable(uint32_t pid, std::string var_name)
{
    for (int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            for (int j = 0; j < _processes[i]->variables.size(); j++)
            {
                if (_processes[i]->variables[j]->name == var_name)
                {
                    return _processes[i]->variables[j];
                }
            }
        }
    }
    return NULL;
}

void Mmu::removeProcess(uint32_t pid)
{
    for (int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            // free each variable in the process
            for (int j = 0; j < _processes[i]->variables.size(); j++)
            {
                delete _processes[i]->variables[j];
            }
            delete _processes[i];
            _processes.erase(_processes.begin() + i);
            return;
        }
    }
}

std::vector<std::string> Mmu::getVariableNames(uint32_t pid)
{
    std::vector<std::string> names;
    for (int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            for (int j = 0; j < _processes[i]->variables.size(); j++)
            {
                // skip free space entries
                if (_processes[i]->variables[j]->type != DataType::FreeSpace)
                {
                    names.push_back(_processes[i]->variables[j]->name);
                }
            }
        }
    }
    return names;
}

void Mmu::freeVariable(uint32_t pid, std::string var_name)
{
    for (int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            for (int j = 0; j < _processes[i]->variables.size(); j++)
            {
                if (_processes[i]->variables[j]->name == var_name)
                {
                    // convert to free space instead of removing
                    _processes[i]->variables[j]->name = "<FREE_SPACE>";
                    _processes[i]->variables[j]->type = DataType::FreeSpace;
                    return;
                }
            }
        }
    }
}

bool Mmu::isPageFree(uint32_t pid, int page_number, int page_size)
{
    int page_start = page_number * page_size;
    int page_end = page_start + page_size - 1;
    for (int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            for (int j = 0; j < _processes[i]->variables.size(); j++)
            {
                Variable *var = _processes[i]->variables[j];
                // skip free space entries
                if (var->type == DataType::FreeSpace) continue;

                // check if variable overlaps with page
                uint32_t var_start = var->virtual_address;
                uint32_t var_end = var->virtual_address + var->size - 1;

                if (var_start <= page_end && var_end >= page_start) return false; // page still has content
            }
        }
    }
    return true; // nothing on this page
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
