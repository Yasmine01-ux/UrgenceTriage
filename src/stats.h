#ifndef STATS_H
#define STATS_H

#include "file.h"

#include <stdio.h>

typedef struct {
    int patientsAccueillis;
    int patientsEnAttente;
    int parGravite[6];
    double attenteMoyenne;
    int attenteMinimum;
    int attenteMaximum;
    int nombreConsultations;
    double ageMoyen;
    char symptomeFrequent[TAILLE_SYMPTOME];
    int occurrencesSymptome;
} Statistiques;

void stats_calculer(const FileAttente *file, const char *chemin_historique,
                    Statistiques *statistiques);
void stats_afficher(const Statistiques *statistiques, FILE *sortie);

#endif