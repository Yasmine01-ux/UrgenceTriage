#ifndef PATIENT_H
#define PATIENT_H

#include <time.h>

#define TAILLE_NOM 30
#define TAILLE_SYMPTOME 60

typedef enum {
    CRITIQUE = 1,
    TRES_URGENT,
    URGENT,
    MOINS_URGENT,
    NON_URGENT
} Gravite;

typedef struct Patient {
    int numero;
    char nom[TAILLE_NOM];
    char prenom[TAILLE_NOM];
    int age;
    char symptome[TAILLE_SYMPTOME];
    Gravite gravite;
    time_t heureArrivee;
    struct Patient *suivant;
} Patient;

Patient *patient_creer(int numero, const char *nom, const char *prenom,
                       int age, const char *symptome, Gravite gravite,
                       time_t heure_arrivee);
const char *gravite_nom(Gravite gravite);
int gravite_duree_moyenne(Gravite gravite);
int gravite_delai_cible(Gravite gravite);
void patient_detruire(Patient *patient);

#endif