 #include <iterator>
 #include <functional>
 
namespace te_hashed
{
    template <typename T>
    struct te_hashed_entry{
        int8_t offset_distance = -1;
        T value;

        te_hashed_entry() {}

        te_hashed_entry(int8_t offset_distance) :
            offset_distance(offset_distance) {}

        template <typename... Args>
        void emplace(int8_t offset_distance, Args&&... args){
            this->offset_distance = offset_distance;
            new (std::addressof(value)) T(std::forward<Args>(args)...);
        }

        bool is_empty() const {
            return offset_distance == -1;
        }

        void clear(){
            value.~T();
            offset_distance = -1;
        }
    };

    template <typename KeyValue, typename Key, typename Hasher, typename Allocator>  
    class te_hashed_container : Allocator{
        using ContainerEntry = te_hashed::te_hashed_entry<KeyValue>;
        using AllocatorTraits = std::allocator_traits<Allocator>;
        using EntryPointer  = typename AllocatorTraits::pointer;

    public:
        te_hashed_container(){
            rehash(default_capacity);
        }

        te_hashed_container(size_t init_capacity){
            rehash(init_capacity);
        }

        template <typename T>
        struct te_hashed_iterator{
            using iterator_category = std::forward_iterator_tag;
            using difference_type = ptrdiff_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;

            EntryPointer current_entry = EntryPointer();

            te_hashed_iterator();
            te_hashed_iterator(EntryPointer begin) : current_entry(begin) {}

            te_hashed_iterator& operator++(){
                do{
                    ++current_entry;
                }
                while(current_entry->is_empty());
                return *this;
            }

            bool operator==(const te_hashed_iterator& other) const {
                return this->current_entry == other.current_entry;  
            }

            bool operator!=(const te_hashed_iterator& other) const {
                return !(*this == other);
            }

            te_hashed_iterator operator++(int){
                te_hashed_iterator temp = *this; 
                ++(*this); 
                return temp;
            }

            T& operator*() const{
                return current_entry->value;
            }

            T* operator->() const{
                return std::addressof(current_entry->value);
            }
        };

        using iterator = te_hashed_iterator<KeyValue>;
        using const_iterator = te_hashed_iterator<const KeyValue>;

        iterator begin(){
            if (entries->is_empty())
                return ++iterator(entries);
            else
                return iterator(entries);
        }        

        iterator find(const Key& key){
            size_t index = get_index(get_hash(key));
            EntryPointer begin = entries + ptrdiff_t(index);

            for (EntryPointer it = begin; it != begin + capacity; it++){
                if (!it->is_empty()){
                    if (it->value.first == key)
                        return {it};
                }
            }    
        }

        template <typename K, typename... Args>
        std::pair<iterator,bool> emplace(K&& key, Args&&... args){
            double cur_load = (static_cast<double>(size)/(static_cast<double>(capacity)));
            if (cur_load >= load_factor)
                rehash(capacity*2);

            size_t index = get_index(get_hash(key));
            EntryPointer current_entry = entries + ptrdiff_t(index);

            KeyValue entry_data(std::forward<K>(key), std::forward<Args>(args)...);
            int8_t position_offset = 0;

            for(;;++index, ++position_offset){
                if (current_entry->is_empty()){
                    current_entry->emplace(position_offset, std::move(entry_data));
                    ++size;
                    return { {current_entry}, true };
                }

                if (current_entry->offset_distance < position_offset){
                    std::swap(current_entry->offset_distance, position_offset);
                    std::swap(current_entry->value, entry_data);
                }

                index %= capacity;
                current_entry = entries + ptrdiff_t(index);
            }
        }        

        void rehash(size_t resize_capacity){
            EntryPointer pool_begin(AllocatorTraits::allocate(*this, resize_capacity));
            EntryPointer pool_end = pool_begin + resize_capacity; 

            for (EntryPointer it = pool_begin; it != pool_end; it++){
                it->offset_distance = -1;
            }

            std::swap(entries, pool_begin);
            
            for (EntryPointer item_iterator=pool_begin; item_iterator!=pool_begin + capacity; item_iterator++){
                if (!item_iterator->is_empty()){
                    emplace(item_iterator->value.first, item_iterator->value.second);                    
                }
            }

            capacity = resize_capacity;
        }        

        size_t size_v() const{
            return size;
        }        

    private:
        EntryPointer entries = nullptr;
        size_t capacity = 0;
        size_t size = 0;
        Hasher hash_func;
        const double load_factor = 0.75;
        const size_t default_capacity = 4;

        size_t get_hash(const Key& key) const {
            return hash_func(key);
        }

        size_t get_index(size_t hash) const {
            return hash % capacity;
        }       
    };


    template <typename K, typename V, typename Hasher = std::hash<K>,
                typename Allocator = std::allocator<std::pair<K, V> > >
    class te_hashmap : 
        te_hashed_container
        <
            std::pair<K, V>,
            K,
            Hasher,
            typename std::allocator_traits<Allocator>::template rebind_alloc<te_hashed_entry<std::pair<K, V> > >     
        >
    {
        using HashedContainer = te_hashed_container
        <
            std::pair<K, V>,
            K,
            Hasher,
            typename std::allocator_traits<Allocator>::template rebind_alloc<te_hashed_entry<std::pair<K, V> > >     
        >;
    public:

        using HashedContainer::HashedContainer;
        te_hashmap() {}

        std::pair<typename HashedContainer::iterator, bool> insert(K&& key, V&& value){
            return HashedContainer::emplace(std::forward<K>(key), std::forward<V>(value));
        }

        typename HashedContainer::iterator find(const K& key){
            return HashedContainer::find(key);
        }
    };
}
