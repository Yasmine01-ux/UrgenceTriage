#ifndef FICHIER_H
#define FICHIER_H

#include "file.h"

#include <stddef.h>
#include <time.h>

typedef struct {
    int numero;
    char nom[TAILLE_NOM];
    char prenom[TAILLE_NOM];
    int age;
    char symptome[TAILLE_SYMPTOME];
    Gravite gravite;
    time_t heureArrivee;
    time_t heureFin;
    int attenteMinutes;
} HistoriqueEntree;

typedef void (*HistoriqueVisiteur)(const HistoriqueEntree *entree, void *contexte);

int fichier_charger(FileAttente *file, int *prochain_numero, const char *chemin);
int fichier_sauvegarder(const FileAttente *file, int prochain_numero,
                        const char *chemin);
int fichier_ajouter_historique(const Patient *patient, time_t heure_fin,
                               int attente_minutes, const char *chemin);
size_t fichier_parcourir_historique(const char *chemin,
                                    HistoriqueVisiteur visiteur,
                                    void *contexte);

#endif