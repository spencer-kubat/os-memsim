#include <algorithm>
#include "pagetable.h"
#include <cstdint>
#include <sstream>
#include <iostream>
#include <iomanip>

PageTable::PageTable(int page_size)
{
    _page_size = page_size;
}

PageTable::~PageTable()
{
}

std::vector<std::string> PageTable::sortedKeys()
{
    std::vector<std::string> keys;

    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        keys.push_back(it->first);
    }

    std::sort(keys.begin(), keys.end(), PageTableKeyComparator());

    return keys;
}

void PageTable::addEntry(uint32_t pid, int page_number)
{
    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);

    int total_frames = 67108864 / _page_size;
    std::vector<bool> used_frames(total_frames, false);
    
    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        used_frames[it->second] = true;
    }

    int frame = -1; 
    // Find free frame
    for (int i = 0; i < total_frames; i++)
    {
        if (!used_frames[i])
        {
            frame = i;
            break;
        }
    }

    if (frame == -1)
    {
        std::cout << "error: out of physical memory" << std::endl;
        return;
    }

    _table[entry] = frame;
}

void PageTable::removeEntry(uint32_t pid, int page_number)
{
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    _table.erase(entry);
}

int PageTable::getPageSize()
{
    return _page_size;
}

bool PageTable::hasEntry(uint32_t pid, int page_number)
{
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);    // Maps page to frame
    return _table.count(entry) > 0;
}

int PageTable::getPhysicalAddress(uint32_t pid, uint32_t virtual_address)
{
    // Convert virtual address to page_number and page_offset
    int page_number = virtual_address / _page_size;
    int page_offset = virtual_address % _page_size;

    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);

    // If entry exists, look up frame number and convert virtual to physical address
    int address = -1;
    if (_table.count(entry) > 0)
    {
        int frame = _table[entry];
        address = (frame * _page_size) + page_offset;
    }

    return address;
}

void PageTable::print()
{
    int i;

    std::cout << " PID  | Page Number | Frame Number" << std::endl;
    std::cout << "------+-------------+--------------" << std::endl;

    std::vector<std::string> keys = sortedKeys();

    for (i = 0; i < keys.size(); i++)
    {
        // TODO: print all pages
        std::stringstream ss(keys[i]);
        std::string pid_str, page_number_str;
        std::getline(ss, pid_str, '|');
        std::getline(ss, page_number_str, '|');

        int frame = _table[keys[i]];
        std::cout << std::setw(5) << pid_str << " | " << std::setw(11) << page_number_str << " | " << std::setw(12) << frame << std::endl;
    }
}
