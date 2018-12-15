#include <iostream>
#include <list>
#include <unordered_map>
#include <vector>
#include <queue>
using namespace std; 

int max_size, m;
int fifo_page_fault = 0, lru_page_fault = 0, opt_page_fault = 0, nfu_page_fault = 0;

typedef struct Fifo {
    list<int> actual;
    unordered_map<int, list<int>::iterator> ma_p; //просто для более быстрого поиска
}Fifo;

void add_to_fifo(int x, Fifo& fifo) {
    cout << x;
    if(fifo.ma_p.find(x) == fifo.ma_p.end()) { //если он отсутсвует в mape
        fifo_page_fault++; //счетчик страничных нарушений
        cout << "- ";
        if((int)fifo.actual.size() == max_size) { //лист полон
            int last = fifo.actual.back();
            fifo.actual.pop_back();// удаляем последний элемент из списка
            fifo.ma_p.erase(last);// удаляем последний элемент из map
        }
        fifo.actual.push_front(x);
        fifo.ma_p[x] = fifo.actual.begin();
    } else {
        cout << "+ ";
    }
}

typedef struct Lru {
    list<int> actual;
    unordered_map<int, list<int>::iterator> ma_p;
}Lru;

void add_to_lru(int x, Lru& lru) {
    cout << x;
    if(lru.ma_p.find(x) == lru.ma_p.end()) { //если он отсутсвует в mape
        lru_page_fault++; //счетчик страничных нарушений
        cout << "- ";
        if((int)lru.actual.size() == max_size) { //лист полон
            int last = lru.actual.back();
            lru.actual.pop_back();// удаляем последний элемент из списка
            lru.ma_p.erase(last);// удаляем последний элемент из map
        }
    } else {
        cout << "+ ";
        lru.actual.erase(lru.ma_p[x]); //удаляем, тем самым поддерживая "least recently"
    }
    lru.actual.push_front(x);
    lru.ma_p[x] = lru.actual.begin();//сохраняем итераторы на эл-ты листа
}

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
        if ((int)nfu.index[*i] < min) {
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
        if((int)nfu.actual.size() == max_size) { //лист полон
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

typedef struct Opt {
    vector<pair<unsigned long, unsigned long> > cache;
    vector<pair<unsigned long, unsigned long> > pages;
    unordered_map<unsigned long, unsigned long> ma_p;
}Opt;

void add_to_opt(Opt& opt, vector<unsigned long>& first_pages) {
    for (int i = 0; i < m; i++) {
        unsigned long x = first_pages[i];
        opt.pages.push_back(make_pair(x, m + 1));
        if(opt.ma_p.find(x) == opt.ma_p.end()) {//если элемент не нашли
            opt.ma_p.insert(make_pair(x, m + 1));
        }
    }
    for(int i = m - 1; i >= 0; i--) {//вставляем индексы следующих элементов
        opt.pages[i].second = opt.ma_p[opt.pages[i].first];
        opt.ma_p.erase(opt.pages[i].first);
        opt.ma_p.insert(make_pair(opt.pages[i].first, i));
    }
    for(int i = 0; i < (int)opt.pages.size(); i++) {
        cout << opt.pages[i].first;
        bool match = false;
        for(int j = 0; j < (int)opt.cache.size(); j++) {
            if(opt.pages[i].first == opt.cache[j].first) {
                cout << "+ ";
                match = true;
                opt.cache[j] = opt.pages[i];
                break;
            }
        }
        int index = 0;
        int reference = 0;
        if(match == false){
            cout << "- ";
            opt_page_fault++;
            if((int)opt.cache.size() < max_size) {//просто дописываем
                opt.cache.push_back(opt.pages[i]);
            } else {//нужно удалять
                for(int j = 0; j < (int)opt.cache.size(); j++) {//находим самый поздний элемент
                    if(index < (int)opt.cache[j].second) {
                        index = opt.cache[j].second;
                        reference = j;
                    }
                }
                opt.cache[reference] = opt.pages[i];
            }
        }
    }
    return;
}

int main() {
    Opt opt;
    Lru lru;
    Nfu nfu;
    Fifo fifo;
    cin >> max_size >> m;
    vector<unsigned long> first_pages;
    for(int i = 0; i < m; i++) {
        unsigned long x;
        cin >> x;
        first_pages.push_back(x);
    }
    for (int i = 0; i < m; i++) {
        add_to_fifo(first_pages[i], fifo);
    }
    cout << endl;
    for (int i = 0; i < m; i++) {
        add_to_lru(first_pages[i], lru);
    }
    cout << endl;
    for (int i = 0; i < m; i++) {
        add_to_nfu(first_pages[i], nfu);
    }
    cout << endl;
    add_to_opt(opt, first_pages);
    cout << endl << fifo_page_fault << " " << lru_page_fault << " " << nfu_page_fault << " " << opt_page_fault << endl;
    return 0; 
} 