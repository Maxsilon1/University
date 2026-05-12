#pragma once

#include <iostream>      // operator<<
#include <utility>      // pair, forward
#include <memory>       // allocator, allocator_traits
#include <functional>   // hash

template<typename Key, typename Value, class Hasher = std::hash<Key>, class Alloc = std::allocator<std::pair<const Key, Value>>>
class HashMap
{
public:
    using value_type = std::pair<const Key, Value>;

private:
    struct BaseNode
    {
        BaseNode* next_;
        BaseNode() : next_(nullptr) {}
        BaseNode(BaseNode* next) : next_(next) {}
    };

    struct Node : BaseNode
    {
        value_type data;
        size_t hash_;

        template<typename... Args>
        Node(BaseNode* next, size_t hash, Args&&... args) 
            : BaseNode(next), data(std::forward<Args>(args)...), hash_(hash) {}
    };

    using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    using BucketAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<BaseNode*>;

    using NodeAllocTraits = std::allocator_traits<NodeAlloc>;
    using BucketAllocTraits = std::allocator_traits<BucketAlloc>;

    NodeAlloc node_alloc_;
    BucketAlloc bucket_alloc_;

    BaseNode** buckets_;
    BaseNode before_begin_;
    Hasher hasher_;

    size_t size_;
    size_t buckets_count_;
    float max_load_factor_ = 0.75f;

    template<typename... Args>
    Node* alloc_node(size_t hash, Args&&... args)
    {
        Node* new_node = NodeAllocTraits::allocate(node_alloc_, 1);
        try
        {
            NodeAllocTraits::construct(node_alloc_, new_node, nullptr, hash, std::forward<Args>(args)...);
        }
        catch (...)
        {
            NodeAllocTraits::deallocate(node_alloc_, new_node, 1);
            throw;
        }
        return new_node;
    }

    void deallocate_buckets()
    {
        if (buckets_) {
            BucketAllocTraits::deallocate(bucket_alloc_, buckets_, buckets_count_);
            buckets_ = nullptr;
        }
    }

    size_t getBucketIndex(const Key& key) const
    {
        return hasher_(key) % buckets_count_;
    }

public:
    template<bool isConst>
    class base_iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = HashMap::value_type;
        using difference_type = std::ptrdiff_t;
        using pointer_type = std::conditional_t<isConst, const value_type*, value_type*>;
        using reference_type = std::conditional_t<isConst, const value_type&, value_type&>;

        BaseNode* ptr;

        base_iterator(BaseNode* ptr) : ptr(ptr) {}
        base_iterator(const base_iterator<false>& other) : ptr(other.ptr) {}

        reference_type operator*() const { return static_cast<Node*>(ptr)->data; }
        pointer_type operator->() const { return &(static_cast<Node*>(ptr)->data); }

        base_iterator& operator++() { ptr = ptr->next_; return *this; }
        base_iterator operator++(int) { base_iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator==(const base_iterator& a, const base_iterator& b) { return a.ptr == b.ptr; }
        friend bool operator!=(const base_iterator& a, const base_iterator& b) { return a.ptr != b.ptr; }

        Node* get_ptr() const { return static_cast<Node*>(ptr); }
    };

    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;

    HashMap() : buckets_(nullptr), size_(0), buckets_count_(0) {}
    ~HashMap() { clear(); deallocate_buckets(); }

    iterator begin() { return iterator{ before_begin_.next_ }; }
    iterator end() { return iterator{ nullptr }; }
    const_iterator begin() const { return const_iterator{ before_begin_.next_ }; }
    const_iterator end() const { return const_iterator{ nullptr }; }
    const_iterator cbegin() const { return const_iterator{ before_begin_.next_ }; }
    const_iterator cend() const { return const_iterator{ nullptr }; }

    void rehash(size_t new_count)
    {
        if (new_count <= buckets_count_ && buckets_ != nullptr) return;

        BaseNode** new_buckets = BucketAllocTraits::allocate(bucket_alloc_, new_count);
        for (size_t i = 0; i < new_count; ++i) new_buckets[i] = nullptr;

        BaseNode* current = before_begin_.next_;
        before_begin_.next_ = nullptr;

        while (current)
        {
            Node* node = static_cast<Node*>(current);
            BaseNode* next_cache = node->next_;
            size_t idx = node->hash_ % new_count;

            if (new_buckets[idx] == nullptr)
            {
                node->next_ = before_begin_.next_;
                before_begin_.next_ = node;
                if (node->next_) {
                    size_t next_idx = static_cast<Node*>(node->next_)->hash_ % new_count;
                    new_buckets[next_idx] = node;
                }
                new_buckets[idx] = &before_begin_;
            }
            else
            {
                node->next_ = new_buckets[idx]->next_;
                new_buckets[idx]->next_ = node;
                size_t next_idx = static_cast<Node*>(node->next_)->hash_ % new_count;
                if (next_idx != idx) {
                    new_buckets[next_idx] = node;
                }
            }
            current = next_cache;
        }

        deallocate_buckets();
        buckets_ = new_buckets;
        buckets_count_ = new_count;
    }

private:
    template<typename K, typename V>
    void insert_impl(K&& key, V&& val)
    {
        if (buckets_count_ == 0 || (float)(size_ + 1) / buckets_count_ > max_load_factor_)
        {
            rehash(buckets_count_ == 0 ? 8 : buckets_count_ * 2);
        }

        size_t hash = hasher_(key);
        size_t idx = hash % buckets_count_;

        Node* new_node = alloc_node(hash, std::piecewise_construct, 
            std::forward_as_tuple(std::forward<K>(key)), 
            std::forward_as_tuple(std::forward<V>(val)));

        if (buckets_[idx] == nullptr)
        {
            new_node->next_ = before_begin_.next_;
            before_begin_.next_ = new_node;
            if (new_node->next_) {
                size_t n_idx = static_cast<Node*>(new_node->next_)->hash_ % buckets_count_;
                buckets_[n_idx] = new_node;
            }
            buckets_[idx] = &before_begin_;
        }
        else
        {
            new_node->next_ = buckets_[idx]->next_;
            buckets_[idx]->next_ = new_node;
            size_t n_idx = static_cast<Node*>(new_node->next_)->hash_ % buckets_count_;
            if (n_idx != idx) buckets_[n_idx] = new_node;
        }
        ++size_;
    }
public:
    Value& operator[](const Key& key)
    {
        std::pair<bool, iterator> result = get(key);

        if (result.first)
        {
            return result.second->second;
        }

        // Если ключ не найден, вставляем новую пару с использованием 
        // конструктора по умолчанию для типа Value.
        insert(key, Value());

        // Возвращаем ссылку на значение во вновь созданном или перераспределенном узле.
        return get(key).second->second;
    }

    void insert(const std::pair<Key, Value>& val) { insert_impl(val.first, val.second); }
    void insert(std::pair<Key, Value>&& val) { insert_impl(std::move(val.first), std::move(val.second)); }
    void insert(const Key& key, const Value& val) { insert_impl(key, val); }
    void insert(Key&& key, Value&& val) { insert_impl(std::move(key), std::move(val)); }

    void erase(const Key& key)
    {
        if (size_ == 0) return;
        size_t idx = getBucketIndex(key);
        BaseNode* prev = buckets_[idx];
        if (!prev) return;

        while (prev->next_)
        {
            Node* curr = static_cast<Node*>(prev->next_);
            if (curr->hash_ % buckets_count_ != idx) break;
            if (curr->data.first == key)
            {
                BaseNode* to_del = prev->next_;
                prev->next_ = to_del->next_;
                if (prev->next_) {
                    size_t n_idx = static_cast<Node*>(prev->next_)->hash_ % buckets_count_;
                    buckets_[n_idx] = prev;
                }
                NodeAllocTraits::destroy(node_alloc_, static_cast<Node*>(to_del));
                NodeAllocTraits::deallocate(node_alloc_, static_cast<Node*>(to_del), 1);
                --size_;
                return;
            }
            prev = prev->next_;
        }
    }

    iterator erase(iterator it)
    {
        if (it == end()) return end();
        Node* target = it.get_ptr();
        BaseNode* prev = &before_begin_;
        while (prev->next_ != target) prev = prev->next_;

        BaseNode* next_node = target->next_;
        prev->next_ = next_node;

        size_t idx = target->hash_ % buckets_count_;
        if (next_node) {
            size_t n_idx = static_cast<Node*>(next_node)->hash_ % buckets_count_;
            buckets_[n_idx] = prev;
        }

        NodeAllocTraits::destroy(node_alloc_, target);
        NodeAllocTraits::deallocate(node_alloc_, target, 1);
        --size_;
        return iterator(next_node);
    }

    void erase(iterator first, iterator last)
    {
        while (first != last) first = erase(first);
    }

    bool contains(const Key& key) { return get(key).first; }

    void clear()
    {
        BaseNode* current = before_begin_.next_;
        while (current)
        {
            BaseNode* next = current->next_;
            NodeAllocTraits::destroy(node_alloc_, static_cast<Node*>(current));
            NodeAllocTraits::deallocate(node_alloc_, static_cast<Node*>(current), 1);
            current = next;
        }
        before_begin_.next_ = nullptr;
        if (buckets_) for (size_t i = 0; i < buckets_count_; ++i) buckets_[i] = nullptr;
        size_ = 0;
    }

    template<typename Container>
    Container entrySet()
    {
        Container out;
        for (auto it = begin(); it != end(); ++it) out.insert(out.end(), *it);
        return out;
    }

    template<typename Container>
    Container keySet()
    {
        Container out;
        for (auto it = begin(); it != end(); ++it) out.insert(out.end(), it->first);
        return out;
    }

    std::pair<bool, iterator> get(const Key& key)
    {
        if (size_ == 0) return { false, end() };
        size_t idx = getBucketIndex(key);
        BaseNode* cur = buckets_[idx] ? buckets_[idx]->next_ : nullptr;
        while (cur)
        {
            Node* node = static_cast<Node*>(cur);
            if (node->hash_ % buckets_count_ != idx) break;
            if (node->data.first == key) return { true, iterator(cur) };
            cur = cur->next_;
        }
        return { false, end() };
    }

    size_t buckets_count() const { return buckets_count_; }
    size_t size() const { return size_; }
    bool isEmpty() const { return size_ == 0; }

    iterator find(const Key& key)
    {
        std::pair<bool, iterator> result = get(key);
        if (result.first) {
            return result.second;
        }
        return end();
    }

    /*const_iterator find(const Key& key) const
    {
        if (size_ == 0) return end();
        size_t idx = getBucketIndex(key);
        BaseNode* cur = buckets_[idx] ? buckets_[idx]->next_ : nullptr;
        while (cur)
        {
            Node* node = static_cast<Node*>(cur);
            if (node->hash_ % buckets_count_ != idx) break;
            buckets_ && buckets_[idx]) ? buckets_[idx]->next_ : nullptr;

            while (cur)
            {
                Node* node = static_        if (node->data.first == key) return const_iterator(cur);
                cur = cur->next_;
            }
            return end();
        }cast<Node*>(cur);
        if (node->hash_ % buckets_count_ != idx) break;
        if (node->data.first == key)
    }*/
};