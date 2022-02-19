#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef USE_GETTIMEOFDAY
#  include <sys/time.h>
#else
#  include <time.h>
#endif

class Map {
    struct item {
        int val;
        char key[64-sizeof(int)];
        item() { key[0] = 0; }
        void operator=(const item &i) {
            val = i.val;
            strcpy(key, i.key);
        }
    } *begin, *end;
public:
    Map() : begin(0), end(0) {}
    virtual ~Map() { Release(); }
    void Add(const char *s, int v) {
        if(begin == end)
            Resize();
        item *p = const_cast<item*>(ProvideFreePos());
        if(!p) {
            Resize();
            p = const_cast<item*>(ProvideFreePos());
        }
        p->val = v;
        strcpy(p->key, s);
    }
    void Remove(const char *s) {
        item *p = const_cast<item*>(ProvideItem(s));
        if(p)
            p->key[0] = 0;
    }
    void Release() {
        if(begin) {
            delete[] begin;
            begin = 0;
            end = 0;
        }
    }
    int *Provide(const char *s)
        { return &const_cast<item*>(ProvideItem(s))->val; }
    int &operator[](const char *s)
        { return *Provide(s); }
    int operator[](const char *s) const
        { return *const_cast<Map*>(this)->Provide(s); }
private:
    const item *ProvideItem(const char *s) const {
        if(begin == end || !s || !*s)
            return 0;
        for(item *p = begin; p < end; p++) {
            if(0 == strcmp(p->key, s))
                return p;
        }
        return 0;
    }
    const item *ProvideFreePos() const {
        if(begin == end)
            return 0;
        for(item *p = begin; p < end; p++) {
            if(p->key[0] == 0)
                return p;
        }
        return 0;
    }
    void Resize() {
        int old_size = end - begin;
        int new_size = begin ? 2*old_size : 16;
        item *p = new item[new_size];
        if(begin) {
            for(int i = 0; i < old_size; i++)
                p[i] = begin[i];
        }
        begin = p;
        end = p + new_size;
    }
private:
    Map(const Map&) {}
    void operator=(const Map&) {}
};


static inline void test(Map &m)
{
    m.Add("1", 1);
    m.Add("2", 2);
    m.Add("3", m["1"] + m["2"]);
}

int main()
{
#ifdef USE_GETTIMEOFDAY
    timeval begin, end, diff;
    gettimeofday(&begin, 0);
#else
    clock_t begin = clock();
#endif
    for(int i = 0; i < 50*1000*1000; i++) {
#ifdef USE_STACK
        Map m;
        test(m);
#else
        Map *m = new Map;
        test(*m);
        delete m;
#endif
    }
#ifdef USE_GETTIMEOFDAY
    gettimeofday(&end, 0);
    diff.tv_sec = end.tv_sec - begin.tv_sec;
    diff.tv_usec = end.tv_usec - begin.tv_usec;
    if(diff.tv_usec < 0) {
        diff.tv_sec--;
        diff.tv_usec += 1000*1000;
    }
    printf("%ld.%ld\n", diff.tv_sec, diff.tv_usec);
#else
    clock_t end = clock();
    double diff = static_cast<double>(end - begin)/CLOCKS_PER_SEC;
    printf("%.6f\n", diff);
#endif
    return 0;
}
