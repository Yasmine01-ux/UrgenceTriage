#ifndef OUTILS_H
#define OUTILS_H

#include <stddef.h>
#include <time.h>

int outil_lire_entier(const char *invite, int minimum, int maximum, int *valeur);
int outil_lire_texte(const char *invite, char *destination, size_t capacite);
void outil_afficher_date(time_t instant, char *destination, size_t capacite);

#endif