#include "patient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Patient *patient_creer(int numero, const char *nom, const char *prenom,
                       int age, const char *symptome, Gravite gravite,
                       time_t heure_arrivee)
{
    Patient *patient = malloc(sizeof(*patient));

    if (patient == NULL) {
        return NULL;
    }

    patient->numero = numero;
    snprintf(patient->nom, sizeof(patient->nom), "%s", nom);
    snprintf(patient->prenom, sizeof(patient->prenom), "%s", prenom);
    patient->age = age;
    snprintf(patient->symptome, sizeof(patient->symptome), "%s", symptome);
    patient->gravite = gravite;
    patient->heureArrivee = heure_arrivee;
    patient->suivant = NULL;
    return patient;
}

const char *gravite_nom(Gravite gravite)
{
    static const char *const noms[] = {
        "", "CRITIQUE", "TRES_URGENT", "URGENT", "MOINS_URGENT", "NON_URGENT"
    };

    if (gravite < CRITIQUE || gravite > NON_URGENT) {
        return "INCONNUE";
    }
    return noms[gravite];
}

int gravite_duree_moyenne(Gravite gravite)
{
    static const int durees[] = {0, 30, 25, 20, 15, 10};

    if (gravite < CRITIQUE || gravite > NON_URGENT) {
        return 0;
    }
    return durees[gravite];
}

int gravite_delai_cible(Gravite gravite)
{
    static const int delais[] = {0, 0, 10, 30, 60, 120};

    if (gravite < CRITIQUE || gravite > NON_URGENT) {
        return 0;
    }
    return delais[gravite];
}

void patient_detruire(Patient *patient)
{
    free(patient);
}