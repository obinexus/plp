// phenavl.c
// Compile: gcc -std=c11 -O2 -Wall phenavl.c -o phenavl

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
// --------------------- Domain Types ---------------------
typedef enum {
    QUAL_NONE = 0,
    QUAL_RESILIENT = 1<<0,
    QUAL_CREATIVE  = 1<<1,
    QUAL_ANXIOUS   = 1<<2,
    QUAL_OPTIMIST  = 1<<3,
    // add more qualitative flags...
} QualFlags;

typedef struct {
    double score;         // quantitative value (phenovalude)
    uint64_t visits;      // occurrence counter
    QualFlags qual;       // qualitative bitset
    char *meta;           // optional free-form label or tag (phonemeory)
} Phenotype;

// --------------------- AVL (children map) ---------------------
// key is a char (or small int representing phoneme)
// value is pointer to trie node
struct TrieNode; // forward

typedef struct AVLChild {
    char key;
    struct TrieNode *child;
    int height;
    struct AVLChild *left;
    struct AVLChild *right;
} AVLChild;

// --------------------- Trie Node ---------------------
typedef struct TrieNode {
    // Marker: does this node terminate a stored token?
    int terminal;
    Phenotype *p;         // NULL if no phenotype assigned
    // children represented as AVL tree keyed by char
    AVLChild *children;
} TrieNode;

// --------------------- Utility ---------------------
static int max(int a, int b) { return (a > b) ? a : b; }
static int height_avl(AVLChild *n) { return n ? n->height : 0; }
static void update_height(AVLChild *n) { if (n) n->height = 1 + max(height_avl(n->left), height_avl(n->right)); }
static int balance_factor(AVLChild *n) { return n ? height_avl(n->left) - height_avl(n->right) : 0; }

static AVLChild* rotate_right(AVLChild *y) {
    AVLChild *x = y->left;
    AVLChild *T2 = x->right;
    x->right = y;
    y->left = T2;
    update_height(y); update_height(x);
    return x;
}

static AVLChild* rotate_left(AVLChild *x) {
    AVLChild *y = x->right;
    AVLChild *T2 = y->left;
    y->left = x;
    x->right = T2;
    update_height(x); update_height(y);
    return y;
}

static AVLChild* avl_insert_child(AVLChild *root, char key, TrieNode *child) {
    if (!root) {
        AVLChild *n = malloc(sizeof(AVLChild));
        if (!n) { perror("malloc"); exit(1); }
        n->key = key; n->child = child; n->left = n->right = NULL; n->height = 1;
        return n;
    }
    if (key < root->key) root->left = avl_insert_child(root->left, key, child);
    else if (key > root->key) root->right = avl_insert_child(root->right, key, child);
    else { // replace child pointer
        root->child = child;
        return root;
    }

    update_height(root);
    int bf = balance_factor(root);

    // LL
    if (bf > 1 && key < root->left->key) return rotate_right(root);
    // RR
    if (bf < -1 && key > root->right->key) return rotate_left(root);
    // LR
    if (bf > 1 && key > root->left->key) { root->left = rotate_left(root->left); return rotate_right(root); }
    // RL
    if (bf < -1 && key < root->right->key) { root->right = rotate_right(root->right); return rotate_left(root); }

    return root;
}

static AVLChild* avl_find_child(AVLChild *root, char key) {
    while (root) {
        if (key == root->key) return root;
        root = (key < root->key) ? root->left : root->right;
    }
    return NULL;
}

// For enumeration (in-order)
typedef void (*child_visit_fn)(char key, TrieNode *child, void *ctx);
static void avl_inorder(AVLChild *root, child_visit_fn fn, void *ctx) {
    if (!root) return;
    avl_inorder(root->left, fn, ctx);
    fn(root->key, root->child, ctx);
    avl_inorder(root->right, fn, ctx);
}

// --------------------- Trie operations ---------------------
static TrieNode* trie_node_new(void) {
    TrieNode *n = malloc(sizeof(TrieNode));
    if (!n) { perror("malloc"); exit(1); }
    n->terminal = 0;
    n->p = NULL;
    n->children = NULL;
    return n;
}

static Phenotype* phenotype_new(double score, QualFlags qual, const char *meta) {
    Phenotype *p = malloc(sizeof(Phenotype));
    if (!p) { perror("malloc"); exit(1); }
    p->score = score; p->visits = 0; p->qual = qual;
    if (meta) {
        p->meta = strdup(meta);
    } else p->meta = NULL;
    return p;
}

static void phenotype_free(Phenotype *p) {
    if (!p) return;
    if (p->meta) free(p->meta);
    free(p);
}

// insert key (C-string) with phenotype info; if exists, update
void patrie_insert(TrieNode *root, const char *key, double score, QualFlags qual, const char *meta) {
    TrieNode *cur = root;
    for (size_t i = 0; key[i] != '\0'; ++i) {
        char ch = key[i];
        AVLChild *ac = avl_find_child(cur->children, ch);
        if (!ac) {
            TrieNode *newnode = trie_node_new();
            cur->children = avl_insert_child(cur->children, ch, newnode);
            ac = avl_find_child(cur->children, ch);
        }
        cur = ac->child;
    }
    cur->terminal = 1;
    if (!cur->p) cur->p = phenotype_new(score, qual, meta);
    else {
        // update: combine quantitatively and qualitatively (you decide merge strategy)
        cur->p->score = score;
        cur->p->qual = qual;
        if (cur->p->meta) free(cur->p->meta);
        cur->p->meta = meta ? strdup(meta) : NULL;
    }
}

// lookup
Phenotype* patrie_lookup(TrieNode *root, const char *key) {
    TrieNode *cur = root;
    for (size_t i = 0; key[i] != '\0'; ++i) {
        AVLChild *ac = avl_find_child(cur->children, key[i]);
        if (!ac) return NULL;
        cur = ac->child;
    }
    if (cur && cur->terminal) {
        cur->p->visits++;
        return cur->p;
    }
    return NULL;
}

// enumerate all tokens with a callback (DFS)
// ---------- top-level visitor helpers (no nested functions) ----------

// forward
typedef void (*token_cb)(const char *token, Phenotype *p, void *ctx);

// helper used by avl_inorder during enumeration
static void visit_tmp_fn(char key, TrieNode *child, void *ctx_) {
    EnumCtx *e = (EnumCtx*)ctx_;
    ensure_buf(e, e->len + 1);      // ensure room for another char (we also add '\0' later)
    e->buf[e->len++] = key;
    trie_dfs(child, e);
    e->len--;
}

// helper used by avl_inorder during free
static void visit_free_fn(char key, TrieNode *child, void *ctx_) {
    (void)key; (void)ctx_;
    trie_free(child);
}

typedef struct { char *buf; size_t cap; size_t len; token_cb cb; void *ctx; } EnumCtx;

static void ensure_buf(EnumCtx *ec, size_t need) {
    if (ec->cap <= need) {
        size_t nc = (ec->cap==0) ? 64 : ec->cap*2;
        while (nc <= need) nc *= 2;
        ec->buf = realloc(ec->buf, nc);
        ec->cap = nc;
    }
}
static void trie_dfs(TrieNode *node, EnumCtx *ec) {
    if (!node) return;
    if (node->terminal) {
        // ensure there's room for NUL
        ensure_buf(ec, ec->len + 1);
        ec->buf[ec->len] = '\0';
        ec->cb(ec->buf, node->p, ec->ctx);
    }
    // visit children in-order using top-level visitor
    avl_inorder(node->children, visit_tmp_fn, ec);
}


// wrapper
void patrie_enumerate(TrieNode *root, token_cb cb, void *ctx) {
    EnumCtx ec = { .buf = NULL, .cap = 0, .len = 0, .cb = cb, .ctx = ctx };
    trie_dfs(root, &ec);
    free(ec.buf);
}

// free
static void avl_free(AVLChild *root) {
    if (!root) return;
    avl_free(root->left);
    avl_free(root->right);
    // free the trie subtree it points to
    // but caller should free trie nodes separately; we'll not free child->child here
    free(root);
}

static void trie_free(TrieNode *node) {
    if (!node) return;
    // traverse children and free subtries
    avl_inorder(node->children, visit_free_fn, NULL);
    avl_free(node->children);
    if (node->p) phenotype_free(node->p);
    free(node);
}

// --------------------- Example usage ---------------------
static void print_token(const char *token, Phenotype *p, void *ctx) {
    (void)ctx;
    printf("token='%s' score=%.3f visits=%" PRIu64 " qual=0x%x meta=%s\n",
           token, p ? p->score : 0.0, p ? p->visits : 0, p ? p->qual : 0,
           (p && p->meta) ? p->meta : "NULL");
}

int main(void) {
    TrieNode *root = trie_node_new();

    patrie_insert(root, "phenotype", 0.72, QUAL_RESILIENT | QUAL_CREATIVE, "root concept");
    patrie_insert(root, "phenovalude", 0.85, QUAL_OPTIMIST, "value metric");
    patrie_insert(root, "phoneme", 0.45, QUAL_ANXIOUS, "sound unit");

    Phenotype *p = patrie_lookup(root, "phenotype");
    if (p) printf("Found phenotype -> score %.2f meta=%s\n", p->score, p->meta);


    printf("Enumerate all tokens:\n");
    printf("About to enumerate tokens, count=%d\n", count);
    patrie_enumerate(root, print_token, NULL);

    trie_free(root);
    return 0;
}
