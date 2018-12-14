#include <iostream>
#include <list>
#include <unordered_map>
#include <vector>
#include <queue>
using namespace std; 

int max_size, m;
int nfu_page_fault = 0;

typedef struct Nfu {
    list<int> actual;
    unordered_map<int, int> index;//храним счетчики "посещений"
    unordered_map<int, list<int>::iterator> ma_p;
}Nfu;

list<int>::iterator not_frequently_used(Nfu& nfu)
{
    int min = m + 1; //точно больше максимального значения
    list<int>::iterator min_index = nfu.actual.begin();
    for(list<int>::iterator i = min_index; i != nfu.actual.end(); i++) {
        if (nfu.index[*i] < min) {
            min = nfu.index[*i];
            min_index = i;
        }
    }
    return min_index;
}

void add_to_nfu(int x, Nfu& nfu) {
    cout << x;
    if(nfu.ma_p.find(x) == nfu.ma_p.end()) { //если он отсутсвует в mape
        nfu_page_fault++; //счетчик страничных нарушений
        cout << "- ";
        if(nfu.actual.size() == max_size) { //лист полон
            list<int>::iterator not_freq = not_frequently_used(nfu);
            nfu.actual.insert(not_freq, x); //вставили перед not_freq
            nfu.actual.erase(not_freq);
            nfu.ma_p.erase(*not_freq);
            nfu.ma_p[x] = not_freq;
        } else {
            nfu.actual.push_back(x);
            nfu.ma_p[x] = nfu.actual.begin();
        }
        nfu.index.insert(make_pair(x, 1));
    } else {
        cout << "+ ";        
        nfu.index[x]++;
    }
}
  
int main() 
{
    Nfu nfu;
    cin >> max_size >> m;
    vector<int> pages;  
    for (int i = 0; i < m; i++) {
        int x;
        cin >> x;
        pages.push_back(x);
    }
    for (int i = 0; i < m; i++) {
        add_to_nfu(pages[i], nfu);
    }

    cout << nfu_page_fault << endl;
    return 0; 
} 
