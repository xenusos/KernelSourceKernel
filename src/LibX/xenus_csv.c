/*
    Purpose:
    Author: Reece W.
    License: All Rights Reserved J. Reece Wilson
*/
#include <xenus.h>
#include <kernel/libx/xenus_csv.h>

#define CSV_EOF_OFFSET 0xFFFFFFFF

static bool csv_is_splitter(char c)
{
    return c == ',' || c == '\r' || c == '\n' || c == (char) 0x00;
}

static void csv_next_splitter(const char * buffer, uint_t length, uint_t * next_offset, uint_t * read_length)
{
    bool previous_splitter = false;
    bool has_read = false;

    *next_offset = 0;
    *read_length = 0;

    for (uint_t i = 0; i < length; i++)
    {
        char cur = buffer[i];
        bool splitter = csv_is_splitter(cur);
        
        if (previous_splitter)
        {
            *next_offset = i;

            if (cur == ' ')
                *next_offset = i + 1;

            if (splitter)
                continue;
            
            return;
        }

        if (splitter)
            *read_length = i;

        previous_splitter = splitter;
    }
}

XENUS_EXPORT error_t csv_process_buffer(const char * buffer, uint_t buf_length, uint_t * offset, char * string_out, uint_t str_buf_length, uint_t * str_length)
{
    uint_t relative_offset;
    uint_t start;
    uint_t length;
    const char * str;

    start = *offset;
    
    if (start == CSV_EOF_OFFSET)
        return XENUS_ERROR_CSV_EOS;

    relative_offset = 0;
    str = &buffer[start];

    csv_next_splitter(str, buf_length - start, &relative_offset, &length);

    if (str_length)
        *str_length = length;

    if (string_out)
    {
        length = MIN(length, str_buf_length - 1);
        if (length)
        {
            memcpy(string_out, str, length);
            string_out[length] = (char)0x00;
        }
    }

    if (relative_offset == 0)
    {
        *offset = CSV_EOF_OFFSET;
        return XENUS_STATUS_CSV_READ_ALL;
    }

    *offset += relative_offset;
    return XENUS_OKAY;
}
