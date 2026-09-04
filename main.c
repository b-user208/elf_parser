#include <stdio.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

Elf64_Ehdr header;
Elf64_Shdr *sections;
Elf64_Sym *symboles;

Elf64_Ehdr *elf_header(FILE *PATH){
    char elf_sign[] = {0x7F, 'E', 'L', 'F'};
    if(fread(&header, sizeof header, 1, PATH) != 1){
        perror("fread");
        return NULL;
    }
    if(memcmp(header.e_ident, elf_sign, 4) != 0){
        fprintf(stderr, "Le fichier n'est pas un fichier elf.\n");
        return NULL;
    }
    return &header;
}

char *section_headers(FILE *PATH){
    sections = malloc(header.e_shnum * sizeof(Elf64_Shdr));
    if(!sections){
        perror("malloc");
        return NULL;
    }
    if(fseek(PATH, header.e_shoff, SEEK_SET) != 0){
        perror("fseek");
        free(sections);
        return NULL;
    }
    if(fread(sections, sizeof(Elf64_Shdr), header.e_shnum, PATH) != header.e_shnum){
        perror("fread");
        free(sections);
        return NULL;
    }
    if(fseek(PATH, sections[header.e_shstrndx].sh_offset, SEEK_SET) != 0){
        perror("fseek");
        free(sections);
        return NULL;
    }
    char *name = malloc(sections[header.e_shstrndx].sh_size);
    if(!name){
        perror("malloc");
        free(sections);
        return NULL;
    }
    if(fread(name, 1, sections[header.e_shstrndx].sh_size, PATH) != sections[header.e_shstrndx].sh_size){
        perror("fread");
        free(sections);
        free(name);
        return NULL;
    }
    return name;
}

char *symb(FILE *PATH){
    unsigned offset_symb = 0, 
    nb_symb = 0, 
    offset_sechead = 0, 
    offset_strtab = 0,
    size_strtab = 0;
    for(unsigned i = 0; i < header.e_shnum; i++){
        if(sections[i].sh_type == SHT_SYMTAB){
            offset_symb = sections[i].sh_offset;
            nb_symb = sections[i].sh_size / sections[i].sh_entsize;
            offset_sechead = sections[i].sh_link;
            break;
        }
    }
    offset_strtab = sections[offset_sechead].sh_offset;
    size_strtab = sections[offset_sechead].sh_size;

    if(fseek(PATH, offset_strtab, SEEK_SET) != 0){
        perror("fseek");
        return NULL;
    }
    char *strtab = malloc(size_strtab);
    if(!strtab){
        perror("malloc");
        return NULL;
    }
    if(fread(strtab, size_strtab, 1, PATH) != 1){
        perror("fread");
        free(strtab);
        return NULL;
    }

    if(fseek(PATH, offset_symb, SEEK_SET) != 0){
        perror("fseek");
        free(strtab);
        return NULL;
    }
    symboles = malloc(nb_symb * sizeof(Elf64_Sym));
    if(!symboles){
        perror("malloc");
        free(strtab);
        return NULL;
    }
    if(fread(symboles, sizeof(Elf64_Sym), nb_symb, PATH) != nb_symb){
        perror("fread");
        free(strtab);
        free(symboles);
        return NULL;
    }
    return strtab;
}

const char *gettype(Elf64_Half e_type){
    switch(e_type)
    {
        case 0:  return "ET_NONE";
        case 1:  return "ET_REL";
        case 2:  return "ET_EXEC";
        case 3:  return "ET_DYN";
        case 4:  return "ET_CORE";
        default: return "UNKNOWN";
    }
}

const char *getmachine(Elf64_Half machine){
    switch (machine)
    {
        case 3:   return "x86";
        case 62:  return "x86-64";
        case 40:  return "ARM";
        case 183: return "ARM64";
        default:  return "UNKNOWN";
    }
}

const char *getshtype(Elf64_Word type){
    switch (type)
    {
        case  0: return "SHT_NULL";
        case  1: return "SHT_PROGBITS";
        case  2: return "SHT_SYMTAB";
        case  3: return "SHT_STRTAB";
        case  4: return "SHT_RELA";
        case  5: return "SHT_HASH";
        case  6: return "SHT_DYNAMIC";
        case  7: return "SHT_NOTE";
        case  8: return "SHT_NOBITS";
        case  9: return "SHT_REL";
        case 11: return "SHT_DYNSYM";
        case 14: return "SHT_INIT_ARRAY";
        case 15: return "SHT_FINI_ARRAY";
        case 16: return "SHT_PREINIT_ARRAY";
        case 17: return "SHT_GROUP";
        case 18: return "SHT_SYMTAB_SHNDX";
        case 0x6ffffff6: return "SHT_GNU_HASH";
        case 0x6ffffffe: return "SHT_GNU_VERNEED";
        case 0x6fffffff: return "SHT_GNU_VERSYM";
        default: return "UNKNOWN";
    }
}

const char *getstbinding(unsigned char binding){
    switch (binding)
    {
        case 0:  return "LOCAL";
        case 1:  return "GLOBAL";
        case 2:  return "WEAK";
        case 10: return "GNU_UNIQUE";
        default: return "UNKNOWN";
    }
}

const char *getsttype(unsigned char type)
{
    switch (type)
    {
        case 0: return "NOTYPE";
        case 1: return "OBJECT";
        case 2: return "FUNC";
        case 3: return "SECTION";
        case 4: return "FILE";
        case 5: return "COMMON";
        case 6: return "TLS";
        case 10: return "GNU_IFUNC";
        default: return "UNKNOWN";
    }
}

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("Nombre d'arguments insufissants.\n");
        return EXIT_FAILURE;
    }
    FILE *fp;
    Elf64_Ehdr *header;
    char *name,*strtab, i = 1;
    unsigned nb_symb = 0;

    while((argc - 1) > 0){
        fp = fopen(argv[i], "rb");
        if(!fp){ perror("fopen"); goto error;}

        header = elf_header(fp);
        if(!header){ fclose(fp); goto error; }

        name = section_headers(fp);
        if(!name){ fclose(fp); goto error; }

        strtab = symb(fp);
        if(!strtab){ goto strtab; }
        printf("Magic   : ");
        for(unsigned char i = 0; i < 8; i++){ printf("%02X ", header->e_ident[i]);}
        printf("\n");
        printf("Type    : %s\n", gettype(header->e_type));
        printf("Machine : %s\n", getmachine(header->e_machine));
        printf("Entry   : 0x%lX\n", header->e_entry);
        for(unsigned i = 0; i < header->e_shnum; i++){
            printf("%-4u = %-30s %-20s off=0x%-10lX size=0x%lX\n", 
                i, &name[sections[i].sh_name], 
                getshtype(sections[i].sh_type), 
                sections[i].sh_offset, 
                sections[i].sh_size);
        }
        for(unsigned i = 0; i < header->e_shnum; i++){
            if(sections[i].sh_type == SHT_SYMTAB){
                nb_symb = sections[i].sh_size / sections[i].sh_entsize;
                break;
            }
        }
        printf("\nSymboles\n");
        for(unsigned i = 0; i < nb_symb; i++){
            printf("%-4u = %-40s %-10s %-10s %-20s 0x%-10lX size=0x%lX\n", 
                i, 
                &strtab[symboles[i].st_name], 
                getstbinding(ELF64_ST_BIND(symboles[i].st_info)), 
                getsttype(ELF64_ST_TYPE(symboles[i].st_info)), 
                (symboles[i].st_shndx == 0) ? "UND" :
                (symboles[i].st_shndx == 0xfff1) ? "ABS" :
                (symboles[i].st_shndx == 0xfff2) ? "COMMON":
                &name[sections[symboles[i].st_shndx].sh_name],
                symboles[i].st_value, symboles[i].st_size);
        }
            free(symboles);
            free(strtab);
            strtab :
            free(sections);
            free(name);
            fclose(fp);
            error :
            argc--;
            i++;
        printf("\n\n");
    }
    return 0;
}