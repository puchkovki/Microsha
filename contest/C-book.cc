#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#define HASH_SIZE 4096
#define FILE_SIZE 100*1024*1024L

using namespace std;

int Add();
int Delete();
int Update();
int Print();

static string commands_names[] =
{
    "ADD",
    "DELETE",
    "UPDATE",
    "PRINT"
};

static int (*commands_func[]) () = 
{
    &Add,
    &Delete,
    &Update,
    &Print
};

static int fd;
static unsigned *addr;
static unsigned freeplace = HASH_SIZE;
static hash<string> hash_func;

int Add()
{
    string key, value;
    cin >> key;
    cin >> value;
    size_t offset = hash_func(key) % HASH_SIZE;
    unsigned * newaddr = addr + offset;
    while(true)
    {
        if (*newaddr == 0)
        {
            *newaddr = freeplace;
            newaddr = addr + freeplace;
            freeplace += 4 + (key.size() + value.size()) / 4;
            *newaddr = key.size();
            newaddr++;
            *newaddr = value.size();
            newaddr++;
            *newaddr = 0;
            newaddr++;
            string kv = key + value;
            memcpy(newaddr, kv.c_str(), kv.size());
            return 0;
        }
        else
        {
            newaddr = addr + *newaddr;
            unsigned length = (*newaddr);
            newaddr += 3;
            char * sa = new char[length + 1];
            memcpy(sa, newaddr, length);
            sa[length] = '\0';
            string seckey = sa;
            if (key == seckey)
            {
                return -1;
            }
            newaddr--;
        }
    }
    
    return -1;
}

int Add(string key, string value)
{
    size_t offset = hash_func(key) % HASH_SIZE;
    unsigned * newaddr = addr + offset;
    while(true)
    {
        if (*newaddr == 0)
        {
            *newaddr = freeplace;
            newaddr = addr + freeplace;
            freeplace += 8 + (key.size() + value.size()) / 4;
            *newaddr = key.size();
            newaddr++;
            *newaddr = value.size();
            newaddr++;
            newaddr++;
            string kv = key + value;
            memcpy(newaddr, kv.c_str(), kv.size());
            return 0;
        }
        else
        {
            newaddr = addr + *newaddr;
            unsigned length = (*newaddr);
            newaddr += 3;
            char * sa = new char[length + 1];
            memcpy(sa, newaddr, length);
            sa[length] = '\0';
            string seckey = sa;
            if (key == seckey)
            {
                return -1;
            }
            newaddr--;
        }
    }
    
    return -1;
}

int Delete()
{
    string key;
    cin >> key;
    size_t offset = hash_func(key) % HASH_SIZE;
    unsigned *newaddr = addr + offset;
    unsigned *oldaddr = newaddr;
    while(*newaddr != 0)
    {
        newaddr = addr + *newaddr;
        unsigned length = *newaddr;
        newaddr += 3;
        char * sa = new char[length + 1];
        memcpy(sa, newaddr, length);
        sa[length] = '\0';
        string seckey = sa;
        newaddr--;
        if (key == seckey)
        {
            *oldaddr = *newaddr;
            return 0;
        }
        oldaddr = newaddr;
    }
    return -1;
}

int Delete(string key)
{
    size_t offset = hash_func(key) % HASH_SIZE;
    unsigned *newaddr = addr + offset;
    unsigned *oldaddr = newaddr;
    while(*newaddr != 0)
    {
        newaddr = addr + *newaddr;
        unsigned length = *newaddr;
        newaddr += 3;
        char * sa = new char[length + 1];
        memcpy(sa, newaddr, length);
        sa[length] = '\0';
        string seckey = sa;
        newaddr--;
        if (key == seckey)
        {
            *oldaddr = *newaddr;
            return 0;
        }
        oldaddr = newaddr;
    }
    return -1;
}

int Update()
{
    string key, value;
    cin >> key >> value;
    if(Delete(key) < 0)
    {
        return -1;
    }
    if(Add(key, value) < 0)
    {
        return -1;
    }
    return 0;
}

int Print()
{
    string key;
    cin >> key;
    size_t offset = hash_func(key) % HASH_SIZE;
    unsigned *newaddr = addr + offset;
    while(*newaddr != 0)
    {
        newaddr = addr + *newaddr;
        unsigned length = *newaddr;
        newaddr += 3;
        char * sa1 = new char[length + 1];
        memcpy(sa1, newaddr, length);
        sa1[length] = '\0';
        string seckey = sa1;
        newaddr--;
        if (key == seckey)
        {
            cout << seckey << " ";
            newaddr--;
            unsigned value_length = *newaddr;
            newaddr += 2;
            char * sa2 = new char[length + value_length + 1];
            memcpy(sa2, newaddr, length + value_length);
            sa2[length + value_length] = '\0';
            string seckey2 = sa2;
            seckey2.erase(0, length);
            cout << seckey2 << endl;
            return 0;
        }
    }
    return -1;
}

int main(void)
{
    fd = open("auxiliary", O_RDWR | O_CREAT | O_TRUNC, 0777);
    lseek(fd, FILE_SIZE, SEEK_SET);
    write(fd, " ", 1);
    addr = (unsigned*) mmap(nullptr, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SEEK_SET);
    memset(addr, 0, HASH_SIZE * 8);
    int n;
    cin >> n;
    for (int i = 0; i < n; i++)
    {
        string command;
        cin >> command;
        for (int j = 0; j < 4; j++)
        {
            if (command == commands_names[j])
            {
                if((*commands_func[j])() < 0)
                {
                    cout << "ERROR" << endl;
                }
            }
        }
    }
    munmap(addr, FILE_SIZE);
    remove("auxiliary");
    return 0;
}