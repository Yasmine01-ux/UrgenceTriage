#include "file.h"

#include <ctype.h>
#include <string.h>

void file_initialiser(FileAttente *file)
{
    file->tete = NULL;
    file->taille = 0;
}

int file_inserer(FileAttente *file, Patient *patient)
{
    Patient **position;

    if (file == NULL || patient == NULL) {
        return 0;
    }

    position = &file->tete;
    while (*position != NULL &&
           ((*position)->gravite < patient->gravite ||
            ((*position)->gravite == patient->gravite &&
             (*position)->numero <= patient->numero))) {
        position = &(*position)->suivant;
    }
    patient->suivant = *position;
    *position = patient;
    file->taille++;
    return 1;
}

Patient *file_appeler(FileAttente *file)
{
    Patient *patient;

    if (file == NULL || file->tete == NULL) {
        return NULL;
    }
    patient = file->tete;
    file->tete = patient->suivant;
    patient->suivant = NULL;
    file->taille--;
    return patient;
}

Patient *file_rechercher_numero(const FileAttente *file, int numero)
{
    Patient *patient;

    if (file == NULL) {
        return NULL;
    }
    for (patient = file->tete; patient != NULL; patient = patient->suivant) {
        if (patient->numero == numero) {
            return patient;
        }
    }
    return NULL;
}

static int contient_sans_casse(const char *texte, const char *recherche)
{
    size_t i;
    size_t j;

    if (*recherche == '\0') {
        return 1;
    }
    for (i = 0; texte[i] != '\0'; i++) {
        for (j = 0; recherche[j] != '\0' && texte[i + j] != '\0'; j++) {
            if (tolower((unsigned char)texte[i + j]) !=
                tolower((unsigned char)recherche[j])) {
                break;
            }
        }
        if (recherche[j] == '\0') {
            return 1;
        }
    }
    return 0;
}

Patient *file_rechercher_nom(const FileAttente *file, const char *nom)
{
    Patient *patient;

    if (file == NULL || nom == NULL) {
        return NULL;
    }
    for (patient = file->tete; patient != NULL; patient = patient->suivant) {
        if (contient_sans_casse(patient->nom, nom) ||
            contient_sans_casse(patient->prenom, nom)) {
            return patient;
        }
    }
    return NULL;
}

int file_supprimer(FileAttente *file, int numero)
{
    Patient **position;
    Patient *patient;

    if (file == NULL) {
        return 0;
    }
    position = &file->tete;
    while (*position != NULL && (*position)->numero != numero) {
        position = &(*position)->suivant;
    }
    if (*position == NULL) {
        return 0;
    }
    patient = *position;
    *position = patient->suivant;
    patient_detruire(patient);
    file->taille--;
    return 1;
}

int file_modifier_gravite(FileAttente *file, int numero, Gravite gravite)
{
    Patient **position;
    Patient *patient;

    if (file == NULL || gravite < CRITIQUE || gravite > NON_URGENT) {
        return 0;
    }
    position = &file->tete;
    while (*position != NULL && (*position)->numero != numero) {
        position = &(*position)->suivant;
    }
    if (*position == NULL) {
        return 0;
    }
    patient = *position;
    *position = patient->suivant;
    file->taille--;
    patient->gravite = gravite;
    patient->suivant = NULL;
    return file_inserer(file, patient);
}

int file_estimer_attente(const FileAttente *file, const Patient *cible)
{
    const Patient *patient;
    int total = 0;

    if (file == NULL || cible == NULL) {
        return -1;
    }
    for (patient = file->tete; patient != NULL && patient != cible;
         patient = patient->suivant) {
        total += gravite_duree_moyenne(patient->gravite);
    }
    return patient == cible ? total : -1;
}

void file_detruire(FileAttente *file)
{
    Patient *patient;

    if (file == NULL) {
        return;
    }
    while ((patient = file_appeler(file)) != NULL) {
        patient_detruire(patient);
    }
}