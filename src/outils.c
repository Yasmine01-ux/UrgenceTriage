#include "outils.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void retirer_fin_ligne(char *texte)
{
    size_t longueur = strlen(texte);

    while (longueur > 0 && (texte[longueur - 1] == '\n' ||
                            texte[longueur - 1] == '\r')) {
        texte[--longueur] = '\0';
    }
}

int outil_lire_entier(const char *invite, int minimum, int maximum, int *valeur)
{
    char ligne[128];

    for (;;) {
        char *fin;
        long resultat;
        int conversion;

        printf("%s", invite);
        fflush(stdout);
        if (fgets(ligne, sizeof(ligne), stdin) == NULL) {
            return 0;
        }
        retirer_fin_ligne(ligne);
        errno = 0;
        resultat = strtol(ligne, &fin, 10);
        conversion = fin != ligne;
        while (isspace((unsigned char)*fin)) {
            fin++;
        }
        if (conversion && *fin == '\0' && errno != ERANGE &&
            resultat >= minimum && resultat <= maximum) {
            *valeur = (int)resultat;
            return 1;
        }
        printf("Saisie invalide. Entrez un nombre entre %d et %d.\n",
               minimum, maximum);
    }
}

int outil_lire_texte(const char *invite, char *destination, size_t capacite)
{
    char ligne[512];

    if (capacite == 0) {
        return 0;
    }
    for (;;) {
        size_t longueur;
        size_t debut = 0;
        size_t fin;

        printf("%s", invite);
        fflush(stdout);
        if (fgets(ligne, sizeof(ligne), stdin) == NULL) {
            return 0;
        }
        longueur = strlen(ligne);
        if (longueur > 0 && ligne[longueur - 1] != '\n' && !feof(stdin)) {
            int caractere;
            while ((caractere = getchar()) != '\n' && caractere != EOF) {
            }
            printf("Texte trop long. Recommencez.\n");
            continue;
        }
        retirer_fin_ligne(ligne);
        longueur = strlen(ligne);
        while (debut < longueur && isspace((unsigned char)ligne[debut])) {
            debut++;
        }
        fin = longueur;
        while (fin > debut && isspace((unsigned char)ligne[fin - 1])) {
            fin--;
        }
        if (fin == debut || fin - debut >= capacite) {
            printf("Saisie vide ou trop longue. Recommencez.\n");
            continue;
        }
        memmove(destination, ligne + debut, fin - debut);
        destination[fin - debut] = '\0';
        for (longueur = 0; destination[longueur] != '\0'; longueur++) {
            if (destination[longueur] == ';') {
                destination[longueur] = ',';
            }
        }
        return 1;
    }
}

void outil_afficher_date(time_t instant, char *destination, size_t capacite)
{
    struct tm *date_locale = localtime(&instant);

    if (date_locale == NULL || strftime(destination, capacite,
                                         "%d/%m/%Y %H:%M", date_locale) == 0) {
        if (capacite > 0) {
            destination[0] = '\0';
        }
    }
}