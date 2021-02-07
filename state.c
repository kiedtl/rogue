/*
    state.c - Portable Rogue Save State Code

    Copyright (C) 1999, 2000, 2005 Nicholas J. Kisseberth

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name(s) of the author(s) nor the names of other contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

/************************************************************************/
/* Save State Code                                                      */
/************************************************************************/

#define RSID_STATS        0xABCD0001
#define RSID_THING        0xABCD0002
#define RSID_THING_NULL   0xDEAD0002
#define RSID_OBJECT       0xABCD0003
#define RSID_MAGICITEMS   0xABCD0004
#define RSID_KNOWS        0xABCD0005
#define RSID_GUESSES      0xABCD0006
#define RSID_OBJECTLIST   0xABCD0007
#define RSID_BAGOBJECT    0xABCD0008
#define RSID_MONSTERLIST  0xABCD0009
#define RSID_MONSTERSTATS 0xABCD000A
#define RSID_MONSTERS     0xABCD000B
#define RSID_TRAP         0xABCD000C
#define RSID_WINDOW       0xABCD000D
#define RSID_DAEMONS      0xABCD000E
#define RSID_IWEAPS       0xABCD000F
#define RSID_IARMOR       0xABCD0010
#define RSID_SPELLS       0xABCD0011
#define RSID_ILIST        0xABCD0012
#define RSID_HLIST        0xABCD0013
#define RSID_DEATHTYPE    0xABCD0014
#define RSID_CTYPES       0XABCD0015
#define RSID_COORDLIST    0XABCD0016
#define RSID_ROOMS        0XABCD0017

#include <curses.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "rogue.h"

#define READSTAT ((format_error == 0) && (read_error == 0))
#define WRITESTAT (write_error == 0)

int read_error   = FALSE;
int write_error  = FALSE;
int format_error = FALSE;
int end_of_file  = FALSE;
int mybig_endian   = 0;
const char *fmterr = "";

void *
get_list_item(struct linked_list *l, int i)
{
    int count = 0;

    while(l != NULL)
    {   
        if (count == i)
            return(l->l_data);
                        
        l = l->l_next;
        
        count++;
    }
    
    return(NULL);
}

int
find_list_ptr(struct linked_list *l, void *ptr)
{
    int count = 0;

    while(l != NULL)
    {
        if (l->l_data == ptr)
            return(count);
            
        l = l->l_next;
        count++;
    }
    
    return(-1);
}

int
list_size(struct linked_list *l)
{
    int count = 0;
    
    while(l != NULL)
    {
        if (l->l_data == NULL)
            return(count);
            
        count++;
        
        l = l->l_next;
    }
    
    return(count);
}

int
rs_write(FILE *savef, void *ptr, int size)
{
    if (!write_error)
        encwrite(ptr,size,savef);

    if (0)
        write_error = TRUE;
        
    assert(write_error == 0);

    return(WRITESTAT);
}

int
rs_write_char(FILE *savef, char c)
{
    rs_write(savef, &c, 1);
    
    return(WRITESTAT);
}

int
rs_write_boolean(FILE *savef, bool c)
{
    unsigned char buf = (c == 0) ? 0 : 1;
    
    rs_write(savef, &buf, 1);

    return(WRITESTAT);
}

int
rs_write_booleans(FILE *savef, bool *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_boolean(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_shint(FILE *savef, unsigned char c)
{
    unsigned char buf = c;

    rs_write(savef, &buf, 1);

    return(WRITESTAT);
}

int
rs_write_short(FILE *savef, short c)
{
    unsigned char bytes[2];
    unsigned char *buf = (unsigned char *) &c;

    if (mybig_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }

    rs_write(savef, buf, 2);

    return(WRITESTAT);
}

int
rs_write_shorts(FILE *savef, short *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_short(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_ushort(FILE *savef, unsigned short c)
{
    unsigned char bytes[2];
    unsigned char *buf = (unsigned char *) &c;

    if (mybig_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }

    rs_write(savef, buf, 2);

    return(WRITESTAT);
}

int
rs_write_int(FILE *savef, int c)
{
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *) &c;

    if (mybig_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_write_ints(FILE *savef, int *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_int(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_uint(FILE *savef, unsigned int c)
{
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *) &c;

    if (mybig_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_write_long(FILE *savef, long c)
{
    int c2;
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *)&c;

    if (sizeof(long) == 8)
    {
        c2 = c;
        buf = (unsigned char *) &c2;
    }

    if (mybig_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_write_longs(FILE *savef, long *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_long(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_ulong(FILE *savef, unsigned long c)
{
    unsigned int c2;
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *)&c;

    if ( (sizeof(long) == 8) && (sizeof(int) == 4) )
    {
        c2 = c;
        buf = (unsigned char *) &c2;
    }

    if (mybig_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_write_ulongs(FILE *savef, unsigned long *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_ulong(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_string(FILE *savef, char *s)
{
    int len = 0;

    len = (s == NULL) ? 0 : strlen(s) + 1;

    rs_write_int(savef, len);
    rs_write(savef, s, len);
            
    return(WRITESTAT);
}

int
rs_write_string_index(FILE *savef, char *master[], int max, char *str)
{
    int i;

    for(i = 0; i < max; i++)
    {
        if (str == master[i])
        {
            rs_write_int(savef,i);
            return(WRITESTAT);
        }
    }

    rs_write_int(savef,-1);

    return(WRITESTAT);
}

int
rs_write_strings(FILE *savef, char *s[], int count)
{
    int len = 0;
    int n = 0;

    rs_write_int(savef,count);

    for(n = 0; n < count; n++)
    {
        len = (s[n] == NULL) ? 0L : strlen(s[n]) + 1;
        rs_write_int(savef, len);
        rs_write(savef, s[n], len);
    }
    
    return(WRITESTAT);
}

int
rs_read(int inf, void *ptr, int size)
{
    int actual;

    end_of_file = FALSE;

    if (!read_error && !format_error)
    {
        actual = encread(ptr, size, inf);

        if ((actual == 0) && (size != 0))
           end_of_file = TRUE;
    }
       
    if (read_error)
    {
        printf("read error has occurred. restore short-circuited.\n");
        abort();
    }

    if (format_error)
    {
        printf("format error: %s\r\n", fmterr);
        printf("game format invalid. restore short-circuited.\n");
        abort();
    }

    return(READSTAT);
}

int
rs_read_char(int inf, char *c)
{
    rs_read(inf, c, 1);
    
    return(READSTAT);
}

int
rs_read_boolean(int inf, bool *i)
{
    unsigned char buf;
    
    rs_read(inf, &buf, 1);
    
    *i = (bool) buf;
    
    return(READSTAT);
}

int
rs_read_booleans(int inf, bool *i, int count)
{
    int n = 0, value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
        {
            printf("Invalid booleans block. %d != requested %d\n",value,count); 
            format_error = TRUE;
        }
        else
        {
            for(n = 0; n < value; n++)
                rs_read_boolean(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_shint(int inf, unsigned char *i)
{
    unsigned char buf;
    
    rs_read(inf, &buf, 1);
    
    *i = (unsigned char) buf;
    
    return(READSTAT);
}

int
rs_read_short(int inf, short *i)
{
    unsigned char bytes[2];
    short  input;
    unsigned char *buf = (unsigned char *)&input;
    
    rs_read(inf, &input, 2);

    if (mybig_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }
    
    *i = *((short *) buf);

    return(READSTAT);
} 

int
rs_read_shorts(int inf, short *i, int count)
{
    int n = 0, value = 0;

    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n = 0; n < value; n++)
                rs_read_short(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_ushort(int inf, unsigned short *i)
{
    unsigned char bytes[2];
    unsigned short  input;
    unsigned char *buf = (unsigned char *)&input;
    
    rs_read(inf, &input, 2);

    if (mybig_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }
    
    *i = *((unsigned short *) buf);

    return(READSTAT);
} 

int
rs_read_int(int inf, int *i)
{
    unsigned char bytes[4];
    int  input;
    unsigned char *buf = (unsigned char *)&input;
    
    rs_read(inf, &input, 4);

    if (mybig_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((int *) buf);

    return(READSTAT);
}

int
rs_read_ints(int inf, int *i, int count)
{
    int n = 0, value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n = 0; n < value; n++)
                rs_read_int(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_uint(int inf, unsigned int *i)
{
    unsigned char bytes[4];
    int  input;
    unsigned char *buf = (unsigned char *)&input;
    
    rs_read(inf, &input, 4);

    if (mybig_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((unsigned int *) buf);

    return(READSTAT);
}

int
rs_read_long(int inf, long *i)
{
    unsigned char bytes[4];
    long input;
    unsigned char *buf = (unsigned char *) &input;
    
    rs_read(inf, &input, 4);

    if (mybig_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((long *) buf);

    return(READSTAT);
}

int
rs_read_longs(int inf, long *i, int count)
{
    int n = 0, value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n = 0; n < value; n++)
                rs_read_long(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_ulong(int inf, unsigned long *i)
{
    unsigned char bytes[4];
    unsigned long input;
    unsigned char *buf = (unsigned char *) &input;
    
    rs_read(inf, &input, 4);

    if (mybig_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((unsigned long *) buf);

    return(READSTAT);
}

int
rs_read_ulongs(int inf, unsigned long *i, int count)
{
    int n = 0, value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n = 0; n < value; n++)
                rs_read_ulong(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_string(int inf, char *s, int max)
{
    int len = 0;

    if (rs_read_int(inf, &len) != FALSE)
    {
        if (len > max)
        {
            printf("String too long to restore. %d > %d\n",len,max);
            printf("Sorry, invalid save game format\n");
            format_error = TRUE;
        }
    
        rs_read(inf, s, len);
    }
    
    return(READSTAT);
}

int
rs_read_new_string(int inf, char **s)
{
    int len=0;
    char *buf=0;

    if (rs_read_int(inf, &len) != 0)
    {
        if (len == 0)
            *s = NULL;
        else
        { 
            buf = malloc(len);

            if (buf == NULL)            
                read_error = TRUE;
            else
            {
                rs_read(inf, buf, len);
                *s = buf;
            }
        }
    }

    return(READSTAT);
}

int
rs_read_string_index(int inf, char *master[], int maxindex, char **str)
{
    int i;

    if (rs_read_int(inf,&i) != 0)
    {
        if (i > maxindex)
        {
            printf("String index is out of range. %d > %d\n", i, maxindex);
            printf("Sorry, invalid save game format\n");
            format_error = TRUE;
        }
        else if (i >= 0)
            *str = master[i];
        else
            *str = NULL;
    }

    return(READSTAT);
}

int
rs_read_strings(int inf, char **s, int count, int max)
{
    int len   = 0;
    int n     = 0;
    int value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
        {
            printf("Incorrect number of strings in block. %d > %d.", 
                value, count);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else
        {
            for(n = 0; n < value; n++)
            {
                rs_read_string(inf, s[n], max);
            }
        }
    }
    
    return(READSTAT);
}

int
rs_read_new_strings(int inf, char **s, int count)
{
    int len   = 0;
    int n     = 0;
    int value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
        {
            printf("Incorrect number of new strings in block. %d > %d.",
                value,count);abort();
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else
            for(n=0; n<value; n++)
            {
                rs_read_int(inf, &len);
            
                if (len == 0)
                    s[n]=0;
                else 
                {
                    s[n] = malloc(len);
                    rs_read(inf,s[n],len);
                }
            }
    }
    
    return(READSTAT);
}

/******************************************************************************/

int
rs_write_coord(FILE *savef, coord c)
{
    rs_write_int(savef, c.x);
    rs_write_int(savef, c.y);
    
    return(WRITESTAT);
}

int
rs_read_coord(int inf, coord *c)
{
    rs_read_int(inf,&c->x);
    rs_read_int(inf,&c->y);
    
    return(READSTAT);
}

int
rs_write_coord_list(FILE *savef, struct linked_list *l)
{
    rs_write_int(savef, RSID_COORDLIST);
    rs_write_int(savef, list_size(l));

    while (l != NULL)
    {
        rs_write_coord(savef, *(coord *) l->l_data);
        l = l->l_next;
    }

    return(WRITESTAT);
}

int
rs_read_coord_list(int inf, struct linked_list **list)
{
    int id;
    int i, cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

    if (rs_read_int(inf,&id) != 0)
    {
        if (id != RSID_COORDLIST)
        {
            printf("Invalid id. %x != %x(RSID_COORDLIST)\n",
                id,RSID_COORDLIST);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf,&cnt) != 0)
        {
            for (i = 0; i < cnt; i++)
            {
                l = new_item(sizeof(coord));
                l->l_prev = previous;
                if (previous != NULL)
                    previous->l_next = l;
                rs_read_coord(inf,(coord *) l->l_data);
                if (previous == NULL)
                    head = l;
                previous = l;
            }

            if (l != NULL)
                l->l_next = NULL;

            *list = head;
        }
        else
            format_error = TRUE;
    }
    else
        format_error = TRUE;

    return(READSTAT);
}


int
rs_write_window(FILE *savef, WINDOW *win)
{
    int row,col,height,width;
    width = getmaxx(win);
    height = getmaxy(win);

    rs_write_int(savef,RSID_WINDOW);
    rs_write_int(savef,height);
    rs_write_int(savef,width);
    
    for(row=0;row<height;row++)
        for(col=0;col<width;col++)
            rs_write_int(savef, mvwinch(win,row,col));
}

int
rs_read_window(int inf, WINDOW *win)
{
    int id,row,col,maxlines,maxcols,value,width,height;
    
    width = getmaxx(win);
    height = getmaxy(win);

    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_WINDOW)
        {
            printf("Invalid head id. %x != %x(RSID_WINDOW)\n", id, RSID_WINDOW);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }   
        else
        {
            rs_read_int(inf,&maxlines);
            rs_read_int(inf,&maxcols);
            if (maxlines > height)
               abort();
            if (maxcols > width)
               abort();
               
            for(row=0;row<maxlines;row++)
                for(col=0;col<maxcols;col++)
                {
                    rs_read_int(inf, &value);
                    mvwaddch(win,row,col,value);
                }
        }
    }
        
    return(READSTAT);
}

int
rs_write_daemons(FILE *savef, struct delayed_action *d_list, int count)
{
    int i = 0;
    int func = 0;
        
    rs_write_int(savef, RSID_DAEMONS);
    rs_write_int(savef, count);
        
    for(i = 0; i < count; i++)
    {
        if ( d_list[i].d_func == rollwand)
            func = 1;
        else if ( d_list[i].d_func == doctor)
            func = 2;
        else if ( d_list[i].d_func == stomach)
            func = 3;
        else if ( d_list[i].d_func == runners)
            func = 4;
        else if ( d_list[i].d_func == swander)
            func = 5;
        else if ( d_list[i].d_func == trap_look)
            func = 6;
        else if ( d_list[i].d_func == ring_search)
            func = 7;
        else if ( d_list[i].d_func == ring_teleport)
            func = 8;
        else if ( d_list[i].d_func == strangle)
            func = 9;
        else if ( d_list[i].d_func == fumble)
            func = 10;
        else if ( d_list[i].d_func == wghtchk)
            func = 11;
        else if ( d_list[i].d_func == unstink)
            func = 12;
        else if ( d_list[i].d_func == res_strength)
            func = 13;
        else if ( d_list[i].d_func == un_itch)
            func = 14;
        else if ( d_list[i].d_func == cure_disease)
            func = 15;
        else if ( d_list[i].d_func == unconfuse)
            func = 16;
        else if ( d_list[i].d_func == suffocate)
            func = 17;
        else if ( d_list[i].d_func == undance)
            func = 18;
        else if ( d_list[i].d_func == alchemy)
            func = 19;
        else if ( d_list[i].d_func == dust_appear)
            func = 20;
        else if ( d_list[i].d_func == unchoke)
            func = 21;
        else if ( d_list[i].d_func == sight)
            func = 22;
        else if ( d_list[i].d_func == noslow)
            func = 23;
        else if ( d_list[i].d_func == nohaste)
            func = 24;
        else if ( d_list[i].d_func == unclrhead)
            func = 25;
        else if ( d_list[i].d_func == unsee)
            func = 26;
        else if ( d_list[i].d_func == unphase)
            func = 27;
        else if ( d_list[i].d_func == land)
            func = 28;
        else if ( d_list[i].d_func == appear)
            func = 29;

        rs_write_int(savef, d_list[i].d_type);
        rs_write_int(savef, func);
        rs_write_int(savef, d_list[i].d_arg);
        rs_write_int(savef, d_list[i].d_time);
    }
    
    return(WRITESTAT);
}       

int
rs_read_daemons(int inf, struct delayed_action *d_list, int count)
{
    int i = 0;
    int func = 0;
    int value = 0;
    int id = 0;
    
    if (d_list == NULL)
        printf("HELP THERE ARE NO DAEMONS\n");
    
    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_DAEMONS)
        {
            printf("Invalid id. %x != %x(RSID_DAEMONS)\n", id, RSID_DAEMONS);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf, &value) != 0)
        {
            if (value > count)
            {
                printf("Incorrect number of daemons in block. %d > %d.",
                    value, count);
                printf("Sorry, invalid save game format");
                format_error = TRUE;
            }
            else
            {
                for(i=0; i < value; i++)
                {
                    func = 0;
                    rs_read_int(inf, &d_list[i].d_type);
                    rs_read_int(inf, &func);
                    rs_read_int(inf, &d_list[i].d_arg);
                    rs_read_int(inf, &d_list[i].d_time);
                    
                    switch(func)
                    {
                        case 1: d_list[i].d_func = rollwand;
                                break;
                        case 2: d_list[i].d_func = doctor;
                                break;
                        case 3: d_list[i].d_func = stomach;
                                break;
                        case 4: d_list[i].d_func = runners;
                                break;
                        case 5: d_list[i].d_func = swander;
                                break;
                        case 6: d_list[i].d_func = trap_look;
                                break;
                        case 7: d_list[i].d_func = ring_search;
                                break;
                        case 8: d_list[i].d_func = ring_teleport;
                                break;
                        case 9: d_list[i].d_func = strangle;
                                break;
                        case 10: d_list[i].d_func = fumble;
                                break;
                        case 11: d_list[i].d_func = wghtchk;
                                break;
                        case 12: d_list[i].d_func = unstink;
                                break;
                        case 13: d_list[i].d_func = res_strength;
                                break;
                        case 14: d_list[i].d_func = un_itch;
                                break;
                        case 15: d_list[i].d_func = cure_disease;
                                break;
                        case 16: d_list[i].d_func = unconfuse;
                                break;
                        case 17: d_list[i].d_func = suffocate;
                                break;
                        case 18: d_list[i].d_func = undance;
                                break;
                        case 19: d_list[i].d_func = alchemy;
                                break;
                        case 20: d_list[i].d_func = dust_appear;
                                break;
                        case 21: d_list[i].d_func = unchoke;
                                break;
                        case 22: d_list[i].d_func = sight;
                                break;
                        case 23: d_list[i].d_func = noslow;
                                break;
                        case 24: d_list[i].d_func = nohaste;
                                break;
                        case 25: d_list[i].d_func = unclrhead;
                                break;
                        case 26: d_list[i].d_func = unsee;
                                break;
                        case 27: d_list[i].d_func = unphase;
                                break;
                        case 28: d_list[i].d_func = land;
                                break;
                        case 29: d_list[i].d_func = appear;
                                break;
                        default: d_list[i].d_func = NULL;
                                break;
                    }   
                }
            }
        }
    }
    
    return(READSTAT);
}        

int
rs_write_levtype(FILE *savef, LEVTYPE lev)
{
    int l;

    if (lev == NORMLEV)
        l = 0;
    else if (lev == POSTLEV)
        l = 1;
    else if (lev == MAZELEV)
        l = 2;
    else if (lev == OUTSIDE)
        l = 3;
    else
        l = -1;

    rs_write_int(savef,l);

    return(WRITESTAT);
}         

int
rs_read_levtype(int inf, LEVTYPE *lev)
{
    int l;

    rs_read_int(inf, &l);

    if (l == 0)
        *lev = NORMLEV;
    else if (l == 1)
        *lev = POSTLEV;
    else if (l == 2)
        *lev = MAZELEV;
    else if (l == 3)
        *lev = OUTSIDE;
    else
        *lev = NORMLEV;

    return(READSTAT);
}

int
rs_write_room_reference(FILE *savef, struct room *rp)
{
    int i, room = -1;
    
    for (i = 0; i < MAXROOMS; i++)
        if (&rooms[i] == rp)
            room = i;

    rs_write_int(savef, room);

    return(WRITESTAT);
}

int
rs_read_room_reference(int inf, struct room **rp)
{
    int i;
    
    rs_read_int(inf, &i);

    *rp = &rooms[i];
            
    return(READSTAT);
}

int
rs_write_rooms(FILE *savef, struct room r[], int count)
{
    int n = 0, i = -1;
    struct linked_list *l;

    rs_write_int(savef, count);
    
    for(n=0; n<count; n++)
    {
        rs_write_coord(savef, r[n].r_pos);
        rs_write_coord(savef, r[n].r_max);
        rs_write_long(savef, r[n].r_flags);
        rs_write_coord_list(savef, r[n].r_exit);

        l = r[n].r_fires;
        rs_write_int(savef, list_size(l));

        while (l != NULL)
        {
            i = find_list_ptr(mlist,l->l_data);
            rs_write_int(savef,i);
            l = l->l_next;
        }
    }
    
    return(WRITESTAT);
}

int
rs_read_rooms(int inf, struct room *r, int count)
{
    int value = 0, n = 0, i = 0, index = 0, id = 0;
    struct linked_list *fires = NULL, *item = NULL;

    if (rs_read_int(inf,&value) != 0)
    {
        if (value > count)
        {
            printf("Incorrect number of rooms in block. %d > %d.",
                value,count);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else
        {
            for(n = 0; n < value; n++)
            {
                rs_read_coord(inf,&r[n].r_pos);
                rs_read_coord(inf,&r[n].r_max);
                rs_read_long(inf,&r[n].r_flags);
                rs_read_coord_list(inf, &r[n].r_exit);

                rs_read_int(inf, &i);
                fires = NULL;
                while (i>0)
                {
                    rs_read_int(inf,&index);

                    if (index >= 0)
                    {
                        void *data;
                        data = get_list_item(mlist,index);
                        item = creat_item();
                        item->l_data = data;
                        if (fires == NULL)
                            fires = item;
                        else
                            attach(fires,item);
                    }
                    i--;
                }
                r[n].r_fires=fires;
            }
        }   
    }

    return(READSTAT);
}

int
rs_write_magic_items(FILE *savef, struct magic_item *i, int count)
{
    int n;
    
    rs_write_int(savef, RSID_MAGICITEMS);
    rs_write_int(savef, count);

    for(n = 0; n < count; n++)
    {
        /* mi_name is constant, defined at compile time in all cases */
        rs_write_int(savef,i[n].mi_prob);
    }
    
    return(WRITESTAT);
}

int
rs_read_magic_items(int inf, struct magic_item *mi, int count)
{
    int id;
    int n;
    int value;

    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_MAGICITEMS)
        {
            printf("Invalid id. %x != %x(RSID_MAGICITEMS)\n",
                id, RSID_MAGICITEMS);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }   
        else if (rs_read_int(inf, &value) != 0)
        {
            if (value > count)
            {
                printf("Incorrect number of magic items in block. %d > %d.",
                    value, count);
                printf("Sorry, invalid save game format");
                format_error = TRUE;
            }
            else
            {
                for(n = 0; n < value; n++)
                {
                    rs_read_int(inf,&mi[n].mi_prob);
                }
            }
        }
    }
    
    return(READSTAT);
}

int
rs_write_stats(FILE *savef, struct stats *s)
{
    rs_write_int(savef, RSID_STATS);
    rs_write_short(savef, s->s_str);
    rs_write_short(savef, s->s_intel);
    rs_write_short(savef, s->s_wisdom);
    rs_write_short(savef, s->s_dext);
    rs_write_short(savef, s->s_const);
    rs_write_short(savef, s->s_charisma);
    rs_write_ulong(savef, s->s_exp);
    rs_write_int(savef, s->s_lvl);
    rs_write_int(savef, s->s_arm);
    rs_write_int(savef, s->s_hpt);
    rs_write_int(savef, s->s_pack);
    rs_write_int(savef, s->s_carry);
    rs_write(savef, s->s_dmg, sizeof(s->s_dmg));

    return(WRITESTAT);
}

int
rs_read_stats(int inf, struct stats *s)
{
    int id;

    rs_read_int(inf, &id);
    rs_read_short(inf,&s->s_str);
    rs_read_short(inf,&s->s_intel);
    rs_read_short(inf,&s->s_wisdom);
    rs_read_short(inf,&s->s_dext);
    rs_read_short(inf,&s->s_const);
    rs_read_short(inf,&s->s_charisma);
    rs_read_ulong(inf,&s->s_exp);
    rs_read_int(inf,&s->s_lvl);
    rs_read_int(inf,&s->s_arm);
    rs_read_int(inf,&s->s_hpt);
    rs_read_int(inf,&s->s_pack);
    rs_read_int(inf,&s->s_carry);

    rs_read(inf,s->s_dmg,sizeof(s->s_dmg));
    
    return(READSTAT);
}

int
rs_write_monster_reference(FILE *savef, struct monster *m)
{
    int i, mon = -1;
    
    for (i = 0; i < (NUMMONST+1); i++)
        if (&monsters[i] == m)
            mon = i;

    rs_write_int(savef, mon);

    return(WRITESTAT);
}

int
rs_read_monster_reference(int inf, struct monster **mp)
{
    int i;
    
    rs_read_int(inf, &i);

    if (i < 0)
        *mp = NULL;
    else
        *mp = &monsters[i];
            
    return(READSTAT);
}

int
rs_write_monster_references(FILE *savef, struct monster *marray[], int count)
{
    int i;

    for(i = 0; i < count; i++)
        rs_write_monster_reference(savef, marray[i]);

    return(WRITESTAT);
}

int
rs_read_monster_references(int inf, struct monster *marray[], int count)
{
    int i;

    for(i = 0; i < count; i++)
        rs_read_monster_reference(inf, &marray[i]);

    return(READSTAT);
}

int
rs_write_object(FILE *savef, struct object *o)
{
    rs_write_int(savef, RSID_OBJECT);
    rs_write_int(savef, o->o_type);
    rs_write_coord(savef, o->o_pos);
    rs_write_char(savef, o->o_launch);
    rs_write(savef, o->o_damage, sizeof(o->o_damage));
    rs_write(savef, o->o_hurldmg, sizeof(o->o_hurldmg));
    rs_write_object_list(savef, o->contents);
    rs_write_int(savef, o->o_count);
    rs_write_int(savef, o->o_which);
    rs_write_int(savef, o->o_hplus);
    rs_write_int(savef, o->o_dplus);
    rs_write_int(savef, o->o_ac);
    rs_write_long(savef, o->o_flags);
    rs_write_int(savef, o->o_group);
    rs_write_int(savef, o->o_weight);
    rs_write(savef, o->o_mark,MARKLEN);

    return(WRITESTAT);
}

int
rs_read_object(int inf, struct object *o)
{
    int id;

    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_OBJECT)
        {
            printf("Invalid id. %x != %x(RSID_OBJECT)\n",
                id,RSID_OBJECT);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else
        {
            rs_read_int(inf, &o->o_type);
            rs_read_coord(inf, &o->o_pos);
            rs_read_char(inf, &o->o_launch);
            rs_read(inf, &o->o_damage, sizeof(o->o_damage));
            rs_read(inf, &o->o_hurldmg, sizeof(o->o_hurldmg));
            rs_read_object_list(inf, &o->contents);
            rs_read_int(inf, &o->o_count);
            rs_read_int(inf, &o->o_which);
            rs_read_int(inf, &o->o_hplus);
            rs_read_int(inf, &o->o_dplus);
            rs_read_int(inf, &o->o_ac);
            rs_read_long(inf, &o->o_flags);
            rs_read_int(inf, &o->o_group);
            rs_read_int(inf, &o->o_weight);
            rs_read(inf, &o->o_mark, MARKLEN);
        }
    }
    
    return(READSTAT);
}

int
rs_read_object_list(int inf, struct linked_list **list)
{
    int id;
    int i, cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

    if (rs_read_int(inf,&id) != 0)
    {
        if (rs_read_int(inf,&cnt) != 0)
        {
            for (i = 0; i < cnt; i++) 
            {
                l = new_item(sizeof(struct object));
                memset(l->l_data,0,sizeof(struct object));
                l->l_prev = previous;
                if (previous != NULL)
                    previous->l_next = l;
                rs_read_object(inf,(struct object *) l->l_data);
                if (previous == NULL)
                    head = l;
                previous = l;
            }
            
            if (l != NULL)
                l->l_next = NULL;
    
            *list = head;
        }
        else
            format_error = TRUE;
    }
    else
        format_error = TRUE;


    return(READSTAT);
}

int
rs_write_object_list(FILE *savef, struct linked_list *l)
{
    rs_write_int(savef, RSID_OBJECTLIST);
    rs_write_int(savef, list_size(l));

    while (l != NULL) 
    {
        rs_write_object(savef, (struct object *) l->l_data);
        l = l->l_next;
    }
    
    return(WRITESTAT);
}

int
rs_write_traps(FILE *savef, struct trap *trap,int count)
{
    int n;

    rs_write_int(savef, RSID_TRAP);
    rs_write_int(savef, count);
    
    for(n=0; n<count; n++)
    {
        rs_write_char(savef, trap[n].tr_type);
        rs_write_char(savef, trap[n].tr_show);
        rs_write_coord(savef, trap[n].tr_pos);
        rs_write_long(savef, trap[n].tr_flags);
    }
}

rs_read_traps(int inf, struct trap *trap, int count)
{
    int id = 0, value = 0, n = 0;

    if (rs_read_int(inf,&id) != 0)
    {
        if (id != RSID_TRAP)
        {
            printf("Invalid id. %x != %x(RSID_TRAP)\n",
                id,RSID_TRAP);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf,&value) != 0)
        {
            if (value > count)
            {
                printf("Incorrect number of traps in block. %d > %d.",
                    value,count);
                printf("Sorry, invalid save game format\n");
                format_error = TRUE;
            }
            else
            {
                for(n=0;n<value;n++)
                {   
                    rs_read_char(inf,&trap[n].tr_type);
                    rs_read_char(inf,&trap[n].tr_show);
                    rs_read_coord(inf,&trap[n].tr_pos);
                    rs_read_long(inf,&trap[n].tr_flags);
                }
            }
        }
        else
            format_error = TRUE;
    }
    
    return(READSTAT);
}

int
rs_write_monsters(FILE * savef, struct monster * m, int count)
{
    int n;
    
    rs_write_int(savef, RSID_MONSTERS);
    rs_write_int(savef, count);

    for(n=0;n<count;n++)
    {
        rs_write_boolean(savef, m[n].m_normal);
        rs_write_boolean(savef, m[n].m_wander);
        rs_write_short(savef, m[n].m_numsum);
    }
    
    return(WRITESTAT);
}

int
rs_read_monsters(int inf, struct monster *m, int count)
{
    int id = 0, value = 0, n = 0;
    
    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_MONSTERS)
        {
            printf("Invalid id. %x != %x(RSID_MONSTERS)\n",
                id,RSID_MONSTERS);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf, &value) != 0)
        {
            for(n=0;n<value;n++)
            {
                rs_read_boolean(inf, &m[n].m_normal);
                rs_read_boolean(inf, &m[n].m_wander);
                rs_read_short(inf, &m[n].m_numsum);
            }
        }
        else
            format_error = TRUE;
    }
    
    return(READSTAT);
}

int
find_thing_coord(struct linked_list *monlist, coord *c)
{
    struct linked_list *mitem;
    struct thing *tp;
    int i = 0;

    for(mitem = monlist; mitem != NULL; mitem = mitem->l_next)
    {
        tp = THINGPTR(mitem);
        if (c == &tp->t_pos)
            return(i);
        i++;
    }

    return(-1);
}

int
find_object_coord(struct linked_list *objlist, coord *c)
{
    struct linked_list *oitem;
    struct object *obj;
    int i = 0;

    for(oitem = objlist; oitem != NULL; oitem = oitem->l_next)
    {
        obj = OBJPTR(oitem);
        if (c == &obj->o_pos)
            return(i);
        i++;
    }

    return(-1);
}

void
rs_fix_thing(struct thing *t)
{
    struct linked_list *item;
    struct thing *tp;

    if (t->t_reserved < 0)
        return;

    item = get_list_item(mlist,t->t_reserved);

    if (item != NULL)
    {
        tp = THINGPTR(item);
        t->t_dest = &tp->t_pos;
    }
}

int 
find_room_exit(coord *c, int *room, int *exit)
{
    int i = 0, count = 0;
    struct linked_list *exitptr;

    if (c != NULL)
        for(i=0; i < MAXROOMS; i++)
	    for (exitptr = rooms[i].r_exit; exitptr; exitptr = next(exitptr)) {
                if (c == DOORPTR(exitptr)) {
                    *room = i;
                    *exit = count;
                    return(0);
                }
                count++;
            }

    *room = -1;
    *exit = -1;

    return(0);
}

coord *
lookup_exit(int room, int exit)
{
    int count = 0;
    struct linked_list *exitptr;
    
    if ((room == -1) || (exit == -1))
        return(NULL);

    for (exitptr = rooms[room].r_exit; exitptr; exitptr = next(exitptr)) {
            if (count == exit)
		return( DOORPTR(exitptr) );

            count++;
        }

    return(NULL);
}

int
rs_write_thing(FILE *savef, struct thing *t)
{
    int i = -1;
    int roomid = -1, index = -1;

    if (t == NULL)
    {
        rs_write_int(savef, RSID_THING_NULL);
        return(WRITESTAT);
    }
    
    rs_write_int(savef, RSID_THING);

    rs_write_boolean(savef, t->t_turn);
    rs_write_boolean(savef, t->t_wasshot);
    rs_write_char(savef, t->t_type);
    rs_write_char(savef, t->t_disguise);
    rs_write_char(savef, t->t_oldch);

    rs_write_short(savef, t->t_ctype);
    rs_write_short(savef, t->t_index);
    rs_write_short(savef, t->t_no_move);
    rs_write_short(savef, t->t_quiet);

    find_room_exit(t->t_doorgoal, &roomid, &index);
 
    rs_write_int(savef,roomid);
    rs_write_int(savef,index);

    rs_write_coord(savef, t->t_pos);
    rs_write_coord(savef, t->t_oldpos);

    /* 
        t_dest can be:
        0,0: NULL
        0,1: location of hero
        0,3: global coord 'delta'
        1,i: location of a thing (monster)
        2,i: location of an object
        3,i: location of gold in a room

        We need to remember what we are chasing rather than 
        the current location of what we are chasing.
    */

    if (t->t_dest == &hero)
    {
        rs_write_int(savef,0);
        rs_write_int(savef,1);
    }
    else if (t->t_dest != NULL)
    {
        i = find_thing_coord(mlist, t->t_dest);
            
        if (i >=0 )
        {
            rs_write_int(savef,1);
            rs_write_int(savef,i);
        }
        else
        {
            i = find_object_coord(lvl_obj, t->t_dest);
            
            if (i >= 0)
            {
                rs_write_int(savef,2);
                rs_write_int(savef,i);
            }
            else
            {
                rs_write_int(savef, 0);
                rs_write_int(savef,1); /* chase the hero anyway */
            }
        }
    }
    else
    {
        rs_write_int(savef,0);
        rs_write_int(savef,0);
    }
    
    rs_write_longs(savef, t->t_flags, 16);
    rs_write_object_list(savef, t->t_pack);
    rs_write_stats(savef, &t->t_stats);
    rs_write_stats(savef, &t->maxstats);
    
    return(WRITESTAT);
}

int
rs_read_thing(int inf, struct thing *t)
{
    int id;
    int listid = 0, index = -1;
    struct linked_list *item;
        
    if (rs_read_int(inf, &id) != 0)
    {
        if ((id != RSID_THING) && (id != RSID_THING_NULL)) {
            fmterr = "RSID_THING mismatch";
            format_error = TRUE;
        }
        else if (id == RSID_THING_NULL)
        {
            printf("NULL Thing?\n\r");
        }
        else
        {
            rs_read_boolean(inf, &t->t_turn);
            rs_read_boolean(inf, &t->t_wasshot);
            rs_read_char(inf, &t->t_type);
            rs_read_char(inf, &t->t_disguise);
            rs_read_char(inf, &t->t_oldch);
            rs_read_short(inf, &t->t_ctype);
            rs_read_short(inf, &t->t_index);
            rs_read_short(inf, &t->t_no_move);
            rs_read_short(inf, &t->t_quiet);
            rs_read_int(inf, &listid);
            rs_read_int(inf, &index);

            t->t_doorgoal = lookup_exit(listid, index);

            rs_read_coord(inf, &t->t_pos);
            rs_read_coord(inf, &t->t_oldpos);

            /* 
                t_dest can be (listid,index):
                0,0: NULL
                0,1: location of hero
                1,i: location of a thing (monster)
                2,i: location of an object
                3,i: location of gold in a room

                We need to remember what we are chasing rather than 
                the current location of what we are chasing.
            */
            
            rs_read_int(inf, &listid);
            rs_read_int(inf, &index);
            t->t_reserved = -1;

            if (listid == 0) /* hero or NULL */
            {
                if (index == 1)
                    t->t_dest = &hero;
                else
                    t->t_dest = NULL;
            }
            else if (listid == 1) /* monster/thing */
            {
                t->t_dest     = NULL;
                t->t_reserved = index;
            }
            else if (listid == 2) /* object */
            {
                struct object *obj;

                item = get_list_item(lvl_obj, index);

                if (item != NULL)
                {
                    obj = OBJPTR(item);
                    t->t_dest = &obj->o_pos;
                }
            }
            else
                t->t_dest = NULL;

            rs_read_longs(inf, t->t_flags, 16);
            rs_read_object_list(inf, &t->t_pack);
            rs_read_stats(inf, &t->t_stats);
            rs_read_stats(inf, &t->maxstats);
        }
    }
    else format_error = TRUE;
    
    return(READSTAT);
}

rs_fix_monster_list(list)
struct linked_list *list;
{
    struct linked_list *item;

    for(item = list; item != NULL; item = item->l_next)
        rs_fix_thing(THINGPTR(item));
}

int
rs_write_monster_list(FILE *savef, struct linked_list *l)
{
    int cnt = 0;
    
    rs_write_int(savef, RSID_MONSTERLIST);

    cnt = list_size(l);

    rs_write_int(savef, cnt);

    if (cnt < 1)
        return(WRITESTAT);

    while (l != NULL) {
        rs_write_thing(savef, (struct thing *)l->l_data);
        l = l->l_next;
    }
    
    return(WRITESTAT);
}

int
rs_read_monster_list(int inf, struct linked_list **list)
{
    int id;
    int i, cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

    if (rs_read_int(inf,&id) != 0)
    {
        if (id != RSID_MONSTERLIST)
        {
            printf("Invalid id. %x != %x(RSID_MONSTERLIST)\n",
                id,RSID_MONSTERLIST);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf,&cnt) != 0)
        {
            for (i = 0; i < cnt; i++) 
            {
                l = new_item(sizeof(struct thing));
                l->l_prev = previous;
                if (previous != NULL)
                    previous->l_next = l;
                rs_read_thing(inf,(struct thing *)l->l_data);
                if (previous == NULL)
                    head = l;
                previous = l;
            }
        

            if (l != NULL)
                l->l_next = NULL;

            *list = head;
        }
    }
    else format_error = TRUE;
    
    return(READSTAT);
}

int
rs_write_object_reference(FILE *savef, struct linked_list *list, 
    struct object *item)
{
    int i;
    
    i = find_list_ptr(list, item);
    rs_write_int(savef, i);

    return(WRITESTAT);
}

rs_read_object_reference(int inf, struct linked_list *list, 
    struct object **item)
{
    int i;
    
    rs_read_int(inf, &i);
    *item = get_list_item(list,i);
            
    return(READSTAT);
}



int
rs_read_scrolls(int inf)
{
    int i;

    for(i = 0; i < MAXSCROLLS; i++)
    {
        rs_read_new_string(inf,&s_names[i]);
        rs_read_boolean(inf,&s_know[i]);
        rs_read_new_string(inf,&s_guess[i]);
    }

    return(READSTAT);
}

int
rs_write_scrolls(FILE *savef)
{
    int i;

    for(i = 0; i < MAXSCROLLS; i++)
    {
        rs_write_string(savef,s_names[i]);
        rs_write_boolean(savef,s_know[i]);
        rs_write_string(savef,s_guess[i]);
    }
    return(READSTAT);
}

int
rs_read_potions(int inf)
{
    int i;

    for(i = 0; i < MAXPOTIONS; i++)
    {
        rs_read_string_index(inf,rainbow,NCOLORS,&p_colors[i]);
        rs_read_boolean(inf,&p_know[i]);
        rs_read_new_string(inf,&p_guess[i]);
    }

    return(READSTAT);
}

int
rs_write_potions(FILE *savef)
{
    int i;

    for(i = 0; i < MAXPOTIONS; i++)
    {
        rs_write_string_index(savef,rainbow,NCOLORS,p_colors[i]);
        rs_write_boolean(savef,p_know[i]);
        rs_write_string(savef,p_guess[i]);
    }

    return(WRITESTAT);
}

int
rs_read_rings(int inf)
{
    int i;

    for(i = 0; i < MAXRINGS; i++)
    {
        rs_read_string_index(inf,stones,NSTONES,&r_stones[i]);
        rs_read_boolean(inf,&r_know[i]);
        rs_read_new_string(inf,&r_guess[i]);
    }

    return(READSTAT);
}

int
rs_write_rings(FILE *savef)
{
    int i;

    for(i = 0; i < MAXRINGS; i++)
    {
        rs_write_string_index(savef,stones,NSTONES,r_stones[i]);
        rs_write_boolean(savef,r_know[i]);
        rs_write_string(savef,r_guess[i]);
    }

    return(WRITESTAT);
}

int
rs_write_sticks(FILE *savef)
{
    int i;

    for (i = 0; i < MAXSTICKS; i++)
    {
        if (strcmp(ws_type[i],"staff") == 0)
        {
            rs_write_int(savef,0);
            rs_write_string_index(savef, wood, NWOOD, ws_made[i]);
        }
        else
        {
            rs_write_int(savef,1);
            rs_write_string_index(savef, metal, NMETAL, ws_made[i]);
        }
        rs_write_boolean(savef, ws_know[i]);
        rs_write_string(savef, ws_guess[i]);
    }

    return(WRITESTAT);
}

int
rs_read_sticks(int inf)
{
    int i = 0, list = 0;

    for(i = 0; i < MAXSTICKS; i++)
    {
        rs_read_int(inf,&list);
        if (list == 0)
        {
            rs_read_string_index(inf,wood,NWOOD,&ws_made[i]);
            ws_type[i] = "staff";
        }
        else
        {
            rs_read_string_index(inf,metal,NMETAL,&ws_made[i]);
            ws_type[i] = "wand";
        }

        rs_read_boolean(inf, &ws_know[i]);
        rs_read_new_string(inf, &ws_guess[i]);
    }

    return(READSTAT);
}

int
rs_save_file(FILE *savef)
{
    int i;
    int myendian = 0x01020304;
    mybig_endian = ( *((char *)&myendian) == 0x01 );

    rs_write_traps(savef, traps, MAXTRAPS);             
    rs_write_rooms(savef, rooms, MAXROOMS);
    rs_write_room_reference(savef, oldrp);
    rs_write_thing(savef, &player);
    rs_write_object_reference(savef, player.t_pack, cur_armor);
    for(i = 0; i < NUM_FINGERS; i++)
        rs_write_object_reference(savef, player.t_pack, cur_ring[i]);
    for(i = 0; i < NUM_MM; i++)
        rs_write_object_reference(savef, player.t_pack, cur_misc[i]);
    rs_write_ints(savef, cur_relic, MAXRELIC);
    rs_write_object_list(savef, lvl_obj);
    rs_write_monster_list(savef, mlist);
    rs_write_monster_list(savef, tlist);
    rs_write_monster_list(savef, monst_dead);
    rs_write_object_reference(savef, player.t_pack, cur_weapon); 
    rs_write_int(savef, char_type);
    rs_write_int(savef, foodlev);
    rs_write_int(savef, ntraps);
    rs_write_int(savef, trader);
    rs_write_int(savef, curprice);
    rs_write_int(savef, no_move);
    rs_write_int(savef, seed);
    rs_write_int(savef, dnum);
    rs_write_int(savef, max_level);
    rs_write_int(savef, cur_max);
    rs_write_int(savef, lost_dext);
    rs_write_int(savef, no_command);
    rs_write_int(savef, level);
    rs_write_int(savef, purse);
    rs_write_int(savef, inpack);
    rs_write_int(savef, total);
    rs_write_int(savef, no_food);
    rs_write_int(savef, foods_this_level);
    rs_write_int(savef, count);
    rs_write_int(savef, food_left);
    rs_write_int(savef, group);
    rs_write_int(savef, hungry_state);
    rs_write_int(savef, infest_dam);
    rs_write_int(savef, lost_str);
    rs_write_int(savef, lastscore);
    rs_write_int(savef, hold_count);
    rs_write_int(savef, trap_tries);
    rs_write_int(savef, pray_time);
    rs_write_int(savef, spell_power);
    rs_write_int(savef, turns);
    rs_write_int(savef, quest_item);
    rs_write_char(savef, nfloors);
    rs_write(savef, curpurch, 15);
    rs_write_char(savef, PLAYER);
    rs_write_char(savef, take);
    rs_write(savef, prbuf, LINELEN);
    rs_write_char(savef, runch);
    rs_write(savef, whoami, LINELEN);
    rs_write(savef, fruit, LINELEN);
    rs_write_scrolls(savef);
    rs_write_potions(savef);
    rs_write_rings(savef);
    rs_write_sticks(savef);
    for(i = 0; i < MAXMM; i++)
        rs_write_string(savef, m_guess[i]);
    rs_write_window(savef, cw);
    rs_write_window(savef, mw);
    rs_write_window(savef, stdscr);

    rs_write_boolean(savef, pool_teleport);
    rs_write_boolean(savef, inwhgt);
    rs_write_boolean(savef, after);
    rs_write_boolean(savef, waswizard);
    rs_write_booleans(savef, m_know, MAXMM);
    rs_write_boolean(savef, playing);
    rs_write_boolean(savef, running);
    rs_write_boolean(savef, wizard);
    rs_write_boolean(savef, notify);
    rs_write_boolean(savef, fight_flush);
    rs_write_boolean(savef, terse);
    rs_write_boolean(savef, auto_pickup);
    rs_write_boolean(savef, door_stop);
    rs_write_boolean(savef, jump);
    rs_write_boolean(savef, slow_invent);
    rs_write_boolean(savef, firstmove);
    rs_write_boolean(savef, askme);
    rs_write_boolean(savef, in_shell);
    rs_write_boolean(savef, daytime);
    rs_write_coord(savef, delta);
    rs_write_levtype(savef, levtype);

    rs_write_monsters(savef, monsters, NUMMONST);
    rs_write_magic_items(savef, things, NUMTHINGS);
    rs_write_magic_items(savef, s_magic, MAXSCROLLS);
    rs_write_magic_items(savef, p_magic, MAXPOTIONS);
    rs_write_magic_items(savef, r_magic, MAXRINGS);
    rs_write_magic_items(savef, ws_magic, MAXSTICKS);
    rs_write_magic_items(savef, m_magic, MAXMM);


    rs_write_coord(savef, ch_ret);               /* 5.8 chase.c   */
    rs_write_int(savef, demoncnt);               /* 5.8 daemon.c  */
    rs_write_int(savef, fusecnt);                /* 5.8 daemon.c  */
    rs_write_daemons(savef, d_list, MAXDAEMONS); /* 5.8 daemon.c  */
    rs_write_daemons(savef, f_list, MAXFUSES);   /* 5.8 daemon.c  */
    rs_write_int(savef, between);                /* 5.8 daemons.c */

    fflush(savef);

    return(WRITESTAT);
}

rs_restore_file(int inf)
{
    int i;
    bool junk;
    int endian = 0x01020304;
    mybig_endian = ( *((char *)&endian) == 0x01 );
    
    rs_read_traps(inf, traps, MAXTRAPS);             
    rs_read_rooms(inf, rooms, MAXROOMS);
    rs_read_room_reference(inf, &oldrp);
    rs_read_thing(inf, &player);
    rs_read_object_reference(inf, player.t_pack, &cur_armor);
    for(i = 0; i < NUM_FINGERS; i++)
        rs_read_object_reference(inf, player.t_pack, &cur_ring[i]);
    for(i = 0; i < NUM_MM; i++)
        rs_read_object_reference(inf, player.t_pack, &cur_misc[i]);
    rs_read_ints(inf, cur_relic, MAXRELIC);
    rs_read_object_list(inf, &lvl_obj);
    rs_read_monster_list(inf, &mlist);
    rs_read_monster_list(inf, &tlist);
    rs_read_monster_list(inf, &monst_dead);
    rs_read_object_reference(inf, player.t_pack, &cur_weapon); 
    rs_read_int(inf, &char_type);
    rs_read_int(inf, &foodlev);
    rs_read_int(inf, &ntraps);
    rs_read_int(inf, &trader);
    rs_read_int(inf, &curprice);
    rs_read_int(inf, &no_move);
    rs_read_int(inf, &seed);
    rs_read_int(inf, &dnum);
    rs_read_int(inf, &max_level);
    rs_read_int(inf, &cur_max);
    rs_read_int(inf, &lost_dext);
    rs_read_int(inf, &no_command);
    rs_read_int(inf, &level);
    rs_read_int(inf, &purse);
    rs_read_int(inf, &inpack);
    rs_read_int(inf, &total);
    rs_read_int(inf, &no_food);
    rs_read_int(inf, &foods_this_level);
    rs_read_int(inf, &count);
    rs_read_int(inf, &food_left);
    rs_read_int(inf, &group);
    rs_read_int(inf, &hungry_state);
    rs_read_int(inf, &infest_dam);
    rs_read_int(inf, &lost_str);
    rs_read_int(inf, &lastscore);
    rs_read_int(inf, &hold_count);
    rs_read_int(inf, &trap_tries);
    rs_read_int(inf, &pray_time);
    rs_read_int(inf, &spell_power);
    rs_read_int(inf, &turns);
    rs_read_int(inf, &quest_item);
    rs_read_char(inf, &nfloors);
    rs_read(inf, &curpurch, 15);
    rs_read_char(inf, &PLAYER);
    rs_read_char(inf, &take);
    rs_read(inf, &prbuf, LINELEN);
    rs_read_char(inf, &runch);
    rs_read(inf, &whoami, LINELEN);
    rs_read(inf, &fruit, LINELEN);
    rs_read_scrolls(inf);
    rs_read_potions(inf);
    rs_read_rings(inf);
    rs_read_sticks(inf);
    for(i = 0; i < MAXMM; i++)
        rs_read_new_string(inf, &m_guess[i]);
    rs_read_window(inf, cw);
    rs_read_window(inf, mw);
    rs_read_window(inf, stdscr);

    rs_read_boolean(inf, &pool_teleport);
    rs_read_boolean(inf, &inwhgt);
    rs_read_boolean(inf, &after);
    rs_read_boolean(inf, &waswizard);
    rs_read_booleans(inf, m_know, MAXMM);
    rs_read_boolean(inf, &playing);
    rs_read_boolean(inf, &running);
    rs_read_boolean(inf, &wizard);
    rs_read_boolean(inf, &notify);
    rs_read_boolean(inf, &fight_flush);
    rs_read_boolean(inf, &terse);
    rs_read_boolean(inf, &auto_pickup);
    rs_read_boolean(inf, &door_stop);
    rs_read_boolean(inf, &jump);
    rs_read_boolean(inf, &slow_invent);
    rs_read_boolean(inf, &firstmove);
    rs_read_boolean(inf, &askme);
    rs_read_boolean(inf, &in_shell);
    rs_read_boolean(inf, &daytime);
    rs_read_coord(inf, &delta);
    rs_read_levtype(inf, &levtype);

    rs_read_monsters(inf, monsters, NUMMONST);
    rs_read_magic_items(inf, things, NUMTHINGS);
    rs_read_magic_items(inf, s_magic, MAXSCROLLS);
    rs_read_magic_items(inf, p_magic, MAXPOTIONS);
    rs_read_magic_items(inf, r_magic, MAXRINGS);
    rs_read_magic_items(inf, ws_magic, MAXSTICKS);
    rs_read_magic_items(inf, m_magic, MAXMM);


    rs_read_coord(inf, &ch_ret);               /* 5.8 chase.c   */
    rs_read_int(inf, &demoncnt);               /* 5.8 daemon.c  */
    rs_read_int(inf, &fusecnt);                /* 5.8 daemon.c  */
    rs_read_daemons(inf, d_list, MAXDAEMONS); /* 5.8 daemon.c  */
    rs_read_daemons(inf, f_list, MAXFUSES);   /* 5.8 daemon.c  */
    rs_read_int(inf, &between);                /* 5.8 daemons.c */


    return(READSTAT);
}
