#ifndef FILE_H
#define FILE_H

#include "patient.h"

typedef struct {
    Patient *tete;
    int taille;
} FileAttente;

void file_initialiser(FileAttente *file);
int file_inserer(FileAttente *file, Patient *patient);
Patient *file_appeler(FileAttente *file);
Patient *file_rechercher_numero(const FileAttente *file, int numero);
Patient *file_rechercher_nom(const FileAttente *file, const char *nom);
int file_supprimer(FileAttente *file, int numero);
int file_modifier_gravite(FileAttente *file, int numero, Gravite gravite);
int file_estimer_attente(const FileAttente *file, const Patient *cible);
void file_detruire(FileAttente *file);

#endif