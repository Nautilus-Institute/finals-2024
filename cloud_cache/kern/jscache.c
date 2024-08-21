//#define MODULE
#define LINUX
//#define __KERNEL__

#include <linux/module.h>  /* Needed by all modules */
#include <linux/kernel.h>  /* Needed for KERN_ALERT */
#include <linux/types.h>
#include <linux/string.h>

#include <linux/slab.h>
#include <asm/uaccess.h>

#include <linux/fs.h>
#include <linux/miscdevice.h>

#include <linux/string.h>
#include <linux/string_helpers.h>
#include <linux/ctype.h>
#include <linux/kallsyms.h>

#include <crypto/hash.h>

#define KALLSYMS_PATH "/proc/kallsyms"
#define MAX_LINE_LENGTH 256

//https://github.com/xcellerator/linux_kernel_hacking/issues/3
unsigned long kaddr_lookup_name(const char *fname_raw)
{
    int i;
    unsigned long kaddr;
    char *fname_lookup, *fname;

    fname_lookup = kzalloc(NAME_MAX, GFP_KERNEL);
    if (!fname_lookup)
        return 0;

    fname = kzalloc(strlen(fname_raw)+4, GFP_KERNEL);
    if (!fname)
        return 0;

    /*
     * We have to add "+0x0" to the end of our function name
     * because that's the format that sprint_symbol() returns
     * to us. If we don't do this, then our search can stop
     * prematurely and give us the wrong function address!
     */
    strcpy(fname, fname_raw);
    strcat(fname, "+0x0");

    /*
     * Get the kernel base address:
     * sprint_symbol() is less than 0x100000 from the start of the kernel, so
     * we can just AND-out the last 3 bytes from it's address to the the base
     * address.
     * There might be a better symbol-name to use?
     */
    kaddr = (unsigned long) &sprint_symbol;
    kaddr &= 0xffffffffff000000;

    /*
     * All the syscalls (and all interesting kernel functions I've seen so far)
     * are within the first 0x100000 bytes of the base address. However, the kernel
     * functions are all aligned so that the final nibble is 0x0, so we only
     * have to check every 16th address.
     */
    for ( i = 0x0 ; i < 0x100000 ; i++ )
    {
        /*
         * Lookup the name ascribed to the current kernel address
         */
        sprint_symbol(fname_lookup, kaddr);

        /*
         * Compare the looked-up name to the one we want
         */
        if ( strncmp(fname_lookup, fname, strlen(fname)) == 0 )
        {
            /*
             * Clean up and return the found address
             */
            kfree(fname_lookup);
            return kaddr;
        }
        /*
         * Jump 16 addresses to next possible address
         */
        kaddr += 0x10;
    }
    /*
     * We didn't find the name, so clean up and return 0
     */
    kfree(fname_lookup);
    return 0;
}

#define TYPE_STR 0
#define TYPE_OBJ 1
#define TYPE_INT 2
#define TYPE_DOUBLE 3
#define TYPE_ARR 4

struct value {
    unsigned char type;
    unsigned long data;
};

struct dbl {
    unsigned char type;
    double data;
};

struct str {
    unsigned char type;
    unsigned int len;
    unsigned char data[];
};

struct prop {
    struct str* key;
    struct value* value;
};


struct obj {
    unsigned char type;
    unsigned int len;
    struct prop* props;
};

struct arr {
    unsigned char type;
    unsigned int len;
    struct value** values;
};

struct qjs_str {
    unsigned int ref;
    unsigned int len;
    unsigned int hash;
    unsigned int flags;
};

struct qjs_obj {
    unsigned int ref;
    unsigned char gc;
    unsigned char flags;
    unsigned short class_id;
    unsigned long obj_prev; // 8
    unsigned long obj_next; // 16
    unsigned long shape; // 24
    unsigned long prop; // 32
    unsigned long weak_ref; // 40
}; // 48

struct qjs_shape {
    unsigned int ref;
    unsigned char gc;
    unsigned char is_hashed;
    unsigned char has_small_index;
    unsigned char pad;
    unsigned int hash;
    unsigned int prop_hash_mask;
    int prop_size;
    int prop_count;
    unsigned long next_hash;
    unsigned long proto;
};

struct str* g_selector = NULL;

unsigned long runtime_ptr = 0;
unsigned long response_buffer_ptr = 0;

void dump(void* data, int len) {
    /*
        Dump kernel space memory
    */
    unsigned long ptr = (unsigned long)data;
    for (int i = 0; i < len; i+=8) {
        printk("%d: %lx\n", i, *(unsigned long*)(ptr+i));
    }
}
void user_dump(void* data, int len) {
    /*
        Dump user space memory
    */
    unsigned long ptr = (unsigned long)data;
    printk("========== %lx ==========\n", ptr);
    for (int i = 0; i < len; i+=8) {
        unsigned long value;
        get_user(value, (unsigned long*)(ptr+i));
        printk("%d: %lx\n", i, value);
    }
    printk("=========================\n\n");
}

unsigned long copy_from_user_maybe(void* dst, void* src, unsigned long len) {
    /*
        Copy from user space, but fallback to memcpy if it fails
    */
    unsigned long bad_res = copy_from_user(dst, src, len);
    if (bad_res) {
        printk("WARN: Failed to copy %ld bytes from %lx to %lx\n", bad_res, (unsigned long)src, (unsigned long)dst);
    } else {
        return 0;
    }
    // Fallback
    memcpy(dst, src, len);
    return 0;
}
unsigned long copy_to_user_maybe(void* dst, void* src, unsigned long len) {
    /*
        Copy to user space, but fallback to memcpy if it fails
    */
    unsigned long bad_res = copy_to_user(dst, src, len);
    if (bad_res) {
        printk("WARN: Failed to copy %ld bytes from %lx to %lx\n", bad_res, (unsigned long)src, (unsigned long)dst);
    } else {
        return 0;
    }
    // Fallback
    memcpy(dst, src, len);
    return 0;
}
    

struct str* get_user_string_raw(void* ptr, unsigned int len) {
    /*
        Get a string from a pointer
    */
    struct str* s = kmalloc(sizeof(struct str) + len + 1, GFP_KERNEL);
    s->type = TYPE_STR;
    s->len = len;
    //printk("Copying %d bytes from %lx to %lx\n", len, (unsigned long)ptr, (unsigned long)s->data);
    unsigned long bad_res = copy_from_user_maybe(s->data, ptr, len);
    if (bad_res) {
        printk("WARN: Failed to copy %ld bytes from %lx to %lx\n", bad_res, (unsigned long)ptr, (unsigned long)s->data);
    }
    s->data[len] = 0;
    //printk("Copied %d bytes from %lx to %lx: %lx\n", bad_res, ptr, s->data, *(unsigned long*)s->data);
    //dump(s, 128);
    return s;
}

static struct str* consume_str(void* ptr) {
    /*
        Consume a string from a pointer
    */
    unsigned long ptr_ = (unsigned long)ptr;
    printk("Consuming String @ %lx\n", (unsigned long)ptr);
    user_dump(ptr, 128);
    unsigned int len = 0;
    get_user(len, (unsigned int*)(ptr_ + 4));
    printk("Length: %d\n", len);
    struct str* s = get_user_string_raw((void*)(ptr_ + 16), len);
    printk("Got string `%s`\n", s->data);
    return s;
}

struct value* get_atom_str(unsigned int offset) {
    /*
        Get an atom string from the runtime
    */
    printk("Getting Atom %d\n", offset);
    unsigned long table_ptr;
    get_user(table_ptr, (unsigned long*)(runtime_ptr + 0x60));
    printk("--> Table @ %lx\n", table_ptr);
    unsigned long atom_ptr = table_ptr + 8 * offset;
    printk("--> Atom @ %lx\n", atom_ptr);
    unsigned long atom;
    get_user(atom, (unsigned long*)atom_ptr);
    printk("--> Atom = %lx\n", atom);

    // TODO check if int or str
    
    struct str* s = consume_str((void*)atom);
    printk("Got atom `%s`\n", s->data);
    return (struct value*)s;
}

static struct value* consume(void* ptr);

static void json_write(struct value* val, unsigned long* output_p) {
    char buf[256] = {0};

    if (output_p == NULL || *output_p == 0) {
        *output_p = response_buffer_ptr;
    }
    if (*output_p == 0) {
        printk("No output buffer...\n");
        return;
    }
    unsigned long v_ = (unsigned long)val;
    if ((v_&0x8000000000000000) == 0) {
        printk("Writing inline int %ld\n", (unsigned int)val);
        // Integer type
        snprintf(buf, 256, "%ld", (unsigned int)val);
        copy_to_user_maybe((void*)*output_p, buf, strlen(buf)+1);
        *output_p += strlen(buf);
        return;
    }

    if (val == NULL) {
        printk("Writing null\n");
        copy_to_user_maybe((void*)*output_p, "null", 5);
        *output_p += 4;
        return;
    }

    if (val->type == TYPE_STR) {
        printk("Writing string\n");
        struct str* s = (struct str*)val;
        copy_to_user_maybe((void*)*output_p, "\"", 1);
        *output_p += 1;
        copy_to_user_maybe((void*)*output_p, s->data, s->len);
        *output_p += s->len;
        copy_to_user_maybe((void*)*output_p, "\"", 2);
        *output_p += 1;
    } else if (val->type == TYPE_OBJ) {
        printk("Writing object\n");
        struct obj* o = (struct obj*)val;
        copy_to_user_maybe((void*)*output_p, "{", 1);
        *output_p += 1;
        for (int i = 0; i < o->len; i++) {
            struct prop* p = &o->props[i];
            json_write((struct value*)p->key, output_p);
            copy_to_user_maybe((void*)*output_p, ":", 1);
            *output_p += 1;
            json_write((struct value*)p->value, output_p);
            if (i < o->len - 1) {
                copy_to_user_maybe((void*)*output_p, ",", 1);
                *output_p += 1;
            }
        }
        copy_to_user_maybe((void*)*output_p, "}", 2);
        *output_p += 1;
    } else if (val->type == TYPE_INT) {
        printk("Writing int\n");
        snprintf(buf, 256, "%ld", val->data);
        copy_to_user_maybe((void*)*output_p, buf, strlen(buf));
        *output_p += strlen(buf);
    } else if (val->type == TYPE_DOUBLE) {
        printk("Writing double\n");
        struct dbl* d = (struct dbl*)val;
        snprintf(buf, 256, "%ld", d->data);
        copy_to_user_maybe((void*)*output_p, buf, strlen(buf));
        *output_p += strlen(buf);
    } else if (val->type == TYPE_ARR) {
        printk("Writing array\n");
        struct arr* a = (struct arr*)val;
        copy_to_user_maybe((void*)*output_p, "[", 1);
        *output_p += 1;
        for (int i = 0; i < a->len; i++) {
            json_write((struct value*)a->values[i], output_p);
            if (i < a->len - 1) {
                copy_to_user_maybe((void*)*output_p, ",", 1);
                *output_p += 1;
            }
        }
        copy_to_user_maybe((void*)*output_p, "]", 2);
        *output_p += 1;
    } else {
        printk("Writing unknown\n");
        copy_to_user_maybe((void*)*output_p, "null", 4);
        *output_p += 4;
    }
}

static char* read_authenticity_token(void)
{
    struct file *file;
    loff_t pos = 0;
    char *buffer = NULL;
    int ret;

    file = filp_open("/flag", O_RDONLY, 0);
    if (IS_ERR(file)) {
        pr_err("Failed to open %s\n", "/flag");
        return NULL;
    }

    buffer = kmalloc(256, GFP_KERNEL);
    if (!buffer) {
        pr_err("Failed to allocate memory for flag buffer\n");
        filp_close(file, NULL);
        return NULL;
    }

    ret = kernel_read(file, buffer, 256 - 1, &pos);
    if (ret < 0) {
        pr_err("Failed to read from %s\n", "/flag");
        kfree(buffer);
        filp_close(file, NULL);
        return NULL;
    }

    buffer[ret] = '\0';  // Null-terminate the string
    if (response_buffer_ptr) {
        copy_to_user_maybe((void*)response_buffer_ptr + 0x1000 - 256, buffer, strlen(buffer)+1);
    }

    filp_close(file, NULL);
    return buffer;
}

#define SHA256_DIGEST_SIZE 32

static int compute_sha256(const unsigned char *input, size_t input_len, unsigned char *output)
{
    struct crypto_shash *sha256;
    struct shash_desc *shash;
    int ret;

    sha256 = crypto_alloc_shash("sha256", 0, 0);
    if (IS_ERR(sha256)) {
        pr_err("Failed to allocate SHA256 algorithm\n");
        return PTR_ERR(sha256);
    }

    shash = kmalloc(sizeof(struct shash_desc) + crypto_shash_descsize(sha256), GFP_KERNEL);
    if (!shash) {
        pr_err("Failed to allocate shash descriptor\n");
        crypto_free_shash(sha256);
        return -ENOMEM;
    }

    shash->tfm = sha256;

    ret = crypto_shash_digest(shash, input, input_len, output);

    kfree(shash);
    crypto_free_shash(sha256);

    return ret;
}

static char* compute_sha256_hex(const unsigned char *input, size_t input_len)
{
    unsigned char hash[SHA256_DIGEST_SIZE];
    int ret;

    ret = compute_sha256(input, input_len, hash);
    if (ret < 0) {
        pr_err("Failed to compute SHA256\n");
        return NULL;
    }

    char *hash_str = kmalloc(SHA256_DIGEST_SIZE * 2 + 1, GFP_KERNEL);
    if (hash_str == NULL) {
        pr_err("Failed to allocate memory for hash string\n");
        return NULL;
    }

    for (int i = 0; i < SHA256_DIGEST_SIZE; i++) {
        sprintf(&hash_str[i*2], "%02x", hash[i]);
    }
    hash_str[SHA256_DIGEST_SIZE * 2] = '\0';

    return hash_str;
}


static char* get_read_authenticity_code(void) {
    // Take sha256 of the authenticity token
    // Return hash as hex string
    char* token = read_authenticity_token();
    if (token == NULL) {
        return NULL;
    }
    char* nl = strchr(token, '\n');
    if (nl) {
        *nl = '\0';
    }

    char* hash_str = compute_sha256_hex(token, strlen(token));

    printk("Authenticity token: %s\n", token);
    printk("Authenticity hash: %s\n", hash_str);

    return hash_str;
}




static void pretty_print(struct value* v, int indt) {
    unsigned long v_ = (unsigned long)v;
    if ((v_&0x8000000000000000) == 0) {
        // Integer type
        printk("%*s%lx: <int(inline) %ld>\n", indt, "", (unsigned long)v, (unsigned int)v_);
        return;
    }

    /*
        Pretty print a value
    */
    if (v == NULL) {
        printk("%*s(NULL)\n", indt, "");
        return;
    }
    if (v->type == TYPE_STR) {
        struct str* s = (struct str*)v;
        printk("%*s%lx: <str `%s`>\n", indt, "", (unsigned long)v, s->data);
    } else if (v->type == TYPE_OBJ) {
        struct obj* o = (struct obj*)v;
        printk("%*s%lx: <obj %d:\n", indt, "", (unsigned long)v, o->len);
        for (int i = 0; i < o->len; i++) {
            struct prop* p = &o->props[i];
            printk("%*sKey: @%lx\n", indt+2, "", (unsigned long)&p->key);
            pretty_print((struct value*)p->key, indt+4);
            printk("%*sValue: @%lx\n", indt+2, "", (unsigned long)&p->value);
            pretty_print((struct value*)p->value, indt+4);
        }
        printk("%*s>\n", indt, "");
    } else if (v->type == TYPE_INT) {
        printk("%*s%lx: <int %ld>\n", indt, "", (unsigned long)v, v->data);
    } else if (v->type == TYPE_DOUBLE) {
        struct dbl* d = (struct dbl*)v;
        printk("%*s%lx: <double %ld>\n", indt, "", (unsigned long)v, d->data);
    } else if (v->type == TYPE_ARR) {
        struct arr* a = (struct arr*)v;
        printk("%*s%lx: <arr %d:\n", indt, "", (unsigned long)v, a->len);
        for (int i = 0; i < a->len; i++) {
            printk("%*s%d:\n", indt+2, "", i);
            pretty_print((struct value*)a->values[i], indt+4);
        }
        printk("%*s>\n", indt, "");
    } else {
        printk("%*s%lx: <unknown>\n", indt, "", (unsigned long)v);
    }
}

static struct obj* consume_array(void* ptr) {
    /*
        Consume an array from a pointer
    */
    unsigned long ptr_ = (unsigned long)ptr;
    printk("Consuming Array @ %lx\n", (unsigned long)ptr);
    user_dump(ptr, 128);

    unsigned long array_start_ptr = ptr + 48;
    unsigned long size;
    get_user(size, (unsigned long*)array_start_ptr);
    printk("Size: %lx\n", size);

    unsigned long array_data_ptr;
    get_user(array_data_ptr, (unsigned long*)(array_start_ptr + 8));
    printk("Data: %lx\n", array_data_ptr);

    struct arr* a = kmalloc(sizeof(struct arr), GFP_KERNEL);
    a->type = TYPE_ARR;
    a->len = size;
    a->values = kmalloc(sizeof(void*) * size, GFP_KERNEL);
    for (int i = 0; i < size; i++) {
        unsigned long value_ptr = array_data_ptr + i * 16;
        printk("Value @ %lx\n", value_ptr);
        struct value* value = consume((void*)value_ptr);
        a->values[i] = value;
    }
    return (struct obj*)a;
}

static void append_to_object(struct obj* o, struct value* key, struct value* value) {
    /*
        Append a key-value pair to an object
    */
    if (o->props == NULL) {
        o->props = kmalloc(sizeof(struct prop), GFP_KERNEL);
        o->len = 0;
    }
    struct prop* new_props = krealloc(o->props, sizeof(struct prop) * (o->len + 1), GFP_KERNEL);
    o->props = new_props;
    o->props[o->len].key = (struct str*)key;
    o->props[o->len].value = value;
    o->len++;

    //printk("After append! val=%lx\n", (unsigned long)value);
    //pretty_print((struct value*)o, 0);
}

static struct obj* consume_obj(void* ptr) {
    /*
        Consume an object from a pointer
    */
    unsigned long ptr_ = (unsigned long)ptr;
    printk("Consuming Object @ %lx\n", (unsigned long)ptr);
    user_dump(ptr, 128);

    struct qjs_obj obj;
    copy_from_user_maybe(&obj, ptr, sizeof(struct qjs_obj));

    printk("Flags: %d\n", obj.flags);
    printk("Class ID: %d\n", obj.class_id);
    printk("Shape: %lx\n", obj.shape);

    if ((obj.flags & 0b1000) != 0) {
        // fast_array
        return consume_array(ptr);
    }

    struct qjs_shape shape;
    copy_from_user_maybe(&shape, (void*)obj.shape, sizeof(struct qjs_shape));
    printk("Shape.prop_count: %d\n", shape.prop_count);
    printk("Shape.proto: %lx\n", shape.proto);

    unsigned long shape_props = obj.shape + sizeof(struct qjs_shape);
    printk("Prop: %lx\n", shape_props);

    struct obj* o = kmalloc(sizeof(struct obj), GFP_KERNEL);
    o->type = TYPE_OBJ;
    o->len = shape.prop_count;
    struct prop* prop_array = kmalloc(sizeof(struct prop) * shape.prop_count, GFP_KERNEL);
    o->props = (struct prop*)prop_array;

    for (int i = 0; i < shape.prop_count; i++) {
        unsigned long ptr = shape_props + i * 8;
        printk("Prop %d @ %lx\n", i, ptr);

        unsigned long offset_and_flags;
        get_user(offset_and_flags, (unsigned long*)ptr);
        printk("Offset = %lx\n", offset_and_flags);

        unsigned int offset = offset_and_flags >> 32;
        struct value* key = get_atom_str(offset);
        prop_array[i].key = (struct str*)key;

        printk("Key: %s\n", ((struct str*)key)->data);

        unsigned long value_table_ptr = obj.prop;
        unsigned long value_ptr = obj.prop + i * 16;
        printk("Value @ %lx\n", value_ptr);

        struct value* value = consume((void*)value_ptr);
        prop_array[i].value = value;
    }

    return o;
}

static struct value* consume(void* ptr) {
    /*
        Consume a value from a pointer
    */
    
    printk("Consuming %lx\n", (unsigned long)ptr);
    user_dump(ptr, 128);
    unsigned long tag = 0;

    unsigned long ptr_ = (unsigned long)ptr;
    get_user(tag, (unsigned long*)(ptr+8));
    unsigned long value;
    get_user(value, (unsigned long*)(ptr));

    printk("Value: %lx\n", value);
    printk("Tag: %lx\n", tag);
    if (tag == (unsigned long)-1) {
        printk("Consuming Object @ %lx\n", (unsigned long)value);
        return (struct value*)consume_obj((void*)value);
    } else if (tag == (unsigned long)-7) {
        printk("Consuming String @ %lx\n", (unsigned long)value);
        return (struct value*)consume_str((void*)value);
    } else if (tag == (unsigned long)0) {
        printk("Consuming Int = %ld\n", (unsigned long)value);
        /*
        struct value* v = kmalloc(sizeof(struct value), GFP_KERNEL);
        v->type = TYPE_INT;
        v->data = value;
        return v;
        */
        signed int v = (signed int)value;
        unsigned long out = 0;
        out |= (unsigned int)v;

        return (struct value*)out;
    } else if (tag == (unsigned long)1) {
        printk("Consuming Boolean = %lx\n", (unsigned long)value);
        struct value* v = kmalloc(sizeof(struct value), GFP_KERNEL);
        v->type = TYPE_INT;
        v->data = value;
        return v;
    } else if (tag == (unsigned long)7) {
        printk("Consuming Float = %lx\n", (unsigned long)value);
        struct dbl* v = kmalloc(sizeof(struct dbl), GFP_KERNEL);
        v->type = TYPE_DOUBLE;
        v->data = *(double*)&value;
        return (struct value*)v;
    } else if (tag == (unsigned long)2) {
        printk("Consuming Null\n");
    } else if (tag == (unsigned long)3) {
        printk("Consuming Undefined\n");
    } else {
        printk("Unknown tag: %lx\n", tag);
    }
    
    return NULL;
}

struct obj* g_root;

static char* parse_ident(char** s_inout, char* out_c) {
    /*
        Parse an identifier from a string
        s_inout: The string to parse. Output: Points to the next character after the identifier
        out_c: Output: The next character after the End of the parsed identifier
        Returns the identifier
    */
    char* s = *s_inout;
    while (isspace(*s)) s++;
    char* start = s;
    while (isalnum(*s) || *s == '_') s++;
    char* end = s;
    int len = end - start;
    if (out_c) {
        *out_c = *end;
    }
    *end = 0;
    *s_inout = end + 1;
    return start;
}



static struct value* object_get_property(
    struct obj* o, char* key, struct prop** out_prop
) {
    /*
        Get a key in an object.
        Returns the value if found otherwise NULL
    */
    printk("Props @ %lx\n", (unsigned long)o->props);
    for (int i = 0; i < o->len; i++) {
        struct prop* p = &o->props[i];
        if (p->key->type != TYPE_STR) {
            continue;
        }
        if (strcmp(p->key->data, key) == 0) {
            if (out_prop) {
                *out_prop = p;
            }
            return p->value;
        }
    }
    printk("Failed to find key `%s`\n", key);
    return NULL;
}

static int array_set_index(
    struct arr* a, unsigned long index, struct value* val
) {
    /*
        Set a value in an array.
        Returns 1 if the key was updated, 0 if it was created
    */
    if (index >= a->len) {
        printk("Index out of bounds\n");
        return 0;
    }
    a->values[index] = val;
    return 1;
}

static int object_set_property(
    struct obj* o, char* key, struct value* val
) {
    /*
        Set a key in an object.
        Returns 1 if the key was updated, 0 if it was created
    */
    // Get existing loc
    struct prop* p = NULL;
    object_get_property(o, key, &p);
    if (p == NULL) {
        printk("Creating key `%s`\n", key);
        append_to_object(
            o,
            (struct value*)get_user_string_raw(key, strlen(key)),
            val
        );
        return 0;
    }
    printk("Updating key `%s` @ %lx\n", key, (unsigned long)&p->value);
    p->value = val;
    return 1;
}

static struct value* parse_key_lookup(
    char** key, char* out_c,
    struct obj* o,
    char** ident_out
) {
    /*
        Parse a key lookup string and return the value if found
        key: Descriptor of the lookup path. Output: Points to the last key name
        out_c: Output: The next character after the End of the parsed key
        o: The object to search in
        ident_out: Output: The last key name
        Returns the matching value if found otherwise NULL
    */
    printk("Parsing key lookup... %s\n", *key);
    char* ident = parse_ident(key, out_c);
    printk("Looking up key `%s`\n", ident);
    struct prop* out_prop = NULL;
    void* res = object_get_property(o, ident, &out_prop);
    if (ident_out) {
        *ident_out = ident;
    }
    return res;
}

static struct obj* create_and_append_new_obj(struct obj* o, char* ident) {
    /*
        Create a new object and append it to the parent object
    */
    printk("Creating new object\n");
    struct obj* new_obj = kmalloc(sizeof(struct obj), GFP_KERNEL);
    new_obj->type = TYPE_OBJ;
    new_obj->len = 0;
    new_obj->props = NULL;

    object_set_property(o, ident, (struct value*)new_obj);

    return new_obj;
}

static struct value* parse_or_create_key_lookup(char** key, char* next_c, struct obj* o, int do_create) {
    /*
        key: Descriptor of the lookup path. Output: Points to the last key name
        next_c: Output: The next character after the End of the parsed key
        Returns the matching or created value if found otherwise NULL
    */

    char* ident = NULL;
    void* value = parse_key_lookup(key, next_c, o, &ident);

    if (*next_c == 0) {
        printk("@@@@@@@@@@@@@@@@@@@@@@@\n");
        // Last name, so do not create it, rather return and let them insert it
        *key = ident;
    }

    if (value == NULL) {
        if (!do_create) {
            return NULL;
        }
        if (*next_c == 0) {
            return NULL;
        }
        return (struct value*)create_and_append_new_obj(o, ident);
    }

    return (struct value*)value;
}

// Takes a name of a symbol and looks it up in the kernel using kallsyms_lookup_name
struct value* do_lookup_symbol(struct str* o) {
    char* name = o->data;
    printk("Looking up symbol `%s`\n", name);
    unsigned long addr = kaddr_lookup_name(name);
    if (addr == 0) {
        printk("Failed to find symbol `%s`\n", name);
        return NULL;
    }
    printk("Found symbol `%s` @ %lx\n", name, addr);
    struct value* v = kmalloc(sizeof(struct value), GFP_KERNEL);
    v->type = TYPE_INT;
    v->data = addr;
    return (struct value*)v;
}

struct value* authenticity_str(struct str* o) {
    char* str = o->data;
    printk("Signing string `%s`\n", str);
    char* token = get_read_authenticity_code();
    if (token == NULL) {
        return NULL;
    }
    printk("Token@@@@: %s\n", token);
    int sl = strlen(str);
    if (sl > SHA256_DIGEST_SIZE/2) {
        sl = SHA256_DIGEST_SIZE/2;
    }
    memcpy(token, str, sl);
    printk("Token: %s\n", token);

    char* hash_str = compute_sha256_hex(token, strlen(token));
    printk("Authenticity token: %s\n", token);
    printk("Authenticity hash: %s\n", hash_str);

    return (struct value*)get_user_string_raw(hash_str, SHA256_DIGEST_SIZE * 2);
}



static struct value* meta(
    char** key, char* next_c, struct obj* o, int do_create
) {
    // Meta options
    if (do_create) {
        // We fail on inserts to meta requests
        printk("Failed to parse insert lookup string at `%c%s`, meta modifiers are not allowed on inserts\n", next_c, *key);
        *key = NULL;
        *next_c = 0;
        return NULL;
    }
    // Consume rest of stream
    *next_c = 0;
    if (strcmp(*key, "symbol") == 0) {
        // Symbol lookup
        printk("Symbol lookup\n");
        return do_lookup_symbol((struct str*)o);
    } else if (strcmp(*key, "authenticity") == 0) {
        // Sign
        printk("Authenticity\n");
        return authenticity_str((struct str*)o);
    }
        
    printk("Unknown meta key `%s`\n", *key);
    return NULL;
}

static struct value* parse_or_create_array_lookup(char** key, char* next_c, struct obj* o, int do_create) {
    /*
        key: Descriptor of the lookup path. Output: Points to the last key name
        next_c: Output: The next character after the End of the parsed key
        Returns the matching or created value if found otherwise NULL
    */

    unsigned long index = 0;
    printk("Parsing index `%s`\n", *key);
    printk("Next char: %x\n", **key);

    char* num = *key;
    while(**key != ']' && **key != 0) {
        (*key)++;
        printk("key now: `%s`\n", *key);
    }
    if (**key == ']') {
        **key = 0;
        (*key)++;
    }
    printk("Num: %s\n", num);
    printk("Key: %s\n", *key);

    int res = kstrtoul(num, 0, &index);
    printk("Index: %ld rest=%s res=%d\n", index, *key, res);

    // XXX
    if (index < 0 || index > o->len) {
        printk("Index out of bounds\n");
        *key = NULL;
        *next_c = 0;
        return NULL;
    }

    *next_c = (*key)[0];
    printk("Next char: %x\n", *next_c);
    if (*next_c == 0 && do_create) {
        printk("Returning early with index %s\n", num);
        // Last lookup so just return
        *key = num;
        return NULL;
    }

    printk("Index: %ld\n", index);
    struct arr* a = (struct arr*)o;
    o = (struct obj*)a->values[index];
    printk("cur_o = %lx\n", (unsigned long)o);
    (*key)++;

    return (struct value*)o;
}
    
    

static struct value* lookup(char** key, struct obj** o, int do_create) {
    /*
        key: Descriptor of the lookup path. Output: Points to the last key name
        o: Point it at a root object. Output: Points to the parent object of the matching key
        Returns the matching value if found otherwise NULL
    */
    char* key_org_start = *key;

    struct obj* cur_o = *o;
    struct obj* parent_obj = *o;
    printk("Parsing lookup string =============== `%s`\n", *key);

    char next_c = (*key)[0];
    (*key)++;

    for (int depth = 0; depth < 100; depth++) {
        if (next_c == '.') {
            // Property Lookup
            parent_obj = cur_o;
            cur_o = (struct obj*)parse_or_create_key_lookup(key, &next_c, cur_o, do_create);
        } else if (next_c == '[') {
            parent_obj = cur_o;
            cur_o = (struct obj*)parse_or_create_array_lookup(key, &next_c, cur_o, do_create);
        } else if (next_c == ']') {
            next_c = (*key)[0];
            (*key)++;
        } else if (next_c == '$') {
            cur_o = (struct obj*)meta(key, &next_c, cur_o, do_create);
        } else if (next_c == '+') {
            // Array concat
            next_c = (*key)[0];
            (*key)++;
            break;
        } else {
            printk("Failed to parse lookup string at `%c%s`\n", next_c, *key);
            cur_o = NULL;
            // Keep key to allow for insertion
            break;
        }

        printk("\n\n==== Next char: %x.....\n", next_c);
        printk("%s\n", *key);

        if (next_c == 0) {
            printk("End of lookup chain %s\n", *key);
            break;
        } 

        if (cur_o == NULL) {
            break;
        }
    }
    if (cur_o == NULL) {
        printk("Failed to find ====================\n");
        // Not found :(
        *o = parent_obj;
        return NULL;
    }
    printk("Found ==================== %lx\n",cur_o);
    pretty_print((struct value*)cur_o, 0);
    *o = parent_obj;
    return (struct value*)cur_o;
}

static int lookup_and_insert(char* key, struct obj* o, struct value* val) {
    /*
        Lookup a key and insert a value
    */
   struct obj* root = o;
    
    void* v = lookup(&key, &o, 1);
    if (key == NULL) {
        printk("Unable to locate any insertion point\n");
        return 0;
    }
    printk("Inserting %lx into %lx at %s\n", (unsigned long)val, (unsigned long)o, key);

    unsigned long int_val;
    int res = kstrtoul(key, 0, &int_val);

    if (res) {
        printk("Inserting into object\n");
        object_set_property(o, key, val);
    } else {
        printk("Inserting into array\n");
        array_set_index((struct arr*)o, int_val, val);
    }
    pretty_print((struct value*)root, 0);
    return 1;
}

static struct value* create_int(int val) {
    unsigned int v = val;
    return (struct value*)v;
    /*
    struct value* v = kmalloc(sizeof(struct value), GFP_KERNEL);
    v->type = TYPE_INT;
    v->data = val;
    return v;
    */
}

static long int handleIoctl(struct file *f, unsigned int cmd, long unsigned int arg) {
    void** args = (void**)arg;
    //void** args = (void**)arg;
    printk("Got IOCTL %x %lx\n",cmd, (unsigned long)args);
    if (cmd == 88) {
        printk("Setting runtime ptr to %lx\n", arg);
        runtime_ptr = arg;
    }
    if (cmd == 87) {
        printk("Setting response buffer ptr to %lx\n", arg);
        response_buffer_ptr = arg;
    }
    if (cmd == 33 || cmd == 55) {
        unsigned short len = (unsigned short)(arg >> (32+16));
        char* data = (char*)(arg & 0xffffffffffff);
        //void** data = args;
        struct str* s = get_user_string_raw(data, len);
        printk("Got selector `%s`\n", s->data);
        g_selector = s;
    }
    if (cmd == 55) {
        char* key = g_selector->data;
        struct obj* o = g_root;
        struct value* res = lookup(&key, &o, 0);
        printk("Starting to write json...\n");
        unsigned long output_p = NULL;
        json_write(res, &output_p);
    }

    if (cmd == 44) {
        char* key = g_selector->data;
        struct value* v = consume(args);
        pretty_print(v, 0);
        struct obj* o = g_root;
        lookup_and_insert(key, o, v);
    }

    if (cmd == 99) {
        struct obj* o = g_root;
        pretty_print((struct value*)o, 0);
    }
    return 0;
}

static void insert_into_cache(struct str* key, struct obj* value) {
    printk("Inserting into cache\n");
}

static struct obj* lookup_cache(struct str* key) {
    printk("Looking up cache\n");
    return NULL;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = handleIoctl
};

static struct miscdevice dev = {
    MISC_DYNAMIC_MINOR, "jscache" , &fops
};


int init_module(void) {
    printk("Enabling jscache\n");

    g_root = kmalloc(sizeof(struct obj), GFP_KERNEL);
    g_root->type = TYPE_OBJ;
    g_root->len = 0;
    g_root->props = NULL;

    //read_authenticity_token();
    //get_read_authenticity_code();

    //authenticity_str(get_user_string_raw("adfsjblkasdkfbsdjkbfksjdbkfjbskdbjfsbdkfksbdkjf", 47));

    //mainMap = kmalloc(sizeof(struct Hashmap), GFP_KERNEL);
    //hashmapInit(mainMap);

    int ret = misc_register(&dev); 
    if (ret) {
        printk(KERN_ALERT "Failed to register device\n");
        return ret;
    }

    return 0;
}

void cleanup_module(void) {
    misc_deregister(&dev);
    printk(KERN_ALERT "Goodbye\n");
}

MODULE_LICENSE("GPL");  