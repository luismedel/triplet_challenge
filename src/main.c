
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 4096

#define IS_UPPER(c) ((c) >= 'A' && (c) <= 'Z')
#define IS_VALID(c) ((c) == '\'' || (c) >= 'a' && (c) <= 'z' || (c) >= 'A' && (c) <= 'Z')

/**
 * We store the triplets in a tree, ordered by key hash.
 * 
 * A triplet node consists of:
 * - The 3 items that conforms the key
 * - A hash of the key (using a slightly modification of MurmurHash to work on 3 parts)
 * - A count and pointers to child triplets, based on hash order.
 */
typedef struct triplet {
    char *a, *b, *c;
    int hash;
    int count;

    struct triplet *left;
    struct triplet *right;
} triplet_t;

/**
 * We create chunks of BUFFER_SIZE items to minimize the number of mallocs and, hopefully, memory fragmentation.
 * This should help when traversing the tree searching for triplets.
 * 
 * For cleaning this mess, we rely on the system :-)
 */
triplet_t *buffer;
triplet_t *bp; // points to the next available triplet

// Our triplet tree root
triplet_t *root;

void new_triplet_buffer ()
{
    buffer = (triplet_t *) calloc (BUFFER_SIZE, sizeof (triplet_t));
    bp = buffer + BUFFER_SIZE - 1;
}

/**
 * Modification of MurmurHash, by Austin Appleby (https://sites.google.com/site/murmurhash/)
 */
int murmurhash (char *a, char *b, char *c, int seed)
{
    const unsigned int m = 0xc6a4a793;
    const int r = 16;

    unsigned int lena = strlen (a);
    unsigned int lenb = strlen (b);
    unsigned int lenc = strlen (c);

    unsigned int h = seed ^ ((lena + lenb + lenc) * m);

    #define MURMUR_LOOP(data,len) while (len >= 4) { unsigned int k = *(unsigned int *)data; h += k; h *= m; h ^= h >> 16; data += 4; len -= 4; }
    #define MURMUR_REMAINING(data,len) switch(len) { case 3: h += data[2] << 16; case 2: h += data[1] << 8; case 1: h += data[0]; h *= m; h ^= h >> r; }

    MURMUR_LOOP (a, lena);
    MURMUR_REMAINING (a, lena);

    MURMUR_LOOP (b, lenb);
    MURMUR_REMAINING (b, lenb);

    MURMUR_LOOP (c, lenc);
    MURMUR_REMAINING (c, lenc);

    h *= m;
    h ^= h >> 10;
    h *= m;
    h ^= h >> 17;

    return h;
}

triplet_t *make_triplet (char *a, char *b, char *c, int hash)
{
    triplet_t *result = bp--;
    result->a = a;
    result->b = b;
    result->c = c;
    result->hash = hash;

    // If the buffer is exhausted, create a new one.
    if (bp < buffer)
        new_triplet_buffer ();

    return result;
}

triplet_t *find (triplet_t* node, char *a, char *b, char *c, int hash)
{
    int comp = node->hash - hash;

    if (comp < 0)
    {
        if (node->left)
            return find (node->left, a, b, c, hash);

        node->left = make_triplet (a, b, c, hash);
        return find (node->left, a, b, c, hash);
    }
    else if (comp > 0)
    {
        if (node->right)
            return find (node->right, a, b, c, hash);
        node->right = make_triplet (a, b, c, hash);

        return find (node->right, a, b, c, hash);
    }

    return node;
}

char* read_file (char* path)
{
    FILE *fp = fopen (path, "r");
    if (fp == NULL)
        return NULL;

    fseek (fp, 0, SEEK_END);
    long filesize = ftell (fp);
    fseek (fp, 0, SEEK_SET);  
 
    char *result = (char *) malloc (sizeof (char) * filesize);   
    if (result == NULL)
        return NULL;
 
    fread (result, sizeof (char), filesize, fp);
    fclose (fp);

    return result;
}

int next_word (char **input, char **dest)
{
    char c = **input;
    if (c == '\0')
        return 0;

    // Discard invalid chars
    while (c != '\0' && !IS_VALID (c))
        c = *(++ *input);

    if (c == '\0')
        return 0;

    *dest = *input;
    while (c != '\0' && IS_VALID (c))
    {
        // Normalize to lowercase
        if (IS_UPPER(c))
            **input += 32;

        c = *(++ *input);
    }

    // Mark the end of the word
    if (c != '\0')
    {
        **input = '\0';
        ++ *input;
    }

    return 1;
}

void inc (char *a, char *b, char *c)
{
    int hash = murmurhash (a, b, c, 0);
    triplet_t *node = find (root, a, b, c, hash);
    node->count++;
}

int generate_triplets (char *input)
{
    char *a, *b, *c;

    if (!next_word (&input, &a))
        return 0;

    if (!next_word (&input, &b))
        return 0;

    int count = 0;
    while (next_word (&input, &c))
    {
        inc (a, b, c);
        a = b;
        b = c;

        count++;
    }

    return count;
}

void get_top (triplet_t *node, triplet_t **top, int count)
{
    int idx = -1;
    for (int i = 0; i < count; i++)
    {
        if (node->count < top[i]->count)
            break;
        idx = i;
    }

    if (idx != -1)
    {
        for (int i = idx - 1; i >= 0; i--)
            top[i] = top[i + 1];

        top[idx] = node;
    }

    if (node->left)
        get_top (node->left, top, count);

    if (node->right)
        get_top (node->right, top, count);
}

void print_top_triplets ()
{
    triplet_t **ranking = malloc (sizeof (triplet_t *) * 3);
    for (int i = 0; i < 3; i++)
        ranking[i] = root;

    get_top (root, ranking, 3);

    for (int i = 2; i >= 0; i--)
        printf ("%s %s %s - %d\n", ranking[i]->a, ranking[i]->b, ranking[i]->c, ranking[i]->count);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf ("Filename expected");
        return 1;
    }

    char* input = read_file (argv[1]);
    if (input == NULL)
    {
        printf ("Error loading '%s'.", argv[1]);
        return 1;
    }

    new_triplet_buffer ();
    root = make_triplet (NULL, NULL, NULL, 0);

    if (generate_triplets (input) == 0)
    {
        printf ("Too few words.");
        return 1;
    }

    print_top_triplets ();

    return 0;
}
