//////////////////////////////////////////////////////////////////////
// eeprom emulation flash functions
// see flash.h for interface

#include "main.h"
#include "flash.h"

//#define TEST_FLASH    // to test it on a ram buffer

#if !defined(TEST_FLASH)

#define FLASH_OFFSET (16384 - 2048)

#else

#undef FLASH_BASE
#undef FLASH_PAGE_SIZE

#define FLASH_PAGE_SIZE 1024

static byte test_area[FLASH_PAGE_SIZE * 2];

#define FLASH_BASE test_area
#define FLASH_OFFSET 0

#endif

//////////////////////////////////////////////////////////////////////
// notes
// should work on most 32 bit STM32 medium density devices, only tested on STM32F103C8T6 & STM32F030F4P6
// it's all synchronous
// error checking is very basic, it just aborts the operation if it sees an error
//
// #define LOG_FLASH    // to see lots of printf about how it's going
//
// define these 2 variables to set the area used for storage
static int const page_size = FLASH_PAGE_SIZE;                            // usually 1K or 2K
static uint16_t *page_base = (uint16_t *)(FLASH_BASE + FLASH_OFFSET);    // this puts your 2 pages somewhere in the flash area
//
// define these functions to interface with the hardware (see versions for STM32F1xB medium density at the end of this file)
//
// write a 16 bit word to a flash address
int flash_write_16(uint16_t *dest, uint16_t src);
//
// erase page 0 or page 1 of the flash
int flash_erase_page(int which_page);
//
// wait for the most recent flash operation to complete
void flash_wait();
//
// check whether the most recent flash operation succeded
int flash_result();
//
// lock the flash against writes (it's locked by default after reset)
int flash_lock();
//
// unlock the flash for writing (not necessary for reading)
int flash_unlock();
//
// all functions must return one of the status codes defined in flash.h
//
//////////////////////////////////////////////////////////////////////
// method:
//
// 2 pages of flash are used, one is 'active' at a time (except for a brief moment during migration)
//
// the first uint16_t in a flash page contains `page_state`
// valid `page_state` can be
//      "empty"  = 0xFFFF (because erasing the flash sets it to that value
//      "active" = 0x5555 (arbitrarily chosen)
//
// the first uint16_t in a page should be one of these values
// if it's not, it will erase that page to empty
//
// a 2 byte header precedes each stored variable in the flash
//      byte 0 is the id, which can be any value except 0xFF
//      byte 1 is the length, in bytes, of the variable (excluding the var header)
//
// to store a variable, just stash it at the first available free slot
//
// to look up a variable with id X, find the active page, scan for the most recent version of
// the variable with id X (which will be the last one found)
//
// when a page doesn't have room to store the new version of a variable, the page
// is migrated (garbage collected) into the other page, discarding old versions of
// variables and the old page is erased

//////////////////////////////////////////////////////////////////////

#if defined(LOG_FLASH)
#include <stdio.h>
#define log printf
#else
void        dummy(...)
{
}
#define log   \
    if(false) \
    {         \
    }         \
    else      \
        dummy
#endif

//////////////////////////////////////////////////////////////////////
// call a flash function and bomb if it doesn't go well

#define check(x)                                                \
    {                                                           \
        int r = (x);                                            \
        if(r != flash::ok)                                      \
        {                                                       \
            log("Error %d (%s)\n", r, flash::error_message(r)); \
            return r;                                           \
        }                                                       \
    }

//////////////////////////////////////////////////////////////////////

namespace
{

//////////////////////////////////////////////////////////////////////

char const *flash_error_strings[] = { "ok", "bad_id", "not_found", "bad_len", "write_error", "cant_unlock", "locked", "not_blank", "no_room", "cant_lock" };

//////////////////////////////////////////////////////////////////////

enum page_state
{
    active = 0x5555,
    empty = 0xffff
};

int const page_size_in_uint16_ts = page_size / sizeof(uint16_t);

//////////////////////////////////////////////////////////////////////
// round a length up to a multiple of 2

int roundup(int len)
{
    return (len + 1) & -2;
}

//////////////////////////////////////////////////////////////////////
// get the id byte of a variable

int var_id(uint16_t *p)
{
    return reinterpret_cast<byte *>(p)[0];
}

//////////////////////////////////////////////////////////////////////
// get the length byte of a variable

int var_len(uint16_t *p)
{
    return reinterpret_cast<byte *>(p)[1];
}

//////////////////////////////////////////////////////////////////////
// get the total # of bytes of a variable including 2 byte header

int total_length(uint16_t *var)
{
    return 2 + roundup(var_len(var));
}

//////////////////////////////////////////////////////////////////////
// get a pointer to page 0 or 1

uint16_t *page_addr(int n)
{
    return page_base + n * page_size_in_uint16_ts;
}

//////////////////////////////////////////////////////////////////////
// get pointer to the data in the page (skip the uint16_t page header)

uint16_t *page_data(int n)
{
    return page_addr(n) + 1;
}

//////////////////////////////////////////////////////////////////////
// get a pointer to just past the end of page 0 or 1

uint16_t *page_end_addr(int n)
{
    return page_addr(n) + page_size_in_uint16_ts;
}

//////////////////////////////////////////////////////////////////////
// get the state of a page, should be `active` or `empty`
// if it's neither of those, the page should be erased

uint16_t get_page_state(int n)
{
    return page_addr(n)[0];
}

//////////////////////////////////////////////////////////////////////
// is a page the active page? page_state::active (0x5555) marks the active page

bool is_active(int which_page)
{
    return get_page_state(which_page) == page_state::active;
}

//////////////////////////////////////////////////////////////////////
// is a page empty? page_state::empty (0xffff) means it's empty (or should be)

bool is_empty(int which_page)
{
    return get_page_state(which_page) == page_state::empty;
}

//////////////////////////////////////////////////////////////////////
// return pointer to the next var

uint16_t *next(uint16_t *cur)
{
    return cur + total_length(cur) / sizeof(uint16_t);
}

//////////////////////////////////////////////////////////////////////
// check if an area is blank

int blank_check(uint16_t *p, uint16_t *e)
{
    while(p < e)
    {
        if(*p++ != 0xffff)
        {    // found something other than 0xffff, page is corrupt
            return flash::not_blank;
        }
    }
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// get pointer to 1st free slot in a page
// returns null if it's full
// it assumes the page is active

uint16_t *get_free_offset(int which_page)
{
    uint16_t *p = page_data(which_page);
    uint16_t *page_end = page_end_addr(which_page);

    // assert(*p == page_state::active);

    while(p < page_end)
    {
        if(var_id(p) == 0xff)
        {
            return p;
        }
        p = next(p);
    }
    return null;
}

//////////////////////////////////////////////////////////////////////
// find a variable in a page
// returns null if it's not found

uint16_t *find_var(int page, int id)
{
    uint16_t *p = page_data(page);
    uint16_t *page_end = page_end_addr(page);
    uint16_t *var = null;
    while(p < page_end)
    {
        int this_id = var_id(p);
        if(this_id == 0xff)
        {
            break;
        }
        if(this_id == id)
        {
            var = p;    // last one found is the most current one
        }
        p = next(p);
    }
    return var;
}

//////////////////////////////////////////////////////////////////////
// write a var to flash
// pointer is incremented to point at next available address (which may be beyond the page end)

int flash_write_var(uint16_t *&p, uint16_t id, uint16_t len, byte *data)
{
    // next available address
    uint16_t *next_space = p + (roundup(len) + 2) / sizeof(uint16_t);    // +2 for the header

    uint16_t header = id | (len << 8);
    check(flash_write_16(p++, header));    // write the header

    // write data as uint16_ts
    uint16_t *s = reinterpret_cast<uint16_t *>(data);
    while(len > 1)
    {
        check(flash_write_16(p++, *s++));
        len -= 2;
    }
    // and last byte for odd length ones
    if(len == 1)
    {
        check(flash_write_16(p, reinterpret_cast<byte *>(s)[0]));    // so we don't read off the end of the array which would probably be fine but you know how people can get
    }
    p = next_space;
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// mark a page as active
// assumes page is empty

int mark_page_active(int page)
{
    log("Marking page %d as active\n", page);
    check(flash_write_16(page_addr(page), page_state::active));
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// migrate all the vars (and a new var) from one page to the other
// assumes other_page is freshly erased
// if there isn't room for the new one, the old value is migrated

int migrate_page(uint16_t id, int old_page, uint16_t len, byte *data)
{
    int new_page = 1 - old_page;

    log("Flash migrating to page %d\n", new_page);

    check(flash_erase_page(new_page));    // erase the other page

    // scan the old vars
    uint16_t *scan_base = page_data(old_page);

    // for each var in the old page that isn't already in the new page, write latest version to new page
    // inefficient but simple

    uint16_t *dst_base = page_data(new_page);
    uint16_t *old_value = null;

    while(var_id(scan_base) != 0xff)
    {
        if(var_id(scan_base) == id)
        {                             // omit old copies of the new var for now
            old_value = scan_base;    // but remember it in case we need the old value (not enough room for the new one)
        }
        else
        {
            uint16_t *f = find_var(new_page, var_id(scan_base));    // already written to the new page?
            if(f == null)
            {
                uint16_t *n = scan_base;    // no, scan for latest version
                uint16_t *scan_var = scan_base;
                while(var_id(scan_var) != 0xff)
                {
                    if(var_id(scan_var) == var_id(scan_base))
                    {
                        n = scan_var;
                    }
                    scan_var = next(scan_var);
                }
                check(flash_write_var(dst_base, var_id(n), var_len(n), (byte *)n + 1));    // and copy it
                if(dst_base >= page_end_addr(new_page))
                {
                    return flash::no_room;    // Hmph, not enough room in the flash, this should never happen, we haven't added the new one yet!?
                }
            }
            scan_base = next(scan_base);
        }
    }

    // write the new value if there's room
    uint16 words_remaining = page_end_addr(new_page) - dst_base;
    int    total_words_required = roundup(len) / 2 + 1;
    if(words_remaining < total_words_required)
    {
        log("Not enough room for new value (id: %d, len: %d)\n", id, len);
        if(old_value != null)
        {
            id = var_id(old_value);    // no room, write the old value instead
            len = var_len(old_value);
            data = reinterpret_cast<byte *>(old_value) + 2;    // +2 skips the 2 byte header
        }
    }
    check(flash_write_var(dst_base, id, len, data));

    check(mark_page_active(new_page));    // finally mark it as active. Now both pages are marked as active (but should have effectively the same contents)
    check(flash_erase_page(old_page));    // erase old page, now just one active page and all is ok
    log("Flash migration complete\n");
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// write a variable to a page
// this might migrate the pages which can take a long time

int write_var(int page, uint16_t id, uint16_t len, byte *data)
{
    uint16_t  required = roundup(len) / sizeof(uint16_t);    // space needed in uint16s
    uint16_t *loc = get_free_offset(page);                   // find some free space at the end
    uint16 *  end = page_end_addr(page);
    if(loc == null || (end - loc) < required)
    {                                                // if page is full or not enough space left
        check(migrate_page(id, page, len, data));    // copy all old values and this new value to the other page
        return flash::ok;
    }
    log("Writing %02x (len %d) at offset %04x\n", id, len, (loc - page_addr(page)) * 2);
    check(flash_write_var(loc, id, len, data));    // there's enough space, just write the var
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// check integrity of a page, erase it if necessary
// this is not at all exhaustive!

int verify_page(int page)
{
    uint16_t *p = page_data(page);
    uint16_t *e = page_end_addr(page);
    if(is_active(page))
    {
        p = get_free_offset(page);
    }
    if(p == null || blank_check(p, e) != flash::ok)
    {
        log("Flash page %d corrupt, erasing it\n", page);
        check(flash_erase_page(page));
    }
    return flash::ok;
}

}    // namespace

//////////////////////////////////////////////////////////////////////

namespace flash
{

//////////////////////////////////////////////////////////////////////
// call this to check the flash is in a good state
// if it's bad in some way it will try to make it good
// by erasing any pages in a bad state

int verify()
{
    log("Checking flash storage\n");
    verify_page(0);
    verify_page(1);
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// reformat the whole thing back to a known good state, losing everything

int format()
{
    log("Formatting flash storage\n");

    check(flash_erase_page(0));
    check(flash_erase_page(1));
    check(mark_page_active(0));
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// load a variable from the flash
// if the stored length doesn't match, it doesn't load it

int load(byte id, byte len, byte *data)
{
    if(id == 0xff)
    {
        log("Can't use 0xff as a flash variable ID\n");
        return flash::bad_id;
    }
    uint16_t *v = null;
    for(int i = 0; i < 2; ++i)
    {
        if(is_active(i))
        {
            v = find_var(i, id);
            if(v != null)
            {
                if(var_len(v) != len)
                {
                    log("Wrong length, not loaded\n");
                    return flash::bad_len;
                }
                byte *src = reinterpret_cast<byte *>(v + 1);
                memcpy(data, src, len);    // flash is memory mapped
                return flash::ok;
            }
        }
    }
    log("Var id %d not found\n", id);
    return flash::not_found;
}

//////////////////////////////////////////////////////////////////////
// save a blob of data with 'id' to a location in flash

int save(byte id, byte len, byte *data)
{
    log("Saving var %02x (%d bytes) to flash\n", id, len);

    if(id == 0xff)
    {
        return flash::bad_id;
    }
    for(int p = 0; p < 2; ++p)
    {
        if(is_active(p))
        {
            // see if it's already the same value and if it is, don't bother
            uint16_t *existing = find_var(p, id);
            if(existing == null)
            {
                log("var not found, saving...\n");
            }
            else if(memcmp(existing + 1, data, len) == 0)
            {
                log("Value unchanged, not saving...\n");
                return flash::ok;
            }
            check(write_var(p, id, len, data));
            log("Saved var %02x (%d bytes) to page %d\n", id, len, p);
            return flash::ok;
        }
    }
    for(int p = 0; p < 2; ++p)
    {
        if(is_empty(p))
        {
            check(write_var(p, id, len, data));
            check(mark_page_active(p));
            return flash::ok;
        }
    }
    check(format());
    check(write_var(0, id, len, data));
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// lock the flash after writing

void lock()
{
    flash_lock();
}

//////////////////////////////////////////////////////////////////////
// unlock the flash for writing (can read at any time)

int unlock()
{
    return flash_unlock();
}

//////////////////////////////////////////////////////////////////////
// get text for an error code

char const *error_message(int code)
{
    if(code < 0 || code >= countof(flash_error_strings))
    {
        return "not a flash error";
    }
    return flash_error_strings[code];
}

//////////////////////////////////////////////////////////////////////
// hex dump a page of flash

int dump(int page)
{
    uint16_t *p = page_addr(page);
    uint16_t *e = page_end_addr(page);
    uint16_t *base = p;
    int const row = 32;
    while(p < e)
    {
        log("%08x ", p);
        byte *g = (byte *)p;
        for(int i = 0; i < row; ++i)
        {
            byte c = *g++;
            if(c < ' ' || c > 126)
            {
                c = '.';
            }
            log("%c", c);
        }
        g = (byte *)p;
        for(int i = 0; i < row; ++i)
        {
            log(" %02x", *g++);
        }
        log("\n");
        p += row / 2;
    }
    return flash::ok;
}

}    // namespace flash

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
// USER FUNCTIONS
// no HAL or hardware access outside these functions allowed
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_FLASH)

//////////////////////////////////////////////////////////////////////
// wait for a flash operation to complete

void flash_wait()
{
    while((FLASH->SR & FLASH_SR_BSY) != 0)
    {
    }
}

//////////////////////////////////////////////////////////////////////
// check whether a flash operation succeded

int flash_result()
{
    flash_wait();

    if((FLASH->SR & FLASH_SR_WRPRTERR) != 0)
    {
        return flash::locked;
    }
    if((FLASH->SR & FLASH_SR_PGERR) != 0)
    {
        return flash::write_error;
    }
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// write a 16 bit word to flash

int flash_write_16(uint16_t *dest, uint16_t src)
{
    flash_wait();
    SET_BIT(FLASH->CR, FLASH_CR_PG);
    *(__IO uint16_t *)dest = src;
    return flash_result();
}

//////////////////////////////////////////////////////////////////////
// erase page 0 or 1

int flash_erase_page(int which_page)
{
    // assert(which_page >= 0 && which_page <= 1);
    uint32_t base = (uint32_t)page_addr(which_page);
    log("Erasing flash page %d (%p)\n", which_page, base);
    flash_wait();
    SET_BIT(FLASH->CR, FLASH_CR_PER);
    WRITE_REG(FLASH->AR, base);
    SET_BIT(FLASH->CR, FLASH_CR_STRT);
    check(flash_result());
    check(blank_check((uint16_t *)base, page_end_addr(which_page)));
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// lock the flash against writes (it's locked by default after reset)

int flash_lock()
{
    log("Locking flash against writes\n");
    if(HAL_FLASH_Lock() == HAL_OK)
    {
        return flash::ok;
    }
    return flash::cant_lock;
}

//////////////////////////////////////////////////////////////////////
// unlock the flash for writing (not necessary for reading)

int flash_unlock()
{
    log("Unlocking flash for writes\n");
    if(HAL_FLASH_Unlock() != HAL_OK)
    {
        log("Error unlocking flash\n");
        return flash::cant_unlock;
    }
    return flash::ok;
}

#else

//////////////////////////////////////////////////////////////////////
// FLASH TEST MODE - just use some ram
//////////////////////////////////////////////////////////////////////

namespace
{
bool test_locked = true;

}    // namespace

//////////////////////////////////////////////////////////////////////
// wait for a flash operation to complete

void flash_wait()
{
}

//////////////////////////////////////////////////////////////////////
// check whether a flash operation succeded

int flash_result()
{
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// write a 16 bit word to flash

int flash_write_16(uint16_t *dest, uint16_t src)
{
    if(test_locked)
    {
        return flash::locked;
    }
    *dest = src;
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// erase page 0 or 1

int flash_erase_page(int which_page)
{
    memset(page_addr(which_page), 0xff, 1024);
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// lock the flash against writes (it's locked by default after reset)

int flash_lock()
{
    test_locked = true;
    return flash::ok;
}

//////////////////////////////////////////////////////////////////////
// unlock the flash for writing (not necessary for reading)

int flash_unlock()
{
    test_locked = false;
    return flash::ok;
}

#endif
