# Intro

Comme le nom le designe , ceci est un simple parseur elf, un outil permettant l'analyse de la structure interne d'un binaire ELF — sections, types, symboles et adresses. Je me suis engagé dans ce projet pour ma première expérience en reverse engineering, un domaine que je vise.

## Fonctionnalités

Il inclut différentes fonctionnalités, notamment :

#### Affichage des infos globales du fichier

**Magic** : La signature permettant d'identifier un fichier ELF.

**Type** : Le type de fichier ELF, par exemple un exécutable ou une bibliothèque partagée.

**Machine** : L'architecture processeur pour laquelle le programme a été compilé.

**Entry** : L'adresse mémoire de la première instruction exécutée lors du lancement du programme.


#### Affichage des sections et de leurs types

##### Sections de codes

* `.text`
* `.init`
* `.fini`
* `.plt`

##### Sections de données

* `.rodata`
* `.data`
* `.bss`
* `.got`

Et bien d'autres sections.

#### Affichage des symboles et leurs propriétés 

| N° | Nom | Binding | Type | Section | Adresse | Taille |
|---:|---|---|---|---|---:|---:|
| 2 | `__abi_tag` | `LOCAL` | `OBJECT` | `.note.ABI-tag` | `0x38C` | `0x20` |
| 3 | `crtstuff.c` | `LOCAL` | `FILE` | `ABS` | `0x0` | `0x0` |
| 4 | `deregister_tm_clones` | `LOCAL` | `FUNC` | `.text` | `0x1210` | `0x0` |
| 5 | `register_tm_clones` | `LOCAL` | `FUNC` | `.text` | `0x1240` | `0x0` |
| 6 | `__do_global_dtors_aux` | `LOCAL` | `FUNC` | `.text` | `0x1280` | `0x0` |



## Compilation 

```console
> Requiert Linux et GCC.
gcc main.c -o elfparser  
./elfparser <fichier>
```

## Exemple de sortie 

```console
Magic   : 7F 45 4C 46 02 01 01 00 
Type    : ET_DYN
Machine : x86-64
Entry   : 0x11E0
0    =                                SHT_NULL             off=0x0          size=0x0
1    = .interp                        SHT_PROGBITS         off=0x318        size=0x1C
2    = .note.gnu.property             SHT_NOTE             off=0x338        size=0x30
3    = .note.gnu.build-id             SHT_NOTE             off=0x368        size=0x24
...
30   = .shstrtab                      SHT_STRTAB           off=0x491B       size=0x11A

Symboles
0    =                                          LOCAL      NOTYPE     UND                  0x0          size=0x0
1    = Scrt1.o                                  LOCAL      FILE       ABS                  0x0          size=0x0
2    = __abi_tag                                LOCAL      OBJECT     .note.ABI-tag        0x38C        size=0x20
...
59   = stderr@GLIBC_2.2.5                       GLOBAL     OBJECT     .bss                 0x5020       size=0x8
```
