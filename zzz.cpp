#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef USE_GETTIMEOFDAY
#  include <sys/time.h>
#else
#  include <time.h>
#endif

#if 0
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
    void AddItem(const char *s, int v) {
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
    int *Provide(const char *s) {
        item *pos = const_cast<item*>(ProvideItem(s));
        if(!pos)
            return 0;
        return &pos->val;
    }
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
#else
class Map {
    typedef unsigned int hash_t;
    struct item {
        int val;
        char key[64-sizeof(int)];
        item() throw() { key[0] = 0; }
        void operator=(const item &i) throw() {
            val = i.val;
            strcpy(key, i.key);
        }
    } *the_array;
    int array_size;
    int array_filled;
public:
    Map() throw() : the_array(0), array_size(0), array_filled(0) {}
    virtual ~Map() throw() { Release(); }
    void AddItem(const char *key, int val) throw(int) {
        if(!key || !*key)
            return;
        if(!array_size || array_filled > ((array_size + 1)*2)/3)
            Resize();
        int pos = static_cast<int>(KnuthHash(key)) % array_size;
        item *i = const_cast<item*>(ProvideFreePosition(pos));
        i->val = val;
        strcpy(i->key, key);
        array_filled++;
    }
    void AddOrAssign(const char *key, int val) throw(int) {
        int *pos = ProvideValue(key);
        if(!pos)
            AddItem(key, val);
        else
            *pos = val;
    }
    int *ProvideValue(const char *key) throw() {
        item *pos = const_cast<item*>(ProvideItemByKey(key));
        if(!pos)
            return 0;
        return &pos->val;
    }
    int &operator[](const char *key) throw()
        { return *ProvideValue(key); }
    int operator[](const char *key) const throw()
        { return *const_cast<Map*>(this)->ProvideValue(key); }
    void Release() {
        if(the_array) {
            delete[] the_array;
            the_array = 0;
            array_size = 0;
            array_filled = 0;
        }
    }
private:
    const item *ProvideItemByKey(const char *s) const throw() {
        int pos = static_cast<int>(KnuthHash(s)) % array_size;
        for(int i = pos; i < array_size; i++) {
            if(0 == strcmp(the_array[i].key, s))
                return &the_array[i];
        }
        for(int i = 0; i < pos; i++) {
            if(0 == strcmp(the_array[i].key, s))
                return &the_array[i];
        }
        return 0;
    }
    const item *ProvideFreePosition(int pos) const throw() {
        for(int i = pos; i < array_size; i++) {
            if(the_array[i].key[0] == 0)
                return &the_array[i];
        }
        for(int i = 0; i < pos; i++) {
            if(the_array[i].key[0] == 0)
                return &the_array[i];
        }
        return 0;
    }
    static const int array_size_table[];
    void Resize() throw(int) {
        int new_size = array_size_table[1];
        for(int i = 1; array_size_table[i]; i++) {
            if(array_size_table[i] == array_size) {
                new_size = array_size_table[i + 1];
                break;
            }
        }
        if(!new_size) {
            Release();
            return;
        }
        item *p = the_array;
        the_array = new item[new_size];
        for(int i = 0; i < array_size; i++) {
            if(p[i].key[0]) {
                int pos = static_cast<int>(KnuthHash(p[i].key));
                item *addr = const_cast<item*>(ProvideFreePosition(pos));
                addr->val = p[i].val;
                strcpy(addr->key, p[i].key);
            }
        }
        delete[] p;
        array_size = new_size;
    }
private:
    static const hash_t hash_multiplier = 0x9c406bb5;
    static const hash_t hash_xor = 0x13fade37;
    static hash_t KnuthHash(const char *s) throw() {
        if(!*s)
            return 0;
        hash_t res = 13;
        for(int i = 0; *s; s++, i++)
            res ^= (static_cast<hash_t>(*s)) << (8*(i%sizeof(hash_t)));
        return hash_multiplier * (hash_xor ^ res);
    }
};
const int Map::array_size_table[] = {0, 7, 11, 17, 37, 67, 131, 257, 0};
#endif


static inline void test(Map &m)
{
    m.AddItem("1", 1);
    m.AddItem("2", 2);
    m.AddItem("3", m["1"] + m["2"]);
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
    printf("%.3f\n", diff);
#endif
    return 0;
}