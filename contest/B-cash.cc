#include <iostream>
#include <list>
#include <unordered_map>
#include <vector>
#include <queue>
using namespace std; 

int max_size, m;
int opt_page_fault = 0;

typedef struct Opt {
    vector<pair<unsigned long, unsigned long> > cache;
    vector<pair<unsigned long, unsigned long> > pages;
    unordered_map<unsigned long, unsigned long> ma_p;
}Opt;

int add_to_opt(Opt& opt, vector<unsigned long>& first_pages) {
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
        bool match = false;
        for(int j = 0; j < (int)opt.cache.size(); j++) {
            if(opt.pages[i].first == opt.cache[j].first) {
                match = true;
                opt.cache[j] = opt.pages[i];
                break;
            }
        }
        int index = 0;
        int reference = 0;
        if(match == false){
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
    return opt_page_fault;
}

int main() 
{
    Opt opt;
    cin >> max_size >> m;
    vector<unsigned long> first_pages;
    for(int i = 0; i < m; i++) {
        unsigned long x;
        cin >> x;
        first_pages.push_back(x);
    }
    cout << add_to_opt(opt, first_pages) << endl;
    return 0; 
} 
