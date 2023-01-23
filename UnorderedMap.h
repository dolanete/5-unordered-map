#include <cstddef>    // size_t
#include <functional> // std::hash
#include <ios>
#include <utility>    // std::pair
#include <iostream>

#include "primes.h"



template <typename Key, typename T, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key>>
class UnorderedMap {
    public:

    using key_type = Key;
    using mapped_type = T;
    using const_mapped_type = const T;
    using hasher = Hash;
    using key_equal = Pred;
    using value_type = std::pair<const key_type, mapped_type>;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    private:

    struct HashNode {
        HashNode *next;
        value_type val;

        HashNode(HashNode *next = nullptr) : next{next} {}
        HashNode(const value_type & val, HashNode * next = nullptr) : next { next }, val { val } { }
        HashNode(value_type && val, HashNode * next = nullptr) : next { next }, val { std::move(val) } { }
    };

    size_type _bucket_count;
    HashNode **_buckets;

    HashNode * _head;
    size_type _size;

    Hash _hash;
    key_equal _equal;

    static size_type _range_hash(size_type hash_code, size_type bucket_count) {
        return hash_code % bucket_count;
    }

    public:

    template <typename pointer_type, typename reference_type, typename _value_type>
    class basic_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = _value_type;
        using difference_type = ptrdiff_t;
        using pointer = value_type *;
        using reference = value_type &;

    private:
        friend class UnorderedMap<Key, T, Hash, key_equal>;
        using HashNode = typename UnorderedMap<Key, T, Hash, key_equal>::HashNode;

        const UnorderedMap * _map;
        HashNode * _ptr;

        explicit basic_iterator(UnorderedMap const * map, HashNode *ptr) noexcept { 
            _map = map;
            _ptr = ptr;
        }

    public:
        basic_iterator() { 
            _map = nullptr;
            _ptr = nullptr;
         };

        basic_iterator(const basic_iterator &) = default;
        basic_iterator(basic_iterator &&) = default;
        ~basic_iterator() = default;
        basic_iterator &operator=(const basic_iterator &) = default;
        basic_iterator &operator=(basic_iterator &&) = default;
        reference operator*() const { return _ptr->val; }
        pointer operator->() const { return &_ptr->val; }
        basic_iterator &operator++() { 
            if (_ptr->next) { //This checks if we are not at the end
                _ptr = _ptr->next;
            } else {
                size_t bucket = _map->_bucket(_ptr->val); //We want to call the bucket function for this specific map
                while (++bucket < _map->_bucket_count) {
                    _ptr = _map->_buckets[bucket];
                    if (_ptr) {
                        return *this;
                    }
                }
                _ptr = nullptr;
            }
            return *this;
        }
        basic_iterator operator++(int) { 
            basic_iterator old = *this;
            ++*this;
            return old;
         }
        bool operator==(const basic_iterator &other) const noexcept { return _ptr == other._ptr; }
        bool operator!=(const basic_iterator &other) const noexcept { return !(*this == other);}
    };

    using iterator = basic_iterator<pointer, reference, value_type>;
    using const_iterator = basic_iterator<const_pointer, const_reference, const value_type>;

    class local_iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::pair<const key_type, mapped_type>;
            using difference_type = ptrdiff_t;
            using pointer = value_type *;
            using reference = value_type &;

        private:
            friend class UnorderedMap<Key, T, Hash, key_equal>;
            using HashNode = typename UnorderedMap<Key, T, Hash, key_equal>::HashNode;

            HashNode * _node;

            explicit local_iterator( HashNode * node ) noexcept : _node{node} { /* TODO */ } //Done?

        public:
            local_iterator() : _node(nullptr) { } //default

            local_iterator(const local_iterator &) = default;
            local_iterator(local_iterator &&) = default;
            ~local_iterator() = default;
            local_iterator &operator=(const local_iterator &) = default;
            local_iterator &operator=(local_iterator &&) = default;
            reference operator*() const { return _node->val; }
            pointer operator->() const { return &(_node->val); }
            local_iterator & operator++() { 
                _node = _node->next;
                return *this;
            }
            local_iterator operator++(int) { 
                local_iterator temp = *this;
                ++(*this);
                return temp;
             }

            bool operator==(const local_iterator &other) const noexcept { return _node == other._node; }
            bool operator!=(const local_iterator &other) const noexcept { return _node != other._node; }
    };

private:

    size_type _bucket(size_t code) const { return _range_hash(code,_bucket_count); } // Returns the index of the bucket for hash code code
    size_type _bucket(const Key & key) const { return _bucket(_hash(key)); } // Returns the index of the bucket for key key
    size_type _bucket(const value_type & val) const { return _bucket(val.first); } // Returns the index of the bucket for val val

    HashNode*& _find(size_type code, size_type bucket, const Key & key) {
        HashNode ** temp = &_buckets[bucket]; //Uses the given bucket from parameter
        while (*temp != nullptr) { //Loop while the temp has not reached the end of the list.
            if ((_hash((*temp)->val.first))==code && _equal((*temp)->val.first,key)) { // Grabs the hash of the key to find the code of val.first
                return *temp;
            }
            temp = &((*temp)->next); //Iterate
        }
        return *temp; //Returns the node where the location is
            
    }

    HashNode*& _find(const Key & key) { return _find(_hash(key), _bucket(key), key); }

    HashNode * _insert_into_bucket(size_type bucket, value_type && value) {
        HashNode *& temp = _buckets[bucket]; //Access the given bucket
        HashNode* insertedNode = new HashNode(std::move(value), temp);
        if (_head == nullptr || _bucket(_head->val) >= bucket) {
            _head = insertedNode;
        }
        _size++;
        temp = insertedNode;
        return insertedNode;
    }

    void _move_content(UnorderedMap & src, UnorderedMap & dst) { /* TODO */ }

public:
    explicit UnorderedMap(size_type bucket_count, const Hash & hash = Hash { },
                const key_equal & equal = key_equal { }) { 
                    _bucket_count = next_greater_prime(bucket_count);
                    _hash = hash;
                    _equal = equal;
                    _size = 0;
                    _head = nullptr;
                    _buckets = new HashNode*[_bucket_count]();
                 }

    ~UnorderedMap() { 
        clear();
     }

    UnorderedMap(const UnorderedMap & other) { 
        _bucket_count = other._bucket_count; //Copy values from the other map
        _hash = other._hash;
        _equal = other._equal;
        _size = 0;
        _head = nullptr;
        _buckets = new HashNode*[_bucket_count]();

        for (const_iterator iter = other.cbegin(); iter != other.cend(); iter++) {

            insert(*iter);
        }
     }

    UnorderedMap(UnorderedMap && other) { 
        _bucket_count = (other._bucket_count); //Copy values from the other map
        _hash = std::move(other._hash);
        _equal = std::move(other._equal);
        _size = std::move(other._size);
        _head = std::move(other._head);
        _buckets = other._buckets;

        /* for (const_iterator iter = other.cbegin(); iter != other.cend(); iter++) {

        } */

        other._head = nullptr;
        other._size = 0;
        other._buckets = new HashNode*[_bucket_count]();
     }

    UnorderedMap & operator=(const UnorderedMap & other) {
        if (this != &other) {
            clear();
            delete[] _buckets;
            _bucket_count = other._bucket_count; //Copy values from the other map
            _hash = other._hash;
            _equal = other._equal;
            _size = 0;
            _head = nullptr;
            _buckets = new HashNode*[_bucket_count]();

            for (const_iterator iter = other.cbegin(); iter != other.cend(); iter++) {
                insert(*iter);
            }
        }
        return *this;
     }

    UnorderedMap & operator=(UnorderedMap && other) { 
        if (this != &other) {
            clear();
            delete[] _buckets;
            _bucket_count = (other._bucket_count); //Copy values from the other map
            _hash = std::move(other._hash);
            _equal = std::move(other._equal);
            _size = std::move(other._size);
            _head = std::move(other._head);
            _buckets = other._buckets;

            /* for (const_iterator iter = other.cbegin(); iter != other.cend(); iter++) {

            } */

            other._head = nullptr;
            other._size = 0;
            other._buckets = new HashNode*[_bucket_count]();
        }
        return *this;
    }

    void clear() noexcept { 
        for (iterator i = begin(); i != end(); i++) {
            erase(i);
        }
     }

    size_type size() const noexcept { return _size; }

    bool empty() const noexcept { 
        if (_size == 0) {
            return true;
        }
        return false;
     }

    size_type bucket_count() const noexcept { return _bucket_count; }

    iterator begin() { return iterator(this, _head); }
    iterator end() { return iterator(this, nullptr); }

    const_iterator cbegin() const { return const_iterator(this, _head); };
    const_iterator cend() const { return const_iterator(this, nullptr); };

    local_iterator begin(size_type n) { return local_iterator(_buckets[n]); }
    local_iterator end(size_type n) { return local_iterator(nullptr); }

    size_type bucket_size(size_type n) { 
        local_iterator iter = begin(n); //place iterator at the start
        size_type count = 0;
        while(iter != end(n)){
            count++;
            iter++;
        }
        return count;
     }

    float load_factor() const { 
        return static_cast<float>(size())/bucket_count();
     }

    size_type bucket(const Key & key) const { 
        return _bucket(_hash(key));
     }

    std::pair<iterator, bool> insert(value_type && value) {
        HashNode * temp = _find(value.first);
        if (temp != nullptr) {
            return {iterator(this, temp), false};
        } else {
            temp = _insert_into_bucket(_bucket(_hash(value.first)), std::move(value));
            return {iterator(this, temp), true};
        }
    }

    std::pair<iterator, bool> insert(const value_type & value) { 
         HashNode * temp = _find(value.first);
         if (temp != nullptr) {
            return {iterator(this, temp), false};

         } else {
            value_type value2 = value;
            temp = _insert_into_bucket(_bucket(value.first), std::move(value2));
            return {iterator(this, temp), true};
         }
     }

    iterator find(const Key & key) { 
        return iterator(this, _find(key));
     }

    T& operator[](const Key & key) { 
        HashNode * temp = _find(key); //This is the node that contains the key or it is nullptr
        if (temp != nullptr) {
            return temp->val.second;
        } else {
            return insert(std::make_pair(key, T())).first->second;
        }
     }

    iterator erase(iterator pos) { 
        HashNode*& temp = _find(pos->first); //set temp to the node at the key of pos
        if(temp != nullptr) { //When not a null pointer
            iterator iter = ++iterator(this, temp); //set an interator to the next element

            HashNode* current = temp; // Take the current node
            temp = temp->next; //Move to the next node for temp
            delete current; //Delete the current node
            _size--; //Decrease size
            if(current == _head) {
                _head = iter._ptr;
            }
            return iter; //Returns the next node
        }
        return end();
    }

    size_type erase(const Key & key) { 
        iterator iter = find(key); //Grabs the iterator at key
        if (iter != end()) {
            erase(iter);
            return 1;
        }
        return 0;
     }

    template<typename KK, typename VV>
    friend void print_map(const UnorderedMap<KK, VV> & map, std::ostream & os);
};

template<typename K, typename V>
void print_map(const UnorderedMap<K, V> & map, std::ostream & os = std::cout) {
    using size_type = typename UnorderedMap<K, V>::size_type;
    using HashNode = typename UnorderedMap<K, V>::HashNode;

    for(size_type bucket = 0; bucket < map.bucket_count(); bucket++) {
        os << bucket << ": ";

        HashNode const * node = map._buckets[bucket];

        while(node) {
            os << "(" << node->val.first << ", " << node->val.second << ") ";
            node = node->next;
        }

        os << std::endl;
    }
}
