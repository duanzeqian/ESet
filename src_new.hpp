#include <utility>
#include <stdexcept>
#include <cstddef>
#include <optional>

template<class Key, class Compare = std::less<Key>>
class ESet2
{
private:
    enum Color { RED, BLACK };

    struct Node
    {
        std::optional<Key> key;
        Node* left;
        Node* right;
        Node* father;
        Color color;

        Node() : left(nullptr), right(nullptr), father(nullptr), color(BLACK) {}
        Node(const Key& k, Color c = RED) : key(k), left(nullptr), right(nullptr), father(nullptr), color(c) {}
    };

    Node* NIL;
    Node* rt;
    Node* header; // header->left = minimum, header->right = maximum
    size_t siz; // size of the subtree
    Compare cmp;
    std::allocator<Node> _alloc;

    Node* create_node(const Key& k, Color c = RED)
    {
        Node* p = _alloc.allocate(1);
        new (p) Node(k, c);
        return p;
    }

    Node* create_sentinel()
    {
        Node* p = _alloc.allocate(1);
        new (p) Node();
        return p;
    }

    void destroy_node(Node* p)
    {
        if (p)
        {
            p->~Node();
            _alloc.deallocate(p, 1);
        }
    }

    void leftRotate(Node* x)
    {
        Node* y = x->right;
        x->right = y->left;
        if (y->left != NIL) y->left->father = x;
        y->father = x->father;
        if (x->father == NIL) rt = y; // x = root
        else if (x == x->father->left) x->father->left = y; // x is left child
        else x->father->right = y; // x is right child

        y->left = x;
        x->father = y;
    }

    void rightRotate(Node* x)
    {
        Node* y = x->left;
        x->left = y->right;
        if (y->right != NIL) y->right->father = x;
        y->father = x->father;
        if (x->father == NIL) rt = y; // x = root
        else if (x == x->father->left) x->father->left = y; // x is left child
        else x->father->right = y; // x is right child

        y->right = x;
        x->father = y;
    }

    inline void insertFix(Node* n) // insert a new node and fix the balance
    {
        while (n->father->color == RED) // the son of RED should be BLACK
        {
            Node* p = n->father;
            Node* g = p->father;
            if (p == g->left) // determine which is the uncle node
            {
                // the following cases refer to the description on oi-wiki
                Node* u = g->right;
                if (u->color == RED) // Case 1: recolor the tree
                {
                    p->color = BLACK;
                    u->color = BLACK;
                    g->color = RED;
                    n = g;
                }
                else
                {
                    if (n == p->right) // Case 2: rotate p to Case 3
                    {
                        n = p;
                        leftRotate(n);
                        p = n->father;
                        g = p->father;
                    }
                    p->color = BLACK; // Case 3: rotate g and recolor p, g
                    g->color = RED;
                    rightRotate(g);
                }
            }
            else // another direction
            {
                Node* u = g->left;
                if (u->color == RED) // Case 1: recolor the tree
                {
                    p->color = BLACK;
                    u->color = BLACK;
                    g->color = RED;
                    n = g;
                }
                else
                {
                    if (n == p->left) // Case 2: rotate p to Case 3
                    {
                        n = p;
                        rightRotate(n);
                        p = n->father;
                        g = p->father;
                    }
                    p->color = BLACK; // Case 3: rotate g and recolor p, g
                    g->color = RED;
                    leftRotate(g);
                }
            }
        }
        rt->color = BLACK; // the fifth requirement of RBTree
    }

    inline void updateMinMax() // update the range of all subtrees
    {
        if (rt == NIL)
        {
            header->left = header;
            header->right = header;
        }
        else
        {
            Node* x = rt;
            while (x->left != NIL) x = x->left;
            header->left = x;

            x = rt;
            while (x->right != NIL) x = x->right;
            header->right = x;
        }
    }

    void transplant(Node* u, Node* v) // substitute u with v
    {
        if (u->father == NIL) rt = v;
        else if (u == u->father->left) u->father->left = v;
        else u->father->right = v;
        v->father = u->father;
    }

    inline void deleteFix(Node* n) // delete a node and fix the balance
    {
        while (n != rt && n->color == BLACK)
        {
            Node* p = n->father;
            if (n == p->left)
            {
                Node* s = p->right;
                if (s->color == RED) // Case 1: rotate s to top and recolor s, p
                {
                    s->color = BLACK;
                    p->color = RED;
                    leftRotate(p);
                    p = n->father;
                    s = p->right; // switch to other cases
                }
                if (s->left->color == BLACK && s->right->color == BLACK) // Case 2: let s color red
                {
                    s->color = RED;
                    n = p;
                }
                else
                {
                    if (s->right->color == BLACK) // Case 3: rotate to switch to Case 4
                    {
                        s->left->color = BLACK;
                        s->color = RED;
                        rightRotate(s);
                        p = n->father;
                        s = p->right;
                    }
                    // Case 4: recolor s, p, d (i.e., s->right) and rotate p
                    s->color = p->color;
                    p->color = BLACK;
                    s->right->color = BLACK;
                    leftRotate(p);
                    n = rt; // finish balancing
                }
            }
            else // all the same
            {
                Node* s = p->left;
                if (s->color == RED)
                {
                    s->color = BLACK;
                    p->color = RED;
                    rightRotate(p);
                    p = n->father;
                    s = p->left;
                }
                if (s->right->color == BLACK && s->left->color == BLACK)
                {
                    s->color = RED;
                    n = p;
                }
                else
                {
                    if (s->left->color == BLACK)
                    {
                        s->right->color = BLACK;
                        s->color = RED;
                        leftRotate(s);
                        p = n->father;
                        s = p->left;
                    }
                    s->color = p->color;
                    p->color = BLACK;
                    s->left->color = BLACK;
                    rightRotate(p);
                    n = rt;
                }
            }
        }
        n->color = BLACK;
    }

    Node* copyTree(Node* oldNode, Node* father, Node* nil, Node*& newNil) // copy a subtree by recursion
    {
        if (oldNode == nil) return newNil;
        Node* newNode = create_node(oldNode->key.value(), oldNode->color);
        newNode->father = father;
        newNode->left = copyTree(oldNode->left, newNode, nil, newNil);
        newNode->right = copyTree(oldNode->right, newNode, nil, newNil);
        return newNode;
    }

    void clearTree(Node* n) // delete a subtree by recursion
    {
        if (n == NIL) return;
        clearTree(n->left);
        clearTree(n->right);
        destroy_node(n);
    }

    Node* findNode(const Key& key) const // find the node with value 'key'
    {
        Node* cur = rt;
        while (cur != NIL) // divide and conquer
        {
            if (cmp(key, cur->key.value())) cur = cur->left;
            else if (cmp(cur->key.value(), key)) cur = cur->right;
            else return cur;
        }
        return NIL;
    }

    size_t countRange(Node* n, const Key& l, const Key& r) const
    {
        if (n == NIL) return 0;
        if (cmp(n->key.value(), l)) return countRange(n->right, l, r);
        else if (cmp(r, n->key.value())) return countRange(n->left, l, r);
        else return 1 + countRange(n->left, l, r) + countRange(n->right, l, r);
    }

    void swap(ESet2& other) noexcept
    {
        std::swap(NIL, other.NIL);
        std::swap(rt, other.rt);
        std::swap(header, other.header);
        std::swap(siz, other.siz);
        std::swap(cmp, other.cmp);
    }

public:
    class iterator
    {
    private:
        Node* iter;
        const ESet2* eset;

    public:
        iterator(Node* it = nullptr, const ESet2* es = nullptr) : iter(it), eset(es) {}

        /**
         * ++iter
         */
        iterator &operator++()
        {
            if (iter == eset->header) return *this; // ++end()

            if (iter->right != eset->NIL) // suc is in the right subtree
            {
                Node* suc = iter->right;
                while (suc->left != eset->NIL) suc = suc->left;
                iter = suc;
                return *this;
            } 
            else // suc is somewhere else
            {
                Node* suc = iter->father;
                while (suc != eset->NIL && iter == suc->right) // find where can we go right
                {
                    iter = suc;
                    suc = suc->father;
                }
                if (suc == eset->NIL) iter = eset->header; // reach the end()
                else iter = suc; // find the suc by left-middle-right
                return *this;
            }
        }
        /**
         * iter++
         */
        iterator operator++(int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        /**
         * --iter
         */
        iterator &operator--()
        {
            auto header = eset->header;
            if (iter == header) // --end()
            {
                if (header->right != header) // have the maximum, otherwise the tree empty
                {
                    iter = header->right;
                }
                return *this;
            }

            if (iter == header->left) // --begin()
            {
                return *this;
            }

            if (iter->left != eset->NIL)
            {
                Node* pre = iter->left;
                while (pre->right != eset->NIL) pre = pre->right;
                iter = pre;
                return *this;
            }
            else
            {
                Node* pre = iter->father;
                while (pre != eset->NIL && iter == pre->left)
                {
                    iter = pre;
                    pre = pre->father;
                }
                iter = pre;
                return *this;
            }
        }
        /**
         * iter--
         */
        iterator operator--(int)
        {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

        const Key& operator*() const
        {
            if (iter == nullptr || iter == eset->header) throw("invalid"); // nothing or end()
            return iter->key.value();
        }

        const Key* operator->() const noexcept
        {
            return &(iter->key.value());
        }

        bool operator==(const iterator& other) const
        {
            return iter == other.iter && eset == other.eset;
        }

        bool operator!=(const iterator& other) const
        {
            return !(*this == other);
        }
    };

    // Basic functions
    ESet2() : NIL(create_sentinel()), rt(NIL), header(create_sentinel()), siz(0), cmp()
    {
        NIL->color = BLACK;
        NIL->left = NIL->right = NIL->father = NIL;
        header->left = header->right = header;
    }

    ~ESet2()
    {
        clearTree(rt);
        destroy_node(NIL);
        destroy_node(header);
    }

    ESet2(const ESet2& other) : NIL(create_sentinel()), rt(NIL), header(create_sentinel()), siz(other.siz), cmp(other.cmp)
    {
        *NIL = *(other.NIL);
        NIL->left = NIL->right = NIL->father = NIL;
        header->left = header->right = header;
        if (other.rt != other.NIL)
        {
            rt = copyTree(other.rt, NIL, other.NIL, NIL);
            updateMinMax();
        }
    }

    ESet2(ESet2&& other) noexcept : NIL(other.NIL), rt(other.rt), header(other.header), siz(other.siz), cmp(std::move(other.cmp))
    {
        other.NIL = nullptr;
        other.rt = nullptr;
        other.header = nullptr;
        other.siz = 0;
    }

    ESet2& operator=(const ESet2& other)
    {
        if (this != &other)
        {
            ESet2 tmp(other);
            swap(tmp);
        }
        return *this;
    }

    ESet2& operator=(ESet2&& other) noexcept
    {
        if (this != &other)
        {
            clearTree(rt);
            destroy_node(NIL);
            destroy_node(header);
            NIL = other.NIL;
            rt = other.rt;
            header = other.header;
            siz = other.siz;
            cmp = std::move(other.cmp);
            other.NIL = nullptr;
            other.rt = nullptr;
            other.header = nullptr;
            other.siz = 0;
        }
        return *this;
    }

    // ESet functions
    size_t range( const Key& l, const Key& r ) const
    {
        if (cmp(r, l)) return 0;
        return countRange(rt, l, r);
    }

    size_t size() const noexcept
    {
        return siz;
    }

    iterator find(const Key& key) const
    {
        Node* n = findNode(key);
        if (n == NIL) return end();
        else return iterator(n, this);
    }

    iterator lower_bound(const Key& key) const
    {
        Node* cur = rt;
        Node* ans = header;
        while (cur != NIL)
        {
            if (!cmp(cur->key.value(), key))
            {
                ans = cur;
                cur = cur->left;
            }
            else cur = cur->right;
        }
        return iterator(ans, this);
    }

    iterator upper_bound(const Key& key) const
    {
        Node* cur = rt;
        Node* ans = header;
        while (cur != NIL)
        {
            if (cmp(key, cur->key.value()))
            {
                ans = cur;
                cur = cur->left;
            }
            else cur = cur->right;
        }
        return iterator(ans, this);
    }

    iterator begin() const noexcept
    {
        return iterator(header->left, this);
    }

    iterator end() const noexcept
    {
        return iterator(header, this);
    }

    template< class... Args >
    std::pair<iterator, bool> emplace( Args&&... args )
    {
        Key key(std::forward<Args>(args)...);
        Node* existing = findNode(key);
        if (existing != NIL) // already have
        {
            return std::make_pair(iterator(existing, this), false);
        }

        Node* n = create_node(key, RED);
        Node* p = NIL;
        Node* x = rt;
        while (x != NIL)
        {
            p = x;
            if (cmp(n->key.value(), x->key.value())) x = x->left; // no key in ESet, so just 2 cases
            else x = x->right;
        }
        n->father = p;
        n->left = n->right = NIL;

        if (p == NIL) rt = n;
        else if (cmp(n->key.value(), p->key.value())) p->left = n;
        else p->right = n;
        
        insertFix(n);
        siz++;

        // update minimum and maximum
        if (header->left == header || cmp(n->key.value(), header->left->key.value())) header->left = n;
        if (header->right == header || cmp(header->right->key.value(), n->key.value())) header->right = n;

        return std::make_pair(iterator(n, this), true);
    }

    size_t erase( const Key& key )
    {
        Node* n = findNode(key);
        if (n == NIL) return 0;

        Node* y = n;
        Node* x;
        Color deleteColor = y->color;

        if (n->left == NIL) // only right subtree left
        {
            x = n->right;
            transplant(n, n->right);
        }
        else if (n->right == NIL) // only left subtree left
        {
            x = n->left;
            transplant(n, n->left);
        }
        else
        {
            y = n->right;
            while (y->left != NIL) y = y->left; // let the element just bigger than n be the new root
            deleteColor = y->color;

            x = y->right;
            if (y->father == n) x->father = y; // no necessity to delete y, cause it is exactly the right subtree of n
            else
            {
                transplant(y, y->right); // delete y in the original place
                y->right = n->right; // y become the right subtree of n
                y->right->father = y;
            }
            transplant(n, y); // delete n and substitute it with y
            y->left = n->left;
            y->left->father = y;
            y->color = n->color;
        }

        Node* node = x->father; // the size may be changed starting from the original y->right

        if (deleteColor == BLACK) deleteFix(x); // may be unbalanced
        destroy_node(n);
        siz--;

        updateMinMax();
        return 1;
    }
};