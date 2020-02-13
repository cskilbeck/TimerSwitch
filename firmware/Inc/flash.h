#pragma once

namespace flash {

//////////////////////////////////////////////////////////////////////
// flash functions return one of these, only ok (0) means success

enum {
    ok = 0,             // flash function succeeded
    bad_id = 1,         // a bad id was passed in (can't be 0xff)
    not_found = 2,      // couldn't find a variable to load
    bad_len = 3,        // length mismatch between load request and stored var
    write_error = 4,    // error writing to flash
    cant_unlock = 5,    // error unlocking flash
    locked = 6,         // didn't call unlock() before a write operation
    not_blank = 7,      // some area not blank when it should be
    no_room = 8,        // not enough room in the flash
    cant_lock = 9       // error locking the flash
};

//////////////////////////////////////////////////////////////////////
// unlock - if you don't call this before save, it will fail with `locked`

int unlock();

//////////////////////////////////////////////////////////////////////
// call this when you've finished saving things, regardless of how it went

void lock();

//////////////////////////////////////////////////////////////////////
// format - reset the flash contents to empty
// this will be called internally if corrupt flash is detected
// can also be called in response to user request

int format();

//////////////////////////////////////////////////////////////////////
// verify - checks for corruption and calls format() if necessary

int verify();

//////////////////////////////////////////////////////////////////////
// load - load variable with a unique `id` of `len` bytes

int load(byte id, byte len, byte *data);

//////////////////////////////////////////////////////////////////////
// save - write a variable with a unique `id` of `len` bytes to the flash

int save(byte id, byte len, byte *data);

//////////////////////////////////////////////////////////////////////

#ifdef __cplusplus

template<typename T> int load(int id, T const &data)
{
    return flash::load(id, sizeof(T), (byte *)&data);
}

//////////////////////////////////////////////////////////////////////

template<typename T> int save(int id, T const &data)
{
    return flash::save(id, sizeof(T), (byte *)&data);
}

#endif

//////////////////////////////////////////////////////////////////////
// error_message - get a string describing an error code

char const *error_message(int code);

int dump(int page);

}    // namespace flash

