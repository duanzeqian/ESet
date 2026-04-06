#include <utility>
#include <stdexcept>
#include <cstddef>
#include <optional>

template<class Key, class Compare = std::less<Key>>
class ESet
{
private:
    enum Color { RED, BLACK };

    struct Node
    {
        std::optional<Key> key;
        Color color;
        Node* left;
        Node* right;
        Node* father;
        size_t siz;

        Node() : color(BLACK), left(nullptr), right(nullptr), father(nullptr), siz(0) {}
        Node(const Key& k, Color c = RED) : key(k), color(c), left(nullptr), right(nullptr), father(nullptr), siz(1) {}
    };

    Node* NIL;
    Node* rt;
    Node* header; // header->left = minimum, header->right = maximum
    size_t siz; // size of the subtree
    Compare cmp;

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
        y->siz = x->siz;
        x->siz = (x->left != NIL ? x->left->siz : 0) + (x->right != NIL ? x->right->siz : 0) + 1;
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
        y->siz = x->siz;
        x->siz = (x->left != NIL ? x->left->siz : 0) + (x->right != NIL ? x->right->siz : 0) + 1;
    }

    void insertFix(Node* n) // insert a new node and fix the balance
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

    void updateMinMax() // update the range of all subtrees
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

    void deleteFix(Node* n) // delete a node and fix the balance
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
        Node* newNode = new Node(oldNode->key.value(), oldNode->color);
        newNode->father = father;
        newNode->siz = oldNode->siz;
        newNode->left = copyTree(oldNode->left, newNode, nil, newNil);
        newNode->right = copyTree(oldNode->right, newNode, nil, newNil);
        return newNode;
    }

    void clearTree(Node* n) // delete a subtree by recursion
    {
        if (n == NIL) return;
        clearTree(n->left);
        clearTree(n->right);
        delete n;
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

    void swap(ESet& other) noexcept
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
        Node* header;
        const ESet* eset;

    public:
        iterator(Node* it = nullptr, Node* h = nullptr, const ESet* es = nullptr) : iter(it), header(h), eset(es) {}

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
            if (iter == nullptr || iter == header) throw("invalid"); // nothing or end()
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
    ESet() : NIL(new Node()), rt(NIL), header(new Node()), siz(0), cmp()
    {
        NIL->color = BLACK;
        NIL->left = NIL->right = NIL->father = NIL;
        header->left = header->right = header;
    }

    ~ESet()
    {
        clearTree(rt);
        delete NIL;
        delete header;
    }

    ESet(const ESet& other) : NIL(new Node()), rt(NIL), header(new Node()), siz(other.siz), cmp(other.cmp)
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

    ESet(ESet&& other) noexcept : NIL(other.NIL), rt(other.rt), header(other.header), siz(other.siz), cmp(std::move(other.cmp))
    {
        other.NIL = nullptr;
        other.rt = nullptr;
        other.header = nullptr;
        other.siz = 0;
    }

    ESet& operator=(const ESet& other)
    {
        if (this != &other)
        {
            ESet tmp(other);
            swap(tmp);
        }
        return *this;
    }

    ESet& operator=(ESet&& other) noexcept
    {
        if (this != &other)
        {
            clearTree(rt);
            delete NIL;
            delete header;
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
        else return iterator(n, header, this);
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
        return iterator(ans, header, this);
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
        return iterator(ans, header, this);
    }

    iterator begin() const noexcept
    {
        return iterator(header->left, header, this);
    }

    iterator end() const noexcept
    {
        return iterator(header, header, this);
    }

    template< class... Args >
    std::pair<iterator, bool> emplace( Args&&... args )
    {
        Key key(std::forward<Args>(args)...);
        Node* existing = findNode(key);
        if (existing != NIL) // already have
        {
            return std::make_pair(iterator(existing, header, this), false);
        }

        Node* n = new Node(key, RED);
        Node* p = NIL;
        Node* x = rt;
        while (x != NIL)
        {
            p = x;
            x->siz++;
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

        return std::make_pair(iterator(n, header, this), true);
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
            y->siz = n->siz;
        }

        Node* node = x->father; // the size may be changed starting from the original y->right
        while (node != NIL)
        {
            node->siz = (node->left != NIL ? node->left->siz : 0) + (node->right != NIL ? node->right->siz : 0) + 1;
            node = node->father;
        }

        if (deleteColor == BLACK) deleteFix(x); // may be unbalanced
        delete n;
        siz--;

        updateMinMax();
        return 1;
    }
};