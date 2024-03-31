/////////////////////////////////////////////////////////////////
//
//      GRP format by   - Ken Silverman/3D Realms.
//      Program by      - Cerium.
//
//
//      This is a new GRP file extraction tool. I wrote
//  this because I Ken's old version was really old. Plus I
//  thought it would be a fun little project to work on. I plan
//  on making this tool a little more useful and allowing it to 
//  create/append GRPs. Unfortunately I'm too lazy so it might
//  be a while before that happens.
//
//  At the start of the file is a signature and the number 
//  of files. After that is a header specifiying name and size
//  for each individual file. Past that can be found the raw
//  data for each file.  Organized in the same order as the 
//  file headers.
//
//      START OF FILE
//  - Signature | 12 bytes | Ken's name "KenSilverman"
//  - FileCount |  4 bytes | Number of files in archive.
//
//      FOR EACH FILE 
//  - Name   | 12 bytes | Name of the file.
//  - Length |  4 bytes | Unsigned length of the file.
//
/////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <strings.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;

//
// Note: Does not align with the BUILD version.
//
// We really don't need a hash system, but I 
// thought it might be useful. Even if it 
// doesn't increase performance significantly.
//
typedef struct grpent_t {
    u8  name[14];
    u32 size;
    u32 offs;
    u32 key; 
} grpent_t;

grpent_t *ents;
u8  kensig[12];
u32 numfiles;
FILE *fgrp;



void quick_dump(const char *filename, grpent_t entry)
{
    void *buf = calloc(1, entry.size);
    FILE *out = fopen(filename, "wb");               
    fseek(fgrp, entry.offs, SEEK_SET);
    fread (buf, entry.size, 1, fgrp); // get file contents.
    fwrite(buf, entry.size, 1, out); // dump into output file.
    fclose(out);    
    free(buf);
}

u32 encode(const char *s)
{
    u32 hash   = 0;
    char *str           = (char*)s;
    char c;

    while((c = *str++))
    {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

void grp_openr(const char *groupfilename)
{
    if((fgrp = fopen(groupfilename, "rb")) == NULL)
    {
        printf("Cannot read file '%s'\n", groupfilename);
        exit(-2);
        return;
    }
    
    fread(kensig, 12, 1, fgrp);
    fread(&numfiles, 4, 1, fgrp);
    
    if(strncasecmp(kensig, "KenSilverman", 12) != 0)
    {
        printf("Invalid GRP file\n");
        fclose(fgrp);
        exit(-3);
        return;
    }
    
    ents = calloc(numfiles, sizeof(grpent_t));
    
    if(ents == NULL)
    {
        printf("Failed allocation of %i bytes.\n", numfiles * sizeof(grpent_t));
        fclose(fgrp);
        exit(-4);
        return;
    }
    
        
    int offsets = 16 + (16 * numfiles);
    
    for(int i = 0; i < numfiles; i++)
    {      
        fread(&ents[i].name, 12, 1, fgrp);
        fread(&ents[i].size, 4, 1, fgrp);    
        ents[i].offs = offsets;
        offsets += ents[i].size;
        ents[i].key = encode(ents[i].name);
        printf("[%i] = %d - %.12s\n", i, ents[i].key, ents[i].name);
    }
}

void grp_close(void)
{
    free(ents);
    fclose(fgrp);
}

int grp_search(const char *name)
{
    int e = encode(name);
    for(int i = 0; i < numfiles; i++)
    {
        if(ents[i].key == e)
            if(strncasecmp(ents[i].name, name, 12) == 0)
                return i;
    }
    return -1;
}


int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("Requested file name not given.\nExample: 'E1L1.MAP'/GOTHAM.MID/...\n");
        return -1;
    }
    
    grp_openr("DUKE3D.GRP");

    int i = grp_search(argv[1]);
    if(i == -1)
    {
        printf("File '%s' not found in GRP.\n", argv[1]);
        fclose(fgrp);
        return -3;
    }

    quick_dump(argv[1], ents[i]);
    
    grp_close();
    return 0;
}
